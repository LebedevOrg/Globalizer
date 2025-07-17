/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      method.cpp                                                  //
//                                                                         //
//  Purpose:   Source file for method class                                //
//                                                                         //
//  Author(s): Sysoyev A., Barkalov K.                                     //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "method.h"
#include "local_method.h"
#include "ParallelHookeJeevesMethod.h"
#include "approximation.h"
#include "separable_method.h"
#include "exception.h"
#include "common.h"
#include "output_system.h"
#include "SearcDataIterator.h"
#include "TaskFactory.h"
#include "TrialFactory.h"
#include "CalculationFactory.h"
#include "TLBFGSBMethod.h"
#include "TParallelLBFGSBMethod.h"

#include <cmath>
#include <vector>
#include <fstream>
#include <algorithm>


//#define NUM_POINTS_FOR_APPROXIMATION 30

#ifdef USE_PLOTTER
#include <../lib/dislin/include/discpp.h>
typedef std::pair<double, double> point2d;
#endif

#ifdef USE_OpenCV
#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/ml/ml.hpp"

//using namespace cv;
//using namespace ml;
//
#endif

#include <string>
#include <cmath>



/// Массив для сохранения точек для последующей печати и рисования
std::vector<TTrial*> TMethod::printPoints;
/// количество точек вычисленных локальным методом
int TMethod::localPointCount = 0;
/// число запусков локально метода
int TMethod::numberLocalMethodtStart;
/// Количество вызовов рисовалки
int TMethod::printCount = 0;


// ------------------------------------------------------------------------------------------------
//TMethod::TMethod(int _MaxNumOfTrials, double _Eps, double _r, int _m, int _L, EMapType _MapType,
//    TTask *_pTask, TSearchData *_pData, TOptimEstimation *_pEstimation)
TMethod::TMethod(TTask& _pTask, TSearchData& _pData,
  TCalculation& _Calculation, TEvolvent& _Evolvent) :
  pTask(_pTask), pData(&_pData),
  Calculation(_Calculation), Evolvent(_Evolvent)
{
  isFoundOptimalPoint = false;

  MaxNumOfTrials = parameters.MaxNumOfPoints[_pTask.GetProcLevel()];
  if (MaxNumOfTrials < 1)
  {
    throw EXCEPTION("MaxNumOfTrials is out of range");
  }
  Epsilon = parameters.Eps[_pTask.GetProcLevel()];
  if (Epsilon <= 0.0)
  {
    throw EXCEPTION("Epsilon is out of range");
  }
  r = parameters.rs[_pTask.GetProcLevel()];
  if (r <= 1.0)
  {
    throw EXCEPTION("r is out of range");
  }

  m = parameters.m;

  if ((parameters.rEps < 0.0) || (parameters.rEps > 0.5))
  {
    throw EXCEPTION("Epsilon reserv parameter is out of range");
  }

  alfa = parameters.localAlpha; // пока локальная адаптация - фиксированная

  // число порождаемых точек на итерации совпадает с числом потомков в дереве процессов,
  //  если процесс - не лист
  if (pTask.GetProcLevel() < parameters.NumOfTaskLevels - 1)
  {
    NumPoints = parameters.ChildInProcLevel[pTask.GetProcLevel()];
    if (NumPoints <= 0)
    {
      throw EXCEPTION("ChildInProcLevel and NumPoints <= 0");
    }
  }
  else
  {
    NumPoints = parameters.NumPoints;
    if (NumPoints <= 0)
    {
      throw EXCEPTION("NumPoints parameter <= 0");
    }
  }

  iteration.IterationCount = 0;

  AchievedAccuracy = MaxDouble;
  // Массив для текущих итераций
  iteration.pCurTrials.resize(NumPoints);
  //// Лучшая точка пока не найдена
  //pData->GetBestTrial()->index = -2;
  //// МПИ лучшей точки пока не найдена
  //pData->GetBestTrial()->simVal = NULL;

  //===========================================================================================================================================
  mu = new double [pTask.GetNumOfFunc()];
  for (int i = 0; i < pTask.GetNumOfFunc(); i++)
    mu[i] = 0;
  Xmax = new double [pTask.GetNumOfFunc()];
  for (int i = 0; i < pTask.GetNumOfFunc(); i++)
    Xmax[i] = 0;
  //===========================================================================================================================================

  //for (int i = 0; i < parameters.Dimension; i++)
  //{
  //  pData->GetBestTrial()->y[i] = pTask.GetA()[i] - 1;
  //}

  // Выделяем память под массив лучших интервалов
  //iteration.BestIntervals.resize(NumPoints);
  //Смешанный алгоритм лучше включаеть при NumOfTrials > N*100
  //  StartLocalIteration = MaxNumOfTrials / 10;

  rootDim = pTask.GetFreeN();
  if (pTask.IsLeaf())
    rootDim = pTask.GetFreeN() - pTask.GetNumberOfDiscreteVariable();

  if (pTask.GetFreeN() == 1)
    StartLocalIteration = 5;
  else
    StartLocalIteration = rootDim * 70 / NumPoints;

  functionCalculationCount.resize(pTask.GetNumOfFunc());
  for (int i = 0; i < pTask.GetNumOfFunc(); i++)
    functionCalculationCount[i] = 0;

  isFindInterval = false;

  inputSet.trials.resize(NumPoints);
  inputSet.tasks.resize(NumPoints);

  globalM.resize(pTask.GetNumOfFunc());
  for (unsigned i = 0; i < globalM.size(); i++)
    globalM[i] = 1;

  isGlobalMUpdate = false;

  isSetInLocalMinimumInterval = false;

  isStop = false;


  intervalXMax = 0;
  isSearchXMax = true;

  numberOfRepetitions = 0;

  Calculation.SetSearchData(&_pData);

  isLocalZUpdate = false;

}

// ------------------------------------------------------------------------------------------------
TMethod::~TMethod()
{}

// ------------------------------------------------------------------------------------------------
void TMethod::SetDiscreteValue(int u, std::vector< std::vector<double> > dvs)
{
  //int numDV = 1;
  int z = u;
  int w = 0;
  for (int e = 0; e < pTask.GetNumberOfDiscreteVariable(); e++)
  {
    w = z % pTask.GetNumberOfValues(startDiscreteVariable + e);
    mDiscreteValues[u][e] = dvs[e][w];
    z = z / pTask.GetNumberOfValues(startDiscreteVariable + e);
  }
}

TSearchData* TMethod::GetSearchData(TTrial* trial)
{
  return pData;
}

// ------------------------------------------------------------------------------------------------
bool TMethod::IsIntervalInSegment(TSearchInterval* basicInterval, TSearchInterval* newInterval)
{
  double start = basicInterval->LeftPoint->GetFloor();
  if (basicInterval->LeftPoint->GetFloor() != newInterval->LeftPoint->GetFloor())
    return false;
  double end = start + 1;
  if ((newInterval->xl() >= start) && (newInterval->xl() <= end) &&
    (newInterval->xr() >= start) && (newInterval->xr() <= end))
    return true;
  return false;
}

// ------------------------------------------------------------------------------------------------
void TMethod::SetNumPoints(int newNP)
{
  if (newNP <= 0)
    newNP = 1;
  if (NumPoints != newNP)
  {
    NumPoints = newNP;

    if (iteration.pCurTrials.size() != NumPoints)
    {
      for (unsigned int i = 0; i < iteration.pCurTrials.size(); i++)
        iteration.pCurTrials[i] = 0;

      iteration.pCurTrials.resize(NumPoints);
    }

    //if (iteration.BestIntervals.size() != NumPoints)
    //  iteration.BestIntervals.resize(NumPoints);

    inputSet.Resize(NumPoints);
    outputSet.Resize(NumPoints);
  }
}


double TMethod::Update_r(int iter, int procLevel)
{
  int pl = 0;
  if (procLevel < 0)
    pl = pTask.GetProcLevel();
  else
    pl = procLevel;
  double baseR = parameters.rs[pl];
  double iterationCount = 0;
  if (iter <= 1)
    iterationCount = (double)(iteration.IterationCount);
  else
    iterationCount = iter;

  if (iterationCount <= 0)
    iterationCount = 1;

  double p = 1.0 / rootDim;
  double resR = baseR + parameters.rDynamic / pow(iterationCount, p);

  return resR;
}

// ------------------------------------------------------------------------------------------------
void TMethod::CalculateImage(TTrial& pCurTrialsj)
{
  Evolvent.GetImage(pCurTrialsj.X(), pCurTrialsj.y);
}

// ------------------------------------------------------------------------------------------------
/// Вычисляет координаты на отрезке 0..1 для всех разверток по образу проведенного испытания
void TMethod::CalculateCurrentPoint(TTrial& pCurTrialsj, TSearchInterval* BestIntervalsj)
{
  // Вычисляем x
  pCurTrialsj.discreteValuesIndex = BestIntervalsj->discreteValuesIndex();
  if (BestIntervalsj->izl() != BestIntervalsj->izr())
  {
    pCurTrialsj.SetX(0.5 * (BestIntervalsj->xl() + BestIntervalsj->xr()));
    //      pCurTrialsj.x = BestIntervalsj->xl() + 0.5*BestIntervalsj->dx;
  }
  else
  {
    pCurTrialsj.SetX(0.5 * (BestIntervalsj->xl() + BestIntervalsj->xr()) -
      (((BestIntervalsj->zr() - BestIntervalsj->zl()) > 0) ? 1 : -1)*
      pow(fabs(BestIntervalsj->zr() - BestIntervalsj->zl()) /
        pData->M[BestIntervalsj->izl()], rootDim) / 2 / r);
    //      pCurTrialsj.x = BestIntervalsj->xl() + (0.5*BestIntervalsj->dx -
    //(((BestIntervalsj->zr() - BestIntervalsj->zl)>0)?1:-1)*pow(fabs(BestIntervalsj->zr() -
    //BestIntervalsj->zl)/pData->M[BestIntervalsj->izl],pTask.GetFreeN())/(2*r));
  }

  pCurTrialsj.leftInterval = BestIntervalsj;
  pCurTrialsj.rightInterval = BestIntervalsj;

  //Точка новой итерации должна быть в интервале, иначе - ошибка!!!
  if (pCurTrialsj.X() <= BestIntervalsj->xl() || pCurTrialsj.X() >= BestIntervalsj->xr())
  {
    //throw EXCEPTION("Point is outside the interval !");
    pCurTrialsj.SetX(0.5 * (BestIntervalsj->xl() + BestIntervalsj->xr()));
  }

  // Вычисляем y
  // Вычисляем образ точки итерации - образ записывается в начальные позиции массива y
  CalculateImage(pCurTrialsj);

  for (int k = pTask.GetFreeN() - 1; k >= 0; k--)
    pCurTrialsj.y[pTask.GetFixedN() + k] = pCurTrialsj.y[k];

  // Записываем фиксированные координаты - они всегда расположены вначае
  for (int j = 0; j < pTask.GetFixedN(); j++)
  {
    pCurTrialsj.y[j] = pTask.GetFixedY()[j];
  }

  // Записываем значение дискретной переменной
  for (int j = 0; j < pTask.GetNumberOfDiscreteVariable(); j++)
    pCurTrialsj.y[startDiscreteVariable + j] =
    mDiscreteValues[pCurTrialsj.discreteValuesIndex][j];
}

// ------------------------------------------------------------------------------------------------
void TMethod::FirstIteration()
{
  printCount = 0;
  // Задаем границы интервалов изменения параметров
  // Указатель на границы интервалов в подзадаче - это указатель на исходные границы,
  //   смещенный на число фиксированных размерностей
  Evolvent.SetBounds(pTask.GetA() + pTask.GetFixedN(), pTask.GetB() + pTask.GetFixedN());

  // Это первая итерация, сбрасываем счетчик
  iteration.IterationCount = 1;
  // И сбрасываем достигнутую точность
  AchievedAccuracy = 1.0;
  // И лучшую итерацию
  //pData->GetBestTrial()->index = -2;
  // Формируем интервал [0,1]

    // Вычисляем чилсло значений дискретных параметров
  mDiscreteValuesCount = 1;
  int numberOfDiscreteVariable = pTask.GetNumberOfDiscreteVariable();


  std::vector< std::vector<double> > dvs(numberOfDiscreteVariable);
  startDiscreteVariable = pTask.GetN() - numberOfDiscreteVariable;
  for (int e = 0; e < numberOfDiscreteVariable; e++)
  {
    dvs[e].resize(pTask.GetNumberOfValues(startDiscreteVariable + e));
    pTask.GetAllDiscreteValues(startDiscreteVariable + e, dvs[e].data());
    mDiscreteValuesCount = mDiscreteValuesCount *
      pTask.GetNumberOfValues(startDiscreteVariable + e);
  }


    mDiscreteValues.resize(mDiscreteValuesCount);

    for (int u = 0; u < mDiscreteValuesCount; u++)
    {
      mDiscreteValues[u].resize(numberOfDiscreteVariable);
      SetDiscreteValue(u, dvs);
    }


  //for (int e = 0; e < numberOfDiscreteVariable(); e++)
  //{
  //  delete[] dvs[e];
  //}
  //delete[] dvs;

  iteration.pCurTrials.resize(NumPoints * mDiscreteValuesCount);

  TSearchInterval** NewInterval = new TSearchInterval*[mDiscreteValuesCount];
    //TSearchIntervalFactory::CreateSearchInterval();
  for (int e = 0; e < mDiscreteValuesCount; e++)
  {
    NewInterval[e] = TSearchIntervalFactory::CreateSearchInterval();
    NewInterval[e]->ind = iteration.IterationCount;
    NewInterval[e]->K = 0;
    NewInterval[e]->CreatePoint();
    NewInterval[e]->LeftPoint->discreteValuesIndex = e;
    NewInterval[e]->RightPoint->discreteValuesIndex = e;
    // Гельдеровская длина
    NewInterval[e]->delta = 1.0;
    //Добавляем интервал
    TSearchInterval* p = pData->InsertInterval(*(NewInterval[e]));
    delete NewInterval[e];
    NewInterval[e] = p;
    pData->GetTrials().push_back(p->LeftPoint);
    pData->GetTrials().push_back(p->RightPoint);
    ///Необходимо сосчитать значения на границах
    CalculateImage(*p->LeftPoint);

    for (int j = 0; j < numberOfDiscreteVariable; j++)
      p->LeftPoint->y[startDiscreteVariable + j] =
      mDiscreteValues[p->LeftPoint->discreteValuesIndex][j];

    CalculateImage(*p->RightPoint);

    for (int j = 0; j < numberOfDiscreteVariable; j++)
      p->RightPoint->y[startDiscreteVariable + j] =
      mDiscreteValues[p->RightPoint->discreteValuesIndex][j];

    if (pData->GetBestTrial() == 0)
      pData->SetBestTrial(p->LeftPoint);

    //====================================================================
    if ((parameters.isCalculationInBorderPoint == true) || (parameters.LocalTuningType != 0))
    {
      //if (pTask.GetFreeN() == 1)
      {
        // Эта функция вызывается только в листе дерева - поэтому вычисляем функционалы здесь
        for (int j = 0; j < pTask.GetNumOfFunc(); j++)
        {
          p->LeftPoint->FuncValues[j] = MaxDouble;
          p->RightPoint->FuncValues[j] = MaxDouble;
        }
        p->LeftPoint->K = 1;
        //p->RightPoint->K = 1;

        TInformationForCalculation inputlocal;
        TResultForCalculation outputlocal;
        inputlocal.Resize(2);
        outputlocal.Resize(2);

        inputlocal.trials[0] = p->LeftPoint;
        if (inputlocal.tasks[0] == 0)
          inputlocal.tasks[0] = TTaskFactory::CreateTask();

        inputlocal.trials[1] = p->RightPoint;
        if (inputlocal.tasks[1] == 0)
          inputlocal.tasks[1] = TTaskFactory::CreateTask();

        TCalculation* Calculation_ = TCalculationFactory::CreateCalculation2(pTask, &Evolvent);

        Calculation_->Calculate(inputlocal, outputlocal);

        for (int j = 0; j < pTask.GetNumOfFunc(); j++)
        {
          functionCalculationCount[j] = outputlocal.countCalcTrials[j];
        }
        UpdateOptimumEstimation(*(p->RightPoint));
        UpdateOptimumEstimation(*(p->LeftPoint));

      }
    }
      
  }
  intervalXMax = NewInterval[0];
  //====================================================================

  // На первой итерации - единственный лучший интервал
  //for (i = 0; i < NumPoints; i++)
  //  iteration.BestIntervals[i] = p;

  // Флаг пересчета - поднят
  pData->SetRecalc(true);

  // Точки первой итерации выбираются по особому правилу
  // Равномерно ставим NumPoints точек c шагом h
  // А надо бы случайно...
  double h = 1.0 / (NumPoints + 1);
  if (!parameters.isLoadFirstPointFromFile) // равномерно распределяем начальные точки
  {
    for (int e = 0; e < mDiscreteValuesCount; e++)
    {
      for (int q = 0; q < NumPoints; q++)
      {

        if (parameters.TypeDistributionStartingPoints == Evenly)
        {
          int ind = e * NumPoints + q;
          iteration.pCurTrials[ind] = TTrialFactory::CreateTrial();
          iteration.pCurTrials[ind]->discreteValuesIndex = e;
          pData->GetTrials().push_back(iteration.pCurTrials[ind]);
          iteration.pCurTrials[ind]->SetX((q + 1) * h);

          // Вычисляем образ точки итерации - образ записывается в начальные позиции массива y
          CalculateImage(*iteration.pCurTrials[ind]);
          // Смещаем вычисленные координаты в соответствии с уровнем подзадачи
          // Смещение надо делать начиная с координаты с бОльшим номером
          for (int j = pTask.GetFreeN() - 1; j >= 0; j--)
          {
            iteration.pCurTrials[ind]->y[pTask.GetFixedN() + j] = iteration.pCurTrials[ind]->y[j];
          }

          // Записываем фиксированные координаты - они всегда расположены вначае
          for (int j = 0; j < pTask.GetFixedN(); j++)
            iteration.pCurTrials[ind]->y[j] = pTask.GetFixedY()[j];

          for (int j = 0; j < numberOfDiscreteVariable; j++)
            iteration.pCurTrials[ind]->y[startDiscreteVariable + j] =
            mDiscreteValues[iteration.pCurTrials[ind]->discreteValuesIndex][j];

          iteration.pCurTrials[ind]->leftInterval = NewInterval[e];
          iteration.pCurTrials[ind]->rightInterval = NewInterval[e];
        }
        else
        {
          int ind = e * NumPoints + q;
          iteration.pCurTrials[ind] = TTrialFactory::CreateTrial();
          iteration.pCurTrials[ind]->discreteValuesIndex = e;
          pData->GetTrials().push_back(iteration.pCurTrials[ind]);
          
          //iteration.pCurTrials[ind]->SetX((q + 1)* h);
          //CalculateImage(*iteration.pCurTrials[ind]);

          for (size_t iCNP = 0; iCNP < pTask.GetFreeN(); iCNP++)
          {
            iteration.pCurTrials[ind]->y[iCNP] = pTask.GetA()[iCNP] + ((double(q) + 1.0) * h) * (pTask.GetB()[iCNP] - pTask.GetA()[iCNP]);
          }

          Extended genX(0.0);
          Evolvent.GetInverseImage(iteration.pCurTrials[ind]->y, genX);
          iteration.pCurTrials[ind]->SetX(genX);

          // Смещаем вычисленные координаты в соответствии с уровнем подзадачи
          // Смещение надо делать начиная с координаты с бОльшим номером
          for (int j = pTask.GetFreeN() - 1; j >= 0; j--)
          {
            iteration.pCurTrials[ind]->y[pTask.GetFixedN() + j] = iteration.pCurTrials[ind]->y[j];
          }

          // Записываем фиксированные координаты - они всегда расположены вначае
          for (int j = 0; j < pTask.GetFixedN(); j++)
            iteration.pCurTrials[ind]->y[j] = pTask.GetFixedY()[j];

          for (int j = 0; j < numberOfDiscreteVariable; j++)
            iteration.pCurTrials[ind]->y[startDiscreteVariable + j] =
            mDiscreteValues[iteration.pCurTrials[ind]->discreteValuesIndex][j];

          iteration.pCurTrials[ind]->leftInterval = NewInterval[e];
          iteration.pCurTrials[ind]->rightInterval = NewInterval[e];
        }
      }
    }

    //pData->SetBestTrial(iteration.pCurTrials[0]);
    //NumPoints = NumPoints * mDiscreteValuesCount;
    SetNumPoints(NumPoints* mDiscreteValuesCount);
  }
  else // читаем из файла FirstPointFilePath
  {
    std::string pointsPath = parameters.FirstPointFilePath;

    std::vector<std::vector<double>> points;
    std::vector<double> pointVal;
    std::vector<int> typeColor;

    std::ifstream input;
    std::string currentLine(512, ' ');
    size_t numberOfPoints = 0;

    input.open(pointsPath, std::ios_base::in);

    if (input.is_open())
    {
      input.getline(&currentLine[0], currentLine.size());
      numberOfPoints = std::stoi(currentLine, NULL);
      points.reserve(numberOfPoints + 2);
      typeColor.reserve(numberOfPoints + 2);


      while (!input.eof()) {
        size_t nextPosition = 0;
        std::vector<double> currentPoint(parameters.Dimension);
        double curVal;
        int s = currentLine.size();
        input.getline(&currentLine[0], currentLine.size());
        int t = currentLine.find('|');
        int l = currentLine.length();

        const char* cstr = currentLine.c_str();

        if (cstr[0] == '\0')
          continue;
        if (currentLine == "\n")
          continue;
        if (t == -1 || currentLine == "" || l == 0)
          continue;

        std::string curStr = currentLine;

        currentPoint[0] = std::stod(curStr, &nextPosition);

        for (int iDim = 1; iDim < parameters.Dimension; iDim++)
        {
          curStr = curStr.substr(nextPosition);
          currentPoint[iDim] = std::stod(curStr, &nextPosition);
        }

        std::string a = currentLine.substr(t + 1);
        curVal = std::stod(a);

        t = a.find('|');
        if (t == -1)
        {
          typeColor.push_back(0);
          continue;
        }
        else
        {
          std::string b = a.substr(t + 1);
          typeColor.push_back(std::stod(b));
        }

        points.push_back(currentPoint);
        pointVal.push_back(curVal);

      }
      input.close();

      int numberLoadedPoints = points.size();
      std::vector<TTrial*> newPoint(numberLoadedPoints);

      for (int i = 0; i < numberLoadedPoints; i++)
      {
        newPoint[i] = TTrialFactory::CreateTrial();
        for (int iDim = 0; iDim < parameters.Dimension; iDim++)
        {
          newPoint[i]->y[iDim] = points[i][iDim];
        }

        newPoint[i]->FuncValues[0] = pointVal[i];

        newPoint[i]->K = 1;
        newPoint[i]->index = 0;

        Extended genX(0.0);
        Evolvent.GetInverseImage(newPoint[i]->y, genX);

        newPoint[i]->SetX(genX);

        pData->GetTrials().push_back(newPoint[i]);
      }

      this->InsertPoints(newPoint);

      this->iteration.IterationCount += numberLoadedPoints;
      parameters.iterationNumber = iteration.IterationCount;
    }


  }

}

// ------------------------------------------------------------------------------------------------
void TMethod::Recalc()
{
  if (pData->IsRecalc())
  {
    // Обновить текущие значение минимумов
    for (int v = 0; v <= pData->GetBestTrial()->index; v++)
    {
      if (v < pData->GetBestTrial()->index)
      {
        pData->Z[v] = -pData->M[v] * parameters.rEps;
      }
      else
      {
        if (pData->GetBestTrial()->FuncValues[v] != MaxDouble)
          pData->Z[v] = pData->GetBestTrial()->FuncValues[v];
        else
          pData->Z[v] = 0;
      }
    }

    pData->ClearQueue();
    for (TSearcDataIterator it = pData->GetBeginIterator(); it; ++it)
    {
      it->R = CalculateGlobalR(*it);
      it->locR = CalculateLocalR(*it);

      pData->PushToQueue(*it);
    }
    // После пересчета флаг опускаем
    pData->SetRecalc(false);
  }

  pData->pRecalcDatas.clear();
}

// ------------------------------------------------------------------------------------------------
void TMethod::CalculateIterationPoints()
{

  //r = Update_r(iteration.IterationCount);

  if (iteration.IterationCount == 1)
  {
    return;
  }
  else if (iteration.IterationCount == 2)
  {
    // число порождаемых точек на итерации совпадает с числом потомков в дереве процессов,
//  если процесс - не лист
    if (pTask.GetProcLevel() < parameters.NumOfTaskLevels - 1)
    {
      this->SetNumPoints(parameters.ChildInProcLevel[pTask.GetProcLevel()]);

      if (NumPoints <= 0)
      {
        throw EXCEPTION("ChildInProcLevel and NumPoints <= 0");
      }
    }
    else
    {
      this->SetNumPoints(parameters.NumPoints);
      if (NumPoints <= 0)
      {
        throw EXCEPTION("NumPoints parameter <= 0");
      }
    }
    
    //NumPoints = parameters.NumPoints;
  }

  //PlotDecisionTrees();

  // Если поднят флаг - то пересчитать все характеристики
  Recalc();

  // Здесь надо взять NumPoints лучших характеристик из очереди
  // Очередь пока одна - очередь глобальных характеристик
  // В ней должно быть нужное количество интервалов, т.к. на первом шаге проводится NumPoints
  // испытаний
  std::vector<TSearchInterval*> BestIntervals(NumPoints);

  int localMix = parameters.localMix;
  //if (pTask.IsLeaf())
  //  localMix = 0;

  if (GetIterationType(iteration.IterationCount, localMix) == Global)
  {

    bool f = parameters.PointTakingType;
    if (f == 0)
      pData->GetBestIntervals(BestIntervals.data(), NumPoints);
    else
    {
#ifdef USE_OpenCV
      int numberOfRepetitionsInIteration = 0;
      int count = pData->GetCount();
      int countMinInterval = 0;
      for (int i = 0; i < NumPoints; i++)
      {
        int j = 0;
        TSearchInterval* interval = 0;
        while (j < (count + 1))
        {
          interval = pData->GetIntervalWithMaxR();

          if (interval->delta <= parameters.Epsilon)
            countMinInterval++;

          bool fl = true;

          // Проверяем границы интервала, не попали ли они в окрестность (0.1) точек из массива (массив с локальными минимумами)
          for (int s = 0; s < localMins.size(); s++) {
            int localAreaCounter = 0;
            for (int r = 0; r < parameters.Dimension; r++) {
              if (fabs(localMins[s]->y[r] - interval->LeftPoint->y[r]) < 0.1) {
                localAreaCounter++;
              }
            }
            if (localAreaCounter == parameters.Dimension) {
              fl = false;
              break;
            }
            localAreaCounter = 0;
            for (int r = 0; r < parameters.Dimension; r++) {
              if (fabs(localMins[s]->y[r] - interval->RightPoint->y[r]) < 0.1) {
                localAreaCounter++;
              }
            }
            if (localAreaCounter == parameters.Dimension) {
              fl = false;
              break;
            }
          }

          if (interval->status != TSearchInterval::local_area) {
            fl = fl && true;
          }
          else {
            fl = false;
          }

          if (true == fl)
            break;

          j++;

          numberOfRepetitions++;
          numberOfRepetitionsInIteration++;
        }
        BestIntervals[i] = interval;
        if (BestIntervals[i]->status == TSearchInterval::educational_local_method)
        {
          isSetInLocalMinimumInterval = true;
        }
      }
      if (numberOfRepetitionsInIteration > 0)
        pData->SetRecalc(true);
      if (numberOfRepetitionsInIteration >= (count / 2.0))
        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\nprocess 0, iteration %d\tpovt = %d\tcountPoint %d\n",
          GetIterationCount(), numberOfRepetitionsInIteration, count);
      if (!(GetIterationCount() % parameters.StepPrintMessages))
      {
        printf("process 0, iteration %d\t povt = %d\n", GetIterationCount(), numberOfRepetitions);
        numberOfRepetitions = 0;
      }
#endif
    }
  }
  else
    pData->GetBestLocalIntervals(BestIntervals.data(), NumPoints);
  // Пока заполняем одновременно вектор CurTrials, и вектор интервалов

  CalculateCurrentPoints(BestIntervals);
}

// ------------------------------------------------------------------------------------------------
void TMethod::CalculateCurrentPoints(std::vector<TSearchInterval*>& BestIntervals)
{
  for (unsigned int i = 0; i < BestIntervals.size(); i++)
  {
    iteration.pCurTrials[i] = TTrialFactory::CreateTrial();
    pData->GetTrials().push_back(iteration.pCurTrials[i]);
    CalculateCurrentPoint(*iteration.pCurTrials[i], BestIntervals[i]);
  }
}

// ------------------------------------------------------------------------------------------------
bool TMethod::CheckStopCondition()
{
  bool res = false;
  int numOfOptima = 1;
  double allOptimumPoints[MAX_TRIAL_DIMENSION * MAX_NUM_MIN];

  double CurrentAccuracy = 0.1;
  if (CurrentAccuracy < AchievedAccuracy)
    AchievedAccuracy = CurrentAccuracy;

  if (pTask.GetFixedN() != 0) //если метод не в корне, то остановка только по точности
  {
    //if (AchievedAccuracy < Epsilon)
    //  res = true;
    double fm = Epsilon;
    if ((isSetInLocalMinimumInterval == true) || (AchievedAccuracy < fm))
      res = true;
  }
  else
  {
    switch (parameters.stopCondition)
    {
    case Accuracy:
      if (AchievedAccuracy < Epsilon)
        res = true;
      break;
    case OptimumVicinity:
    {
      res = true;
      //numOfOptima = ;
      if (pTask.getProblem()->GetAllOptimumPoint(allOptimumPoints, numOfOptima) ==
        IProblem::UNDEFINED)
      {
        for (int i = 0; i < parameters.Dimension; i++)
        {
          double fabsx = fabs(pData->GetBestTrial()->y[i] - pTask.GetOptimumPoint()[i]);
          double fm = parameters.Epsilon * (pTask.GetB()[i] - pTask.GetA()[i]);
          if (fabsx > fm)
          {
            res = false;
            break;
          }
        }
      }
      else
      {
        for (int j = 0; j < numOfOptima; j++)
        {
          for (int i = 0; i < parameters.Dimension; i++)
          {
            double fabsx = fabs(pData->GetBestTrial()->y[i] - allOptimumPoints[parameters.Dimension * j + i]);
            double fm = parameters.Epsilon * (pTask.GetB()[i] - pTask.GetA()[i]);
            if (fabsx > fm)
            {
              res = false;
              break;
            }
            if (i == parameters.Dimension - 1)
            {
              res = true;
            }
          }
          if (res == true)
          {
            break;
          }
        }
      }
    }
    break;
    case OptimumVicinity2:
    {
      res = true;
      for (int i = 0; i < pTask.GetN(); i++)
      {
        if (fabs(pData->GetBestTrial()->y[i] - pTask.GetOptimumPoint()[i]) > Epsilon)
        {
          res = false;
          break;
        }
      }
    }
    break;
    case OptimumValue:
      if (pData->GetBestTrial()->index == pTask.GetNumOfFunc() - 1 &&
        pData->GetBestTrial()->FuncValues[pData->GetBestTrial()->index] - pTask.GetOptimumValue() < Epsilon)
        res = true;
      break;

    case AccuracyWithCheck:
      //res = true;
      //for (int i = 0; i < pTask.GetN(); i++)
      //{
      //  if (fabs(pData->GetBestTrial()->y[i] - pTask.GetOptimumPoint()[i]) > Epsilon)
      //  {
      //    res = false;
      //  }
      //}

      //if((res) && ((int)(parameters.itrEps) == 0))
      //{
      //  parameters.itrEps.SetValue(&iteration.IterationCount);
      //
      //}
      res = false;
      if (AchievedAccuracy < Epsilon)
      {
        FILE* pf;
        if (parameters.GetProcRank() == 0)
        {
          pf = fopen("optim_pareto.txt", "a");
          int found = 0;
          if (pTask.getProblem()->isOptimal(pData->GetBestTrial()->y, pTask.getMin(), pTask.getMax()))
          {
            found = 1;
          }
          fprintf(pf, "%d ", found);
          fprintf(pf, "%d ", (int)(parameters.itrEps));

          fprintf(pf, "%1.4lf ", pData->M[0]);

          //fprintf(pf, "\n");
          fclose(pf);

          int itr = 0;
          parameters.itrEps.SetValue(&itr);

          //int cnt_const= pTask>GetNumberOfConstraints();
          int cnt_const = 5;

          printf("########################## point #########################\n");
          printf("best alg\n");
          for (int i = 0; i < pTask.GetN(); i++)
          {
            printf("x[%d] = %1.5lf \n", i, pData->GetBestTrial()->y[i]);
          }
          for (int i = 0; i < cnt_const; i++)
          {
            printf(" %1.5lf \n", pTask.CalculateFuncs(pData->GetBestTrial()->y, i));
          }
          printf(" %1.5lf \n", pTask.CalculateFuncs(pData->GetBestTrial()->y, 100));
          printf("best aprox\n");
          for (int i = 0; i < pTask.GetN(); i++)
          {
            printf("x[%d] = %1.5lf \n", i, pTask.GetOptimumPoint()[i]);
          }

          const double* point = pTask.GetOptimumPoint();
          double value = pTask.CalculateFuncs(point, 100);
          for (int i = 0; i < cnt_const; i++)
          {
            printf(" %1.5lf \n", pTask.CalculateFuncs(point, i));
          }
          printf(" %1.5lf \n", value);
          printf("########################## point #########################\n");
        }
        res = true;
      }
      break;


    case InLocalArea:
    {
      double fm = Epsilon; //* (pTask.GetB()[pTask.GetProcLevel()] - pTask.GetA()[pTask.GetProcLevel()]);
      if ((isSetInLocalMinimumInterval == true) || (AchievedAccuracy < fm))
        res = true;
    }
    break;
    }
  }

  if (iteration.IterationCount >= MaxNumOfTrials)
    res = true;

  isStop = res;
  return res;
}


/// Проверяет попала ли точка в окрестность глобального манимума
bool TMethod::CheckLocalityOptimalPoint(TTrial* trial)
{
  if (pTask.GetIsOptimumPointDefined())
  {
    if (!isFoundOptimalPoint)
    {
      double e = parameters.Epsilon;
      bool res = true;
      double ffmax = 0;

      for (int j = 0; j < parameters.Dimension; j++)
      {
        double fm = e * (pTask.GetB()[j] - pTask.GetA()[j]);
        double fabsx = fabs(trial->y[j] - pTask.GetOptimumPoint()[j]);
        fm = e * (pTask.GetB()[j] - pTask.GetA()[j]);
        if (fabsx > fm)
        {
          res = false;
          break;
        }

        if (fabsx > ffmax)
          ffmax = fabsx;
      }
      if (res)
      {
        isFoundOptimalPoint = true;
        print_l1 << "\n\nFound optimal point!\nIter = "
          << iteration.IterationCount << "\nDifPoint = " << ffmax << "\n\n";
      }
    }
  }
  return isFoundOptimalPoint;
}


// ------------------------------------------------------------------------------------------------
void TMethod::CalculateFunctionals()
{
  /// Входные данные для вычислителя, формирубтся в CalculateFunctionals()
  /*TInformationForCalculation inputSet;
  /// Выходные данные вычислителя, обрабатывается в CalculateFunctionals()
  TResultForCalculation outputSet;

  inputSet.Resize(iteration.pCurTrials.size());
  outputSet.Resize(iteration.pCurTrials.size());*/

  std::vector<TSearchInterval*> intervalArr(iteration.pCurTrials.size());



  // Эта функция вызывается только в листе дерева - поэтому вычисляем функционалы здесь
  for (unsigned int i = 0; i < iteration.pCurTrials.size(); i++)
  {
    if (iteration.pCurTrials[i] == 0)
      continue;
    // Записываем значения MaxDouble
    for (int j = 0; j < pTask.GetNumOfFunc(); j++)
      iteration.pCurTrials[i]->FuncValues[j] = MaxDouble;

    // Так как вычисление в листе дерева, то вложенных итераций нет
    iteration.pCurTrials[i]->K = 1;
    inputSet.trials[i] = iteration.pCurTrials[i];
    if (inputSet.tasks[i] == 0)
      inputSet.tasks[i] = TTaskFactory::CreateTask();

    intervalArr[i] = iteration.pCurTrials[i]->leftInterval;
  }


  Calculation.Calculate(inputSet, outputSet);

  for (unsigned int i = 0; i < iteration.pCurTrials.size(); i++)
  {
    if (outputSet.trials[i] == 0)
      iteration.pCurTrials[i] = 0;
    else if (parameters.TypeCalculation == 8)
    {
      iteration.pCurTrials[i] = outputSet.trials[i];
    }

    if (parameters.TypeCalculation == ApproximationScheme)
    {
      std::cout << "OutputSet.trials[i] = " << outputSet.trials[i] << std::endl;
      if ((outputSet.trials.size() < i) && (outputSet.trials[i] != 0))
      {
        if (outputSet.trials[i]->isNeedRecalculateCoordinates)
        {
          Extended genX(0.0);
          Evolvent.GetInverseImage(outputSet.trials[i]->y, genX);
          outputSet.trials[i]->SetX(genX);

          pData->PushToQueue(intervalArr[i]);
        }
      }
      std::cout << "My message 1" << std::endl;
    }
  }

  for (int j = 0; j < pTask.GetNumOfFunc(); j++)
  {
    functionCalculationCount[j] = outputSet.countCalcTrials[j];
  }

  //pData->SetRecalc(true);
}


// ------------------------------------------------------------------------------------------------
std::vector<int> TMethod::GetFunctionCalculationCount()
{
  return functionCalculationCount;
}

// ------------------------------------------------------------------------------------------------
void TMethod::InsertLocalPoints(const std::vector<TTrial*>& points, TTask* task)
{
  for (size_t j = 0; j < points.size(); j++)
  {
    TTrial* currentPoint = points[j];
	Extended  x;
    Evolvent.GetInverseImage(&(currentPoint->y[pTask.GetFixedN()]), x);
	currentPoint->SetX(x);
    TSearchInterval* CoveringInterval = pData->FindCoveringInterval(currentPoint);

    if (!CoveringInterval)
      throw EXCEPTION("Covering interval does not exists");
    if (!(currentPoint->X() < CoveringInterval->xr() || currentPoint->X() > CoveringInterval->xl()))
      throw EXCEPTION("Wrong covering interval");

    if (points[j]->K <= 0)
      points[j]->K = 1;

    TSearchInterval* p = pData->InsertPoint(CoveringInterval, *currentPoint,
      iteration.IterationCount, pTask.GetFreeN());

    UpdateOptimumEstimation(*currentPoint);

    if (parameters.TypeAddLocalPoint == 0)
    {
      if (AchievedAccuracy > CoveringInterval->delta)
        AchievedAccuracy = CoveringInterval->delta;
      if (p)
        if (AchievedAccuracy > p->delta)
          AchievedAccuracy = p->delta;
    }

    if (!p)
    {
      //print << "Not create intrval!\n";
      continue;
    }
    else
    {
      pData->DeleteIntervalFromQueue(CoveringInterval);
    }

    // Вычисляем оценку константы
    CalculateM(p);
    CalculateM(CoveringInterval);

    // Если полный пересчет не нужен - обновляем только очереди характеристик
    if (!pData->IsRecalc())
    {
      // Удалять интервалы из очереди не надо - они уже удалены в GetBestIntervals
      // Вставляем два новых интервала
      p->R = CalculateGlobalR(p);
      p->locR = CalculateLocalR(p);
      pData->PushToQueue(p);

      CoveringInterval->R = CalculateGlobalR(CoveringInterval);
      CoveringInterval->locR = CalculateLocalR(CoveringInterval);

      pData->PushToQueue(CoveringInterval);
    }


  }
}


// ------------------------------------------------------------------------------------------------
void TMethod::InsertPoints(const std::vector<TTrial*>& points)
{
  for (size_t j = 0; j < points.size(); j++)
  {
    TTrial* currentPoint = points[j];
    Extended  x;
    Evolvent.GetInverseImage(currentPoint->y, x);
    currentPoint->SetX(x);
    TSearchInterval* CoveringInterval = pData->FindCoveringInterval(currentPoint);

    if (AchievedAccuracy > CoveringInterval->delta)
      AchievedAccuracy = CoveringInterval->delta;

    if (!CoveringInterval)
      throw EXCEPTION("Covering interval does not exists");
    if (!(currentPoint->X() < CoveringInterval->xr() || currentPoint->X() > CoveringInterval->xl()))
      throw EXCEPTION("Wrong covering interval");

    TSearchInterval* p = pData->InsertPoint(CoveringInterval, *currentPoint,
      iteration.IterationCount, pTask.GetFreeN());

    UpdateOptimumEstimation(*currentPoint);

    if (p)
    {
      CalculateM(p);
      CalculateM(CoveringInterval);
    }
  }
}

// ------------------------------------------------------------------------------------------------
bool TMethod::UpdateOptimumEstimation(TTrial& trial)
{
  if (trial.index > pData->GetBestTrial()->index || trial.index == pData->GetBestTrial()->index &&
    trial.FuncValues[pData->GetBestTrial()->index] < pData->GetBestTrial()->FuncValues[pData->GetBestTrial()->index])
  {
    //pData->GetBestTrial()->index = trial.index;
    //pData->GetBestTrial()->x = trial.x;
    //memcpy(pData->GetBestTrial()->FuncValues, trial.FuncValues, pTask.GetNumOfFunc() *
    //  sizeof(double));

    //memcpy(pData->GetBestTrial()->y, trial.y, pTask.GetN() * sizeof(double));
    //// Оптимум обновился - нужен пересчет
    //pData->SetRecalc(true);
    //pData->GetBestTrial()->simVal = trial.simVal;
    pData->SetBestTrial(trial.Clone());

    // Оптимум обновился - нужен пересчет
    pData->SetRecalc(true);
    isLocalZUpdate = true;
    return true;
  }
  return false;
}

void TMethod::SavePoints()
{
  if (static_cast<std::string>(parameters.iterPointsSavePath).size() > 0)
  {
    if (parameters.iterPointsSavePath.ToString() != "")
    {
      TSearcDataIterator it = pData->GetBeginIterator();

      for (++it; it; ++it)
      {
        printPoints.push_back((*it)->LeftPoint->Clone());
      }
    }
  }
  //if (parameters.IsPlot)
  //  PlotPoint();


}

void TMethod::PlotPoint()
{
#ifdef USE_PLOTTER
  std::vector<point2d> points;
  std::vector<int> typeColor;

  //std::ifstream input;
  std::string currentLine(512, ' ');
  size_t numberOfPoints = 0;

  //input.open(pointsPath, std::ios_base::in);

  //if (input.is_open())
  {
    //input.getline(&currentLine[0], currentLine.size());
    numberOfPoints = pData->GetCount();
    points.reserve(numberOfPoints + 2);
    typeColor.reserve(numberOfPoints + 2);

    for (TSearcDataIterator it = pData->GetBeginIterator(); it; ++it)
    {
      if (it->LeftPoint->index >= 0)
      {
        point2d currentPoint;

        currentPoint.first = it->LeftPoint->y[pTask.GetFixedN()];//std::stod(currentLine, &nextPosition);

        currentPoint.second = it->LeftPoint->FuncValues[it->LeftPoint->index];//std::stod(a);
        points.push_back(currentPoint);
        typeColor.push_back(it->LeftPoint->TypeColor);
      }
      else
        numberOfPoints--;
    }

  }

  size_t totalPoints = numberOfPoints;

  int n = 300, i, j, width = 900, height = 900;
  double  x_step, y_step, x_left, y_left, x_right, y_right;
  double arg[2];
  std::vector <double> xArray(n), yArray(n);

  //double lb[2], ub[2];
  const double* lb = pTask.GetA();//GetBounds(lb, ub);
  const double* ub = pTask.GetB();
  x_left = lb[0];
  y_left = lb[0];
  x_right = ub[0];
  y_right = ub[0];

  x_step = (x_right - x_left) / (n - 1);
  y_step = (y_right - y_left) / (n - 1);

  for (i = 0; i < n; i++)
  {
    xArray[i] = x_left + i * x_step;
  }
  
  printCount++;
  Dislin g;
  g.metafl("png");
  g.winsiz(width, height);
  g.pagmod("LAND");
  g.page(2400, 2400);
  std::string pName = "loc_" + std::to_string(pTask.num) + "_" + std::to_string(this->printCount) + "_" + parameters.GetPlotFileName();
  g.setfil(pName.c_str());
  g.sclfac(2.0);
  g.filmod("VERSION");
  g.scrmod("revers");
  g.disini();
  g.complx();
  g.name("X-axis", "x");
  g.name("Values", "y");

  g.labdig(-1, "x");
  g.ticks(9, "x");
  g.ticks(10, "y");

  g.axspos(240, 2200);
  g.axslen(2100, 2100);

  double minx = x_left, miny = 0, maxx = x_right, maxy = 0;

  for (int iz = 0; iz < pTask.GetFixedN(); iz++)
    arg[iz] = pTask.GetFixedY()[iz];

  for (i = 0; i < n; i++)
  {
    arg[pTask.GetFixedN()] = xArray[i]; //arg[1] = yray[j];
    yArray[i] = pTask.CalculateFuncs(arg, 0);

    if (maxy < yArray[i])
      maxy = yArray[i];
    if (miny > yArray[i])
      miny = yArray[i];
  }

  if (maxx == minx)
    maxx += 1;
  if (maxy == miny)
    maxy += 1;

  double xstep = (maxx - minx) / 4.0;
  double ystep = (maxy - miny) / 4.0;
  g.graf(minx, maxx, minx, xstep,
    miny - ((maxy - miny) * 0.1), maxy + ((maxy - miny) * 0.1), miny, ystep);

  g.height(30);

  int colors[] = { 150 };
  g.shdmod("LOWER", "CELL");
  g.shdmod("UPPER", "COLOR");
  g.conclr(colors, 1);

  g.color("fore");
  g.height(50);
  g.title();


  g.setrgb(0.7, 0.7, 0.7);
  g.grid(1, 1);

  g.color("red");

  int xn = yArray.size();
  g.curve(xArray.data(), yArray.data(), xn);

  if (points.size())
  {

    g.hsymbl(15);

    for (size_t k = 0; k < totalPoints; k++)
    {
      if (typeColor[k] == 0)
        g.color("white");
      else if (typeColor[k] == 1)
        g.color("green");
      else if (typeColor[k] == 2)
        g.color("blue");
      else
        g.color("white");

      g.rlsymb(21, points[k].first, points[k].second);
    }

    //g.color("yellow");
    //g.hsymbl(25);
    //g.rlsymb(21, points[totalPoints].first, points[totalPoints].second);
    //g.hsymbl(20);
    //if (totalPoints == points.size() - 2)
    //{
    //  g.color("red");
    //  g.rlsymb(21, points[totalPoints + 1].first, points[totalPoints + 1].second);
    //}
  }

  g.height(50);
  g.color("fore");
  g.title();
  g.disfin();

  //PrintLevelPoints(pName + "_.txt");
#endif
}


void TMethod::PlotDecisionTrees()
{
#ifdef USE_PLOTTER
#ifdef USE_OpenCV
  if (parameters.IsPlot)
  {
    std::vector<point2d> points;
    std::vector<int> typeColor;

    //std::ifstream input;
    std::string currentLine(512, ' ');
    size_t numberOfPoints = 0;

    //input.open(pointsPath, std::ios_base::in);

    //if (input.is_open())
    {
      //input.getline(&currentLine[0], currentLine.size());
      numberOfPoints = pData->GetCount();
      points.reserve(numberOfPoints + 2);
      typeColor.reserve(numberOfPoints + 2);

      for (TSearcDataIterator it = pData->GetBeginIterator(); it; ++it)
      {
        if (it->LeftPoint->index >= 0)
        {
          point2d currentPoint;

          currentPoint.first = it->LeftPoint->X().toDouble();//std::stod(currentLine, &nextPosition);

          currentPoint.second = it->LeftPoint->FuncValues[it->LeftPoint->index];//std::stod(a);
          points.push_back(currentPoint);
          typeColor.push_back(it->LeftPoint->TypeColor);
        }
        else
          numberOfPoints--;
      }

    }

    size_t totalPoints = numberOfPoints;

    double heightWidthMult = 2.5;
    int n = 1500, j;
    int height = 1500; //Высота от которой считается размер полотна
    int  width = int(double(height) * heightWidthMult);
    double  x_step, y_step, x_left, y_left, x_right, y_right;
    double arg[2];
    std::vector <double> xArray(n), xArray2(n), yArray(n), yArray2(n);

    TSearchData* data = pData;
    bool isStartLocalMethod = true;

    int N = data->GetCount() - 1;

    if (parameters.isCalculationInBorderPoint == 1)
      N += 1;

    int flag = 0;

    cv::Mat X(N, 1, CV_32FC1);
    cv::Mat ff(N, 1, CV_32FC1);


    int indexX = -1;
    std::vector< TTrial*> allPoints(N);
    int i = 0;
    for (TSearcDataIterator it = pData->GetBeginIterator(); it; ++it)
    {

      if (it->LeftPoint->index >= 0)
      {
        X.at<float>(i, 0) = it->LeftPoint->X().toDouble();
        ff.at<float>(i, 0) = it->LeftPoint->GetValue();
        allPoints[i] = it->LeftPoint;
        i++;
      }
    }


    cv::Ptr< cv::ml::DTrees > Mytree = cv::ml::DTrees::create();

    Mytree->setMinSampleCount(1);
    Mytree->setCVFolds(1);
    Mytree->setMaxDepth(parameters.DecisionTreesMaxDepth);
    Mytree->setRegressionAccuracy(parameters.DecisionTreesRegressionAccuracy);

    Mytree->train(X, cv::ml::ROW_SAMPLE, ff); // тренирует дерево на TrainData


      //double lb[2], ub[2];
    const double* lbN = pTask.GetA();//GetBounds(lb, ub);
    const double* ubN = pTask.GetB();

    //x_left = lb[0];
    //y_left = lb[0];
    //x_right = ub[0];
    //y_right = ub[0];

    double lb = lbN[0];
    double ub = ubN[0];

    double luDiff = ub - lb;

    x_left = lb;
    y_left = lb;
    x_right = ub;
    y_right = ub;

    //x_step = (x_right - x_left) / (n - 1);
    x_step = 1.0 / (n - 1);
    y_step = (y_right - y_left) / (n - 1);

    for (i = 0; i < n; i++)
    {
      xArray[i] = i * x_step;
    }

    int NN = n;
    cv::Mat X_preduct(NN, 1, CV_32FC1);
    cv::Mat results;
    float hh = 1.0 / NN;
    for (int i = 0; i < NN; i++)
    {
      X_preduct.at<float>(i, 0) = i * hh;
    }

    Mytree->predict(X_preduct, results);

    cv::Mat results2;
    Mytree->predict(X, results2);
    std::vector<double> r2(N);
    double oldVal = 1000;
    double err = 0;
    for (indexX = 0; indexX < results2.rows; indexX++)
    {

      r2[indexX] = results2.at<float>(indexX, 0);

      if (r2[indexX] != oldVal)
      {
        if (err > parameters.DecisionTreesRegressionAccuracy)
          print << "Iter = " << iteration.IterationCount << "\terr = " << err << "\n";
        err = 0;
        oldVal = r2[indexX];
      }

      err += fabs(allPoints[indexX]->GetValue() - r2[indexX]) * fabs(allPoints[indexX]->GetValue() - r2[indexX]);

    }

    if (err > parameters.DecisionTreesRegressionAccuracy)
      print << "Iter = " << iteration.IterationCount << "\terr = " << err << "\n";

    for (indexX = 0; indexX < results.rows; indexX++)
    {
      yArray2[indexX] = results.at<float>(indexX, 0);
    }

    //'BLACK', 'RED', 'GREEN', 'BLUE', 'CYAN', 'YELLOW', 'ORANGE', 'MAGENTA', 'WHITE', 'FORE', 'BACK', 'GRAY' and 'HALF'. 

    Dislin g;
    g.metafl("png");
    g.winsiz(width, height);
    g.pagmod("LAND");

    int pageHeight = height * 1.2;
    int pageWidth = int(double(pageHeight) * heightWidthMult);

    g.page(pageWidth, pageHeight);
    g.setfil(("loc_" + parameters.GetPlotFileName()).c_str());
    g.sclfac(2.0);
    g.filmod("VERSION");
    g.scrmod("revers");
    g.disini();

    g.height(70);//Размер шрифта
    g.helve();


    //g.name("X-axis", "x");
    //g.name("Values", "y");

    //g.labdig(-1, "x");
    g.ticks(10, "x");
    g.ticks(10, "y");

    double pageAxHeightWidthMult = 0.875;

    int nxa = int(double(pageWidth) * 0.1);
    int nya = int(double(pageHeight) * 0.90);

    g.axspos(nxa, nya);
    //g.axslen(2100, 2100);

    g.axslen(pageWidth * pageAxHeightWidthMult, pageHeight * pageAxHeightWidthMult);
    
    double minx = x_left, miny = 0, maxx = x_right, maxy = 0;

    for (int iz = 0; iz < pTask.GetFixedN(); iz++)
      arg[iz] = pTask.GetFixedY()[iz];

    std::vector <double> xArrayOr(n);

    for (i = 0; i < n; i++)
    {
      arg[pTask.GetFixedN()] = xArray[i]; //arg[1] = yray[j];
      Evolvent.GetImage(xArray[i], arg);
      yArray[i] = pTask.CalculateFuncs(arg, 0);

      xArrayOr[i] = arg[0];

      if (maxy < yArray[i])
        maxy = yArray[i];
      if (miny > yArray[i])
        miny = yArray[i];
    }

    if (maxx == minx)
      maxx += 1;
    if (maxy == miny)
      maxy += 1;

    double xstep = (maxx - minx) / 4.0;
    double ystep = (maxy - miny) / 4.0;
    //g.color("RED");
    //
    g.graf(minx, maxx, minx, xstep,
      miny - ((maxy - miny) * 0.1), maxy + ((maxy - miny) * 0.1), miny, ystep);
    //g.color("WHITE");
    g.height(30);

    int colors[] = { 150 };
    g.shdmod("LOWER", "CELL");
    g.shdmod("UPPER", "COLOR");
    g.conclr(colors, 1);

    g.color("fore");
    g.height(50);
    g.title();

  

    g.setrgb(0.7, 0.7, 0.7);
    g.grid(1, 1);

    g.color("red");

    g.thkcrv(21);//толщина линии
    
    int xn = yArray.size();
    g.curve(xArrayOr.data(), yArray.data(), xn);

    g.thkcrv(11);//толщина линии
    g.color("blue");
    g.curve(xArrayOr.data(), yArray2.data(), xn);

    if (points.size())
    {
      int pointSize = 41;
      g.hsymbl(pointSize);

      for (size_t k = 0; k < totalPoints; k++)
      {
        if (typeColor[k] == 0)
          g.color("white");
        else if (typeColor[k] == 1)
          g.color("green");
        else if (typeColor[k] == 2)
          //g.color("blue");
          g.color("green");
        else
          g.color("white");

        arg[pTask.GetFixedN()] = points[k].first; //arg[1] = yray[j];
        Evolvent.GetImage((points[k].first), arg);
        
        g.rlsymb(21, arg[0], points[k].second);
      }

      //g.color("yellow");
      //g.hsymbl(25);
      //g.rlsymb(21, points[totalPoints].first, points[totalPoints].second);
      //g.hsymbl(20);
      //if (totalPoints == points.size() - 2)
      //{
      //  g.color("red");
      //  g.rlsymb(21, points[totalPoints + 1].first, points[totalPoints + 1].second);
      //}
    }

    g.height(50);
    g.color("fore");
    g.title();
    g.disfin();
  }
#endif
#endif
}



void TMethod::PlotPoint2()
{
#ifdef USE_PLOTTER
  parameters.IsPlot = false;
  std::vector<point2d> points;
  std::vector<int> typeColor;

  //std::ifstream input;
  std::string currentLine(512, ' ');
  size_t numberOfPoints = 0;

  //input.open(pointsPath, std::ios_base::in);

  //if (input.is_open())
  {
    //input.getline(&currentLine[0], currentLine.size());
    numberOfPoints = pData->GetCount();
    points.reserve(numberOfPoints + 2);
    typeColor.reserve(numberOfPoints + 2);

    for (TSearcDataIterator it = pData->GetBeginIterator(); it; ++it)
    {
      if (it->LeftPoint->index >= 0)
      {
        point2d currentPoint;

        currentPoint.first = it->LeftPoint->y[0];//std::stod(currentLine, &nextPosition);

        currentPoint.second = it->LeftPoint->FuncValues[it->LeftPoint->index];//std::stod(a);
        points.push_back(currentPoint);
        typeColor.push_back(it->LeftPoint->TypeColor);
      }
      else
        numberOfPoints--;
    }

  }

  size_t totalPoints = numberOfPoints;

  int n = 50, i, j, width = 900, height = 900;
  double  x_step, y_step, x_left, y_left, x_right, y_right;
  double arg[2];
  std::vector <double> xArray(n), yArray(n);

  //double lb[2], ub[2];
  const double* lb = pTask.GetA();//GetBounds(lb, ub);
  const double* ub = pTask.GetB();
  x_left = lb[0];
  y_left = lb[0];
  x_right = ub[0];
  y_right = ub[0];

  x_step = (x_right - x_left) / (n - 1);
  y_step = (y_right - y_left) / (n - 1);

  for (i = 0; i < n; i++)
  {
    xArray[i] = x_left + i * x_step;
  }

  Dislin g;
  g.metafl("png");
  g.winsiz(width, height);
  g.pagmod("LAND");
  g.page(2400, 2400);
  g.setfil(("glob_" + parameters.GetPlotFileName()).c_str());
  //g.setfil((parameters.GetPlotFileName()).c_str());
  g.sclfac(2.0);
  g.filmod("VERSION");
  g.scrmod("revers");
  g.disini();
  g.complx();
  g.name("X-axis", "x");
  g.name("Values", "y");

  g.labdig(-1, "x");
  g.ticks(9, "x");
  g.ticks(10, "y");

  g.axspos(240, 2200);
  g.axslen(2100, 2100);

  double minx = x_left, miny = 0, maxx = x_right, maxy = 0;

  TInformationForCalculation inputlocal;
  TResultForCalculation outputlocal;
  inputlocal.Resize(1);
  outputlocal.Resize(1);


  Calculation.Reset();

  parameters.MaxNumOfPoints[pTask.GetProcLevel() + 1] = GLOBALIZER_MAX(parameters.MaxNumOfPoints[pTask.GetProcLevel() + 1], 1000);
  parameters.Eps[pTask.GetProcLevel() + 1] = GLOBALIZER_MIN(parameters.Eps[pTask.GetProcLevel() + 1], 0.0001);
  parameters.rs[pTask.GetProcLevel() + 1] = GLOBALIZER_MAX(parameters.rs[pTask.GetProcLevel() + 1], 10);

  TTrial* point = TTrialFactory::CreateTrial();
  inputlocal.trials[0] = point;//p->LeftPoint;
  if (inputlocal.tasks[0] == 0)
    inputlocal.tasks[0] = TTaskFactory::CreateTask();

  for (int iz = 0; iz < pTask.GetN(); iz++)
    point->y[iz] = (pTask.GetA()[iz] + pTask.GetB()[iz]) / 2.0;

  //Calculation.Calculate(inputlocal, outputlocal);



  for (i = 0; i < n; i++)
  {
    //arg[0] = xArray[i]; //arg[1] = yray[j];
    point->y[0] = xArray[i];
    inputlocal.tasks[0] = TTaskFactory::CreateTask();
    Calculation.Calculate(inputlocal, outputlocal);
    yArray[i] = point->FuncValues[point->index];
    //yArray[i] = pTask.CalculateFuncs(arg, 0);

    if (maxy < yArray[i])
      maxy = yArray[i];
    if (miny > yArray[i])
      miny = yArray[i];
  }

  if (maxx == minx)
    maxx += 1;
  if (maxy == miny)
    maxy += 1;

  double xstep = (maxx - minx) / 4.0;
  double ystep = (maxy - miny) / 4.0;
  g.graf(minx, maxx, minx, xstep,
    miny - ((maxy - miny) * 0.1), maxy + ((maxy - miny) * 0.1), miny, ystep);

  g.height(30);

  int colors[] = { 150 };
  g.shdmod("LOWER", "CELL");
  g.shdmod("UPPER", "COLOR");
  g.conclr(colors, 1);

  g.color("fore");
  g.height(50);
  g.title();


  g.setrgb(0.7, 0.7, 0.7);
  g.grid(1, 1);

  g.color("red");

  int xn = yArray.size();
  g.curve(xArray.data(), yArray.data(), xn);

  if (points.size())
  {

    g.hsymbl(15);

    for (size_t k = 0; k < totalPoints; k++)
    {
      if (typeColor[k] == 0)
        g.color("white");
      else if (typeColor[k] == 1)
        g.color("green");
      else if (typeColor[k] == 2)
        g.color("blue");
      else
        g.color("white");

      g.rlsymb(21, points[k].first, points[k].second);
    }

    //g.color("yellow");
    //g.hsymbl(25);
    //g.rlsymb(21, points[totalPoints].first, points[totalPoints].second);
    //g.hsymbl(20);
    //if (totalPoints == points.size() - 2)
    //{
    //  g.color("red");
    //  g.rlsymb(21, points[totalPoints + 1].first, points[totalPoints + 1].second);
    //}
  }

  g.height(50);
  g.color("fore");
  g.title();
  g.disfin();
  parameters.IsPlot = true;
#endif
}

// ------------------------------------------------------------------------------------------------
double TMethod::CalculateGlobalR(TSearchInterval* p)
{
  double deltax = p->delta;
  double value = 0;
  int v = 0;
  if ((p->izl() == -2) && (p->izr() == -2))
  {
    if (parameters.GetProcRank() == 3)
    {
      printf("Bad Interval\tProcRank=%d\n", parameters.GetProcRank());
      value = 0;

      printf("iteration.IterationCount  = %d\n", iteration.IterationCount);
      printf("NewInterval! xl() = %lf xr() = %lf izl() = %d izr() = %d zl() = %lf zr() = %lf R = %lf\n",
        p->xl().toDouble(), p->xr().toDouble(), p->izl(), p->izr(), p->zl(), p->zr(), p->R
      );
      printf("Z[%d] = %lf M = %lf alfa = %lf\n", v, pData->Z[v], pData->Z[v], alfa);
      printf("val = %lf\n", value);

      //PrintCurPoint();

      //PrintInfo();
    }
  }
  else if ((p->izl() == -3) || (p->izr() == -3))
  {
    return MinDouble;
  }
  else if (p->izl() == p->izr())
  {
    v = p->izl();
    value =
      deltax + (p->zr() - p->zl()) * (p->zr() - p->zl()) / (deltax * pData->M[v] * pData->M[v] * r * r) -
      2 * (p->zr() + p->zl() - 2 * pData->Z[v]) / (r * pData->M[v]);
  }
  else if (p->izr() > p->izl())
  {
    v = p->izr();
    value = 2 * deltax - 4 * (p->zr() - pData->Z[v]) / (r * pData->M[v]);
  }
  else //if (p->izr() < p->izl)
  {
    v = p->izl();
    value = 2 * deltax - 4 * (p->zl() - pData->Z[v]) / (r * pData->M[v]);
  }

  //Характеристика интервала должна быть конечной, иначе - ошибка
#ifdef WIN32
  if (!_finite(value))
#else
  if (!std::isfinite(value))
#endif
  {
    throw EXCEPTION("Infinite R!");
  }

  return value;
}

// ------------------------------------------------------------------------------------------------
double TMethod::CalculateLocalR(TSearchInterval* p)
{
  double value;
  int v;
  if ((p->izl() == -3) || (p->izr() == -3))
  {
    return MinDouble;
  }
  if (p->izl() == p->izr())
  {
    v = p->izl();
    value =
      //p->R / (sqrt((p->zr() - pData->Z[v]) * (p->zl() - pData->Z[v])) / pData->M[v] + pow(1.5, -alfa));
      p->R / (sqrt((p->zr() - pData->Z[v]) * (p->zl() - pData->Z[v])) / pData->M[v] + pow(10, -alfa));
  }
  else if (p->izl() > p->izr())
  {
    v = p->izl();
    //value = p->R / ((p->zl() - pData->Z[v]) / pData->M[v] + pow(1.5, -alfa));
    value = p->R / ((p->zl() - pData->Z[v]) / pData->M[v] + pow(10, -alfa));
  }
  else //if (p->izl() < p->izr)
  {
    v = p->izr();
    //value = p->R / ((p->zr() - pData->Z[v]) / pData->M[v] + pow(1.5, -alfa));
    value = p->R / ((p->zr() - pData->Z[v]) / pData->M[v] + pow(10, -alfa));
  }

  //Характеристика интервала должна быть конечной, иначе - ошибка
#ifdef WIN32
  if (!_finite(value))
#else
  if (!std::isfinite(value))
#endif
  {
    throw EXCEPTION("Infinite R!");
  }

  return value;
}

// ------------------------------------------------------------------------------------------------
void TMethod::CalculateM(TSearchInterval* p)
{
  // Дополнительные точки с отрицательным индексом не обрабатываем
  if (p->izl() < 0)
    return;

  int boundaryStatus = IsBoundary(p);

  // Самый простой случай - индексы совпадают, рассматриваем только текущий интервал
  if (p->izl() == p->izr())
  {
    //double zl = pTask.CalculateFuncs(p->LeftPoint->y, 0);
    //double zr = pTask.CalculateFuncs(p->RightPoint->y, 0);

    //double ozl = p->zl();
    //double ozr = p->zr();

    //if (zl != ozl || zr != ozr)
    //  std::cout << "Error!!!\n";

    //double newValue = fabs(p->zr() - p->zl()) / p->delta;
    //int index = 0;
    //if (newValue > pData->M[index] || pData->M[index] == 1.0 && newValue > _M_ZERO_LEVEL)
    //{
    //  pData->M[index] = newValue;
    //  pData->SetRecalc(true);
    //}

    UpdateM(fabs(p->zr() - p->zl()) / p->delta, p->izl(), boundaryStatus, p);
  }
  else //if(p->izl() != p->izr)
  {
    if (parameters.LocalTuningType == 0)
    {
      // Просмотр вправо до обнаружения точки с большим или равным индексом,
      // или до правой границы интервала поиска
      TSearcDataIterator i = pData->GetIterator(p);
      ++i;
      while (i != NULL && p->izl() > i->izl() && IsIntervalInSegment(p, (*i)))
        ++i;
      //Если обнаружили точку с большим или равным индексом, то вычисляем оценку константы
      if (i != NULL && p->izl() <= i->izl() && IsIntervalInSegment(p, (*i)))
        UpdateM(fabs(i->z()[p->izl()] - p->zl()) / root(i->xl() - p->xl(), rootDim), p->izl(), boundaryStatus, p);
      //UpdateM(fabs(i->z()[p->izl()] - p->zl()) / root(i->xl() - p->xl(), pTask.GetFreeN()), p->izl(), boundaryStatus, p);
    // Просмотр влево до обнаружения точки с большим или равным индексом,
    // или до левой границы интервала поиска
      i = pData->GetIterator(p);
      --i;
      while (i != NULL && p->izl() > i->izl() && IsIntervalInSegment(p, (*i)))
        --i;
      //Если обнаружили точку с большим или равным индексом, то вычисляем оценку константы
      if (i != NULL && p->izl() <= i->izl() && IsIntervalInSegment(p, (*i)))
        UpdateM(fabs(i->z()[p->izl()] - p->zl()) / root(p->xl() - i->xl(), rootDim), p->izl(), boundaryStatus, p);
      //UpdateM(fabs(i->z()[p->izl()] - p->zl()) / root(p->xl() - i->xl(), pTask.GetFreeN()), p->izl(), boundaryStatus, p);
    }
    else
    {
      UpdateM(0, p->izl(), boundaryStatus, p);
    }
  }

  //Проверка нулевой (либо же очень маленькой) константы Липшица
  if (pData->M[p->izl()] <= _M_ZERO_LEVEL)
  {
    throw EXCEPTION("Lipcshitz constant equals to 0!");
  }
}

// ------------------------------------------------------------------------------------------------
/// Добавление основных (из основной\единственной развертки) точек испытания в базу, возвращиет правый
TSearchInterval* TMethod::AddCurrentPoint(TTrial& pCurTrialsj, TSearchInterval* BestIntervalsj)
{
  // правый подинтервал
  TSearchInterval* NewInterval = TSearchIntervalFactory::CreateSearchInterval();
  if (isFindInterval || BestIntervalsj == 0)//если нужно искать интервал
  {
    //то ищем
    BestIntervalsj = pData->FindCoveringInterval(&pCurTrialsj);
    pCurTrialsj.leftInterval = BestIntervalsj;
    pCurTrialsj.rightInterval = BestIntervalsj;
  }
  else if ((pCurTrialsj.X() > BestIntervalsj->xr()) || (pCurTrialsj.X() < (BestIntervalsj)->xl()))
    //иначе проверяем что точка вне интервала интервала
  {
    //ищем подходящий интервал
    (BestIntervalsj) = pData->FindCoveringInterval(&pCurTrialsj);
    pCurTrialsj.leftInterval = BestIntervalsj;
    pCurTrialsj.rightInterval = BestIntervalsj;
  }

  if (!((BestIntervalsj)->xl() < pCurTrialsj.X() &&
    pCurTrialsj.X() < (BestIntervalsj)->xr()))
  {
    //CoveringPointCount0++;
    if ((BestIntervalsj)->delta < AchievedAccuracy)
    {
      AchievedAccuracy = (BestIntervalsj)->delta;
    }
    return 0;
  }

  // Заполнение интервала
  NewInterval->ind = iteration.IterationCount;
  // Запоминаем число вложенных итераций
  NewInterval->K = pCurTrialsj.K;
  // Левая точка интервала - это точка очередного испытания
  NewInterval->LeftPoint = &pCurTrialsj;


  //Внимание! Конфликт с деструктором класса SearchInterval!!

  //for (int i = 0; i < pCurTrialsj.index + 1; i++)
  //{
  //  NewInterval->z[i] = pCurTrialsj.FuncValues[i];
  //}

  // Правая точка интервала - это правая точка интервала, в котором проведено испытание
  NewInterval->RightPoint = BestIntervalsj->RightPoint;

  //=================================================================================================
  if (parameters.StatusIntervalChangeType == 1)
    NewInterval->status = (BestIntervalsj)->status;
  //=================================================================================================

  //Правая точка должна быть больше левой, если - меньше, ошибка!!!
  if (NewInterval->xr() <= NewInterval->xl())
  {
    throw EXCEPTION("Interval with negative length!");
  }

  // Гельдеровская длина интервала
  NewInterval->delta = root(NewInterval->xr() - NewInterval->xl(), rootDim);
  //NewInterval->delta = root(NewInterval->xr() - NewInterval->xl(), pTask.GetFreeN());
  //    NewInterval->delta = pow(NewInterval->dx,1.0/pTask.GetFreeN());

  // Корректируем существующий интервал
  (BestIntervalsj)->RightPoint = NewInterval->LeftPoint;

  //    (*BestIntervalsj)->dx -= NewInterval->dx;

  // Обновляем достигнутую точность - по старой гельдеровской длине лучшего интервала
  if ((BestIntervalsj)->delta < AchievedAccuracy)
  {
    AchievedAccuracy = (BestIntervalsj)->delta;
  }
  // После чего вычисляем новую гельдеровскую длину лучшего интервала
  (BestIntervalsj)->delta = root((BestIntervalsj)->xr() - (BestIntervalsj)->xl(), rootDim);
  //(BestIntervalsj)->delta = root((BestIntervalsj)->xr() - (BestIntervalsj)->xl(), pTask.GetFreeN());
  //    (*BestIntervalsj)->delta = pow((*BestIntervalsj)->dx,1.0/pTask.GetFreeN());

  int j = BestIntervalsj->izr();
  if (BestIntervalsj->izl() > j)
    j = BestIntervalsj->izl();

  if (Xmax[j] < (BestIntervalsj)->delta)
  {
    Xmax[j] = (BestIntervalsj)->delta;
    intervalXMax = BestIntervalsj;
  }

  j = NewInterval->izr();
  if (NewInterval->izl() > j)
    j = NewInterval->izl();

  if (Xmax[j] < (NewInterval)->delta)
  {
    Xmax[j] = (NewInterval)->delta;
    intervalXMax = NewInterval;
  }

  // Интервал сформирован - можно добавлять
  // Вставка завершается корректно, или же выбрасывает исключение
  TSearchInterval* p = pData->InsertInterval(*NewInterval);

  delete NewInterval;

  pCurTrialsj.leftInterval = (BestIntervalsj);
  pCurTrialsj.rightInterval = p;

  pCurTrialsj.leftInterval->LeftPoint->rightInterval = (BestIntervalsj);
  pCurTrialsj.rightInterval->RightPoint->leftInterval = p;

  return p;

}

bool TMethod::UpdateStatusNPoint(TTrial* trial, TTrial*& inflection,
  std::vector<TTrial*>& leftLocalMinPoint, std::vector<TTrial*>& RightLocalMinPoint,
  int& countPointLeft, int& countPointRight)
{

  bool isStartLocalMethod = false;

  TSearchInterval* li = trial->leftInterval;
  TSearchInterval* ri = trial->rightInterval;


  if (parameters.StatusIntervalChangeType == 1)
    if ((li->status != 0) && (ri->status != 0))
      return isStartLocalMethod;

  TTrial* lt = li->LeftPoint;
  TTrial* rt = ri->RightPoint;


  TTrial* parent_inflection = 0;

  if ((lt->pointStatus == TTrial::local_min) || (rt->pointStatus == TTrial::local_min))
    return isStartLocalMethod;

  ETypeStartLocalMethod oldTypeStartLocalMethod = parameters.TypeStartLocalMethod;

  if ((lt->index == -2) || (rt->index == -2))
  {
    inflection = trial;
    parameters.TypeStartLocalMethod = AnyPoints;
    if ((lt->index == -2) && (rt->index == -2))
      trial->pointStatus = TTrial::low_inflection;
    else if ((lt->index == -2) && (rt->index >= 0))
    {
      if (trial->FuncValues[trial->index] > rt->FuncValues[rt->index])
        trial->pointStatus = TTrial::low;
      else
        trial->pointStatus = TTrial::up;
    }
    else if ((lt->index >= 0) && (rt->index == -2))
    {
      if (lt->FuncValues[lt->index] > trial->FuncValues[trial->index])
        trial->pointStatus = TTrial::low;
      else
        trial->pointStatus = TTrial::up;
    }
  }
  else
    //Левая точка выше, правая ниже -> убывающая
    if ((lt->FuncValues[lt->index] > trial->FuncValues[trial->index]) && (trial->FuncValues[trial->index] > rt->FuncValues[rt->index]))
    {
      trial->pointStatus = TTrial::low;
      //Если справа была убывающая точка -> спускаемся до перегиба
      if (rt->pointStatus == TTrial::low)
      {
        inflection = rt;
        parent_inflection = inflection;
        while (inflection->pointStatus == TTrial::low)
        {
          TSearchInterval* i = inflection->rightInterval;
          if (i == 0)//дошли до границы
          {
            inflection = 0;
            break;
          }
          parent_inflection = inflection;
          inflection = i->RightPoint;
        }
        if (inflection != 0)
        {
          if ((inflection->pointStatus != TTrial::low_inflection) && (inflection->pointStatus != TTrial::local_min) &&
            ((inflection->rightInterval != 0) && (inflection->leftInterval != 0)))
          {
            parent_inflection->pointStatus = TTrial::low_inflection;
            inflection = parent_inflection;
          }
        }
      }
      else if (rt->pointStatus == TTrial::low_inflection)
        inflection = ri->RightPoint;

    }
    else
      //Левая точка ниже, правая выше -> возрастающая
      if ((lt->FuncValues[lt->index] < trial->FuncValues[trial->index]) && (rt->FuncValues[rt->index] > trial->FuncValues[trial->index]))
      {
        trial->pointStatus = TTrial::up;
        //Если справа была возрастающая точка -> поднимаемся до верхнего перегиба
        if (lt->pointStatus == TTrial::up)
        {
          inflection = lt;
          parent_inflection = inflection;
          while (inflection->pointStatus == TTrial::up)
          {
            TSearchInterval* i = inflection->leftInterval;
            if (i == 0)
              break;
            parent_inflection = inflection;
            inflection = i->LeftPoint;
          }

          if (inflection->pointStatus != TTrial::up_inflection)
          {
            parent_inflection->pointStatus = TTrial::up_inflection;
            inflection = 0;
          }
          else
            inflection = 0;
        }
        else if (lt->pointStatus == TTrial::low_inflection)
          inflection = lt;

      }
      else
        //Левая точка выше, правая выше -> нижний перегиб
        if ((lt->FuncValues[lt->index] > trial->FuncValues[trial->index]) && (rt->FuncValues[rt->index] > trial->FuncValues[trial->index]))
        {
          trial->pointStatus = TTrial::low_inflection;

          //Проверяем, чтобы не было двух нижних перегибов подряд
          if (lt->pointStatus == TTrial::low_inflection)
            lt->pointStatus = TTrial::low;

          if (rt->pointStatus == TTrial::low_inflection)
            rt->pointStatus = TTrial::up;

          inflection = trial;
        }
        else
          //Левая точка ниже, правая ниже -> верхний перегиб
          if ((lt->FuncValues[lt->index] < trial->FuncValues[trial->index]) && (rt->FuncValues[rt->index] < trial->FuncValues[trial->index]))
          {
            trial->pointStatus = TTrial::up_inflection;

            //Проверяем, чтобы не было двух верхних перегибов подряд
            if ((rt->pointStatus == TTrial::up_inflection) && (rt->rightInterval != 0))
              rt->pointStatus = TTrial::low;

            if ((lt->pointStatus == TTrial::up_inflection) && (lt->leftInterval != 0))
              lt->pointStatus = TTrial::up;
          }

  if (inflection != 0)
  {
    li = inflection->leftInterval;
    ri = inflection->rightInterval;

    if (li == 0 || ri == 0)
      return isStartLocalMethod;

    lt = li->LeftPoint;
    rt = ri->RightPoint;

    TTrial* p1 = 0;
    TTrial* p2 = 0;



    inflection->lowAndUpPoints = 1;

    p1 = lt;
    countPointLeft = 0;

    while ((p1->pointStatus == TTrial::low) || (p1->pointStatus == TTrial::up_inflection))
    {
      inflection->lowAndUpPoints++;

      leftLocalMinPoint.push_back(p1);
      countPointLeft++;

      TSearchInterval* i = p1->leftInterval;
      if ((i == 0) || (p1->pointStatus == TTrial::up_inflection))
        break;
      p1 = i->LeftPoint;
    }

    p2 = rt;
    countPointRight = 0;

    while ((p2->pointStatus == TTrial::up) || (p2->pointStatus == TTrial::up_inflection))
    {
      inflection->lowAndUpPoints++;


      RightLocalMinPoint.push_back(p2);
      countPointRight++;

      TSearchInterval* i = p2->rightInterval;
      if ((i == 0) || (p2->pointStatus == TTrial::up_inflection))
        break;
      p2 = i->RightPoint;
    }

    if ((inflection->lowAndUpPoints >= parameters.countPointInLocalMinimum) && (countPointLeft > 0) && (countPointRight > 0))
    {
      int t1 = countPointLeft - (int(parameters.countPointInLocalMinimum / 2));
      int t2 = countPointRight - (int(parameters.countPointInLocalMinimum / 2));
      int d1 = 0;
      int d2 = 0;
      if (countPointLeft < (int(parameters.countPointInLocalMinimum / 2)))
      {
        d2 = -t1;
        if (parameters.TypeStartLocalMethod == 1)
          return isStartLocalMethod;
      }
      else
      {
        countPointLeft = (int(parameters.countPointInLocalMinimum / 2));
      }

      if (countPointRight < (int(parameters.countPointInLocalMinimum / 2)))
      {
        d1 = -t2;
        if (parameters.TypeStartLocalMethod == 1)
          return isStartLocalMethod;
      }
      else
      {
        countPointRight = (int(parameters.countPointInLocalMinimum / 2));
      }

      countPointLeft += d1;
      countPointRight += d2;


      double maxDelta = 1;// pTask.GetB()[0] - pTask.GetA()[0];
      Extended delta = RightLocalMinPoint[countPointRight - 1]->X() - leftLocalMinPoint[countPointLeft - 1]->X();
      //double delta = RightLocalMinPoint[countPointRight - 1]->y[0] - leftLocalMinPoint[countPointLeft - 1]->y[0];

      double parR = 1.0 / parameters.Localr; // 1.0 / (r * r);

      if ((delta / maxDelta) < parR)
        isStartLocalMethod = true;
    }

  }

  parameters.TypeStartLocalMethod = oldTypeStartLocalMethod;

  return isStartLocalMethod;
}


bool TMethod::UpdateStatusDecisionTrees(TTrial* trial, TTrial*& inflection,
  std::vector<TTrial*>& leftLocalMinPoint, std::vector<TTrial*>& RightLocalMinPoint,
  int& countPointLeft, int& countPointRight)
{
#ifdef USE_OpenCV
  //inflection - точка из которой хотим запуститиь метода
  inflection = trial;

  //std::vector<TTrial*>& leftLocalMinPoint, int& countPointLeft - точки слева от inflection
  //std::vector<TTrial*>& RightLocalMinPoint, int& countPointRight - точки справа от inflection
  countPointLeft = 0;
  countPointRight = 0;

  leftLocalMinPoint.clear();
  RightLocalMinPoint.clear();

  TSearchData* data = GetSearchData(trial);
  bool isStartLocalMethod = true;

  int N = data->GetCount() - 1;

  if (parameters.isCalculationInBorderPoint == 1)
    N += 1;

  if (N < 10)
    return false;

  int flag = 0;

  cv::Mat X(N, 1, CV_32FC1);
  cv::Mat ff(N, 1, CV_32FC1);
  float h = 3.14 / N;

  //for (int i = 0; i < N; i++)
  int i = 0;
  int indexX = -1;
  std::vector< TTrial*> allPoints(N);
  for (TSearcDataIterator it = data->GetBeginIterator(); it; ++it)
  {
    if (i >= N)
    {
      print << "Error point count!!!";
      break;
    }

    if (it->LeftPoint->index >= 0)
    {
      X.at<float>(i, 0) = it->LeftPoint->X().toDouble();
      ff.at<float>(i, 0) = it->LeftPoint->GetValue();
      allPoints[i] = it->LeftPoint;
      //if (this->iteration.IterationCount == 30)
      //cout << X.at<float>(i, 0) << " " << ff.at<float>(i, 0) << endl;
      i++;

      if (it->LeftPoint == trial)
        indexX = i;
    }
  }

  if (((N - indexX) <= 1) || (indexX <=1)) 
    return false;


  cv::Ptr< cv::ml::DTrees > Mytree = cv::ml::DTrees::create();

  Mytree->setMinSampleCount(1);
  Mytree->setCVFolds(1);
  Mytree->setMaxDepth(parameters.DecisionTreesMaxDepth);
  Mytree->setRegressionAccuracy(parameters.DecisionTreesRegressionAccuracy);

  Mytree->train(X, cv::ml::ROW_SAMPLE, ff); // тренирует дерево на TrainData


  //int NN = 150;
  //cv::Mat X_preduct(NN, 1, CV_32FC1);
  cv::Mat results;
  //float hh = 3.14 / NN;
  //for (int i = 0; i < NN; i++)
  //{
  //  X_preduct.at<float>(i, 0) = i * hh;
  //}

  Mytree->predict(X, results);

  if (indexX == -1)
  {
    for (; indexX < results.rows; indexX++)
    {
      //if (X_preduct.at<float>(indexX - 1, 0) <= trial->X() && X_preduct.at<float>(indexX + 1, 0) >= trial->X())
      if (X.at<float>(indexX - 1, 0) <= trial->X() && X.at<float>(indexX + 1, 0) >= trial->X())
        break;
    }
  }

  double pointFuncVal = results.at<float>(indexX, 0);
  double curFuncVal = results.at<float>(indexX, 0);
  double oldCurFuncVal = curFuncVal;
  int countLeftStep = 0;
  int lpi = indexX - 1;
  if (lpi <= 0 || lpi >= results.rows)
    print << "ERR";
  for (; lpi > 0; lpi--)
  {
    curFuncVal = results.at<float>(lpi, 0);
    if (curFuncVal < oldCurFuncVal) // Значит слева убывает - конец
      break;
    else if (curFuncVal > oldCurFuncVal) // Значит слева возрастает - параболоид!
    {
      if (oldCurFuncVal != curFuncVal) // Перешагнули возрастающую ступеньку
      {
        oldCurFuncVal = curFuncVal;
        countLeftStep++;
      }
    } 
    leftLocalMinPoint.push_back(allPoints[lpi]);
    countPointLeft++;
  }

  if (countLeftStep < 2) // мало ступенек слева
    return false;


  pointFuncVal = results.at<float>(indexX, 0);
  curFuncVal = results.at<float>(indexX, 0);
  oldCurFuncVal = curFuncVal;
  int countRightStep = 0;
  int rpi = indexX + 1;
  for (; rpi < results.rows; rpi++)
  {
    curFuncVal = results.at<float>(rpi, 0);
    if (curFuncVal < oldCurFuncVal) // Значит справа убывает - конец
      break;
    else if (curFuncVal > oldCurFuncVal)// Значит справа возрастает - параболоид!
    {
      if (oldCurFuncVal != curFuncVal) // Перешагнули возрастающую ступеньку
      {
        oldCurFuncVal = curFuncVal;
        countRightStep++;
      }
    }
    RightLocalMinPoint.push_back(allPoints[rpi]);
    countPointRight++;
  }

  if (countRightStep < 2) // мало ступенек справа
    return false;
   

  //if (this->iteration.IterationCount == 30)
  //for (int i = 0; i < results.rows; i++) 
  //{
  //  //for (int j = 0; j < results.cols; j++)
  //    cout << X_preduct.at<float>(i, 0) << " " << results.at<float>(i, 0);
  //  cout << endl;
  //}



  if ((countPointLeft + countPointRight + 1) < parameters.countPointInLocalMinimum)
    return false;

  double maxDelta = 1;// pTask.GetB()[0] - pTask.GetA()[0];
  if (RightLocalMinPoint.size() < 2 || leftLocalMinPoint.size() < 2)
    return false;
  Extended delta = RightLocalMinPoint[1]->X() - leftLocalMinPoint[1]->X();
  double parR = 1.0 / parameters.Localr; // 1.0 / (r * r);
  if ((delta / maxDelta) < parR)
    isStartLocalMethod = true;
  else
    isStartLocalMethod = false;


  countPointLeft = 2;
  countPointRight = 2;

  return isStartLocalMethod;
#else
  return false;
#endif
}

#ifdef USE_OpenCV
double TMethod::FindDistance(cv::Mat point, int index, TTrial*& inflection) {
  double result = 0;
  for (int i = 0; i < parameters.Dimension; i++) {
    result += (inflection->y[i] - point.at<float>(index, i)) * (inflection->y[i] - point.at<float>(index, i));
  }
  return sqrt(result);
}

void TMethod::recursiveFilling(int dim, const double* lb, const double* ub, int numPointPerDim, cv::Mat *uniformPartition, double* &value, int  &index, int reset) {
  double step = (ub[dim] - lb[dim]) / (numPointPerDim - 1);

  for (int r = reset + 1; r < parameters.Dimension; r++) {
    value[r] = lb[r];
  }

  if (dim == parameters.Dimension - 1) {

    for (int k = 0; k < numPointPerDim; k++) {
      for (int d = 0; d < parameters.Dimension; d++) {
        (*uniformPartition).at<float>(k + numPointPerDim * index, d) = value[d];
        if (d == dim)
          value[d] += step;
      }
    }
    index++;
    return;
  }
  else {
    while (value[dim] < ub[dim]) {
      recursiveFilling(dim + 1, lb, ub, numPointPerDim, uniformPartition, value, index, dim);
      value[dim] += step;
    }
  }
}

void TMethod::fillDataForDecisionTree(TTrial* point) {
  if (point->index >= 0)
  {
    for (int j = 0; j < parameters.Dimension; j++) {
      X.at<float>(indexForTrainingMax, j) = point->y[j];
    }
    ff.at<float>(indexForTrainingMax, 0) = point->GetValue();
    indexForTrainingMax++;
  }
}

void TMethod::PrepareDataForDecisionTree(int N, TSearchData* data) {
  X.create(N, parameters.Dimension, CV_32F);
  ff.create(N, 1, CV_32FC1);

  indexForTrainingMax = 0;
  for (TSearcDataIterator it = data->GetBeginIterator(); it; ++it)
  {
    if (indexForTrainingMax >= N)
    {
      print << "Error point count!!!";
      break;
    }

    fillDataForDecisionTree(it->LeftPoint);
  }
}

void TMethod::CreateTree() {
  Mytree = cv::ml::DTrees::create();

  Mytree->setMinSampleCount(1);
  Mytree->setCVFolds(1);
  Mytree->setMaxDepth(parameters.DecisionTreesMaxDepth);
  Mytree->setRegressionAccuracy(parameters.DecisionTreesRegressionAccuracy);
}

void TMethod::FillTheSegment(int numPointPerDim) {
  uniformPartition = cv::Mat(pow(numPointPerDim, (int)parameters.Dimension), parameters.Dimension, CV_32F);

  const double* lb = pTask.GetA();
  double* value = new double[parameters.Dimension];
  for (int j = 0; j < parameters.Dimension; j++) {
    value[j] = lb[j];
  }
  const double* ub = pTask.GetB();
  int zero = 0;
  recursiveFilling(zero, lb, ub, numPointPerDim, &uniformPartition, value, zero, zero);
}

int TMethod::FindIndexOfNearestPoint(int numPointPerDim, TTrial*& inflection) {
  int nearestPointIndex = 0;
  double distance = DBL_MAX;
  for (int j = 0; j < pow(numPointPerDim, (int)parameters.Dimension); j++) {
    double currentDistance = FindDistance(uniformPartition, j, inflection);
    if (currentDistance < distance) {
      distance = currentDistance;
      nearestPointIndex = j;
    }
  }
  return nearestPointIndex;
}

std::vector<int> TMethod::FindNeighbours(int numPointPerDim, int nearestPointIndex, int* masOfIndexes) {
  int* masOfIndexesForCalc = new int[parameters.Dimension];
  bool flag = false;
  std::vector<int> neighbours;
  int mask[MaxDim]; // триарная маска, -1 - влево, +1 - в право, 0 - остаемся на месте.
  for (int i = 0; i < parameters.Dimension; i++)
  {
    mask[i] = -1;
  }

  int maskIndex = 0; //индекс начиная с которого изменяется маска
  int startMaskIndex = 0; // куда сдвигаем maskIndex
  int maxMaskIndex = 0; // до куда дошли

  mask[0] = -2;

  for (int i = 0; i < pow(3, (int)parameters.Dimension); i++)
  {
    for (int k = 0; k < parameters.Dimension; k++) {
      masOfIndexesForCalc[k] = masOfIndexes[k];
    }

    maskIndex = startMaskIndex; //сдвигаем индекс маски
    while (1) //изменяем маску
    {

      if (maskIndex == parameters.Dimension)
      {
        break;
      }

      mask[maskIndex]++;
      if (mask[maskIndex] == 2)
      {
        mask[maskIndex] = -1;
        maskIndex++;
        if (maxMaskIndex < maskIndex)
          maxMaskIndex = maskIndex;
      }
      else
        break;
    }
    maskIndex = startMaskIndex; //сдвигаем индекс маски

    int neighbourIndex = 0;
    int N = parameters.Dimension;
    for (int k = 0; k < parameters.Dimension; k++) //формируем координаты
    {
      masOfIndexes[k] = masOfIndexes[k] + mask[k];
      if (masOfIndexes[k] < 0 || masOfIndexes[k] >= numPointPerDim) {
        flag = true;
        break;
      }
      neighbourIndex += masOfIndexes[k] * pow(numPointPerDim, N - 1);
      N--;
    }
    if (flag) {
      continue;
    }
    if (neighbourIndex != nearestPointIndex)
      neighbours.push_back(neighbourIndex);
  }
  return neighbours;
}

// Return true if we need to execute local method and false if we don't need to do so
bool TMethod::FindAndCheckPointWithNeighbours(int numPointPerDim, int nearestPointIndex, cv::Mat results) {
  int* masOfIndexes = new int[parameters.Dimension];
  std::vector<int> checkVector;
  checkVector.push_back(nearestPointIndex);
  std::vector<int> errorVector;
  errorVector.push_back(nearestPointIndex);

  int j = 0;
  bool stopCheckingThisPoint = false;
  while (j < checkVector.size()) {
    if (stopCheckingThisPoint) {
      break;
    }
    int checkedValue = checkVector[j];
    for (int k = parameters.Dimension - 1; k >= 0; k--) {
      masOfIndexes[k] = checkedValue % numPointPerDim;
      checkedValue = (int)(checkedValue / numPointPerDim);
    }
    //std::cout << "j = " << j << "; checkVector.size = " << checkVector.size() << "\n";

    // Теперь нужно найти соседей
    std::vector<int> neighbours = FindNeighbours(numPointPerDim, nearestPointIndex, masOfIndexes);

    // и осмотреть
    for (int k = 0; k < neighbours.size(); k++) {
      auto chk = std::find(errorVector.begin(), errorVector.end(), neighbours[k]);
      if (chk != errorVector.end()) {
        continue;
      }

      if (results.at<float>(neighbours[k], 0) < results.at<float>(checkVector[j], 0)) {
        //std::cout << "checked value = " << results.at<float>(checkVector[j], 0) << "; neighbour value = " << results.at<float>(neighbours[k], 0) << "\n";
        //std::cout << "\n";
        //return false;
        stopCheckingThisPoint = true;
        break;
      }
      else if (results.at<float>(neighbours[k], 0) == results.at<float>(checkVector[j], 0)) {
        checkVector.push_back(neighbours[k]);
      }
      errorVector.push_back(neighbours[k]);
    }
    j++;
  }
  return !stopCheckingThisPoint;
}

void TMethod::UpdateStatusDecisionTreesMultiDims(TTrial* trial, TTrial*& inflection)
{
#ifdef USE_OpenCV
  //std::cout << indexForTrainingMax << "\n";
  //inflection - точка из которой хотим запуститиь метода
  inflection = trial;

  TSearchData* data = GetSearchData(trial);
  bool isStartLocalMethod = true;

  int N = data->GetCount() - 1;

  if (parameters.isCalculationInBorderPoint == 1)
    N += 1;

  if (N < 100 * parameters.Dimension)
    return;

  pointsForLocalMethod.clear();

  if (isFirst) {
    PrepareDataForDecisionTree(N, data);

    CreateTree();

    Mytree->train(X, cv::ml::ROW_SAMPLE, ff); // тренирует дерево на TrainData

    // После того, как натренировали дерево на исходных данных
    // нужно организовать дискретную сетку (отображение наших точек на одномерный массив)
    // провести предсказание на новом одномерном массиве, найти точку, близкую к нашей пришедшей точке
    // и проверить ее соседей: находим значение меньшее -> возвращаем фолс; все значения больше -> возвращаем тру;
    // равное значение -> ее тоже проверяем

    // Равномерное заполнение отрезка (от левой до правой границы)
    int numPointPerDim = pow(300, 1.0 / parameters.Dimension);

    FillTheSegment(numPointPerDim);

    // На полученной сетке предсказываем значения функции
    cv::Mat results;
    Mytree->predict(uniformPartition, results);

    // На сетке ищем точку наиболее близкую к inflection
    int nearestPointIndex = FindIndexOfNearestPoint(numPointPerDim, inflection);

    //Надо как-то заполнить соседей и все что ниже завернуть в цикл, чтобы много раз прогонялось.
    //Вычислив индекс нужно посмотреть значение функции в соответствующей ячейке массива и сравнить с "ближайшим"
    //Дальше по ситуации, но нужен какой-то стек(?) куда я буду складывать точки, у которых совпадет значение функции
    //потому что их тоже надо проверять. Соответственно, нужно обернуть в цикл (по размеру этого стека?) проверку соседей
    //И подумать о том, как не зациклиться
    if (FindAndCheckPointWithNeighbours(numPointPerDim, nearestPointIndex, results)) {
      pointsForLocalMethod.push_back(inflection);
    }
    isFirst = false;
  }
  else {
    pointsForTree.push_back(inflection);

    if (pointsForTree.size() < 100 * parameters.Dimension) 
      return;
    
    int sizeOfVector = pointsForTree.size();

    indexForTrainingMax = 0;
    for (int k = 0; k < sizeOfVector; ++k)
    {
      if (indexForTrainingMax >= sizeOfVector)
      {
        print << "Error point count!!!";
        break;
      }

      fillDataForDecisionTree(pointsForTree[k]);
    }

    Mytree->train(X, cv::ml::ROW_SAMPLE, ff); // тренирует дерево на TrainData

    int numPointPerDim = pow(300, 1.0 / parameters.Dimension);

    cv::Mat results;
    Mytree->predict(uniformPartition, results);

    for (int i = 0; i < sizeOfVector; i++) {
      // На сетке ищем точку наиболее близкую к inflection
      int nearestPointIndex = FindIndexOfNearestPoint(numPointPerDim, inflection);

      if (FindAndCheckPointWithNeighbours(numPointPerDim, nearestPointIndex, results)) {
        pointsForLocalMethod.push_back(pointsForTree[i]);
      }
    }
    pointsForTree.clear();
  }

  return;
#else
  return;
#endif
}
#endif

bool TMethod::UpdateStatusAllMinimum(TTrial* trial, TTrial*& inflection,
  std::vector<TTrial*>& leftLocalMinPoint, std::vector<TTrial*>& RightLocalMinPoint,
  int& countPointLeft, int& countPointRight)
{
#ifdef USE_OpenCV
  //inflection - точка из которой хотим запуститиь метода
  inflection = trial;

  //std::vector<TTrial*>& leftLocalMinPoint, int& countPointLeft - точки слева от inflection
  //std::vector<TTrial*>& RightLocalMinPoint, int& countPointRight - точки справа от inflection
  countPointLeft = 0;
  countPointRight = 0;

  leftLocalMinPoint.clear();
  RightLocalMinPoint.clear();
  
  TSearchData* data = GetSearchData(trial);
  bool isStartLocalMethod = true;

  int N = data->GetCount() - 1;

  if (parameters.isCalculationInBorderPoint == 1)
    N += 1;

  if (N < 10)
    return false;

  int flag = 0;

  cv::Mat X(N, 1, CV_32FC1);
  cv::Mat ff(N, 1, CV_32FC1);

  //for (int i = 0; i < N; i++)
  int i = 0;
  int indexX = -1;
  std::vector< TTrial*> allPoints(N);
  for (TSearcDataIterator it = data->GetBeginIterator(); it; ++it)
  {
    if (i >= N)
    {
      print << "Error point count!!!";
      break;
    }

    if (it->LeftPoint->index >= 0)
    {
      X.at<float>(i, 0) = it->LeftPoint->X().toDouble();
      ff.at<float>(i, 0) = it->LeftPoint->GetValue();
      allPoints[i] = it->LeftPoint;
      //if (this->iteration.IterationCount == 30)
      //cout << X.at<float>(i, 0) << " " << ff.at<float>(i, 0) << endl;
      i++;

      //if (it->LeftPoint == trial)
      //  indexX = i;
    }
  }

  //if (((N - indexX) <= 1) || (indexX <= 1))
  //  return false;

  
  cv::Ptr< cv::ml::DTrees > Mytree = cv::ml::DTrees::create();

  Mytree->setMinSampleCount(1);
  Mytree->setCVFolds(1);
  Mytree->setMaxDepth(parameters.DecisionTreesMaxDepth);
  Mytree->setRegressionAccuracy(parameters.DecisionTreesRegressionAccuracy);

  Mytree->train(X, cv::ml::ROW_SAMPLE, ff); // тренирует дерево на TrainData


  int NN = int(1.0 / Epsilon);
  cv::Mat X_preduct(NN, 1, CV_32FC1);
  cv::Mat results;
  float hh = 1.0 / NN;
  for (int i = 0; i < NN; i++)
  {
    X_preduct.at<float>(i, 0) = i * hh;
  }


  Mytree->predict(X_preduct, results);
  indexX = -1;

  
  std::vector<int> counterValue(NN);
  std::vector<int> direction(NN); // -1 - убывает в лево, +1 убывает в право, 0 - минимум

  if (indexX == -1)
  {
    for (indexX = 1; indexX < results.rows - 1; indexX++)
    {
      //if (X_preduct.at<float>(indexX - 1, 0) <= trial->X() && X_preduct.at<float>(indexX + 1, 0) >= trial->X())
      if (X_preduct.at<float>(indexX - 1, 0) <= trial->X() && X_preduct.at<float>(indexX + 1, 0) >= trial->X())
        break;
    }
  }
  
  std::vector<double> XDouble(NN), resDouble(NN);

  for (int i = 0; i < results.rows; i++)
  {
    counterValue[i] = 1;
    direction[i] = -100;
    XDouble[i] = X_preduct.at<float>(i, 0);
    resDouble[i] = results.at<float>(i, 0);
  }
  
  if (results.at<float>(0, 0) > results.at<float>(1, 0))
  {
    direction[0] = 1;
  }
  else if (results.at<float>(0, 0) < results.at<float>(1, 0))
  {
    direction[0] = 0;
  }
  else
  {
    int j = 0;
    while ((results.at<float>(j, 0) == results.at<float>(j + 1, 0)) && (j < NN - 1))
      j = j + 1;
    if (results.at<float>(j, 0) > results.at<float>(j+1, 0))
    {
      for (int i1 = 0; i1 <= j; i1++)
      {
        direction[i1] = 1;
      }
    }
    else if (results.at<float>(j, 0) < results.at<float>(j+1, 0))
    {
      direction[0] = 0;
      for (int i1 = 1; i1 <= j; i1++)
      {
        direction[i1] = -1;
      }      
    }
  }

  if (results.at<float>(NN-1, 0) > results.at<float>(NN-2, 0))
  {
    direction[0] = -1;
  }
  else if (results.at<float>(NN-1, 0) < results.at<float>(NN-2, 0))
  {
    direction[0] = 0;
  }
  else
  {
    int j = NN - 1;
    while ((results.at<float>(j, 0) == results.at<float>(j - 1, 0)) && (j > 0))
      j = j - 1;
    if (results.at<float>(j, 0) > results.at<float>(j - 1, 0))
    {
      for (int i1 = j; i1 < NN; i1++)
      {
        direction[i1] = -1;
      }
    }
    else if (results.at<float>(j, 0) < results.at<float>(j - 1, 0))
    {
      direction[NN-1] = 0;
      for (int i1 = j; i1 < NN-1; i1++)
      {
        direction[i1] = 1;
      }
    }
  }
  
  for (int i = 1; i < results.rows - 1; i++)
  {
    if (direction[i] == -100)
    {
      if ((results.at<float>(i - 1, 0) < results.at<float>(i, 0)) && (results.at<float>(i, 0) < results.at<float>(i + 1, 0))) // убывает на лево
      {
        direction[i] = -1;
      }
      else if ((results.at<float>(i - 1, 0) < results.at<float>(i, 0)) && (results.at<float>(i, 0) > results.at<float>(i + 1, 0))) // локальный максимум
      {
        if (results.at<float>(i - 1, 0) < results.at<float>(i + 1, 0)) // если справо больше то туда и пойдем
          direction[i] = +1;
        else
          direction[i] = -1;
      }
      else if ((results.at<float>(i - 1, 0) > results.at<float>(i, 0)) && (results.at<float>(i, 0) > results.at<float>(i + 1, 0))) // убывает на право
      {
        direction[i] = +1;
      }
      else if ((results.at<float>(i - 1, 0) > results.at<float>(i, 0)) && (results.at<float>(i, 0) < results.at<float>(i + 1, 0))) // локальный минимум
      {
        direction[i] = 0;
      }
      else if ((results.at<float>(i - 1, 0) != results.at<float>(i, 0)) && (results.at<float>(i, 0) == results.at<float>(i + 1, 0))) // плащадка с равенством которая ...
      {
        int j = i;
        while ((results.at<float>(j, 0) == results.at<float>(j + 1, 0)) && (j < NN - 1))
          j = j + 1;

        if ((results.at<float>(i - 1, 0) < results.at<float>(i, 0)) && (results.at<float>(i, 0) < results.at<float>(j + 1, 0))) // убывает на лево
        {
          for (int i1 = i; i1 <= j; i1++)
          {
            direction[i1] = -1;
          }          
        }
        else if ((results.at<float>(i - 1, 0) < results.at<float>(i, 0)) && (results.at<float>(i, 0) > results.at<float>(j + 1, 0))) // локальный максимум
        {
          if (results.at<float>(i - 1, 0) < results.at<float>(i + 1, 0)) // если справо больше то туда и пойдем
          {
            for (int i1 = i; i1 <= j; i1++)
            {
              direction[i1] = +1;
            }
          }
          else
          {
            for (int i1 = i; i1 <= j; i1++)
            {
              direction[i1] = -1;
            }
          }
        }
        else if ((results.at<float>(i - 1, 0) > results.at<float>(i, 0)) && (results.at<float>(i, 0) > results.at<float>(j + 1, 0))) // убывает на право
        {
          for (int i1 = i; i1 <= j; i1++)
          {
            direction[i1] = +1;
          }
        }
        else if ((results.at<float>(i - 1, 0) > results.at<float>(i, 0)) && (results.at<float>(i, 0) < results.at<float>(j + 1, 0))) // локальный минимум
        {
          direction[i] = 0;
          for (int i1 = i + 1; i1 <= j; i1++)
          {
            direction[i1] = -1;
          }
        }

      }

    }
  }

  localMinimumPoints.clear();
  
  for (int i = 0; i < results.rows; i++)
  {
    if (direction[i] == 0) // нашли минимум
    {
      TTrial* a = 0;
      double minZDiff = MaxDouble;
      for (int jndex = 0; jndex < allPoints.size(); jndex++)
      {
        //if (/*(fabs(allPoints[jndex]->X().toDouble() - XDouble[i]) < 10.0 * hh) &&*/ (fabs(allPoints[jndex]->GetValue() - resDouble[i]) < hh))
        //{
          if (fabs(allPoints[jndex]->GetValue() - resDouble[i]) < minZDiff)
          {
            a = allPoints[jndex];
            minZDiff = fabs(allPoints[jndex]->GetValue() - resDouble[i]);
          }
        //}
      }

      if (a != 0)
      {
        localMinimumPoints.push_back(a);
      }
    }
  }

  
  /// Спускаемся в минимум
  int trialIndex = indexX;
  while ((trialIndex < NN) && (trialIndex >= 0) && (direction[trialIndex] != 0))
  {
    trialIndex = trialIndex + direction[trialIndex];
  }
  indexX = trialIndex;
  
  int countStep = parameters.countPointInLocalMinimum / 2;

  if (indexX < 0 || indexX >= results.rows)
    print << "ERRROR indexX !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";

  double pointFuncVal = results.at<float>(indexX, 0);
  double curFuncVal = results.at<float>(indexX, 0);
  
  double oldCurFuncVal = curFuncVal;
  int countLeftStep = 0;
  int lpi = indexX - 1;
  //if (lpi <= 0 || lpi >= results.rows)
  //  print << "ERRROR !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
  for (; lpi > 0; lpi--)
  {
    curFuncVal = results.at<float>(lpi, 0);
    if (curFuncVal < oldCurFuncVal) // Значит слева убывает - конец
      break;
    else if (curFuncVal > oldCurFuncVal) // Значит слева возрастает - параболоид!
    {
      if (oldCurFuncVal != curFuncVal) // Перешагнули возрастающую ступеньку
      {
        oldCurFuncVal = curFuncVal;
        countLeftStep++;
      }
    }
    //leftLocalMinPoint.push_back(allPoints[lpi]);
    //countPointLeft++;
  }
  
  //if (countLeftStep < countStep) // мало ступенек слева
  //  return false;

  
  pointFuncVal = results.at<float>(indexX, 0);
  curFuncVal = results.at<float>(indexX, 0);
  oldCurFuncVal = curFuncVal;
  int countRightStep = 0;
  int rpi = indexX + 1;
  for (; rpi < results.rows; rpi++)
  {
    curFuncVal = results.at<float>(rpi, 0);
    if (curFuncVal < oldCurFuncVal) // Значит справа убывает - конец
      break;
    else if (curFuncVal > oldCurFuncVal)// Значит справа возрастает - параболоид!
    {
      if (oldCurFuncVal != curFuncVal) // Перешагнули возрастающую ступеньку
      {
        oldCurFuncVal = curFuncVal;
        countRightStep++;
      }
    }
    //RightLocalMinPoint.push_back(allPoints[rpi]);
    //countPointRight++;
  }

  //if (countRightStep < countStep) // мало ступенек справа
  //  return false;

  if ((countRightStep + countLeftStep) < (parameters.countPointInLocalMinimum - 3))
    return false;

  TTrial* a = 0;
  double minZDiff = MaxDouble;

  for (int i = 0; i < allPoints.size(); i++)
  {
    //if ((fabs(allPoints[i]->X().toDouble() - XDouble[indexX]) < 10.0 * hh) && (fabs(allPoints[i]->GetValue() - resDouble[indexX]) < hh))
    //{
    //  inflection = allPoints[i];
    //  break;
    //}

    if (fabs(allPoints[i]->GetValue() - resDouble[indexX]) < minZDiff)
    {
      a = allPoints[i];
      minZDiff = fabs(allPoints[i]->GetValue() - resDouble[indexX]);
    }
  }

  inflection = a;

  /// Переходим в минимум ступеньки
  while (inflection->GetRightPoint() != 0)
  {
    if (inflection->GetRightPoint()->GetValue() < inflection->GetValue())
      inflection = inflection->GetRightPoint();
    else
      break;
  }

  int sumPoint = parameters.countPointInLocalMinimum;

  TTrial* lp = inflection->GetLeftPoint();
  curFuncVal = inflection->GetValue();
  if (lp != 0)
  {
    for (int i = 0; i < countStep; i++)
    {
      if (lp->GetValue() < curFuncVal)
        break;
      curFuncVal = lp->GetValue();
      leftLocalMinPoint.push_back(lp);
      lp = lp->GetLeftPoint();
      countPointLeft++;
      if (lp == 0)
        break;
    }
  }

  if (countStep - countPointLeft > 0)
    countStep = countStep + countStep - countPointLeft;

  TTrial* rp = inflection->GetRightPoint();
  curFuncVal = inflection->GetValue();
  if (rp != 0)
  {
    for (int i = 0; i < countStep; i++)
    {
      if (rp->GetValue() < curFuncVal)
        break;
      curFuncVal = rp->GetValue();
      RightLocalMinPoint.push_back(rp);
      rp = rp->GetRightPoint();
      countPointRight++;
      if (rp == 0)
        break;
    }
  }

  if (countStep - countPointRight > 0)
  {
    countStep = countStep - countPointRight ;
    if (leftLocalMinPoint.size() == 0)
      return false;
    curFuncVal = leftLocalMinPoint[leftLocalMinPoint.size() - 1]->GetValue();
    if (lp != 0)
    {
      for (int i = 0; i < countStep; i++)
      {
        if (lp->GetValue() < curFuncVal)
          break;
        curFuncVal = lp->GetValue();
        leftLocalMinPoint.push_back(lp);
        lp = lp->GetLeftPoint();
        countPointLeft++;
        if (lp == 0)
          break;
      }
    }
  }

  if ((countPointLeft + countPointRight + 1) < parameters.countPointInLocalMinimum)
    return false;

  countPointLeft = std::min(countPointLeft, 2);
  countPointRight = std::min(countPointLeft, 2);

  double maxDelta = 1;// pTask.GetB()[0] - pTask.GetA()[0];
  if (RightLocalMinPoint.size() < 2 || leftLocalMinPoint.size() < 2)
    return false;
  Extended delta = RightLocalMinPoint[1]->X() - leftLocalMinPoint[1]->X();
  double parR = 1.0 / parameters.Localr; // 1.0 / (r * r);
  if ((delta / maxDelta) < parR)
    isStartLocalMethod = true;
  else
    isStartLocalMethod = false;

  return isStartLocalMethod;
  //return false;
#else
  return false;
#endif
}

// ------------------------------------------------------------------------------------------------
void TMethod::UpdateStatus(TTrial* trial)
{
  TTrial* inflection = 0;
  std::vector<TTrial*> leftLocalMinPoint;
  std::vector<TTrial*> RightLocalMinPoint;

  int countPointLeft = 0;
  int countPointRight = 0;

  bool isStartLocalMethod = false;

  if (parameters.TypeLocalMinIntervale == NPoints)
  {
    isStartLocalMethod = UpdateStatusNPoint(trial, inflection, leftLocalMinPoint, RightLocalMinPoint,
      countPointLeft, countPointRight);
  }
  else if (parameters.TypeLocalMinIntervale == DecisionTrees)
  {
#ifdef USE_OpenCV
    if (parameters.Dimension >= 2) {
      UpdateStatusDecisionTreesMultiDims(trial, inflection);

      for (int i = 0; i < pointsForLocalMethod.size(); i++) {
        LocalS(*pointsForLocalMethod[i], leftLocalMinPoint, RightLocalMinPoint, countPointLeft, countPointRight);
      }

      isStartLocalMethod = false;
    }
    else {
      isStartLocalMethod = UpdateStatusDecisionTrees(trial, inflection, leftLocalMinPoint, RightLocalMinPoint,
        countPointLeft, countPointRight);
    }
#endif
  }
  else if (parameters.TypeLocalMinIntervale == AllMinimum)
  {
    isStartLocalMethod = UpdateStatusAllMinimum(trial, inflection, leftLocalMinPoint, RightLocalMinPoint,
      countPointLeft, countPointRight);
  }

  if (isStartLocalMethod)
    LocalS(*inflection, leftLocalMinPoint, RightLocalMinPoint, countPointLeft, countPointRight);

  //PlotDecisionTrees();
}

// ------------------------------------------------------------------------------------------------
void TMethod::RenewSearchData()
{
  for (unsigned int j = 0; j < iteration.pCurTrials.size(); j++)
  {
    if (iteration.pCurTrials[j] == 0)
      continue;

    TSearchInterval* p = 0;
    TSearchInterval* interval = iteration.pCurTrials[j]->leftInterval;
    p = AddCurrentPoint(*iteration.pCurTrials[j], interval);

    if (p == 0)
      continue;

    if (interval == 0)
      interval = iteration.pCurTrials[j]->leftInterval;

    //Обработка началной итерации
    if (iteration.IterationCount == 1)
    {
      //bool f = j < NumPoints - 1;
      //// Добавить следующий интервал для обработки
      //if (f && (interval != 0))
      //{
      //  iteration.BestIntervals[j + 1] = p;
      //}
      pData->SetRecalc(true);
    }

    // Вычисляем оценку константы
    CalculateM(p);
    CalculateM((interval));

    //SetIntervalVal(*BestIntervalsj, p, &(pCurTrialsj));

    // Если полный пересчет не нужен - обновляем только очереди характеристик
    if (!pData->IsRecalc())
    {
      // Удалять интервалы из очереди не надо - они уже удалены в GetBestIntervals
      // Вставляем два новых интервала
      p->R = CalculateGlobalR(p);
      p->locR = CalculateLocalR(p);
      pData->PushToQueue(p);

      (interval)->R = CalculateGlobalR((interval));
      (interval)->locR = CalculateLocalR((interval));

      pData->PushToQueue((interval));
    }
    if (parameters.isUseLocalUpdate == true)
      UpdateStatus(iteration.pCurTrials[j]);
  }
  isFindInterval = false;
}

// ------------------------------------------------------------------------------------------------
bool TMethod::EstimateOptimum()
{
  bool isOptimumUpdated = false;
  // Сравниваем значение в текущем оптимуме с текущими точками
  for (unsigned int j = 0; j < iteration.pCurTrials.size(); j++)
  {
    if (iteration.pCurTrials[j] == 0)
      continue;
    isOptimumUpdated = UpdateOptimumEstimation(*iteration.pCurTrials[j]);
  }
  return isOptimumUpdated;
}

// ------------------------------------------------------------------------------------------------
void TMethod::FinalizeIteration()
{
  iteration.IterationCount++;
  parameters.iterationNumber = iteration.IterationCount;

  for (unsigned int i = 0; i < iteration.pCurTrials.size(); i++)
    iteration.pCurTrials[i] = 0;

  if (parameters.TypeCalculation == 8) {
    SetNumPoints(1);
    parameters.NumPoints = 1;
  }
  //if (pTask.GetProcLevel() == 0)
  //  printf("%d\t;\t%lf\t;\t%lf\n",iteration.IterationCount, pData->M[0], globalM[0]);

  if (isStop)
  {
    if (parameters.IsPlot)
    {
      //PlotPoint2();
      //PlotDecisionTrees();
      ///Plot3DDecisionTrees();
    }

    //Plot3DDecisionTrees();
  }

  if (isLocalZUpdate)//Если нужен пересчет - обновился минимум
  {
    LocalSearch();
  }
  isLocalZUpdate = false;

  //if (parameters.IsPlot)
    //if (iteration.IterationCount % 10 == 0)
      //PlotPoint();
}



IterationType TMethod::GetIterationType(int iterationNumber, int localMixParameter)
{
  if (iterationNumber < StartLocalIteration)
    return   Global;

  IterationType type;
  if (localMixParameter > 0) {
    localMixParameter++;

    if (iterationNumber % localMixParameter != 0)
      type = Global;
    else
      type = Local;
  }
  else if (localMixParameter < 0) {
    localMixParameter = -localMixParameter;
    localMixParameter++;

    if (iterationNumber % localMixParameter != 0)
      type = Local;
    else
      type = Global;
  }
  else //localMixParameter == 0
    type = Global;

  return type;
}

int TMethod::IsBoundary(TSearchInterval* p) {
  int ans = 0;
  if (p->izl() == -2)
    ans = 1;
  else if (p->izr() == -2)
    ans = 2;
  else if (p->LeftPoint->leftInterval == NULL)
    ans = 1;
  else if (p->RightPoint->rightInterval == NULL)
    ans = 2;

  return ans;
}

// ------------------------------------------------------------------------------------------------
void TMethod::UpdateM(double newValue, int index, int boundaryStatus, TSearchInterval* p)
{
  double lambda = 0;
  double gamma = 0;
  double Xm = 0;
  double temp = 0;
  double H = 0;
  int begin = 0;
  int end = 0;
  double z1 = 0;
  double z2 = 0;
  int j = 0;
  int max = 0;

  switch (parameters.LocalTuningType) {
  case 0:
    if (newValue > pData->M[index] || pData->M[index] == 1.0 && newValue > _M_ZERO_LEVEL)
    {
      pData->M[index] = newValue;
      pData->SetRecalc(true);
      pData->pRecalcDatas.push_back(pData);
      if (globalM[index] < pData->M[index])
      {
        globalM[index] = pData->M[index];
        isGlobalMUpdate = true;
      }
    }
    break;
  case 1:
    //LT
    
    // Вычисление лямбды

    lambda = newValue;

    //Если левая граница
    if (boundaryStatus == 1) {
      if ((p->RightPoint->rightInterval->izr() == p->RightPoint->rightInterval->izl()) && (p->izr() >= p->izl()))
      {
        temp = fabs(p->RightPoint->rightInterval->zr() - p->RightPoint->rightInterval->zl()) / p->RightPoint->rightInterval->delta;
      }
      else
        temp = 0;
      if (temp > lambda)
        lambda = temp;
    }
    //Если правая граница
    else if (boundaryStatus == 2) {
      if ((p->LeftPoint->leftInterval->izl() == p->LeftPoint->leftInterval->izr()) && (p->izl() >= p->izr()))
      {
        temp = fabs(p->LeftPoint->leftInterval->zr() - p->LeftPoint->leftInterval->zl()) / p->LeftPoint->leftInterval->delta;
      }
      else
        temp = 0;

      if (temp > lambda)
        lambda = temp;
    }
    //если не граница
    else {
      if ((p->LeftPoint->leftInterval->izl() == p->LeftPoint->leftInterval->izr()) && (p->izl() >= p->izr()))
      {
        temp = fabs(p->LeftPoint->leftInterval->zr() - p->LeftPoint->leftInterval->zl()) / p->LeftPoint->leftInterval->delta;
      }
      else
        temp = 0;
      if (temp > lambda)
        lambda = temp;

      if ((p->RightPoint->rightInterval->izr() == p->RightPoint->rightInterval->izl()) && (p->izr() >= p->izl()))
      {
        temp = fabs(p->RightPoint->rightInterval->zr() - p->RightPoint->rightInterval->zl()) / p->RightPoint->rightInterval->delta;
      }
      else
        temp = 0;
      if (temp > lambda)
        lambda = temp;
    }

    // Вычисление гаммы
    j = p->izr();
    if (p->izl() > j)
      j = p->izl();

    if (p->izr() == p->izl()) {
      if (newValue > mu[j])
        mu[j] = newValue;
    }
    else {
      TSearcDataIterator i = pData->GetIterator(p);
      ++i;
      while (i != NULL && p->izl() > i->izl() && IsIntervalInSegment(p, (*i)))
        ++i;
      if (i != NULL && p->izl() == i->izl() && IsIntervalInSegment(p, (*i)) && p->izl() == j)
      {
        temp = fabs(i->z()[p->izl()] - p->zl()) / root(i->xl() - p->xl(), rootDim);
        if (temp > mu[j])
          mu[j] = temp;
      }

      i = pData->GetIterator(p);
      --i;
      while (i != NULL && p->izl() > i->izl() && IsIntervalInSegment(p, (*i)))
        --i;
      if (i != NULL && p->izl() == i->izl() && IsIntervalInSegment(p, (*i)) && p->izl() == j)
      {
        temp = fabs(i->z()[p->izl()] - p->zl()) / root(p->xl() - i->xl(), rootDim);
        if (temp > mu[j])
          mu[j] = temp;
      }
    }

    Xm = p->delta;

    if (isSearchXMax)
    {
      Xmax[j] = 0;
      for (TSearcDataIterator it = pData->GetBeginIterator(); it; ++it)
      {
        Xm = it->delta;
        max = it->izl();
        if (it->izr() > max)
          max = it->izr();

        if ((Xm > Xmax[max]) && (max == j))
        {
          Xmax[max] = Xm;
          intervalXMax = *it;
        }
      }
      isSearchXMax = false;
    }
    else
    {
      if (Xm > Xmax[j])
      {
        Xmax[j] = Xm;
        intervalXMax = p;
      }
    }
    gamma = (mu[j] * p->delta) / Xmax[j];

    //Запоминаем конечное мю
    if (parameters.ltXi > pData->M[index] || pData->M[index] == 1.0 && newValue > _M_ZERO_LEVEL)
    {
      pData->M[index] = parameters.ltXi;
      pData->SetRecalc(true);

      if (globalM[index] < pData->M[index])
      {
        globalM[index] = pData->M[index];
        isGlobalMUpdate = true;
      }
    }
    if (lambda > pData->M[index])
    {
      pData->M[index] = lambda;
      pData->SetRecalc(true);

      if (globalM[index] < pData->M[index])
      {
        globalM[index] = pData->M[index];
        isGlobalMUpdate = true;
      }
    }
    if (gamma > pData->M[index])
    {
      pData->M[index] = gamma;
      pData->SetRecalc(true);

      if (globalM[index] < pData->M[index])
      {
        globalM[index] = pData->M[index];
        isGlobalMUpdate = true;
      }
    }

    break;

  case 2:
    //LTA
    // Вычисление лямбды

    //Если левая граница
    if (boundaryStatus == 1) {
      lambda = newValue;
      temp = fabs(p->RightPoint->rightInterval->zr() - p->RightPoint->rightInterval->zl()) / p->RightPoint->rightInterval->delta;
      if (temp > lambda)
        lambda = temp;
    }
    //Если правая граница
    else if (boundaryStatus == 2) {
      lambda = newValue;
      temp = fabs(p->LeftPoint->leftInterval->zr() - p->LeftPoint->leftInterval->zl()) / p->LeftPoint->leftInterval->delta;
      if (temp > lambda)
        lambda = temp;
    }
    //если не граница
    else {
      lambda = newValue;
      temp = fabs(p->LeftPoint->leftInterval->zr() - p->LeftPoint->leftInterval->zl()) / p->LeftPoint->leftInterval->delta;
      if (temp > lambda)
        lambda = temp;
      temp = fabs(p->RightPoint->rightInterval->zr() - p->RightPoint->rightInterval->zl()) / p->RightPoint->rightInterval->delta;
      if (temp > lambda)
        lambda = temp;
    }

    // Вычисление гаммы
    if (newValue > mu[0]) {
      mu[0] = newValue;
    }
    Xm = p->delta;//pow(p->delta, pTask.GetFreeN());

    if (isSearchXMax)
    {
      Xmax[0] = MinDouble;
      for (TSearcDataIterator it = pData->GetBeginIterator(); it; ++it)
      {
        Xm = it->delta;
        if (Xm > Xmax[0])
        {
          Xmax[0] = Xm;
          intervalXMax = *it;
        }
      }
      isSearchXMax = false;
    }
    else
    {
      if (Xm > Xmax[0])
      {
        Xmax[0] = Xm;
        intervalXMax = p;
      }
    }
    gamma = (mu[0] * p->delta) / Xmax[0];//pow(Xmax, 1. / pTask.GetFreeN());
    //gamma = mu;

    //Запоминаем конечное мю
    if (parameters.ltXi > pData->M[index] || pData->M[index] == 1.0 && newValue > _M_ZERO_LEVEL)
    {
      pData->M[index] = parameters.ltXi;
      pData->SetRecalc(true);

      if (globalM[index] < pData->M[index])
      {
        globalM[index] = pData->M[index];
        isGlobalMUpdate = true;
      }
    }

    temp = (lambda / r) + (((r - 1) * gamma) / r);
    //temp = mu;

    if (temp > pData->M[index])
    {
      pData->M[index] = temp;
      pData->SetRecalc(true);

      if (globalM[index] < pData->M[index])
      {
        globalM[index] = pData->M[index];
        isGlobalMUpdate = true;
      }
    }
    break;

  case 3:
    //LTMA
    // Вычисление лямбды

    //Если левая граница
    if (boundaryStatus == 1) {
      lambda = newValue;
      if (p->RightPoint->rightInterval->izl() < 0 || p->RightPoint->rightInterval->izr() < 0)
      {
        temp = 0;
      }
      else
      {
        temp = fabs(p->RightPoint->rightInterval->zr() - p->RightPoint->rightInterval->zl()) / p->RightPoint->rightInterval->delta;
      }
      if (temp > lambda)
        lambda = temp;
    }
    //Если правая граница
    else if (boundaryStatus == 2) {
      lambda = newValue;
      if (p->LeftPoint->leftInterval->izl() < 0 || p->LeftPoint->leftInterval->izr() < 0)
      {
        temp = 0;
      }
      else
      {
        temp = fabs(p->LeftPoint->leftInterval->zr() - p->LeftPoint->leftInterval->zl()) / p->LeftPoint->leftInterval->delta;
      }
      if (temp > lambda)
        lambda = temp;
    }
    //если не граница
    else {
      lambda = newValue;
      if (p->LeftPoint->leftInterval->izl() < 0 || p->LeftPoint->leftInterval->izr() < 0)
      {
        temp = 0;
      }
      else
      {
        temp = fabs(p->LeftPoint->leftInterval->zr() - p->LeftPoint->leftInterval->zl()) / p->LeftPoint->leftInterval->delta;
      }
      if (temp > lambda)
        lambda = temp;
      if (p->RightPoint->rightInterval->izl() < 0 || p->RightPoint->rightInterval->izr() < 0)
      {
        temp = 0;
      }
      else
      {
        temp = fabs(p->RightPoint->rightInterval->zr() - p->RightPoint->rightInterval->zl()) / p->RightPoint->rightInterval->delta;
      }
      if (temp > lambda)
        lambda = temp;
    }

    // Вычисление гаммы
    if (newValue > mu[0]) {
      mu[0] = newValue;
    }
    Xm = p->delta;//pow(p->delta, pTask.GetFreeN());

    if (isSearchXMax)
    {
      Xmax[0] = MinDouble;
      for (TSearcDataIterator it = pData->GetBeginIterator(); it; ++it)
      {
        Xm = it->delta;
        if (Xm > Xmax[0])
        {
          Xmax[0] = Xm;
          intervalXMax = *it;
        }
      }
      isSearchXMax = false;
    }
    else
    {
      if (Xm > Xmax[0])
      {
        Xmax[0] = Xm;
        intervalXMax = p;
      }
    }
    gamma = (mu[0] * p->delta) / Xmax[0];//pow(Xmax, 1. / rootDim);

    //Запоминаем конечное мю
    if (parameters.ltXi > pData->M[index] || pData->M[index] == 1.0 && newValue > _M_ZERO_LEVEL) {
      pData->M[index] = parameters.ltXi;
      pData->SetRecalc(true);

      if (globalM[index] < pData->M[index])
      {
        globalM[index] = pData->M[index];
        isGlobalMUpdate = true;
      }
    }

    temp = (lambda / r) + (((r - 1) * gamma) / r);

    if (temp > pData->M[index]) {
      pData->M[index] = temp;
      pData->SetRecalc(true);

      if (globalM[index] < pData->M[index])
      {
        globalM[index] = pData->M[index];
        isGlobalMUpdate = true;
      }
    }

    //H = fabs(p->zr() - p->zl()) / (p->xr() - p->xl());
    H = newValue / pow(p->delta, rootDim - 1);

    if (H > pData->M[index]) {
      pData->M[index] = H;
      pData->SetRecalc(true);

      if (globalM[index] < pData->M[index])
      {
        globalM[index] = pData->M[index];
        isGlobalMUpdate = true;
      }
    }
    break;
  }
}

// ------------------------------------------------------------------------------------------------
TTrial* TMethod::GetOptimEstimation()
{
  return pData->GetBestTrial();
}

// ------------------------------------------------------------------------------------------------
int TMethod::GetNumberOfTrials()
{
  // Если задача последнего уровня, т.е. сумма размерностей равна общей размерности
  if (pTask.GetN() == pTask.GetFixedN() + pTask.GetFreeN())
  {
    // Число итераций равно числу интервалов в таблице - 1
    return pData->GetCount() - 1;
  }
  // Если задача верхнего уровня - надо просуммировать все данные из интервалов
  int NumberOfTrials = 0;

  for (TSearcDataIterator it = pData->GetBeginIterator(); it; ++it)
  {
    NumberOfTrials += it->K;
  }

  return NumberOfTrials;
}

void TMethod::PrintSection()
{
  if (parameters.IsPrintSectionPoint == true)
  {
    // Если задача верхнего уровня - надо просуммировать все данные из интервалов
    int NumberOfTrials = 0;
    int count = 0;
    for (TSearcDataIterator it = pData->GetBeginIterator(); it; ++it)
    {
      NumberOfTrials += it->K;
      if (it->K > 1)
        print << "Section " << it->xl().toDouble() << "\t iteration = " << it->K << "\n";
      count++;
    }

    double sum = double(NumberOfTrials) / count;
    print << "Section average iteration = " << sum << "\n";
  }

  //PlotDecisionTrees();
}

void TMethod::PrintLevelPoints(const std::string& fileName)
{
  std::ofstream fout;
  fout.open(fileName, std::ios_base::out);
  double* tmpPoint = new double[pTask.GetN()];
  fout << pData->GetCount() - 1 << " " <<
    pTask.GetNumOfFunc() << "\n";
  TSearcDataIterator it = pData->GetBeginIterator();
  int t = 0;
  for (++it; it; ++it)
  {
    //Evolvent.GetImage((*it)->xl(), tmpPoint);
    tmpPoint = (*it)->LeftPoint->y;
    for (int i = 0; i < pTask.GetN(); i++)
      fout << tmpPoint[i] << " ";
    fout << "| ";
    for (int i = 0; i < (*it)->izl() + 1; i++)
      fout << (*it)->z()[i] << " ";
    fout << "| " << (*it)->LeftPoint->TypeColor << " ";
    fout << "\n";
    t++;
  }

  for (int i = 0; i < pTask.GetN(); i++)
    fout << pData->GetBestTrial()->y[i] << " ";
  fout << "| ";
  for (int i = 0; i < pData->GetBestTrial()->index + 1; i++)
    fout << pData->GetBestTrial()->FuncValues[i] << " ";
  if (pTask.GetIsOptimumPointDefined())
  {
    fout << "\n" << pTask.GetOptimumPoint()[0];
    for (int i = 1; i < pTask.GetN(); i++)
      fout << " " << pTask.GetOptimumPoint()[i];
    fout << " | ";
    fout << pTask.GetOptimumValue() << " ";
  }
  fout.close();


}

void TMethod::PrintPoints(const std::string& fileName)
{
  std::ofstream fout;
  fout.open(fileName, std::ios_base::out);
  double* tmpPoint = new double[pTask.GetN()];
  fout << pData->GetCount() - 1 + printPoints.size() << " " <<
    pTask.GetNumOfFunc() << "\n";
  TSearcDataIterator it = pData->GetBeginIterator();
  int t = 0;
  for (++it; it; ++it)
  {
    //Evolvent.GetImage((*it)->xl(), tmpPoint);
    tmpPoint = (*it)->LeftPoint->y;
    for (int i = 0; i < pTask.GetN(); i++)
      fout << tmpPoint[i] << " ";
    fout << "| ";
    for (int i = 0; i < (*it)->izl() + 1; i++)
      fout << (*it)->z()[i] << " ";
    fout << "| " << (*it)->LeftPoint->TypeColor << " ";
    fout << "\n";
    t++;
  }

  for (int k = 0; k < printPoints.size(); k++)
  {
    tmpPoint = printPoints[k]->y;
    for (int i = 0; i < pTask.GetN(); i++)
      fout << tmpPoint[i] << " ";
    fout << "| ";
    for (int i = 0; i < printPoints[k]->index + 1; i++)
      fout << printPoints[k]->FuncValues[i] << " ";
    fout << "| " << printPoints[k]->TypeColor << " ";
    fout << "\n";
  }

  for (int i = 0; i < pTask.GetN(); i++)
    fout << pData->GetBestTrial()->y[i] << " ";
  fout << "| ";
  for (int i = 0; i < pData->GetBestTrial()->index + 1; i++)
    fout << pData->GetBestTrial()->FuncValues[i] << " ";
  if (pTask.GetIsOptimumPointDefined())
  {
    fout << "\n" << pTask.GetOptimumPoint()[0];
    for (int i = 1; i < pTask.GetN(); i++)
      fout << " " << pTask.GetOptimumPoint()[i];
    fout << " | ";
    fout << pTask.GetOptimumValue() << " ";
  }
  fout.close();

  if (pTask.GetProcLevel() == 0)
    printf("M = %lf\n", globalM[0]);

  //if (parameters.IsPlot)
  //  PlotPoint2();
}

#include "omp_calculation.h"

void TMethod::HookeJeevesMethod(TTrial& point, std::vector<TTrial*>& localPoints)
{
  int addAllLocalPoints = 2;
  TLocalMethod* localMethod = nullptr;

  int oldNP = parameters.NumPoints;

  parameters.NumPoints = 2 * parameters.NumPoints;

  switch (parameters.TypeLocalMethod) {
  case ParallelHookeJeeves:
  {
    TCalculation* LocalCalculation = new TOMPCalculation(pTask);

    localMethod = new TParallelHookeJeevesMethod(&pTask, point, *LocalCalculation, addAllLocalPoints);
    break;
  }
  case Parallel_L_BFGS_B:
  {
    TCalculation* LocalCalculation = new TOMPCalculation(pTask);
    localMethod = new TParallelLBFGSBMethod(&pTask, point, *LocalCalculation, addAllLocalPoints);
    break;
  }
  default:
  {
    localMethod = new TLocalMethod(&pTask, point, addAllLocalPoints);
    break;
  }
  }



  double initialStep = 0;
  for (int i = pTask.GetFixedN(); i < pTask.GetN(); i++)
    initialStep += pTask.GetB()[i] - pTask.GetA()[i];
  initialStep /= rootDim;
  // начальный шаг равен среднему размеру стороны гиперкуба, умноженному на коэффициент
  localMethod->SetEps(parameters.localVerificationEpsilon);
  localMethod->SetInitialStep(0.07 * initialStep);
  localMethod->SetMaxTrials(parameters.localVerificationIteration);
  TTrial newpoint2 = localMethod->StartOptimization();
  TTrial* newpoint = TTrialFactory::CreateTrial(&newpoint2);

  parameters.NumPoints = oldNP;

  std::vector<TTrial> points = localMethod->GetSearchSequence();

  int s = points.size();
  for (int i = 0; i < s; i++)
  {
    bool isOutOfRange = false;
    for (int g = 0; g < pTask.GetN(); g++)
    {
      if (points[i].y[g] > pTask.GetB()[g] || points[i].y[g] < pTask.GetA()[g])
        isOutOfRange = true;
    }

    if (isOutOfRange)
      continue;

    TTrial* temp = TTrialFactory::CreateTrial(&points[i]);
    Extended x;
    Evolvent.GetInverseImage(&(temp->y[pTask.GetProcLevel()]), x);
	temp->SetX(x);
    temp->FuncValues[0] = pTask.CalculateFuncs(temp->y, 0);
    temp->TypeColor = 3;
    localPoints.push_back(temp);
  }

  InsertLocalPoints(localPoints);
}

void TMethod::LocalS(TTrial& point, std::vector<TTrial*> leftLocalMinPoint, std::vector<TTrial*> RightLocalMinPoint,
  int countPointLeft, int countPointRight)
{

  //PlotDecisionTrees();

  //if (!pTask.IsLeaf() && (parameters.NumOfTaskLevels > 1))
  //  return;

  numberLocalMethodtStart++;
  //point.TypeColor = 1;
  //for (int u = 0; u < countPointLeft; u++)
  //  leftLocalMinPoint[u]->TypeColor = 1;
  //for (int u = 0; u < countPointRight; u++)
  //  RightLocalMinPoint[u]->TypeColor = 1;

  std::vector<TTrial*> localPoints;
  bool isStop = false;

  bool isLeastSquareMethod = false;

  //if ((parameters.TypeLocalMethod == 0) || (!pTask.IsLeaf() && (parameters.NumOfTaskLevels > 1)))
  if (parameters.TypeLocalMethod == 0)
  {
    HookeJeevesMethod(point, localPoints);
  }
  else if (parameters.TypeLocalMethod == 1)
  {
    bool addAllLocalPoints = true;
    TCalculation* calc = &(this->Calculation);
    TTask* task = &(pTask);

    TApproximation approxim(task, point, addAllLocalPoints, *calc);

    double initialStep = 0;
    for (int i = pTask.GetFixedN(); i < pTask.GetN(); i++)
      initialStep += pTask.GetB()[i] - pTask.GetA()[i];
    initialStep /= rootDim;
    // начальный шаг равен среднему размеру стороны гиперкуба, умноженному на коэффициент
    approxim.SetEps(parameters.Epsilon);
    approxim.SetInitialStep(0.07 * initialStep);
    approxim.SetMaxTrials(parameters.maxCountLocalPoint);

    //1 - заменяем точку; 2 - добавляем
    int type = 2;
    double eps = parameters.Epsilon;
    std::vector<TTrial*> points = approxim.DoApproximation(&point, leftLocalMinPoint, RightLocalMinPoint,
      countPointLeft, countPointRight, type, eps, Evolvent);
    int s = points.size();
    
    isStop = true;

    for (int ii = 0; ii < s; ii++)
    {
      if (points[ii]->GetValue() < point.GetValue())
      {
        isStop = false;
        break;
      }
    }

    if (parameters.TypeLocalMinIntervale == NPoints)
    {
      isStop = false;
    }

    if (isStop)
    {
      for (int i = 0; i < s; i++)
      {
        bool isOutOfRange = false;
        for (int g = 0; g < pTask.GetN(); g++)
        {
          if (points[i]->y[g] > pTask.GetB()[g] || points[i]->y[g] < pTask.GetA()[g])
            isOutOfRange = true;
        }

        if (isOutOfRange)
          continue;

        TTrial* temp = points[i];
        Extended  x;
        Evolvent.GetInverseImage(&(temp->y[pTask.GetFixedN()]), x);
        temp->SetX(x);
        temp->TypeColor = 2;
        localPoints.push_back(temp);
      }
    }
    else
    {
      
      int i = points.size() - 1;
      if (i < 0)
        return;
      bool isOutOfRange = false;
      for (int g = 0; g < pTask.GetN(); g++)
      {
        if (points[i]->y[g] > pTask.GetB()[g] || points[i]->y[g] < pTask.GetA()[g])
          isOutOfRange = true;
      }

      if (!isOutOfRange)
      {

        TTrial* temp = points[i];
        Extended  x;
        Evolvent.GetInverseImage(&(temp->y[pTask.GetFixedN()]), x);
        temp->SetX(x);
        temp->TypeColor = 2;
        localPoints.push_back(temp);
      }
    }


    if (localPoints.size() > 0)
    {
      InsertLocalPoints(localPoints);
      isLeastSquareMethod = true;
    }
    else
      HookeJeevesMethod(point, localPoints);
  }

  int l = localPoints.size();

  if ((l == 0) || (isStop))
    return;

  
  if (isLeastSquareMethod)
    point.TypeColor = 1;
  point.pointStatus = TTrial::local_min;
  for (int u = 0; u < countPointLeft; u++)
  {
    if (isLeastSquareMethod)
      leftLocalMinPoint[u]->TypeColor = 1;
    leftLocalMinPoint[u]->pointStatus = TTrial::local_min;

    TSearchInterval* li = leftLocalMinPoint[u]->leftInterval;
    TSearchInterval* ri = leftLocalMinPoint[u]->rightInterval;

    if (li != 0)
      if (parameters.TypeLocalMinIntervale == NPoints)
        li->status = TSearchInterval::local_area;
    if (ri != 0)
      ri->status = TSearchInterval::local_area;
  }
  for (int u = 0; u < countPointRight; u++)
  {
    if (isLeastSquareMethod)
      RightLocalMinPoint[u]->TypeColor = 1;
    RightLocalMinPoint[u]->pointStatus = TTrial::local_min;

    TSearchInterval* li = RightLocalMinPoint[u]->leftInterval;
    TSearchInterval* ri = RightLocalMinPoint[u]->rightInterval;

    if (li != 0)
      li->status = TSearchInterval::local_area;
    if (ri != 0)
      if (parameters.TypeLocalMinIntervale == NPoints)
        ri->status = TSearchInterval::local_area;
  }

  //pData->SetRecalc(true);

  if (l == 0)
    return;

  //print << "\tlocalPoints = " << l << "\n";

  //Необходимо проверить метки у всех новых точек
  for (int i = 0; i < l; i++)
  {
    localPoints[i]->pointStatus = TTrial::local_min;

    TSearchInterval* li = localPoints[i]->leftInterval;
    TSearchInterval* ri = localPoints[i]->rightInterval;

    if (li != 0)
      li->status = TSearchInterval::educational_local_method;
    if (ri != 0)
      ri->status = TSearchInterval::educational_local_method;
  }

  //localPoints[s - 1]->pointStatus = TTrial::local_min;

  if (localPoints[l - 1]->leftInterval == 0 || localPoints[l - 1]->rightInterval == 0)
    return;

  TTrial* lt = localPoints[l - 1]->leftInterval->LeftPoint;
  TTrial* rt = localPoints[l - 1]->rightInterval->RightPoint;

  TTrial* p1 = 0;
  TTrial* p2 = 0;

  localPoints[l - 1]->lowAndUpPoints = 0;

  p1 = lt;
  //Обходим соседей и меняем метки
  while ((p1->pointStatus == TTrial::low) || (p1->pointStatus == TTrial::low_inflection))
  {
    TSearchInterval* i = p1->leftInterval;

    if ((p1->pointStatus == TTrial::low_inflection))
      p1->pointStatus = TTrial::low;

    if (i == 0)
      break;
    p1 = i->LeftPoint;
  }

  p2 = rt;

  while ((p2->pointStatus == TTrial::up) || (p2->pointStatus == TTrial::low_inflection))
  {
    TSearchInterval* i = p2->rightInterval;

    if ((p2->pointStatus == TTrial::low_inflection))
      p2->pointStatus = TTrial::up;

    if (i == 0)
      break;
    p2 = i->RightPoint;
  }

  //PlotDecisionTrees();

}


// ------------------------------------------------------------------------------------------------
void TMethod::LocalSearch()
{
  if ( ((parameters.localVerificationType == FinalStart && isStop) ||
      (parameters.localVerificationType == UpdatedMinimum))
    && GetOptimEstimation()->index == pTask.GetNumOfFunc() - 1)
  {

    int oldNP = parameters.NumPoints;
    int oldNT = parameters.NumThread;

    if (parameters.localVerificationNumPoint <= 0)
    {
      parameters.localVerificationNumPoint = parameters.NumPoints;
    }

    parameters.NumPoints = parameters.localVerificationNumPoint.GetData();
    parameters.NumThread = parameters.localVerificationNumPoint.GetData();

    numberLocalMethodtStart++;
    std::vector<TTrial*> localPoints;

    TTrial* point = GetOptimEstimation();

    //HookeJeevesMethod(point, localPoints);

    TLocalMethod* localMethod = nullptr;

    int addAllLocalPoints = std::max(0, int(parameters.TypeAddLocalPoint) - 2);
    switch (parameters.TypeLocalMethod) {
    case ParallelHookeJeeves:
    {

      //TCalculation* newCalculation = new TOMPCalculation(pTask);

      //localMethod = new TParallelHookeJeevesMethod(&(pTask), *point, *newCalculation, addAllLocalPoints);
      localMethod = new TParallelHookeJeevesMethod(&(pTask), *point, Calculation, addAllLocalPoints);


      //localMethod = new TParallelHookeJeevesMethod(&(pTask), *point, this->Calculation, addAllLocalPoints);
      break;
    }
    case Parallel_L_BFGS_B:
      localMethod = new TParallelLBFGSBMethod(&(pTask), *point, this->Calculation, addAllLocalPoints);
      break;
    default:
      localMethod = new TLocalMethod(&(pTask), *point, addAllLocalPoints);
      break;
    }

    double initialStep = 0;
    for (int i = pTask.GetFixedN(); i < pTask.GetN(); i++)
      initialStep += pTask.GetB()[i] - pTask.GetA()[i];
    initialStep /= rootDim;
    // начальный шаг равен среднему размеру стороны гиперкуба, умноженному на коэффициент
    localMethod->SetEps(parameters.localVerificationEpsilon);
    //localMethod.SetEps(parameters.Epsilon / 100.0);
    localMethod->SetInitialStep(0.07 * initialStep);// parameters.Epsilon * 10
    //localMethod.SetInitialStep(parameters.Epsilon * 10);// parameters.Epsilon * 10
    //localMethod.SetMaxTrials(1000);
    localMethod->SetMaxTrials(parameters.localVerificationIteration);
    TTrial point2 = localMethod->StartOptimization();
    TTrial* newpoint = TTrialFactory::CreateTrial(&point2);

    std::vector<TTrial> points = localMethod->GetSearchSequence();
    //localMethod.GetTrialsCounter

    int s = points.size();
    for (int i = 0; i < s; i++)
    {
      bool isOutOfRange = false;
      for (int g = 0; g < pTask.GetN(); g++)
      {
        if (points[i].y[g] > pTask.GetB()[g] || points[i].y[g] < pTask.GetA()[g])
          isOutOfRange = true;
      }

      if (isOutOfRange)
        continue;

      TTrial* temp = TTrialFactory::CreateTrial(&points[i]);
      Extended x;
      Evolvent.GetInverseImage(temp->y, x);
      temp->SetX(x);
      temp->FuncValues[0] = pTask.CalculateFuncs(temp->y, 0);
      temp->TypeColor = 3;
      localPoints.push_back(temp);
    }
    parameters.NumPoints = oldNP;
    parameters.NumThread = oldNT;
    InsertLocalPoints(localPoints);


    localPointCount += localMethod->GetTrialsCounter();

    if (parameters.localVerificationType == UpdatedMinimum)
      pData->SetRecalc(true);

  }
}

int TMethod::GetLocalPointCount()
{
  return localPointCount;
}

int TMethod::GetNumberLocalMethodtStart()
{
  return numberLocalMethodtStart;
}

void TMethod::Plot3DDecisionTrees()
{
#ifdef USE_PLOTTER
  #ifdef USE_OpenCV
  //if (parameters.IsPlot)
  {

    TSearchData* data = pData;

    int N = data->GetCount() - 1;

    if (parameters.isCalculationInBorderPoint == 1)
      N += 1;

    pointsForLocalMethod.clear();


    PrepareDataForDecisionTree(N, data);

    CreateTree();

    Mytree->train(X, cv::ml::ROW_SAMPLE, ff); // тренирует дерево на TrainData

    // После того, как натренировали дерево на исходных данных
    // нужно организовать дискретную сетку (отображение наших точек на одномерный массив)
    // провести предсказание на новом одномерном массиве, найти точку, близкую к нашей пришедшей точке
    // и проверить ее соседей: находим значение меньшее -> возвращаем фолс; все значения больше -> возвращаем тру;
    // равное значение -> ее тоже проверяем

    // Равномерное заполнение отрезка (от левой до правой границы)
    int numPointPerDim = pow(2000, 1.0 / parameters.Dimension);

    FillTheSegment(numPointPerDim);

    // На полученной сетке предсказываем значения функции
    cv::Mat results;
    Mytree->predict(uniformPartition, results);

    
    int sizeGride = pow(numPointPerDim, (int)parameters.Dimension);

    std::vector<std::vector<float>> Y(sizeGride);
    std::vector<float> Z(sizeGride);

    for (int index = 0; index < sizeGride; index++)
    {
      Y[index].resize(parameters.Dimension);
      for (int i = 0; i < parameters.Dimension; i++)
      {
        Y[index][i] = uniformPartition.at<float>(index, i);
      }
      Z[index] = results.at<float>(index, 0);
     }

    std::cout << "Y1 = [";
    for (int index = 0; index < sizeGride; index++)
    {
      std::cout << Y[index][0] << ", ";
    }
    std::cout << "]";
    std::cout << std::endl;
    std::cout << std::endl;

    std::cout << "Y2 = [";
    for (int index = 0; index < sizeGride; index++)
    {
      std::cout << Y[index][1] << ", ";
    }
    std::cout << "]";
    std::cout << std::endl;
    std::cout << std::endl;

    std::cout << "Z = [";
    for (int index = 0; index < sizeGride; index++)
    {
      std::cout << Z[index] << ", ";
    }
    std::cout << "]";
    std::cout << std::endl;
    std::cout << std::endl;


    std::cout << std::endl;
    std::cout << std::endl;

    //for (int index = 0; index < sizeGride; index++)
    //{
    //  double yy[2] = { Y[index][0], Y[index][1] };
    //  std::cout << pTask.CalculateFuncs(yy, 0) << ", ";
    //}
    //std::cout << std::endl;
    //std::cout << std::endl;
  }
  #endif
#endif
}


//// ------------------------------------------------------------------------------------------------
//void TMethod::SeparableSearch()
//{
//  TSeparableMethod SeparableMethod(parameters, pTask);
//  TTrial SepTrial = SeparableMethod.StartOptimization();
//
//  UpdateOptimumEstimation(SepTrial);
//
//  std::vector<TTrial> localPoints = SeparableMethod.GetSearchSequence();
//  InsertPoints(localPoints);
//  //После сепарабельного поиска - поднять флаг пересчета
//  recalc = true;
//}
//
//// ------------------------------------------------------------------------------------------------
//void TMethod::RandomSearh()
//{
//  int numRandomPoints = 10 * pTask.GetFreeN();
//  int maxRndLocIters = 1000;
//  TTrial *randomTrials = new TTrial[numRandomPoints];
//
//  TRandomGenerator generator(777);
//
//  double initialStep = 0;// начальный шаг равен среднему размеру стороны гиперкуба,
//  //  умноженному на коэффициент
//  for (int i = pTask.GetFixedN(); i < pTask.GetN(); i++)
//    initialStep += pTask.GetB()[i] - pTask.GetA()[i];
//  initialStep /= pTask.GetFreeN();
//  //только для задач без ограничений и для корня дерева процессов
//  for (int i = 0; i < numRandomPoints; i++)
//  {
//    //generate new trial
//    //copy fixed vars and generate free
//    for (int j = 0; j < pTask.GetFixedN(); j++)
//      randomTrials[i].y[j] = pTask.GetFixedY()[j];
//
//    generator.GetRandomVector(pTask.GetFreeN(), pTask.GetA() + pTask.GetFixedN(),
//      pTask.GetB() + pTask.GetFixedN(), randomTrials[i].y + pTask.GetFixedN());
//
//    randomTrials[i].index = pTask.GetNumOfFunc() - 1;
//    randomTrials[i].FuncValues[randomTrials[i].index] = HUGE_VAL;
//    //start local method from generated points
//    TLocalMethod localMethod(parameters, pTask, randomTrials[i], true);
//    localMethod.SetEps(0.0001);
//    localMethod.SetInitialStep(0.1*initialStep);
//    localMethod.SetMaxTrials(maxRndLocIters);
//    randomTrials[i] = localMethod.StartOptimization();
//
//    UpdateOptimumEstimation(randomTrials[i]);
//
//    std::vector<TTrial> localPoints = localMethod.GetSearchSequence();
//    InsertPoints(localPoints);
//  }
//  recalc = true;
//  delete[] randomTrials;
//}


// - end of file ----------------------------------------------------------------------------------

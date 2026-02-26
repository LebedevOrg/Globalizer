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

#include "Method.h"
#include "LocalMethod.h"
#include "Exception.h"
#include "Common.h"
#include "OutputSystem.h"
#include "SearcDataIterator.h"
#include "TaskFactory.h"
#include "TrialFactory.h"
#include "CalculationFactory.h"
#include "ParallelHookeJeevesMethod.h"

#include "OMPCalculation.h"


#include <cmath>
#include <vector>
#include <fstream>
#include <algorithm>


#include <string>
#include <cmath>



// ------------------------------------------------------------------------------------------------
Method::Method(Task& _pTask, SearchData& _pData,
  Calculation& _Calculation, Evolvent& _Evolvent) :
  pTask(_pTask), pData(&_pData),
  calculation(_Calculation), evolvent(_Evolvent)
{
  isFoundOptimalPoint = false;

  MaxNumOfTrials = parameters.MaxNumOfPoints[0];
  if (MaxNumOfTrials < 1)
  {
    throw EXCEPTION("MaxNumOfTrials is out of range");
  }

  if (parameters.Epsilon <= 0.0)
  {
    throw EXCEPTION("Epsilon is out of range");
  }

  if (parameters.r <= 1.0)
  {
    throw EXCEPTION("r is out of range");
  }

  if ((parameters.rEps < 0.0) || (parameters.rEps > 0.5))
  {
    throw EXCEPTION("Epsilon reserv parameter is out of range");
  }

  alfa = parameters.localAlpha; // пока локальная адаптация - фиксированная



  if (parameters.NumPoints <= 0)
  {
    throw EXCEPTION("NumPoints parameter <= 0");
  }


  iteration.IterationCount = 0;

  AchievedAccuracy = MaxDouble;
  // Массив для текущих итераций
  iteration.pCurTrials.resize(parameters.NumPoints);

  //===========================================================================================================================================
  mu = new double[pTask.GetNumOfFunc()];
  for (int i = 0; i < pTask.GetNumOfFunc(); i++)
    mu[i] = 0;
  Xmax = new double[pTask.GetNumOfFunc()];
  for (int i = 0; i < pTask.GetNumOfFunc(); i++)
    Xmax[i] = 0;
  //===========================================================================================================================================


  if (parameters.Dimension == 1)
    StartLocalIteration = 5;
  else
    StartLocalIteration = parameters.Dimension * 70 / parameters.NumPoints;

  functionCalculationCount.resize(pTask.GetNumOfFunc());
  for (int i = 0; i < pTask.GetNumOfFunc(); i++)
    functionCalculationCount[i] = 0;

  isFindInterval = false;

  inputSet.trials.resize(parameters.NumPoints);

  isGlobalMUpdate = false;

  isSetInLocalMinimumInterval = false;

  isStop = false;

  isSearchXMax = true;

  calculation.SetSearchData(&_pData);

  isLocalZUpdate = false;

  /// количество точек вычисленных локальным методом
  localPointCount = 0;
  /// число запусков локально метода
  numberLocalMethodtStart = 0;
}

// ------------------------------------------------------------------------------------------------
Method::~Method()
{
}



SearchData* Method::GetSearchData(Trial* trial)
{
  return pData;
}

// ------------------------------------------------------------------------------------------------
bool Method::IsIntervalInSegment(SearchInterval* basicInterval, SearchInterval* newInterval)
{
  if (basicInterval == NULL || newInterval == NULL)
  {
    return false;
  }
  if (basicInterval->LeftPoint == NULL || newInterval->LeftPoint == NULL)
  {
    return false;
  }
  double start = basicInterval->LeftPoint->GetFloor();

  if (basicInterval->LeftPoint->GetFloor() != newInterval->LeftPoint->GetFloor())
    return false;
  double end = start + 1;
  if ((newInterval->xl() >= start) && (newInterval->xl() <= end) &&
    (newInterval->xr() >= start) && (newInterval->xr() <= end))
    return true;
  return false;
}


double Method::Update_r(int iter, int procLevel)
{
  double baseR = parameters.r;
  double iterationCount = 0;
  if (iter <= 1)
    iterationCount = (double)(iteration.IterationCount);
  else
    iterationCount = iter;

  if (iterationCount <= 0)
    iterationCount = 1;

  double p = 1.0 / parameters.Dimension;
  double resR = baseR + parameters.rDynamic / pow(iterationCount, p);

  return resR;
}

// ------------------------------------------------------------------------------------------------
void Method::CalculateImage(Trial& pCurTrialsj)
{
  evolvent.GetImage(pCurTrialsj.X(), pCurTrialsj.y);
}

// ------------------------------------------------------------------------------------------------
/// Вычисляет координаты на отрезке 0..1 для всех разверток по образу проведенного испытания
void Method::CalculateCurrentPoint(Trial& pCurTrialsj, SearchInterval* BestIntervalsj)
{
  // Вычисляем x
  if (BestIntervalsj->izl() != BestIntervalsj->izr())
  {
    pCurTrialsj.SetX(0.5 * (BestIntervalsj->xl() + BestIntervalsj->xr()));
  }
  else
  {
    pCurTrialsj.SetX(0.5 * (BestIntervalsj->xl() + BestIntervalsj->xr()) -
      (((BestIntervalsj->zr() - BestIntervalsj->zl()) > 0) ? 1 : -1) *
      pow(fabs(BestIntervalsj->zr() - BestIntervalsj->zl()) /
        pData->M[BestIntervalsj->izl()], parameters.Dimension) / 2 / parameters.r);
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

}

// ------------------------------------------------------------------------------------------------
void Method::FirstIteration()
{
  // Задаем границы интервалов изменения параметров
  // Указатель на границы интервалов в подзадаче - это указатель на исходные границы,
  //   смещенный на число фиксированных размерностей
  evolvent.SetBounds(pTask.GetA(), pTask.GetB());

  // Это первая итерация, сбрасываем счетчик
  iteration.IterationCount = 1;
  // И сбрасываем достигнутую точность
  AchievedAccuracy = 1.0;
  // И лучшую итерацию

  SearchInterval** NewInterval = new SearchInterval * [1];
  //SearchIntervalFactory::CreateSearchInterval();
  for (int e = 0; e < 1; e++)
  {
    NewInterval[e] = SearchIntervalFactory::CreateSearchInterval();
    NewInterval[e]->ind = iteration.IterationCount;
    NewInterval[e]->K = 0;
    NewInterval[e]->CreatePoint();
    // Гельдеровская длина
    NewInterval[e]->delta = 1.0;
    //Добавляем интервал
    SearchInterval* p = pData->InsertInterval(*(NewInterval[e]));
    delete NewInterval[e];
    NewInterval[e] = p;
    pData->GetTrials().push_back(p->LeftPoint);
    pData->GetTrials().push_back(p->RightPoint);
    ///Необходимо сосчитать значения на границах
    CalculateImage(*p->LeftPoint);


    CalculateImage(*p->RightPoint);

    if (pData->GetBestTrial() == 0)
      pData->SetBestTrial(p->LeftPoint);

    //====================================================================
    if ((parameters.isCalculationInBorderPoint == true) || (parameters.LocalTuningType != 0))
    {
      //if (parameters.Dimension == 1)
      {
        // Эта функция вызывается только в листе дерева - поэтому вычисляем функционалы здесь
        for (int j = 0; j < pTask.GetNumOfFunc(); j++)
        {
          p->LeftPoint->FuncValues[j] = MaxDouble;
          p->RightPoint->FuncValues[j] = MaxDouble;
        }
        p->LeftPoint->K = 1;
        //p->RightPoint->K = 1;

        InformationForCalculation inputlocal;
        TResultForCalculation outputlocal;
        inputlocal.Resize(2);
        outputlocal.Resize(2);

        inputlocal.trials[0] = p->LeftPoint;

        inputlocal.trials[1] = p->RightPoint;

        Calculation* Calculation_ = CalculationFactory::CreateCalculation(pTask, &evolvent);

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
  //====================================================================

  // На первой итерации - единственный лучший интервал

  // Флаг пересчета - поднят
  pData->SetRecalc(true);

  // Точки первой итерации выбираются по особому правилу
  // Равномерно ставим NumPoints точек c шагом h
  // А надо бы случайно...
  double h = 1.0 / (parameters.NumPoints + 1);
  if (parameters.startPoint.GetIsChange() && parameters.IsUseStartPoint) //берем начальную точку из параметров
  {

    int firstPointCount = parameters.NumPoints - 1;
    

    std::vector<Trial*> newPoint(parameters.NumPoints);
    newPoint[0] = TrialFactory::CreateTrial();
    
    pTask.CopyPoint(parameters.startPoint.GetData(), newPoint[0]);

    InformationForCalculation inputlocal;
    TResultForCalculation outputlocal;
    int sfipi = 0;

    if (parameters.startPointValues.GetIsChange())
    {
      for (int ifv = 0; ifv < parameters.startPointValues.GetSize(); ifv++)
      {
        newPoint[0]->FuncValues[ifv] = parameters.startPointValues[ifv];
        if ((ifv == (pTask.GetNumOfFunc() - 1)) || (newPoint[0]->FuncValues[ifv] > 0))
        {
          newPoint[0]->index = ifv;
        }
      }
      inputlocal.Resize(firstPointCount);
      outputlocal.Resize(firstPointCount);
      sfipi = 0;
    }
    else
    {
      inputlocal.Resize(parameters.NumPoints);
      outputlocal.Resize(parameters.NumPoints);
      inputlocal.trials[0] = newPoint[0];
      sfipi = 1;
    }

    h = 1.0 / (firstPointCount + 1);
    for (int ifip = 0; ifip < firstPointCount; ifip++)
    {      
      int ind = ifip + 1;
      
      newPoint[ind] = TrialFactory::CreateTrial();

      pData->GetTrials().push_back(newPoint[ind]);
      newPoint[ind]->SetX((ifip + 1)* h);

      // Вычисляем образ точки итерации - образ записывается в начальные позиции массива y
      CalculateImage(*newPoint[ind]);

      newPoint[ind]->leftInterval = NewInterval[0];
      newPoint[ind]->rightInterval = NewInterval[0];
      inputlocal.trials[sfipi + ifip] = newPoint[ind];
    }

    calculation.Calculate(inputlocal, outputlocal);

    for (int j = 0; j < pTask.GetNumOfFunc(); j++)
    {
      functionCalculationCount[j] = outputlocal.countCalcTrials[j];
    }
    

    for (int ifip = 0; ifip < newPoint.size(); ifip++)
    {
      UpdateOptimumEstimation(*(newPoint[ifip]));
      newPoint[ifip]->K = 1;

      Extended genX(0.0);
      evolvent.GetInverseImage(newPoint[ifip]->y, genX);

      newPoint[ifip]->SetX(genX);

      //pData->GetTrials().push_back(newPoint[ifip]);
    }

      this->InsertPoints(newPoint);
    
    this->iteration.IterationCount += 1;
    parameters.iterationNumber = iteration.IterationCount;
  }
  else if (!parameters.isLoadFirstPointFromFile) // равномерно распределяем начальные точки
  {
    for (int q = 0; q < parameters.NumPoints; q++)
    {

      if (parameters.TypeDistributionStartingPoints == Evenly)
      {
        int ind = q;
        iteration.pCurTrials[ind] = TrialFactory::CreateTrial();

        pData->GetTrials().push_back(iteration.pCurTrials[ind]);
        iteration.pCurTrials[ind]->SetX((q + 1) * h);

        // Вычисляем образ точки итерации - образ записывается в начальные позиции массива y
        CalculateImage(*iteration.pCurTrials[ind]);

        iteration.pCurTrials[ind]->leftInterval = NewInterval[0];
        iteration.pCurTrials[ind]->rightInterval = NewInterval[0];
      }
      else
      {
        int ind = q;
        iteration.pCurTrials[ind] = TrialFactory::CreateTrial();
        pData->GetTrials().push_back(iteration.pCurTrials[ind]);

        for (size_t iCNP = 0; iCNP < parameters.Dimension; iCNP++)
        {
          iteration.pCurTrials[ind]->y[iCNP] = pTask.GetA()[iCNP] + ((double(q) + 1.0) * h) * (pTask.GetB()[iCNP] - pTask.GetA()[iCNP]);
        }

        Extended genX(0.0);
        evolvent.GetInverseImage(iteration.pCurTrials[ind]->y, genX);
        iteration.pCurTrials[ind]->SetX(genX);

        iteration.pCurTrials[ind]->leftInterval = NewInterval[0];
        iteration.pCurTrials[ind]->rightInterval = NewInterval[0];
      }
    }

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
      std::vector<Trial*> newPoint(numberLoadedPoints);

      for (int i = 0; i < numberLoadedPoints; i++)
      {
        newPoint[i] = TrialFactory::CreateTrial();
        for (int iDim = 0; iDim < parameters.Dimension; iDim++)
        {
          newPoint[i]->y[iDim] = points[i][iDim];
        }

        newPoint[i]->FuncValues[0] = pointVal[i];

        newPoint[i]->K = 1;
        newPoint[i]->index = 0;

        Extended genX(0.0);
        evolvent.GetInverseImage(newPoint[i]->y, genX);

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
void Method::Recalc()
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
    for (SearcDataIterator it = pData->GetBeginIterator(); it; ++it)
    {
      it->R = CalculateGlobalR(*it);
      //it->locR = CalculateLocalR(*it);

      pData->PushToQueue(*it);
    }
    // После пересчета флаг опускаем
    pData->SetRecalc(false);
  }
}

// ------------------------------------------------------------------------------------------------
void Method::CalculateIterationPoints()
{
  if (iteration.IterationCount == 1)
  {
    return;
  }

  // Если поднят флаг - то пересчитать все характеристики
  Recalc();

  // Здесь надо взять NumPoints лучших характеристик из очереди
  // Очередь пока одна - очередь глобальных характеристик
  // В ней должно быть нужное количество интервалов, т.к. на первом шаге проводится NumPoints
  // испытаний
  std::vector<SearchInterval*> BestIntervals(parameters.NumPoints);

  int localMix = parameters.localMix;

  if (GetIterationType(iteration.IterationCount, localMix) == Global)
  {

    pData->GetBestIntervals(BestIntervals.data(), parameters.NumPoints);

  }
  else
    pData->GetBestLocalIntervals(BestIntervals.data(), parameters.NumPoints);
  // Пока заполняем одновременно вектор CurTrials, и вектор интервалов

  CalculateCurrentPoints(BestIntervals);
}

// ------------------------------------------------------------------------------------------------
void Method::CalculateCurrentPoints(std::vector<SearchInterval*>& BestIntervals)
{
  for (unsigned int i = 0; i < BestIntervals.size(); i++)
  {
    iteration.pCurTrials[i] = TrialFactory::CreateTrial();
    pData->GetTrials().push_back(iteration.pCurTrials[i]);
    CalculateCurrentPoint(*iteration.pCurTrials[i], BestIntervals[i]);
  }
}

// ------------------------------------------------------------------------------------------------
bool Method::CheckStopCondition()
{
  bool res = false;
  int numOfOptima = 1;
  double allOptimumPoints[MAX_TRIAL_DIMENSION * MAX_NUM_MIN];

  double CurrentAccuracy = 0.1;
  if (CurrentAccuracy < AchievedAccuracy)
    AchievedAccuracy = CurrentAccuracy;

  if (pTask.IsLeaf()) //если метод не в корне, то остановка только по точности
  {
    //if (AchievedAccuracy < parameters.Epsilon)
    //  res = true;
    double fm = parameters.Epsilon;
    if ((isSetInLocalMinimumInterval == true) || (AchievedAccuracy < fm))
      res = true;
  }
  else
  {
    switch (parameters.stopCondition)
    {
    case Accuracy:
      if (AchievedAccuracy < parameters.Epsilon)
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
        if (fabs(pData->GetBestTrial()->y[i] - pTask.GetOptimumPoint()[i]) > parameters.Epsilon)
        {
          res = false;
          break;
        }
      }
    }
    break;
    case OptimumValue:
      if (pData->GetBestTrial()->index == pTask.GetNumOfFunc() - 1 &&
        pData->GetBestTrial()->FuncValues[pData->GetBestTrial()->index] - pTask.GetOptimumValue() < parameters.Epsilon)
        res = true;
      break;

    case InLocalArea:
    {
      double fm = parameters.Epsilon;
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


// ------------------------------------------------------------------------------------------------
void Method::CalculateFunctionals()
{
  std::vector<SearchInterval*> intervalArr(iteration.pCurTrials.size());

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

    intervalArr[i] = iteration.pCurTrials[i]->leftInterval;
  }


  calculation.Calculate(inputSet, outputSet);

  for (unsigned int i = 0; i < iteration.pCurTrials.size(); i++)
  {
    if (outputSet.trials[i] == 0)
      iteration.pCurTrials[i] = 0;
  }

  for (int j = 0; j < pTask.GetNumOfFunc(); j++)
  {
    functionCalculationCount[j] += outputSet.countCalcTrials[j];
  }

}


// ------------------------------------------------------------------------------------------------
std::vector<int> Method::GetFunctionCalculationCount()
{
  return functionCalculationCount;
}

double Method::GetAchievedAccuracy()
{
  return AchievedAccuracy;
}

// ------------------------------------------------------------------------------------------------
void Method::InsertLocalPoints(const std::vector<Trial*>& points, Task* task)
{
  for (size_t j = 0; j < points.size(); j++)
  {
    Trial* currentPoint = points[j];
    Extended  x;
    evolvent.GetInverseImage(&(currentPoint->y[0]), x);
    currentPoint->SetX(x);
    SearchInterval* CoveringInterval = pData->FindCoveringInterval(currentPoint);

    if (!CoveringInterval)
      throw EXCEPTION("Covering interval does not exists");
    if (!(currentPoint->X() < CoveringInterval->xr() || currentPoint->X() > CoveringInterval->xl()))
      throw EXCEPTION("Wrong covering interval");

    if (points[j]->K <= 0)
      points[j]->K = 1;

    SearchInterval* p = pData->InsertPoint(CoveringInterval, *currentPoint,
      iteration.IterationCount, parameters.Dimension);

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
      //p->locR = CalculateLocalR(p);
      pData->PushToQueue(p);

      CoveringInterval->R = CalculateGlobalR(CoveringInterval);
      //CoveringInterval->locR = CalculateLocalR(CoveringInterval);

      pData->PushToQueue(CoveringInterval);
    }


  }
}


// ------------------------------------------------------------------------------------------------
void Method::InsertPoints(const std::vector<Trial*>& points)
{
  for (size_t j = 0; j < points.size(); j++)
  {
    Trial* currentPoint = points[j];
    Extended  x;

    for (int j = 0; j <= currentPoint->index; j++)
    {
      functionCalculationCount[j] += 1;
    }

    evolvent.GetInverseImage(currentPoint->y, x);
    currentPoint->SetX(x);
    SearchInterval* CoveringInterval = pData->FindCoveringInterval(currentPoint);

    if (AchievedAccuracy > CoveringInterval->delta)
      AchievedAccuracy = CoveringInterval->delta;

    if (!CoveringInterval)
      throw EXCEPTION("Covering interval does not exists");
    if (!(currentPoint->X() < CoveringInterval->xr() || currentPoint->X() > CoveringInterval->xl()))
      throw EXCEPTION("Wrong covering interval");

    SearchInterval* p = pData->InsertPoint(CoveringInterval, *currentPoint,
      iteration.IterationCount, parameters.Dimension);

    UpdateOptimumEstimation(*currentPoint);

    if (p)
    {
      CalculateM(p);
      CalculateM(CoveringInterval);
    }
    this->iteration.IterationCount++;
  }
}

// ------------------------------------------------------------------------------------------------
bool Method::UpdateOptimumEstimation(Trial& trial)
{
  if (trial.index > pData->GetBestTrial()->index || trial.index == pData->GetBestTrial()->index &&
    trial.FuncValues[pData->GetBestTrial()->index] < pData->GetBestTrial()->FuncValues[pData->GetBestTrial()->index])
  {
    pData->SetBestTrial(trial.Clone());

    // Оптимум обновился - нужен пересчет
    pData->SetRecalc(true);
    isLocalZUpdate = true;
    return true;
  }
  return false;
}

void Method::SavePoints()
{
  if (static_cast<std::string>(parameters.iterPointsSavePath).size() > 0)
  {
    if (parameters.iterPointsSavePath.ToString() != "")
    {
      SearcDataIterator it = pData->GetBeginIterator();

      for (++it; it; ++it)
      {
        printPoints.push_back((*it)->LeftPoint->Clone());
      }
    }
  }

}


// ------------------------------------------------------------------------------------------------
double Method::CalculateGlobalR(SearchInterval* p)
{
  double deltax = p->delta;
  double value = 0;
  int v = 0;
  if ((p->izl() == -2) && (p->izr() == -2))
  {
    if (parameters.GetProcRank() == 3)
    {
      print << "Bad Interval\tProcRank=" << parameters.GetProcRank() << "\n";

      value = 0;
      print << "iteration.IterationCount = " << iteration.IterationCount << "\n";
      
      print << "NewInterval! xl() = " << p->xl().toDouble() << " xr() = " 
        << p->xr().toDouble() << " izl() = " << p->izl() << " izr() = " 
        << p->izr() << " zl() = " << p->zl() << " zr() = " << p->zr() 
        << " R = " << p->R << "\n";
      
      printf("Z[%d] = %lf M = %lf alfa = %lf\n", v, pData->Z[v], pData->Z[v], alfa);
      printf("val = %lf\n", value);

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
      deltax + (p->zr() - p->zl()) * (p->zr() - p->zl()) / (deltax * pData->M[v] * pData->M[v] * parameters.r * parameters.r) -
      2 * (p->zr() + p->zl() - 2 * pData->Z[v]) / (parameters.r * pData->M[v]);
  }
  else if (p->izr() > p->izl())
  {
    v = p->izr();
    value = 2 * deltax - 4 * (p->zr() - pData->Z[v]) / (parameters.r * pData->M[v]);
  }
  else //if (p->izr() < p->izl)
  {
    v = p->izl();
    value = 2 * deltax - 4 * (p->zl() - pData->Z[v]) / (parameters.r * pData->M[v]);
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
double Method::CalculateLocalR(SearchInterval* p)
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
void Method::CalculateM(SearchInterval* p)
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
      SearcDataIterator i = pData->GetIterator(p);
      ++i;
      while (i != NULL && p->izl() > i->izl() && IsIntervalInSegment(p, (*i)))
        ++i;
      //Если обнаружили точку с большим или равным индексом, то вычисляем оценку константы
      if (i != NULL && p->izl() <= i->izl() && IsIntervalInSegment(p, (*i)))
        UpdateM(fabs(i->z()[p->izl()] - p->zl()) / root(i->xl() - p->xl(), parameters.Dimension), p->izl(), boundaryStatus, p);
      //UpdateM(fabs(i->z()[p->izl()] - p->zl()) / root(i->xl() - p->xl(), parameters.Dimension), p->izl(), boundaryStatus, p);
    // Просмотр влево до обнаружения точки с большим или равным индексом,
    // или до левой границы интервала поиска
      i = pData->GetIterator(p);
      --i;
      while (i != NULL && p->izl() > i->izl() && IsIntervalInSegment(p, (*i)))
        --i;
      //Если обнаружили точку с большим или равным индексом, то вычисляем оценку константы
      if (i != NULL && p->izl() <= i->izl() && IsIntervalInSegment(p, (*i)))
        UpdateM(fabs(i->z()[p->izl()] - p->zl()) / root(p->xl() - i->xl(), parameters.Dimension), p->izl(), boundaryStatus, p);
      //UpdateM(fabs(i->z()[p->izl()] - p->zl()) / root(p->xl() - i->xl(), parameters.Dimension), p->izl(), boundaryStatus, p);
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
SearchInterval* Method::AddCurrentPoint(Trial& pCurTrialsj, SearchInterval* BestIntervalsj)
{
  // правый подинтервал
  SearchInterval* NewInterval = SearchIntervalFactory::CreateSearchInterval();
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



  // Правая точка интервала - это правая точка интервала, в котором проведено испытание
  NewInterval->RightPoint = BestIntervalsj->RightPoint;

  //Правая точка должна быть больше левой, если - меньше, ошибка!!!
  if (NewInterval->xr() <= NewInterval->xl())
  {
    throw EXCEPTION("Interval with negative length!");
  }

  // Гельдеровская длина интервала
  NewInterval->delta = root(NewInterval->xr() - NewInterval->xl(), parameters.Dimension);

  // Корректируем существующий интервал
  (BestIntervalsj)->RightPoint = NewInterval->LeftPoint;


  // Обновляем достигнутую точность - по старой гельдеровской длине лучшего интервала
  if ((BestIntervalsj)->delta < AchievedAccuracy)
  {
    AchievedAccuracy = (BestIntervalsj)->delta;
  }
  // После чего вычисляем новую гельдеровскую длину лучшего интервала
  (BestIntervalsj)->delta = root((BestIntervalsj)->xr() - (BestIntervalsj)->xl(), parameters.Dimension);
  //(BestIntervalsj)->delta = root((BestIntervalsj)->xr() - (BestIntervalsj)->xl(), parameters.Dimension);
  //    (*BestIntervalsj)->delta = pow((*BestIntervalsj)->dx,1.0/parameters.Dimension);

  int j = BestIntervalsj->izr();
  if (BestIntervalsj->izl() > j)
    j = BestIntervalsj->izl();

  if (Xmax[j] < (BestIntervalsj)->delta)
  {
    Xmax[j] = (BestIntervalsj)->delta;
  }

  j = NewInterval->izr();
  if (NewInterval->izl() > j)
    j = NewInterval->izl();

  if (Xmax[j] < (NewInterval)->delta)
  {
    Xmax[j] = (NewInterval)->delta;
  }

  // Интервал сформирован - можно добавлять
  // Вставка завершается корректно, или же выбрасывает исключение
  SearchInterval* p = pData->InsertInterval(*NewInterval);

  delete NewInterval;

  pCurTrialsj.leftInterval = (BestIntervalsj);
  pCurTrialsj.rightInterval = p;

  pCurTrialsj.leftInterval->LeftPoint->rightInterval = (BestIntervalsj);
  pCurTrialsj.rightInterval->RightPoint->leftInterval = p;

  return p;

}


// ------------------------------------------------------------------------------------------------
void Method::RenewSearchData()
{
  for (unsigned int j = 0; j < iteration.pCurTrials.size(); j++)
  {
    if (iteration.pCurTrials[j] == 0)
      continue;

    SearchInterval* p = 0;
    SearchInterval* interval = iteration.pCurTrials[j]->leftInterval;
    p = AddCurrentPoint(*iteration.pCurTrials[j], interval);

    if (p == 0)
      continue;

    if (interval == 0)
      interval = iteration.pCurTrials[j]->leftInterval;

    //Обработка началной итерации
    if (iteration.IterationCount == 1)
    {
      //bool f = j < parameters.NumPoints - 1;
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
      //p->locR = CalculateLocalR(p);
      pData->PushToQueue(p);

      (interval)->R = CalculateGlobalR((interval));
      //(interval)->locR = CalculateLocalR((interval));

      pData->PushToQueue((interval));
    }
  }
  isFindInterval = false;
}

// ------------------------------------------------------------------------------------------------
bool Method::EstimateOptimum()
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
void Method::SetNumPoints(int newNP)
{
  if (newNP <= 0)
    newNP = 1;

  if (iteration.pCurTrials.size() != newNP)
  {
    for (unsigned int i = 0; i < iteration.pCurTrials.size(); i++)
      iteration.pCurTrials[i] = 0;

    iteration.pCurTrials.resize(newNP);
  }

  inputSet.Resize(newNP);
  outputSet.Resize(newNP);
}

// ------------------------------------------------------------------------------------------------
void Method::FinalizeIteration()
{
  iteration.IterationCount++;
  parameters.iterationNumber = iteration.IterationCount;

  for (unsigned int i = 0; i < iteration.pCurTrials.size(); i++)
    iteration.pCurTrials[i] = 0;

  if (parameters.TypeCalculation == AsyncMPI)
  {
    SetNumPoints(1);
    parameters.NumPoints = 1;
  }

  if (isLocalZUpdate)//Если нужен пересчет - обновился минимум
  {
    LocalSearch();
  }
  isLocalZUpdate = false;
}

int Method::GetIterationCount()
{
  return iteration.IterationCount;
}



IterationType Method::GetIterationType(int iterationNumber, int localMixParameter)
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

int Method::IsBoundary(SearchInterval* p) {
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
void Method::UpdateM(double newValue, int index, int boundaryStatus, SearchInterval* p)
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
      SearcDataIterator i = pData->GetIterator(p);
      ++i;
      while (i != NULL && p->izl() > i->izl() && IsIntervalInSegment(p, (*i)))
        ++i;
      if (i != NULL && p->izl() == i->izl() && IsIntervalInSegment(p, (*i)) && p->izl() == j)
      {
        temp = fabs(i->z()[p->izl()] - p->zl()) / root(i->xl() - p->xl(), parameters.Dimension);
        if (temp > mu[j])
          mu[j] = temp;
      }

      i = pData->GetIterator(p);
      --i;
      while (i != NULL && p->izl() > i->izl() && IsIntervalInSegment(p, (*i)))
        --i;
      if (i != NULL && p->izl() == i->izl() && IsIntervalInSegment(p, (*i)) && p->izl() == j)
      {
        temp = fabs(i->z()[p->izl()] - p->zl()) / root(p->xl() - i->xl(), parameters.Dimension);
        if (temp > mu[j])
          mu[j] = temp;
      }
    }

    Xm = p->delta;

    if (isSearchXMax)
    {
      Xmax[j] = 0;
      for (SearcDataIterator it = pData->GetBeginIterator(); it; ++it)
      {
        Xm = it->delta;
        max = it->izl();
        if (it->izr() > max)
          max = it->izr();

        if ((Xm > Xmax[max]) && (max == j))
        {
          Xmax[max] = Xm;
        }
      }
      isSearchXMax = false;
    }
    else
    {
      if (Xm > Xmax[j])
      {
        Xmax[j] = Xm;
      }
    }
    gamma = (mu[j] * p->delta) / Xmax[j];

    //Запоминаем конечное мю
    if (parameters.ltXi > pData->M[index] || pData->M[index] == 1.0 && newValue > _M_ZERO_LEVEL)
    {
      pData->M[index] = parameters.ltXi;
      pData->SetRecalc(true);


    }
    if (lambda > pData->M[index])
    {
      pData->M[index] = lambda;
      pData->SetRecalc(true);
    }
    if (gamma > pData->M[index])
    {
      pData->M[index] = gamma;
      pData->SetRecalc(true);

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
    Xm = p->delta;//pow(p->delta, parameters.Dimension);

    if (isSearchXMax)
    {
      Xmax[0] = MinDouble;
      for (SearcDataIterator it = pData->GetBeginIterator(); it; ++it)
      {
        Xm = it->delta;
        if (Xm > Xmax[0])
        {
          Xmax[0] = Xm;
        }
      }
      isSearchXMax = false;
    }
    else
    {
      if (Xm > Xmax[0])
      {
        Xmax[0] = Xm;
      }
    }
    gamma = (mu[0] * p->delta) / Xmax[0];//pow(Xmax, 1. / parameters.Dimension);
    //gamma = mu;

    //Запоминаем конечное мю
    if (parameters.ltXi > pData->M[index] || pData->M[index] == 1.0 && newValue > _M_ZERO_LEVEL)
    {
      pData->M[index] = parameters.ltXi;
      pData->SetRecalc(true);

    }

    temp = (lambda / parameters.r) + (((parameters.r - 1) * gamma) / parameters.r);
    //temp = mu;

    if (temp > pData->M[index])
    {
      pData->M[index] = temp;
      pData->SetRecalc(true);

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
    Xm = p->delta;//pow(p->delta, parameters.Dimension);

    if (isSearchXMax)
    {
      Xmax[0] = MinDouble;
      for (SearcDataIterator it = pData->GetBeginIterator(); it; ++it)
      {
        Xm = it->delta;
        if (Xm > Xmax[0])
        {
          Xmax[0] = Xm;
        }
      }
      isSearchXMax = false;
    }
    else
    {
      if (Xm > Xmax[0])
      {
        Xmax[0] = Xm;
      }
    }
    gamma = (mu[0] * p->delta) / Xmax[0];//pow(Xmax, 1. / parameters.Dimension);

    //Запоминаем конечное мю
    if (parameters.ltXi > pData->M[index] || pData->M[index] == 1.0 && newValue > _M_ZERO_LEVEL) {
      pData->M[index] = parameters.ltXi;
      pData->SetRecalc(true);
    }

    temp = (lambda / parameters.r) + (((parameters.r - 1) * gamma) / parameters.r);

    if (temp > pData->M[index]) {
      pData->M[index] = temp;
      pData->SetRecalc(true);
    }

    //H = fabs(p->zr() - p->zl()) / (p->xr() - p->xl());
    H = newValue / pow(p->delta, parameters.Dimension - 1);

    if (H > pData->M[index]) {
      pData->M[index] = H;
      pData->SetRecalc(true);
    }
    break;
  }
}

// ------------------------------------------------------------------------------------------------
Trial* Method::GetOptimEstimation()
{
  return pData->GetBestTrial();
}

// ------------------------------------------------------------------------------------------------
int Method::GetNumberOfTrials()
{

  // Число итераций равно числу интервалов в таблице - 1
  return pData->GetCount() - 1;

}

void Method::PrintSection()
{
  if (parameters.IsPrintSectionPoint == true)
  {
    // Если задача верхнего уровня - надо просуммировать все данные из интервалов
    int NumberOfTrials = 0;
    int count = 0;
    for (SearcDataIterator it = pData->GetBeginIterator(); it; ++it)
    {
      NumberOfTrials += it->K;
      if (it->K > 1)
        print << "Section " << it->xl().toDouble() << "\t iteration = " << it->K << "\n";
      count++;
    }

    double sum = double(NumberOfTrials) / count;
    print << "Section average iteration = " << sum << "\n";
  }
}

void Method::PrintLevelPoints(const std::string& fileName)
{
  std::ofstream fout;
  fout.open(fileName, std::ios_base::out);
  double* tmpPoint = new double[pTask.GetN()];
  fout << pData->GetCount() - 1 << " " <<
    pTask.GetNumOfFunc() << "\n";
  SearcDataIterator it = pData->GetBeginIterator();
  int t = 0;
  for (++it; it; ++it)
  {
    //evolvent.GetImage((*it)->xl(), tmpPoint);
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

void Method::PrintPoints(const std::string& fileName)
{
  std::ofstream fout;
  fout.open(fileName, std::ios_base::out);
  double* tmpPoint = new double[pTask.GetN()];
  fout << pData->GetCount() - 1 + printPoints.size() << " " <<
    pTask.GetNumOfFunc() << "\n";
  SearcDataIterator it = pData->GetBeginIterator();
  int t = 0;
  for (++it; it; ++it)
  {
    //evolvent.GetImage((*it)->xl(), tmpPoint);
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
}

void Method::HookeJeevesMethod(Trial& point, std::vector<Trial*>& localPoints)
{
  int addAllLocalPoints = 2;
  LocalMethod* localMethod = nullptr;

  int oldNP = parameters.NumPoints;

  parameters.NumPoints = 2 * parameters.NumPoints;

  switch (parameters.TypeLocalMethod) {
  case ParallelHookeJeeves:
  {
    Calculation* LocalCalculation = new OMPCalculation(pTask);

    localMethod = new ParallelHookeJeevesMethod(&pTask, point, *LocalCalculation, addAllLocalPoints);
    break;
  }
  default:
  {
    localMethod = new LocalMethod(&pTask, point, addAllLocalPoints);
    break;
  }
  }



  double initialStep = 0;
  for (int i = 0; i < pTask.GetN(); i++)
    initialStep += pTask.GetB()[i] - pTask.GetA()[i];
  initialStep /= parameters.Dimension;
  // начальный шаг равен среднему размеру стороны гиперкуба, умноженному на коэффициент
  localMethod->SetEps(parameters.localVerificationEpsilon);
  localMethod->SetInitialStep(0.07 * initialStep);
  localMethod->SetMaxTrials(parameters.localIteration);
  Trial newpoint2 = localMethod->StartOptimization();
  Trial* newpoint = TrialFactory::CreateTrial(&newpoint2);

  parameters.NumPoints = oldNP;

  std::vector<Trial> points = localMethod->GetSearchSequence();

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

    Trial* temp = TrialFactory::CreateTrial(&points[i]);
    Extended x;
    evolvent.GetInverseImage(&(temp->y[pTask.GetProcLevel()]), x);
    temp->SetX(x);
    temp->FuncValues[0] = pTask.CalculateFuncs(temp->y, 0);
    temp->TypeColor = 3;
    localPoints.push_back(temp);
  }

  InsertLocalPoints(localPoints);
}



// ------------------------------------------------------------------------------------------------
void Method::LocalSearch()
{
  if (((parameters.localRefineSolution == FinalStart && isStop) ||
    (parameters.localRefineSolution == UpdatedMinimum))
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
    std::vector<Trial*> localPoints;

    Trial* point = GetOptimEstimation();

    //HookeJeevesMethod(point, localPoints);

    LocalMethod* localMethod = nullptr;

    int addAllLocalPoints = std::max(0, int(parameters.TypeAddLocalPoint) - 2);
    switch (parameters.TypeLocalMethod) {
    case ParallelHookeJeeves:
    {

      //Calculation* newCalculation = new OMPCalculation(pTask);

      //localMethod = new ParallelHookeJeevesMethod(&(pTask), *point, *newCalculation, addAllLocalPoints);
      localMethod = new ParallelHookeJeevesMethod(&(pTask), *point, calculation, addAllLocalPoints);


      //localMethod = new ParallelHookeJeevesMethod(&(pTask), *point, this->calculation, addAllLocalPoints);
      break;
    }
    default:
      localMethod = new LocalMethod(&(pTask), *point, addAllLocalPoints);
      break;
    }

    double initialStep = 0;
    for (int i = 0; i < pTask.GetN(); i++)
      initialStep += pTask.GetB()[i] - pTask.GetA()[i];
    initialStep /= parameters.Dimension;
    // начальный шаг равен среднему размеру стороны гиперкуба, умноженному на коэффициент
    localMethod->SetEps(parameters.localVerificationEpsilon);

    localMethod->SetInitialStep(0.07 * initialStep);

    localMethod->SetMaxTrials(parameters.localIteration);
    Trial point2 = localMethod->StartOptimization();
    Trial* newpoint = TrialFactory::CreateTrial(&point2);

    std::vector<Trial> points = localMethod->GetSearchSequence();

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

      Trial* temp = TrialFactory::CreateTrial(&points[i]);
      Extended x;
      evolvent.GetInverseImage(temp->y, x);
      temp->SetX(x);
      temp->FuncValues[0] = pTask.CalculateFuncs(temp->y, 0);
      temp->TypeColor = 3;
      localPoints.push_back(temp);
    }
    parameters.NumPoints = oldNP;
    parameters.NumThread = oldNT;
    InsertLocalPoints(localPoints);


    localPointCount += localMethod->GetTrialsCounter();

    if (parameters.localRefineSolution == UpdatedMinimum)
      pData->SetRecalc(true);

  }
}

int Method::GetLocalPointCount()
{
  return localPointCount;
}

int Method::GetNumberLocalMethodtStart()
{
  return numberLocalMethodtStart;
}



// - end of file ----------------------------------------------------------------------------------

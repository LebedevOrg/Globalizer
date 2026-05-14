
/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2025 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      MixedIntegerMethod.cpp                                      //
//                                                                         //
//  Purpose:   Source file for method class                                //
//                                                                         //
//  Author(s): Lebedev I.                                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


#include "MixedIntegerMethod.h"


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

#include <cmath>
#include <vector>
#include <fstream>
#include <algorithm>


#include <string>
#include <cmath>



// ------------------------------------------------------------------------------------------------
MixedIntegerMethod::MixedIntegerMethod(Task& _pTask, SearchData& _pData,
  Calculation& _Calculation, IEvolvent& _Evolvent) : Method(_pTask, _pData, _Calculation, _Evolvent)
{
  
}

// ------------------------------------------------------------------------------------------------
MixedIntegerMethod::~MixedIntegerMethod()
{}

// ------------------------------------------------------------------------------------------------
void MixedIntegerMethod::SetDiscreteValue(int u, std::vector< std::vector<double> > dvs)
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

SearchData* MixedIntegerMethod::GetSearchData(Trial* trial)
{
  return pData;
}

// ------------------------------------------------------------------------------------------------
/// Вычисляет координаты на отрезке 0..1 для всех разверток по образу проведенного испытания
void MixedIntegerMethod::CalculateCurrentPoint(Trial& pCurTrialsj, SearchInterval* BestIntervalsj)
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
      (((BestIntervalsj->zr() - BestIntervalsj->zl()) > 0) ? 1 : -1) *
      pow(fabs(BestIntervalsj->zr() - BestIntervalsj->zl()) /
        pData->M[BestIntervalsj->izl()], pTask.GetNumberOfContinuousVariable()) / 2 / parameters.r);
    //      pCurTrialsj.x = BestIntervalsj->xl() + (0.5*BestIntervalsj->dx -
    //(((BestIntervalsj->zr() - BestIntervalsj->zl)>0)?1:-1)*pow(fabs(BestIntervalsj->zr() -
    //BestIntervalsj->zl)/pData->M[BestIntervalsj->izl],parameters.Dimension)/(2*r));
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

  // Записываем значение дискретной переменной
  for (int j = 0; j < pTask.GetNumberOfDiscreteVariable(); j++)
    pCurTrialsj.y[startDiscreteVariable + j] =
    mDiscreteValues[pCurTrialsj.discreteValuesIndex][j];
}

// ------------------------------------------------------------------------------------------------
void MixedIntegerMethod::FirstIteration()
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
  SetNumPoints(parameters.NumPoints * mDiscreteValuesCount);

  SearchInterval** NewInterval = new SearchInterval * [mDiscreteValuesCount];
  //SearchIntervalFactory::CreateSearchInterval();
  for (int e = 0; e < mDiscreteValuesCount; e++)
  {
    NewInterval[e] = SearchIntervalFactory::CreateSearchInterval();
    NewInterval[e]->ind = iteration.IterationCount;
    NewInterval[e]->K = 0;
    NewInterval[e]->CreatePoint();
    NewInterval[e]->LeftPoint->discreteValuesIndex = e;
    NewInterval[e]->RightPoint->discreteValuesIndex = e;
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
    if ((parameters.IsCalculationInBorderPoint == true) || (parameters.LocalTuningType != 0))
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
  //for (i = 0; i < NumPoints; i++)
  //  iteration.BestIntervals[i] = p;

  // Флаг пересчета - поднят
  pData->SetRecalc(true);

  // Точки первой итерации выбираются по особому правилу
  // Равномерно ставим NumPoints точек c шагом h
  // А надо бы случайно...
  double h = 1.0 / (parameters.NumPoints + 1);
  if (!parameters.IsLoadFirstPointFromFile) // равномерно распределяем начальные точки
  {
    for (int e = 0; e < mDiscreteValuesCount; e++)
    {
      for (int q = 0; q < parameters.NumPoints; q++)
      {

        if (parameters.TypeDistributionStartingPoints == Evenly)
        {
          int ind = e * parameters.NumPoints + q;
          iteration.pCurTrials[ind] = TrialFactory::CreateTrial();
          iteration.pCurTrials[ind]->discreteValuesIndex = e;
          pData->GetTrials().push_back(iteration.pCurTrials[ind]);
          iteration.pCurTrials[ind]->SetX((q + 1) * h);

          // Вычисляем образ точки итерации - образ записывается в начальные позиции массива y
          CalculateImage(*iteration.pCurTrials[ind]);

          for (int j = 0; j < numberOfDiscreteVariable; j++)
            iteration.pCurTrials[ind]->y[startDiscreteVariable + j] =
            mDiscreteValues[iteration.pCurTrials[ind]->discreteValuesIndex][j];

          iteration.pCurTrials[ind]->leftInterval = NewInterval[e];
          iteration.pCurTrials[ind]->rightInterval = NewInterval[e];
        }
        else
        {
          int ind = e * parameters.NumPoints + q;
          iteration.pCurTrials[ind] = TrialFactory::CreateTrial();
          iteration.pCurTrials[ind]->discreteValuesIndex = e;
          pData->GetTrials().push_back(iteration.pCurTrials[ind]);

          for (size_t iCNP = 0; iCNP < pTask.GetNumberOfContinuousVariable(); iCNP++)
          {
            iteration.pCurTrials[ind]->y[iCNP] = pTask.GetA()[iCNP] + ((double(q) + 1.0) * h) * (pTask.GetB()[iCNP] - pTask.GetA()[iCNP]);
          }

          Extended genX(0.0);
          evolvent.GetInverseImage(iteration.pCurTrials[ind]->y, genX);
          iteration.pCurTrials[ind]->SetX(genX);

          for (int j = 0; j < numberOfDiscreteVariable; j++)
            iteration.pCurTrials[ind]->y[startDiscreteVariable + j] =
            mDiscreteValues[iteration.pCurTrials[ind]->discreteValuesIndex][j];

          iteration.pCurTrials[ind]->leftInterval = NewInterval[e];
          iteration.pCurTrials[ind]->rightInterval = NewInterval[e];
        }
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


      while (!input.eof()) 
      {
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
void MixedIntegerMethod::CalculateIterationPoints()
{
  if (iteration.IterationCount == 1)
  {
    return;
  }
  else if (iteration.IterationCount == 2)
  {
    this->SetNumPoints(parameters.NumPoints);
  }

  // Если поднят флаг - то пересчитать все характеристики
  Recalc();

  // Здесь надо взять NumPoints лучших характеристик из очереди
  // Очередь пока одна - очередь глобальных характеристик
  // В ней должно быть нужное количество интервалов, т.к. на первом шаге проводится NumPoints
  // испытаний
  std::vector<SearchInterval*> BestIntervals(parameters.NumPoints);

  int LocalMix = parameters.LocalMix;

  if (GetIterationType(iteration.IterationCount, LocalMix) == Global)
  {

    pData->GetBestIntervals(BestIntervals.data(), parameters.NumPoints);

  }
  else
    pData->GetBestLocalIntervals(BestIntervals.data(), parameters.NumPoints);
  // Пока заполняем одновременно вектор CurTrials, и вектор интервалов

  CalculateCurrentPoints(BestIntervals);
}




// - end of file ----------------------------------------------------------------------------------

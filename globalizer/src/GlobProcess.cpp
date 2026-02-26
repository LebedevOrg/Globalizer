/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      process.cpp                                                 //
//                                                                         //
//  Purpose:   Source file for optimization process class                  //
//                                                                         //
//  Author(s): Sysoyev A., Barkalov K.                                     //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include <mpi.h>
#include <cmath>
#include <algorithm>
#include <cstring>
#include <cstdio>
#include <stdio.h>
#include <stdlib.h>

#include "Common.h"
#include "GlobProcess.h"
#include "MethodFactory.h"
#include "MPICalculationAsync.h"


#include "SearcDataIterator.h"

#include "TaskFactory.h"


#define DEBUG_MPI

// ------------------------------------------------------------------------------------------------
void ShowIterResults(Process *pProcess)
{
}

// ------------------------------------------------------------------------------------------------
Process::Process(SearchData& data, Task& task) :
pData(&data), pTask(&task)
{
  isFirstRun = true;

  if (parameters.MapType == mpBase)
    evolvent = new Evolvent(parameters.Dimension - pTask->GetNumberOfDiscreteVariable(), parameters.m);
  else
    throw EXCEPTION("Unknown type of evolvent");

  calculation = CalculationFactory::CreateCalculation(*pTask, evolvent);

  pMethod = MethodFactory::CreateMethod(*pTask, *pData, *calculation, *evolvent);

  functionCalculationCount.resize(pTask->GetNumOfFunc());

  isPrintOptimEstimation = false;

  Neighbours.resize(parameters.GetProcNum() - 1);
  for (int i = 0; i < parameters.GetProcNum() - 1; i++)
    Neighbours[i] = i + 1;
  addPoints = nullptr;
}

// ------------------------------------------------------------------------------------------------
Process::~Process()
{
  delete calculation;
  delete pMethod;
  delete evolvent;
}

// ------------------------------------------------------------------------------------------------
double Process::GetSolveTime()
{
  return duration;
}

// ------------------------------------------------------------------------------------------------
void Process::Solve()
{
  Trial OptimEstimation, NewOptimEstimation;

  Timer.Start();
  if (isFirstRun)
  {
    BeginIterations();
    isFirstRun = false;
  }
  OptimEstimation = *(pMethod->GetOptimEstimation());
 
  while (!IsOptimumFound)
  {

    if (!pTask->IsLeaf())
    {
      if (!(pMethod->GetIterationCount() % parameters.StepPrintMessages) && parameters.isPrintResultToConsole)
      {
        print << "process 0, iteration " << pMethod->GetIterationCount() << "\t point count = " << pData->GetCount() << "\n";
        Trial* p = (pMethod->GetOptimEstimation());
        if (p->index >= 0)
        {
          print << "  Cur min = " << p->FuncValues[p->index] << " \n";
          for (int i = 0; i < pTask->GetN(); i++)
          {
            print << "  Cur x[" << i << "] = " << p->y[i] <<" \n";
          }
        }
      }
      
      if (static_cast<std::string>(parameters.iterPointsSavePath).size())
      {
        if (!(pMethod->GetIterationCount() % parameters.StepSavePoint))
        {
          pMethod->PrintPoints(std::to_string(pMethod->GetIterationCount()) + "_" + parameters.iterPointsSavePath.ToString());
        }
      }
    }
    pMethod->GetIterationCount();

    DoIteration();

    NewOptimEstimation = *(pMethod->GetOptimEstimation());
    if (NewOptimEstimation.FuncValues[NewOptimEstimation.index] !=
      OptimEstimation.FuncValues[OptimEstimation.index])
    {
      OptimEstimation = NewOptimEstimation;
      duration = Timer.GetTime();
      PrintOptimEstimationToFile(OptimEstimation);
    }

  }



  EndIterations();

  duration = Timer.GetTime();

  OptimEstimation = *(pMethod->GetOptimEstimation());

  if (!pTask->IsLeaf() && isPrintOptimEstimation)
  {
    //print << "\n";
    PrintOptimEstimationToFile(OptimEstimation);
    if (parameters.IsUseExtendedConsole)
      OldPrintOptimEstimationToConsole(OptimEstimation);
    else
      PrintOptimEstimationToConsole(OptimEstimation);
    PrintResultToFile(OptimEstimation);
  }
}

void Process::Reset(SearchData* data, Task* task)
{
  isFirstRun = true;
  pData = data;
  pTask = task;
  if (evolvent == 0)
  {
    if (parameters.MapType == mpBase)
      evolvent = new Evolvent(parameters.Dimension, parameters.m);
    else
      throw EXCEPTION("Unknown type of evolvent");
  }
  if (calculation == 0)
    calculation = CalculationFactory::CreateCalculation(*pTask, evolvent);
  else
    calculation->SetTask(pTask);

  if (pMethod != 0)
    delete pMethod;
  pMethod = MethodFactory::CreateMethod(*pTask, *pData, *calculation, *evolvent);

  functionCalculationCount.clear();
  functionCalculationCount.resize(pTask->GetNumOfFunc());
}

// ------------------------------------------------------------------------------------------------
void Process::PrintOptimEstimationToFile(Trial OptimEstimation)
{
  int NumberOfTrials;
  FILE* pf;

  if (parameters.IsPrintFile && !pTask->IsLeaf())
  {
    pf = fopen("optim.dat", "a");

    fprintf(pf, "Iteration = %d\n", pMethod->GetIterationCount());
    fprintf(pf, "min = %lf \n", OptimEstimation.FuncValues[OptimEstimation.index]);
    for (int i = 0; i < pTask->GetN(); i++)
      fprintf(pf, "x[%d] = %lf \n", i, OptimEstimation.y[i]);

    fprintf(pf, "\n");
    for (int i = 0; i < pTask->GetNumOfFunc(); i++)
      fprintf(pf, "M[%d] = %lf \n", i, (this->pData->M)[i]);

    NumberOfTrials = pMethod->GetNumberOfTrials();
    fprintf(pf, "NumberOfTrials = %d\n", NumberOfTrials);
    fprintf(pf, "Solve time = %f \n\n\n", duration);

    fclose(pf);
  }
}

// ------------------------------------------------------------------------------------------------
void Process::OldPrintOptimEstimationToConsole(Trial OptimEstimation)
{
  int NumberOfTrials;
  double ValueDifference = HUGE_VAL;
  double PointDifference = HUGE_VAL;
  int numOfOptima = 1;
  double* allOptimumPoints = new double[MAX_TRIAL_DIMENSION * MAX_NUM_MIN];
  double* allPointDifference = new double[MAX_NUM_MIN];

  printf("ProcLevel = %d\n", pTask->GetProcLevel());
  printf("Iteration = %d \n", pMethod->GetIterationCount());
  printf("Point = %d \n", pMethod->GetIterationCount() * parameters.NumPoints);

  printf("min = %lf \n", OptimEstimation.FuncValues[OptimEstimation.index]);
  for (int i = 0; i < pTask->GetN(); i++)
  {
    printf("x[%d] = %lf \n", i, OptimEstimation.y[i]);
  }

  printf("\n");
  printf("constants = ");
  for (int i = 0; i < pTask->GetN(); i++)
  {
    printf("%lf,_", OptimEstimation.y[i]);
  }

  printf("\n");

  if (pTask->GetIsOptimumValueDefined())
  {
    ValueDifference = OptimEstimation.FuncValues[OptimEstimation.index] - pTask->GetOptimumValue();
    printf("\nValue difference  = %lf \n", ValueDifference);
  }

  if (pTask->GetIsOptimumPointDefined())
  {
    PointDifference = 0.0;
    for (int i = 0; i < pTask->GetN(); i++)
    {
      PointDifference = GLOBALIZER_MAX(fabs(OptimEstimation.y[i] - pTask->GetOptimumPoint()[i]), PointDifference);
    }
    if (pTask->getProblem()->GetAllOptimumPoint(allOptimumPoints, numOfOptima) != IProblem::UNDEFINED)
    {
      PointDifference = 0.0;
      for (int j = 0; j < numOfOptima; j++)
      {
        allPointDifference[j] = 0.0;
        for (int i = 0; i < pTask->GetN(); i++)
        {
          allPointDifference[j] = GLOBALIZER_MAX(fabs(OptimEstimation.y[i] -
            allOptimumPoints[j * pTask->GetN() + i]), allPointDifference[j]);
        }
        if ((allPointDifference[j] < PointDifference) || (j == 0))
        {
          PointDifference = allPointDifference[j];
        }
      }
    }
    printf("Coordinates max difference = %lf \n", PointDifference);
  }

  //if (pTask->GetIsOptimumPointDefined() || pTask->GetIsOptimumValueDefined())
  //{
  //  printf("Global optimum %s\n", (PointDifference < parameters.Epsilon ||
  //                                 ValueDifference < parameters.Epsilon) ? "FOUND!" : "NOT FOUND" );
  //}


  bool res = false;

  switch (1)
  {
  case Accuracy:
    if (pMethod->GetAchievedAccuracy() < parameters.Epsilon)
      res = true;
    break;
  case OptimumVicinity:
  {
    res = true;
    //numOfOptima = ;
    if (pTask->getProblem()->GetAllOptimumPoint(allOptimumPoints, numOfOptima) == IProblem::UNDEFINED)
    {
      for (int i = 0; i < parameters.Dimension; i++)
      {

        double fabsx = fabs(OptimEstimation.y[i] - pTask->GetOptimumPoint()[i]);
        double fm = parameters.Epsilon * (pTask->GetB()[i] - pTask->GetA()[i]);
        if (fabsx > fm)
        {
          res = res && false;
        }
      }
    }
    else
    {
      for (int j = 0; j < numOfOptima; j++)
      {
        for (int i = 0; i < parameters.Dimension; i++)
        {
          double fabsx = fabs(OptimEstimation.y[i] - allOptimumPoints[parameters.Dimension * j + i]);
          double fm = parameters.Epsilon * (pTask->GetB()[i] - pTask->GetA()[i]);
          if (fabsx > fm)
          {
            res = res && false;
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
    for (int i = 0; i < pTask->GetN(); i++)
    {
      if (fabs(OptimEstimation.y[i] - pTask->GetOptimumPoint()[i]) > parameters.Epsilon)
      {
        res = false;
        break;
      }
    }
  }
  break;
  case OptimumValue:
    if (OptimEstimation.index == pTask->GetNumOfFunc() - 1 &&
      OptimEstimation.FuncValues[OptimEstimation.index] - pTask->GetOptimumValue() <
      parameters.Epsilon)
      res = true;
    break;
  }
  delete[] allOptimumPoints;
  print << "M = " << pData->M[0] << "\n";
  printf("Global optimum %s\n", (res) ? "FOUND!" : "NOT FOUND");

  NumberOfTrials = pMethod->GetNumberOfTrials();
  pMethod->PrintSection();

  printf("\nNumberOfTrials = %d\n", NumberOfTrials);
  for (int i = 0; i < pTask->GetNumOfFunc(); i++)
    printf("Number of calculations function %d of = %d\n", i,
      pMethod->GetFunctionCalculationCount()[i]);

  printf("\nLocalPointCount = %d\n", pMethod->GetLocalPointCount());
  printf("\nNumberLocalMethodtStart = %d\n", pMethod->GetNumberLocalMethodtStart());

  printf("\nSolve time = %lf\n\n\n", duration);
  parameters.timeSolve += duration;
}

// ------------------------------------------------------------------------------------------------
void Process::PrintOptimEstimationToConsole(Trial OptimEstimation)
{
  if (parameters.isPrintResultToConsole)
  {
    int NumberOfTrials;
    double ValueDifference = HUGE_VAL;
    double PointDifference = HUGE_VAL;
    int numOfOptima = 1;
    double* allOptimumPoints = new double[MAX_TRIAL_DIMENSION * MAX_NUM_MIN];
    double* allPointDifference = new double[MAX_NUM_MIN];

    //  print << "ProcLevel = " << pTask->GetProcLevel() << "\n";
    print << "Iterations = " << pMethod->GetIterationCount() << " \n";
    //print << "Points = " << pMethod->GetIterationCount() * parameters.NumPoints << " \n\n";
    if (OptimEstimation.index >= pTask->getProblem()->GetNumberOfConstraints())
    {
      print << "Minimum value = " << OptimEstimation.FuncValues[OptimEstimation.index] << " \n";
      for (int i = 0; i < pTask->GetN(); i++)
      {
        print << "x[" << i << "] = " << OptimEstimation.y[i] << " \n";
      }
    }
    else
    {
      print << "No solution has been found in the acceptable range.\n";
    }
    /*
    print << "\n";
    print << "constants = ";
    for (int i = 0; i < pTask->GetN(); i++)
    {
      print << OptimEstimation.y[i] << ",_";
    }
    */
    print << "\n";

    if (pTask->GetIsOptimumValueDefined())
    {
      ValueDifference = OptimEstimation.FuncValues[OptimEstimation.index] - pTask->GetOptimumValue();
      print << "\nValue difference  = " << ValueDifference << " \n";
    }

    if (pTask->GetIsOptimumPointDefined())
    {
      PointDifference = 0.0;
      for (int i = 0; i < pTask->GetN(); i++)
      {
        PointDifference = GLOBALIZER_MAX(fabs(OptimEstimation.y[i] - pTask->GetOptimumPoint()[i]), PointDifference);
      }
      if (pTask->getProblem()->GetAllOptimumPoint(allOptimumPoints, numOfOptima) != IProblem::UNDEFINED)
      {
        PointDifference = 0.0;
        for (int j = 0; j < numOfOptima; j++)
        {
          allPointDifference[j] = 0.0;
          for (int i = 0; i < pTask->GetN(); i++)
          {
            allPointDifference[j] = GLOBALIZER_MAX(fabs(OptimEstimation.y[i] -
              allOptimumPoints[j * pTask->GetN() + i]), allPointDifference[j]);
          }
          if ((allPointDifference[j] < PointDifference) || (j == 0))
          {
            PointDifference = allPointDifference[j];
          }
        }
      }
      print << "Coordinates max difference = " << PointDifference << " \n";
    }


    bool res = false;
    double AchievedAccuracy = 0.0;

    switch (parameters.stopCondition)
    {
    case Accuracy:
      if (pMethod->GetAchievedAccuracy() < parameters.Epsilon)
        res = true;
      AchievedAccuracy = pMethod->GetAchievedAccuracy();
      break;
    case OptimumVicinity:
    {
      res = true;
      AchievedAccuracy = parameters.Epsilon;
      if (pTask->getProblem()->GetAllOptimumPoint(allOptimumPoints, numOfOptima) == IProblem::UNDEFINED)
      {
        for (int i = 0; i < parameters.Dimension; i++)
        {

          double fabsx = fabs(OptimEstimation.y[i] - pTask->GetOptimumPoint()[i]);
          double fm = parameters.Epsilon * (pTask->GetB()[i] - pTask->GetA()[i]);
          if (fabsx > fm)
          {
            res = res && false;
            AchievedAccuracy = fabsx;
          }
        }
      }
      else
      {
        for (int j = 0; j < numOfOptima; j++)
        {
          for (int i = 0; i < parameters.Dimension; i++)
          {
            double fabsx = fabs(OptimEstimation.y[i] - allOptimumPoints[parameters.Dimension * j + i]);
            double fm = parameters.Epsilon * (pTask->GetB()[i] - pTask->GetA()[i]);
            if (fabsx > fm)
            {
              res = res && false;
              AchievedAccuracy = fabsx;
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
      AchievedAccuracy = parameters.Epsilon;
      for (int i = 0; i < pTask->GetN(); i++)
      {
        if (fabs(OptimEstimation.y[i] - pTask->GetOptimumPoint()[i]) > parameters.Epsilon)
        {
          res = false;
          AchievedAccuracy = fabs(OptimEstimation.y[i] - pTask->GetOptimumPoint()[i]);
          break;
        }
      }
    }
    break;
    case OptimumValue:
      if (OptimEstimation.index == pTask->GetNumOfFunc() - 1 &&
        OptimEstimation.FuncValues[OptimEstimation.index] - pTask->GetOptimumValue() <
        parameters.Epsilon)
        res = true;
      AchievedAccuracy = OptimEstimation.FuncValues[OptimEstimation.index] - pTask->GetOptimumValue();
      break;
    }
    delete[] allOptimumPoints;

    print << "Accuracy = " << AchievedAccuracy << "\n";

    //  print << "Global optimum " << ((res) ? "FOUND!" : "NOT FOUND") <<"\n";

    NumberOfTrials = pMethod->GetNumberOfTrials();
    pMethod->PrintSection();

    //print << "\nTrials = " << NumberOfTrials << "\n";
    for (int i = 0; i < pTask->GetNumOfFunc(); i++)
    {
      if (i < pTask->getProblem()->GetNumberOfConstraints())
        print << "\tConstraint\t\t" << i << "\tcalculations =\t" << pMethod->GetFunctionCalculationCount()[i] << "\n";
      else
        print << "\tObjective function\t" << i << "\tcalculations =\t" << pMethod->GetFunctionCalculationCount()[i] << "\n";
    }
    //  print << "\nLocalPointCount = " << pMethod->GetLocalPointCount() << "\n";
    //  print << "\nNumberLocalMethodtStart = " << pMethod->GetNumberLocalMethodtStart() << "\n";

    print << "\nSolve time = " << duration << "\n\n\n";
  }

}


// ------------------------------------------------------------------------------------------------
void Process::PrintResultToFile(Trial OptimEstimation)
{
  FILE* pf;

  if (parameters.ResulLog.ToString() != "000" && pTask->GetProcLevel() == 0)
  {
    pf = fopen(parameters.ResulLog.ToString().c_str(), "a");

    parameters.PrintParametersToFile(pf);

    int NumberOfTrials;
    double ValueDifference = HUGE_VAL;
    double PointDifference = HUGE_VAL;
    int numOfOptima = 1;
    double* allOptimumPoints = new double[MAX_TRIAL_DIMENSION * MAX_NUM_MIN];
    double* allPointDifference = new double[MAX_NUM_MIN];

    fprintf(pf, "ProcLevel = %d\n", pTask->GetProcLevel());
    fprintf(pf, "Iteration = %d \n", pMethod->GetIterationCount());
    fprintf(pf, "Point = %d \n", pMethod->GetIterationCount() * parameters.NumPoints);

    fprintf(pf, "min = %lf \n", OptimEstimation.FuncValues[OptimEstimation.index]);
    for (int i = 0; i < pTask->GetN(); i++)
    {
      fprintf(pf, "x[%d] = %lf \n", i, OptimEstimation.y[i]);
    }

    fprintf(pf, "\n");
    fprintf(pf, "constants = ");
    for (int i = 0; i < pTask->GetN(); i++)
    {
      fprintf(pf, "%lf,_", OptimEstimation.y[i]);
    }

    fprintf(pf, "\n");

    if (pTask->GetIsOptimumValueDefined())
    {
      ValueDifference = OptimEstimation.FuncValues[OptimEstimation.index] - pTask->GetOptimumValue();
      fprintf(pf, "\nValue difference  = %lf \n", ValueDifference);
    }

    if (pTask->GetIsOptimumPointDefined())
    {
      PointDifference = 0.0;
      for (int i = 0; i < pTask->GetN(); i++)
      {
        PointDifference = GLOBALIZER_MAX(fabs(OptimEstimation.y[i] - pTask->GetOptimumPoint()[i]), PointDifference);
      }
      if (pTask->getProblem()->GetAllOptimumPoint(allOptimumPoints, numOfOptima) != IProblem::UNDEFINED)
      {
        PointDifference = 0.0;
        for (int j = 0; j < numOfOptima; j++)
        {
          allPointDifference[j] = 0.0;
          for (int i = 0; i < pTask->GetN(); i++)
          {
            allPointDifference[j] = GLOBALIZER_MAX(fabs(OptimEstimation.y[i] -
              allOptimumPoints[j * pTask->GetN() + i]), allPointDifference[j]);
          }
          if ((allPointDifference[j] < PointDifference) || (j == 0))
          {
            PointDifference = allPointDifference[j];
          }
        }
      }
      fprintf(pf, "Coordinates max difference = %lf \n", PointDifference);
    }

    bool res = false;

    switch (1)
    {
    case Accuracy:
      if (pMethod->GetAchievedAccuracy() < parameters.Epsilon)
        res = true;
      break;
    case OptimumVicinity:
    {
      res = true;
      if (pTask->getProblem()->GetAllOptimumPoint(allOptimumPoints, numOfOptima) == IProblem::UNDEFINED)
      {
        for (int i = 0; i < parameters.Dimension; i++)
        {

          double fabsx = fabs(OptimEstimation.y[i] - pTask->GetOptimumPoint()[i]);
          double fm = parameters.Epsilon * (pTask->GetB()[i] - pTask->GetA()[i]);
          if (fabsx > fm)
          {
            res = res && false;
          }
        }
      }
      else
      {
        for (int j = 0; j < numOfOptima; j++)
        {
          for (int i = 0; i < parameters.Dimension; i++)
          {
            double fabsx = fabs(OptimEstimation.y[i] - allOptimumPoints[parameters.Dimension * j + i]);
            double fm = parameters.Epsilon * (pTask->GetB()[i] - pTask->GetA()[i]);
            if (fabsx > fm)
            {
              res = res && false;
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
      for (int i = 0; i < pTask->GetN(); i++)
      {
        if (fabs(OptimEstimation.y[i] - pTask->GetOptimumPoint()[i]) > parameters.Epsilon)
        {
          res = false;
          break;
        }
      }
    }
    break;
    case OptimumValue:
      if (OptimEstimation.index == pTask->GetNumOfFunc() - 1 &&
        OptimEstimation.FuncValues[OptimEstimation.index] - pTask->GetOptimumValue() <
        parameters.Epsilon)
        res = true;
      break;
    }
    delete[] allOptimumPoints;

    fprintf(pf, "Global optimum %s\n", (res) ? "FOUND!" : "NOT FOUND");


    NumberOfTrials = pMethod->GetNumberOfTrials();
    pMethod->PrintSection();

    fprintf(pf, "\nNumberOfTrials = %d\n", NumberOfTrials);
    for (int i = 0; i < pTask->GetNumOfFunc(); i++)
      fprintf(pf, "Number of calculations function %d of = %d\n", i,
        pMethod->GetFunctionCalculationCount()[i]);

    fprintf(pf, "\nLocalPointCount = %d\n", pMethod->GetLocalPointCount());
    fprintf(pf, "\nNumberLocalMethodtStart = %d\n", pMethod->GetNumberLocalMethodtStart());
    fprintf(pf, "\nSolve time = %lf\n\n\n", duration);
    fclose(pf);
  }
}


// ------------------------------------------------------------------------------------------------
void Process::BeginIterations()
{
  IsOptimumFound = false;
  isPrintOptimEstimation = false;

  pMethod->FirstIteration();

  if (addPoints != nullptr)
    pMethod->InsertPoints(*addPoints);
}

// ------------------------------------------------------------------------------------------------
void Process::DoIteration()
{
  bool IsStop;

  //  проверяем критерий остановки
  IsStop = pMethod->CheckStopCondition();
  IsOptimumFound = IsStop;

  if (IsOptimumFound)
    isPrintOptimEstimation = true;

  IsOptimumFound = CheckIsStop(IsOptimumFound);
  IsStop = IsOptimumFound;

  try
  {
    if (!IsStop)
    {
      // вычисляем координаты испытаний
      pMethod->CalculateIterationPoints();

      IsStop = pMethod->CheckStopCondition();
      if (IsStop)
      {
        IsOptimumFound = true;
        isPrintOptimEstimation = true;
      }

      pMethod->CalculateFunctionals();


      for (int j = 0; j < pTask->GetNumOfFunc(); j++)
      {
        functionCalculationCount[j] += pMethod->GetFunctionCalculationCount()[j];
      }
      //Провести оценку оптимума нужно до обновления данных,  т.к. в этой функции может быть поднят флаг recalc
      pMethod->EstimateOptimum();
      //Все случаи поднятия флага recalc обработаны, можно обновлять базу
      pMethod->RenewSearchData();

      pMethod->FinalizeIteration();

    }
    else
    {
      IsOptimumFound = true;
      if (parameters.TypeCalculation == AsyncMPI)
        MPICalculationAsync::AsyncFinilize();  
      pMethod->FinalizeIteration();
    }
  }
  catch (const Exception & e)
  {
    
    if (std::string(e.GetDescription()) == std::string("Point is outside the interval !"))
    {
      IsOptimumFound = true;
      isPrintOptimEstimation = true;
    }
    else
      throw e;
  }
}

// ------------------------------------------------------------------------------------------------
bool Process::CheckIsStop(bool IsStop)
{
  bool f = IsStop;

  MPI_Status status;

  return IsStop;
}


// ------------------------------------------------------------------------------------------------
void Process::EndIterations()
{
  ////Локальное уточнение найденного решения
  pMethod->LocalSearch();
  //Запись в файл точек испытаний
  //if (ProcLevel == 0 && static_cast<std::string>(parameters.iterPointsSavePath).size())
  //  pMethod->PrintPoints(parameters.iterPointsSavePath);
  if (pTask->GetProcLevel() == 0 && static_cast<std::string>(parameters.iterPointsSavePath).size())
    pMethod->PrintPoints(parameters.iterPointsSavePath);
  else
    pMethod->SavePoints();
}

// ------------------------------------------------------------------------------------------------
void Process::InsertPoints(std::vector<Trial*>& points)
{
  addPoints = &points;
}

// - end of file ----------------------------------------------------------------------------------

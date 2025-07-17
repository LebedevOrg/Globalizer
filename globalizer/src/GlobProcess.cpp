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

#include "common.h"
#include "GlobProcess.h"
#include "method_factory.h"
#include "mpi_calculation_async.h"

#include "Pareto.h"
#include "SearcDataIterator.h"

#include "TaskFactory.h"


#define DEBUG_MPI

// ------------------------------------------------------------------------------------------------
void ShowIterResults(TProcess *pProcess)
{
}

// ------------------------------------------------------------------------------------------------
TProcess::TProcess(TSearchData& data, TTask& task) :
pData(&data), pTask(&task)
{
  isFirstRun = true;

  int L = parameters.MapInLevel[pTask->GetProcLevel()];
  int d = pTask->GetFreeN();
  if (pTask->IsLeaf())
    d = pTask->GetFreeN() - pTask->GetNumberOfDiscreteVariable();
  if (parameters.MapType == mpBase)
    Evolvent = new TEvolvent(d, parameters.m);
  else if (parameters.MapType == mpRotated)
    Evolvent = new TRotatedEvolvent(d, parameters.m, L);
  else if (parameters.MapType == mpShifted)
    Evolvent = new TShiftedEvolvent(d, parameters.m, L);
  else
    throw EXCEPTION("Unknown type of evolvent");

  Calculation = TCalculationFactory::CreateCalculation(*pTask, Evolvent);

  pMethod = TMethodFactory::CreateMethod(*pTask, *pData, *Calculation, *Evolvent);

  functionCalculationCount.resize(pTask->GetNumOfFunc());

  isPrintOptimEstimation = false;

  if ((parameters.MapProcInLevel[pTask->GetProcLevel()] - 1) > 0)
  {
    //printf("proc = %d ProcLevel = %d", parameters.GetProcRank(), pTask->GetProcLevel());
    //printf("proc = %d MapProcInLevel = %d", parameters.GetProcRank(), parameters.MapProcInLevel[pTask->GetProcLevel()]);

    Neighbours.resize(parameters.MapProcInLevel[pTask->GetProcLevel()] - 1);

    int p = 0;
    for (int k = 0; k < parameters.MapProcInLevel[pTask->GetProcLevel()]; k++)
    {
      if (parameters.GetProcRank() == k)
        continue;
      Neighbours[p] = k;
      //printf("proc = %d Neighbours = %d Neighbours.size = %d\n", parameters.GetProcRank(), Neighbours[p], Neighbours.size());
      p++;
    }
  }
}

// ------------------------------------------------------------------------------------------------
TProcess::~TProcess()
{
  delete Calculation;
  delete pMethod;
  delete Evolvent;
}

// ------------------------------------------------------------------------------------------------
double TProcess::GetSolveTime()
{
  return duration;
}

// ------------------------------------------------------------------------------------------------
void TProcess::Solve()
{
  ////if (parameters.GetProcRank() == 0) printf("AA0! %d\n", parameters.GetProcRank());

  TTrial OptimEstimation, NewOptimEstimation;

  Timer.Start();
  if (isFirstRun)
  {
    BeginIterations();
    isFirstRun = false;
  }
  OptimEstimation = *(pMethod->GetOptimEstimation());//.FuncValues[pMethod->GetOptimEstimation().index];

  //if (pTask->GetProcLevel() == 0)  printf("CC-3! Proc=%d iter=%d \n", parameters.GetProcRank(), this->pMethod->GetIterationCount());
  
  while (!IsOptimumFound)
  {
    //if (pTask->GetProcLevel() == 0)  printf("CC-2! Proc=%d iter=%d\n", parameters.GetProcRank(), this->pMethod->GetIterationCount());


    if (pTask->GetProcLevel() == 0)
    {
      if (!(pMethod->GetIterationCount() % parameters.StepPrintMessages))
      {
        printf("process 0, iteration %d\t point count = %d\n", pMethod->GetIterationCount(), pData->GetCount());
        TTrial* p = (pMethod->GetOptimEstimation());
        if (p->index >= 0)
        {
          printf("  Cur min = %lf \n", p->FuncValues[p->index]);
          for (int i = 0; i < pTask->GetN(); i++)
          {
            printf("  Cur x[%d] = %lf \n", i, p->y[i]);
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
    //if (pTask->GetProcLevel() == 0)  printf("CC-1! Proc=%d iter=%d PL=%d IsStop=%d\n", parameters.GetProcRank(), this->pMethod->GetIterationCount(), (parameters.MapProcInLevel[pTask->GetProcLevel()] - 1), IsOptimumFound);

    DoIteration();

    //if (pTask->GetProcLevel() == 0)  printf("CC23! Proc=%d iter=%d PL=%d IsStop=%d\n", parameters.GetProcRank(), this->pMethod->GetIterationCount(), (parameters.MapProcInLevel[pTask->GetProcLevel()] - 1), IsOptimumFound);


    NewOptimEstimation = *(pMethod->GetOptimEstimation());
    if (NewOptimEstimation.FuncValues[NewOptimEstimation.index] !=
      OptimEstimation.FuncValues[OptimEstimation.index])
    {
      OptimEstimation = NewOptimEstimation;
      duration = Timer.GetTime();
      PrintOptimEstimationToFile(OptimEstimation);
    }

    //if (IsOptimumFound)
    //  isPrintOptimEstimation = true;
    //if (pTask->GetProcLevel() == 0)  printf("CC26! Proc=%d iter=%d PL=%d IsStop=%d\n", parameters.GetProcRank(), this->pMethod->GetIterationCount(), (parameters.MapProcInLevel[pTask->GetProcLevel()] - 1), IsOptimumFound);

    //IsOptimumFound = CheckIsStop(IsOptimumFound);

    //if (pTask->GetProcLevel() == 0)  printf("CC34! Proc=%d iter=%d PL=%d IsStop=%d\n", parameters.GetProcRank(), this->pMethod->GetIterationCount(), (parameters.MapProcInLevel[pTask->GetProcLevel()] - 1), IsOptimumFound);

  }



  EndIterations();

  duration = Timer.GetTime();

  OptimEstimation = *(pMethod->GetOptimEstimation());

  if ((pTask->GetProcLevel() == 0) && isPrintOptimEstimation)
  {
    printf("\n");
    PrintOptimEstimationToFile(OptimEstimation);
    PrintOptimEstimationToConsole(OptimEstimation);
    PrintResultToFile(OptimEstimation);
  }
  else
  {
    //printPoints;
  }

}

void TProcess::Reset(TSearchData* data, TTask* task)
{
  isFirstRun = true;
  pData = data;
  pTask = task;
  int L = parameters.MapInLevel[pTask->GetProcLevel()];
  if (Evolvent == 0)
  {
    if (parameters.MapType == mpBase)
      Evolvent = new TEvolvent(pTask->GetFreeN() - pTask->GetNumberOfDiscreteVariable(), parameters.m);
    else if (parameters.MapType == mpRotated)
      Evolvent = new TRotatedEvolvent(pTask->GetFreeN(), parameters.m, L);
    else if (parameters.MapType == mpShifted)
      Evolvent = new TShiftedEvolvent(pTask->GetFreeN(), parameters.m, L);
    else
      throw EXCEPTION("Unknown type of evolvent");
  }
  if (Calculation == 0)
    Calculation = TCalculationFactory::CreateCalculation(*pTask, Evolvent);
  else
    Calculation->SetTask(pTask);

  if (pMethod != 0)
    delete pMethod;
  pMethod = TMethodFactory::CreateMethod(*pTask, *pData, *Calculation, *Evolvent);

  functionCalculationCount.clear();
  functionCalculationCount.resize(pTask->GetNumOfFunc());
}

// ------------------------------------------------------------------------------------------------
void TProcess::PrintOptimEstimationToFile(TTrial OptimEstimation)
{
  int NumberOfTrials;
  FILE* pf;

  if (parameters.IsPrintFile && pTask->GetProcLevel() == 0)
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
void TProcess::PrintOptimEstimationToConsole(TTrial OptimEstimation)
{
  int NumberOfTrials;
  double ValueDifference = HUGE_VAL;
  double PointDifference = HUGE_VAL;
  int numOfOptima = 1;
  double* allOptimumPoints = new double[MAX_TRIAL_DIMENSION*MAX_NUM_MIN];
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
            allOptimumPoints[j*pTask->GetN() + i]), allPointDifference[j]);
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
            double fabsx = fabs(OptimEstimation.y[i] - allOptimumPoints[parameters.Dimension*j + i]);
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

  printf("Global optimum %s\n", (res) ? "FOUND!" : "NOT FOUND");

//  /************************************************/
//
//  std::string fileName;
//  if (parameters.TypeCalculation != OMP) {
//    fileName = "seria_full_result.txt";
//
//    std::ofstream vmdelet_out;                    //создаем поток 
//    vmdelet_out.open(fileName, std::ios::app);  // открываем файл для записи в конец
//
//    if (parameters.TypeCalculation != OMP) {
//      vmdelet_out << "Global: " << parameters.DimInTaskLevel[0] << "\nLocal: " << parameters.DimInTaskLevel[1] << "\n";
//
//      vmdelet_out << "Permutations vector: ";
//      for (auto const& element : TTaskFactory::permutations) {
//        vmdelet_out << element << " ";
//      }
//      vmdelet_out << "\n\n";
//    }
//
//    vmdelet_out << "Min: " << OptimEstimation.FuncValues[OptimEstimation.index] << " [ ";
//    for (int i = 0; i < pTask->GetN(); i++)
//    {
//      vmdelet_out << OptimEstimation.y[i] << " ";
//    }
//    vmdelet_out << "]\nREAL Min: " << pTask->GetOptimumValue() << " [ ";
//    for (int i = 0; i < pTask->GetN(); i++)
//    {
//      vmdelet_out << pTask->GetOptimumPoint()[i] << " ";
//    }
//    vmdelet_out << "]\n";
//
//    vmdelet_out << "Coordinates max difference: " << PointDifference << "\n";
//    ValueDifference = OptimEstimation.FuncValues[OptimEstimation.index] - pTask->GetOptimumValue();
//    vmdelet_out << "Value difference: " << ValueDifference << "\n";
//
//    vmdelet_out << "Iteration: " << pMethod->GetIterationCount() << "\n";
//    vmdelet_out << "Point: " << pMethod->GetIterationCount() * parameters.NumPoints << "\n";
//
//    vmdelet_out << "Global optimum " << ((res) ? "FOUND!" : "NOT FOUND") << "\n-------------------------------------\n";
//    vmdelet_out.close();                          // закрываем файл
//  }
//  /************************************************/
//
  NumberOfTrials = pMethod->GetNumberOfTrials();
  pMethod->PrintSection();

  printf("\nNumberOfTrials = %d\n", NumberOfTrials);
  for (int i = 0; i < pTask->GetNumOfFunc(); i++)
    printf("Number of calculations function %d of = %d\n", i,
    pMethod->GetFunctionCalculationCount()[i]);

  printf("\nLocalPointCount = %d\n", pMethod->GetLocalPointCount());
  printf("\nNumberLocalMethodtStart = %d\n", pMethod->GetNumberLocalMethodtStart());
  //if (parameters.printAdvancedInfo)
  //{
  //  printf("\n");
  //  for (int i = 0; i < pTask->GetNumOfFunc(); i++)
  //    printf("M estimation for function %d = %f\n", i, pMethod->GetM()[i]);
  //  printf("\nOptimum index: %i\n\n", OptimEstimation.index);
  //  for (int i = 0; i <= OptimEstimation.index; i++)
  //    printf("Function %d value in estimated optimum: %f\n", i, OptimEstimation.FuncValues[i]);
  //}
  printf("\nSolve time = %lf\n\n\n", duration);
//
//
//  if (parameters.TypeCalculation != OMP) {
//    fileName = "seria_table_result.txt";
//
//    std::ofstream statOut;                    //создаем поток 
//    statOut.open(fileName, std::ios::app);  // открываем файл для записи в конец
//
//    statOut << ((res) ? "FOUND" : "NOT_FOUND") << " "
//      << pMethod->GetIterationCount() << " "
//      << NumberOfTrials << " "
//      << pMethod->GetLocalPointCount() << " "
//      << duration << " "
//      << parameters.DimInTaskLevel[0] << " ";
//    for (int ii = 0; ii < parameters.DimInTaskLevel[0]; ii++) {
//       statOut << TTaskFactory::permutations[ii] + 1 << "_";
//    }
//    statOut << "\n";
//
//    statOut.close();                          // закрываем файл
//  }
}


// ------------------------------------------------------------------------------------------------
void TProcess::PrintResultToFile(TTrial OptimEstimation)
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

    //if (pTask->GetIsOptimumPointDefined() || pTask->GetIsOptimumValueDefined())
    //{
    //  fprintf(pf, "Global optimum %s\n", (PointDifference < parameters.Epsilon ||
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

    fprintf(pf, "Global optimum %s\n", (res) ? "FOUND!" : "NOT FOUND");


    NumberOfTrials = pMethod->GetNumberOfTrials();
    pMethod->PrintSection();

    fprintf(pf, "\nNumberOfTrials = %d\n", NumberOfTrials);
    for (int i = 0; i < pTask->GetNumOfFunc(); i++)
      fprintf(pf, "Number of calculations function %d of = %d\n", i,
        pMethod->GetFunctionCalculationCount()[i]);

    fprintf(pf, "\nLocalPointCount = %d\n", pMethod->GetLocalPointCount());
    fprintf(pf, "\nNumberLocalMethodtStart = %d\n", pMethod->GetNumberLocalMethodtStart());
    //if (parameters.printAdvancedInfo)
    //{
    //  fprintf(pf, "\n");
    //  for (int i = 0; i < pTask->GetNumOfFunc(); i++)
    //    fprintf(pf, "M estimation for function %d = %f\n", i, pMethod->GetM()[i]);
    //  fprintf(pf, "\nOptimum index: %i\n\n", OptimEstimation.index);
    //  for (int i = 0; i <= OptimEstimation.index; i++)
    //    fprintf(pf, "Function %d value in estimated optimum: %f\n", i, OptimEstimation.FuncValues[i]);
    //}
    fprintf(pf, "\nSolve time = %lf\n\n\n", duration);
    fclose(pf);
  }
}


// ------------------------------------------------------------------------------------------------
void TProcess::BeginIterations()
{
  IsOptimumFound = false;
  isPrintOptimEstimation = false;
  pMethod->FirstIteration();
}

// ------------------------------------------------------------------------------------------------
void TProcess::DoIteration()
{
  bool IsStop;

  ////if (parameters.GetProcRank() == 0) printf("AA1! %d\n", parameters.GetProcRank());

  //  проверяем критерий остановки
  IsStop = pMethod->CheckStopCondition();
  IsOptimumFound = IsStop;

  if (IsOptimumFound)
    isPrintOptimEstimation = true;

  IsOptimumFound = CheckIsStop(IsOptimumFound);
  IsStop = IsOptimumFound;

  //if (pTask->GetProcLevel() == 0)  printf("CC-1A! Proc=%d iter=%d PL=%d IsStop=%d\n", parameters.GetProcRank(), this->pMethod->GetIterationCount(), (parameters.MapProcInLevel[pTask->GetProcLevel()] - 1), IsStop);

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

      //if (pTask->GetProcLevel() == 0)
      //{
      //  print << "IterationCount = " << pMethod->GetIterationCount() << "\n";
      //}

      for (int j = 0; j < pTask->GetNumOfFunc(); j++)
      {
        functionCalculationCount[j] += pMethod->GetFunctionCalculationCount()[j];
      }
      ////if (parameters.GetProcRank() == 0) printf("AA9! %d\n", parameters.GetProcRank());
      //Провести оценку оптимума нужно до обновления данных,  т.к. в этой функции может быть поднят флаг recalc
      pMethod->EstimateOptimum();
      //Все случаи поднятия флага recalc обработаны, можно обновлять базу
      ////if (parameters.GetProcRank() == 0) printf("AA10! %d\n", parameters.GetProcRank());
      pMethod->RenewSearchData();
      //на первой итерации проводим сепарабельную оптимизацию
      //if (pMethod->GetNumberOfTrials() == 1)
      //{
      //  if (parameters.sepS)
      //    pMethod->SeparableSearch();
      //  if (parameters.rndS)
      //    pMethod->RandomSearh();
      //}
      ////if (parameters.GetProcRank() == 0)     printf("AA20! %d\n", parameters.GetProcRank());
      //if ((parameters.localVerificationType == IntegratedOnePoint ||
      //  parameters.localVerificationType == IntegratedManyPoints) && isNewOptimumFound)
      //  pMethod->LocalSearch();

      pMethod->FinalizeIteration();

    }
    else
    {
      IsOptimumFound = true;
      if (parameters.TypeCalculation == 8)
        TMPICalculationAsync::AsyncFinilize();  
      pMethod->FinalizeIteration();
    }
  }
  catch (const TException & e)
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
bool TProcess::CheckIsStop(bool IsStop)
{
  bool f = IsStop;

  MPI_Status status;

  if (pTask->GetProcLevel() == 0)
  {
    //if (pTask->GetProcLevel() == 0)  printf("CC30! Proc=%d iter=%d PL=%d IsStop=%d\n", parameters.GetProcRank(), this->pMethod->GetIterationCount(), (parameters.MapProcInLevel[pTask->GetProcLevel()] - 1), IsStop);

    if ((parameters.MapProcInLevel[pTask->GetProcLevel()] - 1) > 0)
    {
      for (int k = 0; k < Neighbours.size(); k++)
      {
        MPI_Send(&f, 1, MPI_CHAR, Neighbours[k], TagChildSolved, MPI_COMM_WORLD);
      }
      //цикл по нашим соседям
      for (int k = 0; k < Neighbours.size(); k++)
      {
        MPI_Recv(&f, 1, MPI_CHAR, Neighbours[k], TagChildSolved, MPI_COMM_WORLD, &status);

        if (isPrintOptimEstimation && f && (parameters.GetProcRank() > Neighbours[k]))
          isPrintOptimEstimation = false;

        IsStop = IsStop || f;
      }
    }

    //if (pTask->GetProcLevel() == 0)  printf("CC33! Proc=%d iter=%d PL=%d IsStop=%d\n", parameters.GetProcRank(), this->pMethod->GetIterationCount(), (parameters.MapProcInLevel[pTask->GetProcLevel()] - 1), IsStop);
  }

  return IsStop;
}


// ------------------------------------------------------------------------------------------------
void TProcess::EndIterations()
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

// - end of file ----------------------------------------------------------------------------------

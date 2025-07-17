/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2021 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      mpi_calculation.cpp                                         //
//                                                                         //
//  Purpose:   Source file for MPI calculation class                       //
//                                                                         //
//  Author(s): Lebedev I.                                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "mpi_calculation.h"

#include <stdlib.h>
#include <string.h>
#include <cstring>



#include "TaskFactory.h"
#include "TrialFactory.h"
#include "omp_calculation.h"

void TMPICalculation::StartCalculate(TInformationForCalculation& inputSet,
  TResultForCalculation& outputSet)
{
  for (unsigned int i = 0; i < parameters.GetProcNum() - 1; i++)
  {
    int isFinish = 0;
    //Отправляем в Solver флаг, что мы работаем
    MPI_Send(&isFinish, 1, MPI_INT, i + 1, TagChildSolved, MPI_COMM_WORLD);

    //Отправляем несколько точек на процессы
    for (unsigned int j = 0; j < parameters.mpiBlockSize; j++) {
      TTrial* trail = inputSet.trials[i*(parameters.mpiBlockSize) + j];
      trail->index = -1;
      //Отправляем координату y
      MPI_Send(trail->y, parameters.Dimension, MPI_DOUBLE, i + 1, TagChildSolved, MPI_COMM_WORLD);
    }
  }

  MPI_Status status;
  for (unsigned int i = 0; i < parameters.GetProcNum() - 1; i++)
  {
    //Принимаем все отправленные точки обратно
    for (unsigned int j = 0; j < parameters.mpiBlockSize; j++) {
      TTrial* trail = inputSet.trials[i*(parameters.mpiBlockSize) + j];
      trail->index = -1;

      //Принимаем вычисленное значение функции из Solver
      MPI_Recv(trail->FuncValues, MaxNumOfFunc, MPI_DOUBLE, i + 1, TagChildSolved, MPI_COMM_WORLD, &status);

      int fNumber = 0;
      while ((trail->index == -1) && (fNumber < pTask->GetNumOfFunc()))
      {
        if ((fNumber == (pTask->GetNumOfFunc() - 1)) || (trail->FuncValues[fNumber] > 0))
        {
          trail->index = fNumber;
        }
        fNumber++;
      }
    }
  }

  for (unsigned int i = 0; i < inputSet.trials.size(); i++)
  {
    for (int j = 0; j <= outputSet.trials[i]->index; j++)
      outputSet.countCalcTrials[j]++;
  }
}

// ------------------------------------------------------------------------------------------------
void TMPICalculation::StartCalculateInBorder(TInformationForCalculation& inputSet,
  TResultForCalculation& outputSet)
{
  //for (unsigned int i = 0; i < 2; i++)
  //{
  //  int isFinish = 0;
  //  //Отправляем в Solver флаг, что мы работаем
  //  MPI_Send(&isFinish, 1, MPI_INT, i + 1, TagChildSolved, MPI_COMM_WORLD);


  //  TTrial* trail = inputSet.trials[i];
  //  trail->index = -1;
  //  //Отправляем координату y
  //  MPI_Send(trail->y, parameters.Dimension, MPI_DOUBLE, i + 1, TagChildSolved, MPI_COMM_WORLD);
  //}

  //MPI_Status status;
  //for (unsigned int i = 0; i < 2; i++)
  //{
  //  //Принимаем все отправленные точки обратно

  //  TTrial* trail = inputSet.trials[i];
  //  trail->index = -1;

  //  //Принимаем вычисленное значение функции из Solver
  //  MPI_Recv(trail->FuncValues, MaxNumOfFunc, MPI_DOUBLE, i + 1, TagChildSolved, MPI_COMM_WORLD, &status);

  //  int fNumber = 0;
  //  while ((trail->index == -1) && (fNumber < pTask->GetNumOfFunc()))
  //  {
  //    if ((fNumber == (pTask->GetNumOfFunc() - 1)) || (trail->FuncValues[fNumber] > 0))
  //    {
  //      trail->index = fNumber;
  //    }
  //    fNumber++;
  //  }
  //}

  TCalculation* calculation;
  calculation = new TOMPCalculation(*pTask);
  calculation->Calculate(inputSet, outputSet);

  for (unsigned int i = 0; i < inputSet.trials.size(); i++)
  {
    for (int j = 0; j <= outputSet.trials[i]->index; j++)
      outputSet.countCalcTrials[j]++;
  }
}


// ------------------------------------------------------------------------------------------------
void TMPICalculation::Calculate(TInformationForCalculation& inputSet,
  TResultForCalculation& outputSet)
{

  if (inputSet.trials.size() > 0)
  {
    outputSet.trials.clear();
    outputSet.trials.resize(inputSet.trials.size());
    for (unsigned i = 0; i < outputSet.trials.size(); i++)
      outputSet.trials[i] = inputSet.trials[i];


    outputSet.countCalcTrials.clear();
    outputSet.countCalcTrials.resize(pTask->GetNumOfFunc());
    for (int i = 0; i < pTask->GetNumOfFunc(); i++)
      outputSet.countCalcTrials[i] = 0;

    outputSet.tasks.clear();
    outputSet.tasks.resize(inputSet.tasks.size());
    for (unsigned i = 0; i < inputSet.tasks.size(); i++)
      outputSet.tasks[i] = inputSet.tasks[i];
  }

  // Запускать вычисления как только пришли данные
  if (isStartComputingAway)
  {
    if ((isFirst) && ((parameters.isCalculationInBorderPoint == true) || (parameters.LocalTuningType != 0)))
    {
      isFirst = false;
      StartCalculateInBorder(inputSet, outputSet);
    }
    else
      StartCalculate(inputSet, outputSet);
  }
  else//собрать данные в один блок, и потом вычислить все сразу
  {
    if (countCalculation > 0)
    {
      countCalculation--;

      for (unsigned i = 0; i < outputSet.trials.size(); i++)
      {
        inputCalculation.trials.push_back(inputSet.trials[i]);
        inputCalculation.tasks.push_back(inputSet.tasks[i]);
      }

      if (countCalculation > 0)
        firstCalculation->ContinueComputing();

      for (unsigned int i = 0; i < inputSet.trials.size(); i++)
      {
        for (int j = 0; j <= outputSet.trials[i]->index; j++)
          outputSet.countCalcTrials[j]++;
      }
    }

    if (countCalculation == 0)
    {
      countCalculation--;
      isStartComputingAway = true;

      resultCalculation.Resize(inputCalculation.tasks.size());

      for (unsigned i = 0; i < resultCalculation.trials.size(); i++)
        resultCalculation.trials[i] = inputCalculation.trials[i];

      StartCalculate(inputCalculation, resultCalculation);
    }
  }

}

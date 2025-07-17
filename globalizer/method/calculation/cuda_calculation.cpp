/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      cuda_calculation.cpp                                        //
//                                                                         //
//  Purpose:   Source file for CUDA calculation class                      //
//                                                                         //
//  Author(s): Lebedev I.                                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "cuda_calculation.h"


void TCUDACalculation::StartCalculate(TInformationForCalculation& inputSet,
  TResultForCalculation& outputSet)
{
  int newCoordinatesSize = inputSet.trials.size() * parameters.Dimension;
  if (newCoordinatesSize > coordinatesSize)
  {
    coordinatesSize = newCoordinatesSize;
    coordinates = new double [coordinatesSize];
  }

  int newFuncValuesSize = inputSet.trials.size() * pTask->GetNumOfFunc();
  if (newFuncValuesSize > FuncValuesSize)
  {
    FuncValuesSize = newFuncValuesSize;
    FuncValues = new double [FuncValuesSize];
  }

  for (unsigned i = 0; i < inputSet.trials.size(); i++)
  {
    for (int k = 0; k < parameters.Dimension; k++)
      coordinates[i * parameters.Dimension + k] = inputSet.trials[i]->y[k];
  }

  pTask->CalculateFuncsInManyPoints(coordinates, 0, int(inputSet.trials.size()), FuncValues);
  //pTask->CalculateFuncsInManyPoints(coordinates, pTask->GetNumOfFunc(), int(inputSet.trials.size()), FuncValues);

  for (unsigned i = 0; i < inputSet.trials.size(); i++)
  {
    outputSet.trials[i] = inputSet.trials[i];
    for (int k = 0; k < pTask->GetNumOfFunc(); k++)
      outputSet.trials[i]->FuncValues[k] = FuncValues[i + k * pTask->GetNumOfFunc()];
    outputSet.trials[i]->index = 0;
  }


  for (unsigned int i = 0; i < inputSet.trials.size(); i++)
  {
    for (int j = 0; j <= outputSet.trials[i]->index; j++)
      outputSet.countCalcTrials[j]++;
  }
}

// ------------------------------------------------------------------------------------------------
void TCUDACalculation::Calculate(TInformationForCalculation& inputSet,
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

  /// Запускать вычисления как только пришли данные
  if (isStartComputingAway)
  {
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
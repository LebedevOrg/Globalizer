/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      omp_calculation.cpp                                         //
//                                                                         //
//  Purpose:   Source file for OpenMP calculation class                    //
//                                                                         //
//  Author(s): Lebedev I.                                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "omp_calculation.h"

#include <stdlib.h>
#include <string.h>
#include <cmath>

void TOMPCalculation::StartCalculate(TInformationForCalculation& inputSet,
  TResultForCalculation& outputSet)
{
  int np = inputSet.trials.size();

#pragma omp parallel for num_threads((int)parameters.NumThread)
  for (int i = 0; i < np; i++)
  {
    TTrial* trail = inputSet.trials[i];
    if (trail != 0)
    {
      trail->index = -1;

      int fNumber = 0;

      while ((trail->index == -1) && (fNumber < pTask->GetNumOfFunc()))
      {
        trail->FuncValues[fNumber] = pTask->CalculateFuncs(trail->y, fNumber);

#ifdef WIN32
        if (!_finite(trail->FuncValues[fNumber]))
#else
        if (!std::isfinite(trail->FuncValues[fNumber]))
#endif
        {
          //throw EXCEPTION("Infinite trail->FuncValues[fNumber]!");
          trail->index = -2;
          std::cout << " CalculateFuncs Error!!!\n";
        }
        else
          if ((fNumber == (pTask->GetNumOfFunc() - 1)) || (trail->FuncValues[fNumber] > 0))
          {
            trail->index = fNumber;
            if (trail->FuncValues[fNumber] >= 1.7e308)
              trail->index = -3;
          }
        fNumber++;
      }
    }
  }

  for (unsigned int i = 0; i < outputSet.trials.size(); i++)
  {
    if (outputSet.trials[i] != 0)
    {
      for (int j = 0; j <= outputSet.trials[i]->index; j++)
        outputSet.countCalcTrials[j]++;
    }
  }
}

// ------------------------------------------------------------------------------------------------
void TOMPCalculation::Calculate(TInformationForCalculation& inputSet,
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

      for (unsigned int i = 0; i < outputSet.trials.size(); i++)
      {
        if (outputSet.trials[i] != 0)
        {
          for (int j = 0; j <= outputSet.trials[i]->index; j++)
            outputSet.countCalcTrials[j]++;
        }
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

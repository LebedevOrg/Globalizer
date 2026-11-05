

#include "CPUMultiPointCalculation.h"
#include <stdlib.h>
#include <string.h>
#include <cmath>

// ------------------------------------------------------------------------------------------------
void CPUMultiPointCalculation::StartCalculate(InformationForCalculation& inputSet,
                      TResultForCalculation& outputSet)
  {
    const int dim = parameters.Dimension;
    const int numOfFunc = pTask->GetNumOfFunc();
    const int numPoints = static_cast<int>(inputSet.trials.size());

    if (numPoints <= 0)
      return;

    // --- Перераспределение памяти под буферы при необходимости ---------------
    const int newCoordinatesSize = numPoints * dim;
    if (newCoordinatesSize > coordinatesSize)
    {
      delete[] coordinates;
      coordinatesSize = newCoordinatesSize;
      coordinates = new double[coordinatesSize];
    }

    const int newFuncValuesSize = numPoints * numOfFunc;
    if (newFuncValuesSize > FuncValuesSize)
    {
      delete[] FuncValues;
      FuncValuesSize = newFuncValuesSize;
      FuncValues = new double[FuncValuesSize];
    }

    // --- Сборка координат всех точек в один непрерывный буфер -----------------
    // Раскладка: coordinates[i * dim + k] = trials[i]->y[k]
    for (size_t i = 0; i < numPoints; i++)
    {
      const double* src = inputSet.trials[i]->y;
      double* dstRow = coordinates + i * dim;
      for (int k = 0; k < dim; k++)
        dstRow[k] = src[k];
    }

    // --- Один вызов задачи на весь блок точек ---------------------------------
    // Внутри задача векторизует вычисления (интринсики) через
    // CalculateFunctionals(y, fNumber, numPoints, values).
    pTask->CalculateFuncsInManyPoints(coordinates, 0, numPoints, FuncValues);

    // --- Раскладка результатов обратно в объекты Trial ------------------------
    // Раскладка сохранена идентичной CUDACalculation для совместимости.
    for (int i = 0; i < numPoints; i++)
    {
      outputSet.trials[i] = inputSet.trials[i];
      for (int k = 0; k < numOfFunc; k++)
      {
        outputSet.trials[i]->FuncValues[k] = FuncValues[i + k * numOfFunc];

#ifdef WIN32
        if (!_finite(outputSet.trials[i]->FuncValues[k]))
#else
        if (!std::isfinite(outputSet.trials[i]->FuncValues[k]))
#endif
        {
          outputSet.trials[i]->index = -2;
          //std::cout << " CalculateFuncs Error!!!\n";
        }
        else  if ((k == (pTask->GetNumOfFunc() - 1)) || (outputSet.trials[i]->FuncValues[k] > 0))
        {
          outputSet.trials[i]->index = k;
          if (outputSet.trials[i]->FuncValues[k] >= 1.7e308)
            outputSet.trials[i]->index = -3;
        }
      }
    }

    // --- Подсчёт количества вычислений ----------------------------------------
    for (int i = 0; i < numPoints; i++)
    {
      for (int j = 0; j <= outputSet.trials[i]->index; j++)
        outputSet.countCalcTrials[j]++;
    }
  }

  // ------------------------------------------------------------------------------------------------
  CPUMultiPointCalculation::CPUMultiPointCalculation(Task& _pTask) : Calculation(_pTask)
  {
    coordinates = nullptr;
    FuncValues = nullptr;
    coordinatesSize = 0;
    FuncValuesSize = 0;
  }

  // ------------------------------------------------------------------------------------------------
  CPUMultiPointCalculation::~CPUMultiPointCalculation()
  {
    delete[] coordinates;
    delete[] FuncValues;
    coordinates = nullptr;
    FuncValues = nullptr;
    coordinatesSize = 0;
    FuncValuesSize = 0;
  }

  // ------------------------------------------------------------------------------------------------
  void CPUMultiPointCalculation::Calculate(InformationForCalculation& inputSet,
                         TResultForCalculation& outputSet)
  {
    const int numOfFunc = pTask->GetNumOfFunc();

    if (inputSet.trials.size() > 0)
    {
      if (outputSet.trials.size() != inputSet.trials.size())
      {
        outputSet.trials.clear();
        outputSet.trials.resize(inputSet.trials.size());
      }
      for (unsigned i = 0; i < outputSet.trials.size(); i++)
        outputSet.trials[i] = inputSet.trials[i];

      if (outputSet.countCalcTrials.size() != numOfFunc)
      {
        outputSet.countCalcTrials.clear();
        outputSet.countCalcTrials.resize(numOfFunc);
      }
      for (int i = 0; i < numOfFunc; i++)
        outputSet.countCalcTrials[i] = 0;

    }

    // Запускать вычисления, как только пришли данные
    if (isStartComputingAway)
    {
      StartCalculate(inputSet, outputSet);
    }
    else // Сначала собрать блок точек, потом посчитать все разом
    {
      if (countCalculation > 0)
      {
        countCalculation--;

        for (unsigned i = 0; i < outputSet.trials.size(); i++)
          inputCalculation.trials.push_back(inputSet.trials[i]);

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

        for (unsigned i = 0; i < resultCalculation.trials.size(); i++)
          resultCalculation.trials[i] = inputCalculation.trials[i];

        StartCalculate(inputCalculation, resultCalculation);
      }
    }
  }

 // ------------------------------------------------------------------------------------------------
 void CPUMultiPointCalculation::Reset()
  {
    Calculation::Reset();
    // Буферы оставляем выделенными — они переиспользуются между итерациями.
  }

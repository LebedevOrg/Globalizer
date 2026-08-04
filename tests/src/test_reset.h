#pragma once
#include "Parameters.h"
#include "Calculation.h"
#include "Extended.h"

// Полный сброс кэша вычислителей фабрики (владелец — фабрика).
inline void ResetCalculationStatics()
{
  if (Calculation::leafCalculation)
  {
    delete Calculation::leafCalculation;
    Calculation::leafCalculation = nullptr;
  }
  if (Calculation::firstCalculation)
  {
    delete Calculation::firstCalculation;
    Calculation::firstCalculation = nullptr;
  }
  // ВНИМАНИЕ: сбрасывать значащие статические поля через фабричный контракт нельзя
  // напрямую — они protected. Их сбрасывают наследники/друзья (см. CalculationTest).
}

// Приводит глобальные parameters к валидным значениям по умолчанию для тестов метода.
inline void ResetParametersToMethodDefaults(int dimension)
{
  Extended::SetTypeID(etDouble);

  parameters.Dimension = dimension;
  parameters.MapType = mpBase;
  parameters.TypeMethod = StandartMethod;
  parameters.TypeCalculation = OMP;
  parameters.Epsilon = 0.01;
  parameters.r = 2.3;
  parameters.rEps = 0.001;
  parameters.rDynamic = 0.0;
  parameters.NumPoints = 1;
  parameters.NumThread = 1;
  parameters.MaxNumOfPoints = 10000;
  parameters.m = 10;
  parameters.LocalRefineSolution = None;
  parameters.LocalTuningType = WithoutLocalTuning;
  parameters.IsCalculationInBorderPoint = false;
  parameters.LocalMix = 0;
  parameters.StopCondition = Accuracy;
  parameters.IsUseStartPoint = false;
  parameters.IsLoadFirstPointFromFile = false;
  parameters.IsSerializeToDashBoard = false;
  parameters.FileSerializer = "";
  parameters.IterPointsSavePath = "";
  parameters.IsPrintSectionPoint = false;
  parameters.MaxIterationsWithoutImprovement = 100;
}
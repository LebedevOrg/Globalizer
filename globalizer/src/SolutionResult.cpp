#include "SolutionResult.h"
#include "Trial.h"

/// Значение целевой функции в найденной точке
double SolutionResult::GetBestValue() const
{
  if (!BestTrial) return 0.0;
  // BestTrial->index == номер последнего вычисленного функционала
  // (для задачи без ограничений == 0, с ограничениями == numConstraints)
  return BestTrial->FuncValues[BestTrial->index];
}

/// Координаты найденной точки (массив размерности задачи)
const double* SolutionResult::GetBestPoint() const
{
  return BestTrial ? BestTrial->y : nullptr;
}

/// Число итераций решателя
int SolutionResult::GetIterationsCount() const
{
  return IterationCount;
}
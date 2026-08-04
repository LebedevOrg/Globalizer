#ifndef __SOLUTION_RESULT_H__
#define __SOLUTION_RESULT_H__

#include "Common.h"
#include "SearchData.h"
/**
Результаты работы системы
*/
struct SolutionResult
{
  /// Лучшая итерация, полученная при данном запуске метода
  Trial* BestTrial;
  /// число выполненных итераций
  int IterationCount;
  /// количество испытаний
  int TrialCount;
  /// Время решения задачи
  double SolvingTime;
};

#endif

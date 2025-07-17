#ifndef __SOLUTION_RESULT_H__
#define __SOLUTION_RESULT_H__

#include "common.h"
#include "SearchData.h"
/**
Результаты работы системы
*/
struct TSolutionResult
{
  /// Лучшая итерация, полученная при данном запуске метода
  TTrial* BestTrial;
  /// число выполненных итераций
  int IterationCount;
  /// количество испытаний
  int TrialCount;
};

#endif

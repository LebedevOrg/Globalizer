#ifndef __SOLUTION_RESULT_H__
#define __SOLUTION_RESULT_H__

#include "Common.h"
#include "SearchData.h"
/**
Результаты работы системы
*/
class SolutionResult
{
public:
  /// Лучшая итерация, полученная при данном запуске метода
  Trial* BestTrial;
  /// число выполненных итераций
  int IterationCount;
  /// количество испытаний
  int TrialCount;
  /// Время решения задачи
  double SolvingTime;

  // -----------------------------------------------------------------------
  // Вспомогательные методы доступа (добавлены для Python-интерфейса)
  // -----------------------------------------------------------------------

  /// Значение целевой функции в найденной точке
  double GetBestValue() const;
  /// Координаты найденной точки (массив размерности задачи)
  const double* GetBestPoint() const;

  /// Число итераций решателя
  int GetIterationsCount() const;
};

#endif

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      process.h                                                   //
//                                                                         //
//  Purpose:   Header file for optimization process class                  //
//                                                                         //
//  Author(s): Sysoyev A.                                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "SearchData.h"
#include "method_interface.h"
#include "task.h"
#include "exception.h"
#include "parameters.h"
#include "performance.h"
#include "problem_interface.h"
#include "parallel_tree.h"
#include "evolvent.h"
#include "CalculationFactory.h"

//extern const int MaxNumOfTaskLevels;

// ------------------------------------------------------------------------------------------------
class TProcess
{
protected:
  /// Печатать ли выходную информацию
  bool isPrintOptimEstimation;
  /// Общие данные для всех процессов
  bool isFirstRun;

  /// Наш таймер
  TPerformance Timer;
  /// время решения задачи
  double duration;

  /// Решилась ли задача
  bool IsOptimumFound;
  /// Задача
  TTask* pTask;
  /// Поисковая информация
  TSearchData* pData;

  /// Методы для одной итерации
  IMethod* pMethod;
  /** Указатель на развертку

  В зависимости от вида отображения это может быть:
  - единственная развертка
  - множественная сдвиговая развертка
  - множественная вращаемая развертка
  */
  TEvolvent* Evolvent;
  /// Вычислитель
  TCalculation* Calculation;

  std::vector<int> Neighbours;

  /// Число вычисленных значений каждой функции
  std::vector<int> functionCalculationCount;

  /// печать текущего минимума в файл
  void PrintOptimEstimationToFile(TTrial OptimEstimation);
  /// печать текущего минимума на экран
  virtual void PrintOptimEstimationToConsole(TTrial OptimEstimation);
  /// Печать результата в файл
  virtual void PrintResultToFile(TTrial OptimEstimation);

  /// Предварительные настройки, запускается только при первом запуске
  virtual void BeginIterations();
  /// Одна итерация
  virtual void DoIteration();
  /// Окончание работы
  virtual void EndIterations();
  /// Место в дереве процессов
  int GetProcLevel() { return pTask->GetProcLevel(); }
  ///Проверяет остановились ли соседи
  bool CheckIsStop(bool IsStop);
public:
  TProcess(TSearchData& data, TTask& task);
  virtual ~TProcess();
  /// Время решения
  double GetSolveTime();
  /// Запуск решения задачи
  void Solve();

  /// Сброс параметров процесса
  void Reset(TSearchData* data, TTask* task);

  /** Получить число испытаний

  \return число испытаний
  */
  virtual int GetIterationCount() { return pMethod->GetIterationCount(); }
  int GetNumberOfTrials() { return pMethod->GetNumberOfTrials(); }

  /** Получить текущую оценку оптимума

  \return испытание, соответствующее текущему оптимуму
  */
  virtual TTrial* GetOptimEstimation() { return pMethod->GetOptimEstimation(); }

};

void ShowIterResults(TProcess *pProcess);

#endif
// - end of file ----------------------------------------------------------------------------------

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
#include "MethodInterface.h"
#include "Task.h"
#include "Exception.h"
#include "Parameters.h"
#include "Performance.h"
#include "ProblemInterface.h"
#include "EvolventInterface.h"
//#include "TLinearEvolvent.h"
//#include "TNoninjectiveEvolvent.h"
//#include "TRotatedEvolvent.h"
//#include "TShiftedEvolvent.h"
//#include "TSmoothEvolvent.h"
#include "CalculationFactory.h"

class Solver;
//extern const int MaxNumOfTaskLevels;

// ------------------------------------------------------------------------------------------------
class Process
{
  friend class Solver;

protected:
  /// Печатать ли выходную информацию
  bool isPrintOptimEstimation;
  /// Общие данные для всех процессов
  bool isFirstRun;

  /// Наш таймер
  Performance Timer;
  /// время решения задачи
  double duration;

  /// Решилась ли задача
  bool IsOptimumFound;
  /// Задача
  Task* pTask;
  /// Поисковая информация
  SearchData* pData;

  /// Методы для одной итерации
  IMethod* pMethod;
  /** Указатель на развертку

  В зависимости от вида отображения это может быть:
  - единственная развертка
  - множественная сдвиговая развертка
  - множественная вращаемая развертка
  */
  IEvolvent* pEvolvent;
  /// Вычислитель
  Calculation* calculation;

  std::vector<int> Neighbours;

  /// Число вычисленных значений каждой функции
  std::vector<int> functionCalculationCount;

  /// печать текущего минимума в файл
  void PrintOptimEstimationToFile(Trial OptimEstimation);
  /// печать текущего минимума на экран
  virtual void PrintOptimEstimationToConsole(Trial OptimEstimation);
  /// Старый вариант печати в консоль
  virtual void OldPrintOptimEstimationToConsole(Trial OptimEstimation);
  /// Печать результата в файл
  virtual void PrintResultToFile(Trial OptimEstimation);

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

  /// Точки которые будут добавлены после первой итерации
  std::vector<Trial*>* addPoints;
public:
  Process(SearchData& data, Task& task);
  virtual ~Process();
  /// Время решения
  double GetSolveTime();
  /// Запуск решения задачи
  void Solve();

  /// Сброс параметров процесса
  void Reset(SearchData* data, Task* task);

  /** Получить число испытаний

  \return число испытаний
  */
  virtual int GetIterationCount() { return pMethod->GetIterationCount(); }
  int GetNumberOfTrials() { return pMethod->GetNumberOfTrials(); }

  /** Получить текущую оценку оптимума

  \return испытание, соответствующее текущему оптимуму
  */
  virtual Trial* GetOptimEstimation() { return pMethod->GetOptimEstimation(); }


  /**Добавляет испытания в поисковую информацию, при этом обновляя константу Гёльдера и
  оценку оптимума

  \param[in] points точки испытаний, которые будут добавлены
  */
  void InsertPoints(std::vector<Trial*>& points);
};

void ShowIterResults(Process *pProcess);

#endif
// - end of file ----------------------------------------------------------------------------------

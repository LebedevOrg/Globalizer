#ifndef __SOLVER_H__
#define __SOLVER_H__

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <mpi.h>
#include <exception>

#include "Common.h"
#include "GlobProcess.h"
#include "Exception.h"
#include "InitProblem.h"
#include "OutputSystem.h"
#include "Messages.h"
#include "SolutionResult.h"
#include "SolverInterface.h"
#include "GlobalizerBenchmarksProblem.h"

#ifdef _GLOBALIZER_BENCHMARKS
#include "IGlobalOptimizationProblem.h"
#include "GlobalOptimizationProblemManager.h"
#endif // _GLOBALIZER_BENCHMARKS


/**
 Базовые классы для решения задач глобальной оптимизации
**/

class Solver : public ISolver
{
protected:
  ///Процесс решающий задачу
  Process* mProcess;
  /// Задача
  IProblem* mProblem;

  /// Общее описание задачи
  Task* pTask;
  /// Задача пораждена в Solver или пришла извне
  bool isExternalTask;
  /// База данных(поисковая информация)
  SearchData* pData;

  /// Результат работы системы
  SolutionResult* result;

  /// Флаг для контроля состояния
  bool m_initialized;

  /// Очистить данные
  virtual void ClearData();
  /** Инициализация чисел с расширенной точностью

  Функция автоматически выбирает используемый тип данных в зависимости от размерности решаемой
  задачи и плотности развертки
  */
  virtual void InitAutoPrecision();
  /// Создание процесса и всего остального
  virtual int CreateProcess();
  /// Проверяет что параметры солвера соответствуют решаемой задаче
  int CheckParameters();

  /// Решатель производящий вычисление при параллельном АГП (распараллеливание по точкам) на MPI
  void MpiCalculation();
  /// Решатель используемый при ассинхронной схеме
  void AsyncCalculation();
  /// Точки которые будут добавлены после первой итерации
  std::vector<Trial*>* addPoints;

public:
  Solver(IProblem* problem);

#ifdef _GLOBALIZER_BENCHMARKS

  Solver(IGlobalOptimizationProblem* problem);

#endif

  /// Решение задачи по умолчанию
  virtual int Solve();
  /// Решение подзадачи с указанными параметрами
  virtual int Solve(Task* task);

  virtual ~Solver();

  /// Задает задачу для решения
  void SetProblem(IProblem* problem);

  /// Возвращает решаемую задачу
  IProblem* GetProblem();

  /// Возвращает полученное решение
  SolutionResult* GetSolutionResult();

  /// Добавляет точки испытаний
  virtual void SetPoint(std::vector<Trial*>& points);

  /// Возврящает все имеющиеся точки испытаний
  virtual std::vector<Trial*>& GetAllPoint();

  /// Возвращает задачу решателя
  Task* GetTask();

  /// Возвращает поисковую информацию
  SearchData* GetData();

  /// Выполняет автоматическую настройку параметров решателя
  virtual void AutoConfig();

  /// Запуск решателя для инициализации, загрузки или выполнения начальной итерации
  int Initialize();

  /// Выполнение очередного шага оптимизации
  int DoIteration(bool& finished);
};

#endif //solver.h

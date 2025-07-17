#ifndef __SOLVER_H__
#define __SOLVER_H__

#define _CRT_SECURE_NO_WARNINGS

#include <mpi.h>
#include <exception>

#include "common.h"
#include "GlobProcess.h"
#include "exception.h"
#include "init_problem.h"
#include "output_system.h"
#include "messages.h"
#include "parallel_tree.h"
#include "solution_result.h"
#include "SolverInterface.h"


/**
* Базовые классы для решения задач глобальной оптимизации
**/
// ------------------------------------------------------------------------------------------------



// ------------------------------------------------------------------------------------------------
/**
Базового класса
*/
class TSolver : public ISolver
{
protected:
  ///Процесс решающий задачу
  TProcess* mProcess;
  /// Задача
  IProblem* mProblem;

  /// Общее описание задачи
  TTask* pTask;
  /// База данных(поисковая информация)
  TSearchData* pData;

  /// Дерева распараллеливания
  //TParallelTree* parTree;
  /// Результат работы системы
  TSolutionResult* result;

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
  /// Решатель производящий вычисления в параллельной блочной многошаговой схеме на mpi
  void BlockSchemeCalculation();
  /// Решатель производящий вычисление при параллельном АГП (распараллеливание по точкам) на MPI
  void MpiCalculation();
  /// Решатель используемый при ассинхронной схеме
  void AsyncCalculation();
  /// Решатель производящий предворительное разбиение параметров на локальные и глобальные, после чего происходит запуск синхронной блочной многошаговой схемы
  void SeparationVariablesSolver();
public:
  TSolver(IProblem* problem);

  /// Решение задачи по умолчанию
  virtual int Solve();
  /// Решение подзадачи с указанными параметрами
  virtual int Solve(TTask* task);
  virtual ~TSolver();

  void SetProblem(IProblem* problem);
  IProblem* GetProblem();
  TSolutionResult* GetSolutionResult();
};


class TSeparableSolver : public TSolver
{
public:
  TSeparableSolver(IProblem* problem) : TSolver(problem)
  {  }

  //TTask * CreateTask(int taskNumber)
  //{
  //  TTask * pTask = new TTask(params->Dimension, params->DimInTask[ProcRank], problem);

  //  return pTask;
  //}
  virtual int Solve();
  virtual int CreateProcess(TProcess** process, IProblem* problem);
};
//
//class TSequentialSolver : public TSolver
//{
//public:
//  TSequentialSolver(IProblem* problem, TParameters& params) : TSolver(problem, params)
//  {
//  }
//};


#endif //solver.h
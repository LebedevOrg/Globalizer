/**
\file MCOSolver.h
\brief Решатель многокритериальных задач
*/

#ifndef __MCOSOLVER_H__
#define __MCOSOLVER_H__

#include "MCOTask.h"
#include "Solver.h"

/**
Решает многокритериальную задачу серией минимаксных сверток.

Пример:
\code
MCOParameters mco;
mco.numberOfLambdas = 50;
mco.rLocal = 1.5;
MCOSolver solver(problem, mco);
solver.Solve();
std::vector<MCOPoint> pareto = solver.GetParetoSet();
\endcode

Работает только в одном процессе; параметры r и Epsilon берутся из parameters.
*/
class GLOBALIZER_API MCOSolver : public Solver
{
protected:
  MCOParameters mcoParameters;
  MCOTask* mcoTask;

public:
  MCOSolver(IProblem* problem, const MCOParameters& _mcoParameters = MCOParameters());
  virtual ~MCOSolver();

  virtual int Solve();

  /// Итоги сверток в порядке их решения
  const std::vector<MCOSubproblem>& GetSubproblems() const;
  /// Все испытания в порядке их проведения
  std::vector<MCOPoint> GetTrials();
  /// Недоминируемые точки среди всех испытаний
  std::vector<MCOPoint> GetParetoSet();
};

#endif
// - end of file ----------------------------------------------------------------------------------

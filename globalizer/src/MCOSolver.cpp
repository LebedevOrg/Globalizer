#include "MCOSolver.h"

// ------------------------------------------------------------------------------------------------
MCOSolver::MCOSolver(IProblem* problem, const MCOParameters& _mcoParameters) :
  Solver(problem), mcoParameters(_mcoParameters), mcoTask(nullptr)
{
}

// ------------------------------------------------------------------------------------------------
MCOSolver::~MCOSolver()
{
  ClearData();
  delete mcoTask;
}

// ------------------------------------------------------------------------------------------------
int MCOSolver::Solve()
{
  if (parameters.GetProcNum() > 1)
    throw EXCEPTION("MCOSolver: only one process is supported");

  parameters.TypeMethod = MultiCriteriaMethod;

  delete mcoTask;
  mcoTask = new MCOTask(mProblem, mcoParameters);

  return Solver::Solve(mcoTask);
}

// ------------------------------------------------------------------------------------------------
const std::vector<MCOSubproblem>& MCOSolver::GetSubproblems() const
{
  return mcoTask->GetSubproblems();
}

// ------------------------------------------------------------------------------------------------
std::vector<MCOPoint> MCOSolver::GetTrials()
{
  const int convolutionIndex = mcoTask->GetConvolutionIndex();
  const std::vector<MCOSubproblem>& subproblems = mcoTask->GetSubproblems();

  std::vector<MCOPoint> points;
  int lambdaIndex = 0;
  int lambdaEnd = subproblems.empty() ? 0 : subproblems[0].newTrials;

  for (Trial* trial : pData->GetTrials())
  {
    if (trial->K == 0)
      continue; // граничные точки, в которых испытание не проводилось

    while (lambdaIndex + 1 < (int)subproblems.size() && (int)points.size() >= lambdaEnd)
      lambdaEnd += subproblems[++lambdaIndex].newTrials;

    MCOPoint point;
    point.y.assign(trial->y, trial->y + mcoTask->GetN());
    if (trial->index == convolutionIndex)
    {
      const double* criteria = trial->FuncValues + convolutionIndex + 1;
      point.criteria.assign(criteria, criteria + mcoTask->GetNumberOfCriteria());
    }
    point.lambdaIndex = lambdaIndex;
    points.push_back(point);
  }
  return points;
}

// ------------------------------------------------------------------------------------------------
std::vector<MCOPoint> MCOSolver::GetParetoSet()
{
  std::vector<MCOPoint> pareto;

  for (const MCOPoint& point : GetTrials())
  {
    if (point.criteria.empty())
      continue;

    bool isDominated = false;
    std::vector<MCOPoint> kept;
    for (const MCOPoint& other : pareto)
    {
      int better = 0;
      int worse = 0;
      for (size_t i = 0; i < point.criteria.size(); i++)
      {
        if (point.criteria[i] < other.criteria[i])
          better++;
        else if (point.criteria[i] > other.criteria[i])
          worse++;
      }
      if (better == 0)
      {
        isDominated = true;
        break;
      }
      if (worse > 0)
        kept.push_back(other);
    }

    if (!isDominated)
    {
      kept.push_back(point);
      pareto.swap(kept);
    }
  }
  return pareto;
}
// - end of file ----------------------------------------------------------------------------------

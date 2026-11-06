#include "MCOMethod.h"

#include "Exception.h"
#include "SearcDataIterator.h"

#include <algorithm>
#include <cmath>

// ------------------------------------------------------------------------------------------------
static MCOTask& ToMCOTask(Task& task)
{
  MCOTask* mcoTask = dynamic_cast<MCOTask*>(&task);
  if (mcoTask == 0)
    throw EXCEPTION("MCOMethod: task must be MCOTask, use MCOSolver");
  return *mcoTask;
}

// ------------------------------------------------------------------------------------------------
MCOMethod::MCOMethod(Task& _pTask, SearchData& _pData, Calculation& _Calculation, IEvolvent& _Evolvent) :
  Method(_pTask, _pData, _Calculation, _Evolvent), mcoTask(ToMCOTask(_pTask))
{
  convolutionIndex = mcoTask.GetConvolutionIndex();
  isRecalcConvolution = true;
  trialCount = 0;
  previousTrialCount = 0;

  rLocal = mcoTask.GetMCOParameters().rLocal;
  isDualR = rLocal > 0.0;
  rho = 1.0;
  if (isDualR)
  {
    if (rLocal <= 1.0)
      throw EXCEPTION("MCOMethod: rLocal is out of range");
    const double ratio = (1.0 - 1.0 / parameters.r) / (1.0 - 1.0 / rLocal);
    rho = ratio * ratio;
  }
}

// ------------------------------------------------------------------------------------------------
void MCOMethod::CalculateIterationPoints()
{
  if (isRecalcConvolution && iteration.IterationCount > 1)
    RecalcAllConvolution();

  Method::CalculateIterationPoints();
}

// ------------------------------------------------------------------------------------------------
void MCOMethod::RecalcAllConvolution()
{
  isRecalcConvolution = false;

  Trial* best = pData->GetBestTrial();
  if (best == 0 || best->index != convolutionIndex)
    return; // допустимых точек еще нет

  for (Trial* trial : pData->GetTrials())
    if (trial->index == convolutionIndex)
      trial->FuncValues[convolutionIndex] = mcoTask.Convolution(Criteria(trial));
  best->FuncValues[convolutionIndex] = mcoTask.Convolution(Criteria(best));

  // Как в iOpt, точка с нулевой сверткой лучшей не становится
  Trial* newBest = best;
  for (Trial* trial : pData->GetTrials())
  {
    if (trial->index == convolutionIndex && trial->FuncValues[convolutionIndex] > 0 &&
      trial->FuncValues[convolutionIndex] < newBest->FuncValues[convolutionIndex])
      newBest = trial;
  }
  if (newBest != best)
    pData->SetBestTrial(newBest->Clone());

  for (SearcDataIterator it = pData->GetBeginIterator(); it; ++it)
    CalculateM(*it);

  pData->SetRecalc(true);
}

// ------------------------------------------------------------------------------------------------
bool MCOMethod::UpdateOptimumEstimation(Trial& trial)
{
  trialCount++;

  const bool isFeasible = trial.index == convolutionIndex;
  if (isFeasible && !mcoTask.TakeCalculatedCriteria(trial.y, Criteria(&trial)))
    mcoTask.CalculateCriteria(trial.y, Criteria(&trial));

  const bool isOptimumUpdated = Method::UpdateOptimumEstimation(trial);

  if (isFeasible && mcoTask.UpdateMinMax(Criteria(&trial)))
    isRecalcConvolution = true;

  return isOptimumUpdated;
}

// ------------------------------------------------------------------------------------------------
int MCOMethod::GetTrialLimit() const
{
  return int(double(MaxNumOfTrials) / mcoTask.GetNumberOfLambdas() * (mcoTask.GetCurrentLambdaIndex() + 1));
}

// ------------------------------------------------------------------------------------------------
bool MCOMethod::CheckStopCondition()
{
  // Как в Method: нельзя остановиться по точности, если последняя точка не вычислена
  const bool isAccuracyReached = AchievedAccuracy < parameters.Epsilon && LastIterationPointType != -1;
  bool isLambdaSolved = isAccuracyReached || trialCount >= GetTrialLimit();

  if (isLambdaSolved && (int)mcoTask.GetSubproblems().size() == mcoTask.GetCurrentLambdaIndex())
    SaveSubproblem();

  if (isLambdaSolved && mcoTask.GetCurrentLambdaIndex() + 1 < mcoTask.GetNumberOfLambdas())
  {
    ChangeLambda();
    isLambdaSolved = false;
  }

  isStop = isLambdaSolved;
  return isStop;
}

// ------------------------------------------------------------------------------------------------
void MCOMethod::SaveSubproblem()
{
  MCOSubproblem subproblem;
  subproblem.lambda = mcoTask.GetLambda();
  subproblem.newTrials = trialCount - previousTrialCount;
  previousTrialCount = trialCount;

  Trial* best = pData->GetBestTrial();
  if (best != 0 && best->index == convolutionIndex)
  {
    subproblem.solution.y.assign(best->y, best->y + pTask.GetN());
    subproblem.solution.criteria.assign(Criteria(best), Criteria(best) + mcoTask.GetNumberOfCriteria());
    subproblem.solution.lambdaIndex = mcoTask.GetCurrentLambdaIndex();
  }
  mcoTask.AddSubproblem(subproblem);
}

// ------------------------------------------------------------------------------------------------
void MCOMethod::ChangeLambda()
{
  mcoTask.NextLambda();
  AchievedAccuracy = 1.0;
  isRecalcConvolution = true;
}

// ------------------------------------------------------------------------------------------------
double MCOMethod::CalculateR(SearchInterval* p, double r) const
{
  const int v = p->izl();
  const double deltax = p->delta;
  return deltax + (p->zr() - p->zl()) * (p->zr() - p->zl()) / (deltax * pData->M[v] * pData->M[v] * r * r) -
    2 * (p->zr() + p->zl() - 2 * pData->Z[v]) / (r * pData->M[v]);
}

// ------------------------------------------------------------------------------------------------
bool MCOMethod::IsLocalR(SearchInterval* p) const
{
  return isDualR && p->izl() >= 0 && p->izl() == p->izr() &&
    rho * CalculateR(p, rLocal) > CalculateR(p, parameters.r);
}

// ------------------------------------------------------------------------------------------------
double MCOMethod::CalculateGlobalR(SearchInterval* p)
{
  if (IsLocalR(p))
    return rho * CalculateR(p, rLocal);
  return Method::CalculateGlobalR(p);
}

// ------------------------------------------------------------------------------------------------
void MCOMethod::CalculateCurrentPoint(Trial& pCurTrialsj, SearchInterval* BestIntervalsj)
{
  if (!IsLocalR(BestIntervalsj))
  {
    Method::CalculateCurrentPoint(pCurTrialsj, BestIntervalsj);
    return;
  }

  pCurTrialsj.SetX(0.5 * (BestIntervalsj->xl() + BestIntervalsj->xr()) -
    (((BestIntervalsj->zr() - BestIntervalsj->zl()) > 0) ? 1 : -1) *
    pow(fabs(BestIntervalsj->zr() - BestIntervalsj->zl()) /
      pData->M[BestIntervalsj->izl()], pTask.GetNumberOfContinuousVariable()) / 2 / rLocal);

  pCurTrialsj.leftInterval = BestIntervalsj;
  pCurTrialsj.rightInterval = BestIntervalsj;

  if (pCurTrialsj.X() <= BestIntervalsj->xl() || pCurTrialsj.X() >= BestIntervalsj->xr())
    pCurTrialsj.SetX(0.5 * (BestIntervalsj->xl() + BestIntervalsj->xr()));

  CalculateImage(pCurTrialsj);
}
// - end of file ----------------------------------------------------------------------------------

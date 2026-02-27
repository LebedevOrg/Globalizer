#include "HDTask.h"
#include "Trial.h"
// ------------------------------------------------------------------------------------------------
HDTask::HDTask(IProblem* _problem, int _ProcLevel) : Task::Task(_problem, _ProcLevel)
{
  startParameterNumber = 0;
}

// ------------------------------------------------------------------------------------------------
HDTask::HDTask() : Task::Task()
{
  startParameterNumber = 0;
}

Task* HDTask::Clone()
{
  HDTask* res = 0;
  if (isInit)
    res = new HDTask(pProblem, ProcLevel);
  else
    res = new HDTask();

  res->startParameterNumber = startParameterNumber;
  res->isInit = isInit;
  return res;
}

// ------------------------------------------------------------------------------------------------
const double* HDTask::GetA() const
{ 
  return &(A[startParameterNumber]);
}

// ------------------------------------------------------------------------------------------------
const double* HDTask::GetB() const
{
  return &(B[startParameterNumber]);
}

// ------------------------------------------------------------------------------------------------
const double* HDTask::GetOptimumPoint() const
{ 
  return &(OptimumPoint[startParameterNumber]);
}

// ------------------------------------------------------------------------------------------------
void HDTask::TransformPoint(double* resPoint, const double* y)
{
  for (int i = 0; i < parameters.startPoint.GetSize(); i++)
  {
    resPoint[i] = parameters.startPoint[i];
  }
  for (int i = 0; i < parameters.Dimension; i++)
  {
    resPoint[i + startParameterNumber] = y[i];
  }
}

// ------------------------------------------------------------------------------------------------
double HDTask::CalculateFuncs(const double* y, int fNumber)
{
  double* point = new double[parameters.startPoint.GetSize()];

  TransformPoint(point, y);

  double multInLevel = parameters.functionSignMultiplier[GetProcLevel()];
  double result = multInLevel * pProblem->CalculateFunctionals(point, fNumber);
  return result;
}

// ------------------------------------------------------------------------------------------------
void HDTask::CalculateFuncsInManyPoints(double* y, int fNumber, int numPoints, double* values)
{
  throw "Not implemented";
  //IGPUProblem* newProblem = dynamic_cast<IGPUProblem*>(pProblem);
  //if (newProblem != 0)
  //{
  //  newProblem->CalculateFunctionals(y, fNumber, numPoints, values);
  //}
}

// ------------------------------------------------------------------------------------------------
void HDTask::SetStartParameterNumber(int _startParameterNumber)
{
  startParameterNumber = _startParameterNumber;
}

// ------------------------------------------------------------------------------------------------
void HDTask::CopyPoint(double* y, Trial* point)
{
  for (int i = 0; i < parameters.Dimension; i++)
  {
    point->y[i] = y[i + startParameterNumber];
  }
}
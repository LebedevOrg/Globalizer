#include "HDTask.h"
#include "Trial.h"

// ------------------------------------------------------------------------------------------------
HDTask::HDTask(IProblem* _problem, int _ProcLevel, bool _isMixedInteger) : Task::Task(_problem, _ProcLevel)
{
  startParameterNumber = 0;
  startIndex = 0;
  isMixedInteger = _isMixedInteger;
}

// ------------------------------------------------------------------------------------------------
HDTask::HDTask(bool _isMixedInteger) : Task::Task()
{
  startParameterNumber = 0;
  isMixedInteger = _isMixedInteger;
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
  for (int i = 0; i < parameters.StartPoint.GetSize(); i++)
  {
    resPoint[i] = parameters.StartPoint[i];
  }
  for (int i = 0; i < parameters.Dimension; i++)
  {
    resPoint[i + startParameterNumber] = y[i];
  }
}

// ------------------------------------------------------------------------------------------------
double HDTask::CalculateFuncs(const double* y, int fNumber)
{
  double* point = new double[parameters.StartPoint.GetSize()];

  TransformPoint(point, y);

  double multInLevel = parameters.FunctionSignMultiplier[GetProcLevel()];
  double result;
  try {
    result = multInLevel * pProblem->CalculateFunctionals(point, fNumber);
  }
  catch (...) {
      result = MaxDouble;
  }
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

// ------------------------------------------------------------------------------------------------
int HDTask::GetNumberOfContinuousVariable()
{
    if (isMixedInteger)
        return GetN() - GetNumberOfDiscreteVariable();
    else
        return GetN();
}

// ------------------------------------------------------------------------------------------------
int HDTask::GetNumberOfDiscreteVariable()
{
    if (isMixedInteger)
        return Task::GetNumberOfDiscreteVariable();
    else
        return 0;
}

// ------------------------------------------------------------------------------------------------
void HDTask::SetMixedInteger() {
    isMixedInteger = true;
}

// ------------------------------------------------------------------------------------------------
void HDTask::UnsetMixedInteger() {
    isMixedInteger = false;
}

// ------------------------------------------------------------------------------------------------
void HDTask::SetStartIndex(int _startIndex) {
    startIndex = _startIndex;
}

// ------------------------------------------------------------------------------------------------
int HDTask::GetStartDiscreteVariable() {
    return Task::GetStartDiscreteVariable() + startIndex;
}

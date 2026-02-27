/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      task.cpp                                                    //
//                                                                         //
//  Purpose:   Source file for optimization task class                     //
//                                                                         //
//  Author(s): Sysoyev A., Barkalov K.                                     //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "Exception.h"
#include "Task.h"
#include <cstring>
#include "Trial.h"

// ------------------------------------------------------------------------------------------------
Task::Task(IProblem* _problem, int _ProcLevel)
{
  if ((parameters.Dimension <= 0) || (parameters.Dimension > MaxDim))
  {
	throw EXCEPTION("N is out of range");
  }

  NumOfFunc = _problem->GetNumberOfFunctions();
  _problem->GetBounds(A, B);
  IsOptimumPointDefined = _problem->GetOptimumPoint(OptimumPoint) == IProblem::OK ? true : false;
  IsOptimumValueDefined = _problem->GetOptimumValue(OptimumValue) == IProblem::OK ? true : false;
  pProblem = _problem;

  ProcLevel = _ProcLevel;
  isInit = true;
}

// ------------------------------------------------------------------------------------------------
Task::Task()
{
  NumOfFunc = 0;
  IsOptimumPointDefined = false;
  IsOptimumValueDefined = false;
  pProblem = 0;

  ProcLevel = 0;
  isInit = false;
}

// ------------------------------------------------------------------------------------------------
Task::~Task()
{
}

Task* Task::Clone()
{
  Task* res = 0;
  if (isInit)
	res = new Task(pProblem, ProcLevel);
  else
	res = new Task();

  res->isInit = isInit;
  return res;
}

// ------------------------------------------------------------------------------------------------
void Task::Init(IProblem * _problem, int _ProcLevel)
{
  if ((parameters.Dimension <= 0) || (parameters.Dimension > MaxDim))
  {
	throw EXCEPTION("N is out of range");
  }

  NumOfFunc = _problem->GetNumberOfFunctions();
  _problem->GetBounds(A, B);
  IsOptimumPointDefined = _problem->GetOptimumPoint(OptimumPoint) == IProblem::OK ? true : false;
  IsOptimumValueDefined = _problem->GetOptimumValue(OptimumValue) == IProblem::OK ? true : false;
  pProblem = _problem;

  ProcLevel = _ProcLevel;
  isInit = true;
}

// ------------------------------------------------------------------------------------------------
Task* Task::CloneWithNewData()
{
  return Clone();
}

// ------------------------------------------------------------------------------------------------
int Task::GetN() const
{
  return parameters.Dimension;
}

// ------------------------------------------------------------------------------------------------
const double* Task::GetA() const
{
  return A;
}

// ------------------------------------------------------------------------------------------------
const double* Task::GetB() const
{
  return B;
}

// ------------------------------------------------------------------------------------------------
double Task::GetOptimumValue() const
{
  return OptimumValue;
}

// ------------------------------------------------------------------------------------------------
void Task::resetOptimumPoint()
{
  pProblem->GetOptimumPoint(OptimumPoint);
}

// ------------------------------------------------------------------------------------------------
const double* Task::GetOptimumPoint() const
{
  return OptimumPoint;
}

// ------------------------------------------------------------------------------------------------
bool Task::GetIsOptimumValueDefined() const
{
  return IsOptimumValueDefined;
}

// ------------------------------------------------------------------------------------------------
bool Task::GetIsOptimumPointDefined() const
{
  return IsOptimumPointDefined;
}

// ------------------------------------------------------------------------------------------------
IProblem* Task::getProblem()
{
  return pProblem;
}

// ------------------------------------------------------------------------------------------------
int Task::GetNumOfFunc() const
{
  return NumOfFunc;
}

// ------------------------------------------------------------------------------------------------
void Task::SetNumofFunc(int nf)
{
  NumOfFunc = nf;
}

// ------------------------------------------------------------------------------------------------
int Task::GetProcLevel()
{
  return ProcLevel;
}

// ------------------------------------------------------------------------------------------------
int Task::GetNumOfFuncAtProblem() const
{
  return NumOfFunc;
}

// ------------------------------------------------------------------------------------------------
double Task::CalculateFuncs(const double* y, int fNumber)
{
  double multInLevel = parameters.functionSignMultiplier[GetProcLevel()];
  double result = multInLevel * pProblem->CalculateFunctionals(y, fNumber);
  return result;
}

// ------------------------------------------------------------------------------------------------
void Task::CalculateFuncsInManyPoints(double* y, int fNumber, int numPoints, double* values)
{
  IGPUProblem* newProblem = dynamic_cast<IGPUProblem*>(pProblem);
  if (newProblem != 0)
  {
	newProblem->CalculateFunctionals(y, fNumber, numPoints, values);  
  }
}

// ------------------------------------------------------------------------------------------------
int Task::GetNumberOfDiscreteVariable()
{
  IIntegerProgrammingProblem* newProblem = dynamic_cast<IIntegerProgrammingProblem*>(pProblem);
  if (newProblem != 0)
  {
	return newProblem->GetNumberOfDiscreteVariable();
  }
  return 0;
}

// ------------------------------------------------------------------------------------------------
int Task::GetNumberOfValues(int discreteVariable)
{
  IIntegerProgrammingProblem* newProblem = dynamic_cast<IIntegerProgrammingProblem*>(pProblem);
  if (newProblem != 0)
  {
	return newProblem->GetNumberOfValues(discreteVariable);
  }
  return -1;
}

// ------------------------------------------------------------------------------------------------
int Task::GetAllDiscreteValues(int discreteVariable, double* values)
{
  IIntegerProgrammingProblem* newProblem = dynamic_cast<IIntegerProgrammingProblem*>(pProblem);
  if (newProblem != 0)
  {
	return newProblem->GetAllDiscreteValues(discreteVariable, values);
  }
  return IProblem::ERROR;
}

// ------------------------------------------------------------------------------------------------
bool Task::IsPermissibleValue(double value, int discreteVariable)
{
  IIntegerProgrammingProblem* newProblem = dynamic_cast<IIntegerProgrammingProblem*>(pProblem);
  if (newProblem != 0)
  {
	return newProblem->IsPermissibleValue(value, discreteVariable);
  }
  return false;
}

// ------------------------------------------------------------------------------------------------
double* Task::getMin()
{
  return NULL;
}

// ------------------------------------------------------------------------------------------------
double* Task::getMax()
{
  return NULL;
}

// ------------------------------------------------------------------------------------------------
bool Task::IsInit()
{
  return isInit;
}

// ------------------------------------------------------------------------------------------------
bool Task::IsLeaf()
{
  return ProcLevel != 0;
}

// ------------------------------------------------------------------------------------------------
void Task::CopyPoint(double* y, Trial* point)
{
  for (int i = 0; i < GetN(); i++)
    point->y[i] = y[i];
}


// ------------------------------------------------------------------------------------------------
void Task::TransformPoint(double* resPoint, const double* y)
{
  for (int i =0; i < parameters.Dimension; i++)
    resPoint[i] = y[i];
}
// - end of file ----------------------------------------------------------------------------------

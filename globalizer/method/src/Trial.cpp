#include "Trial.h"

#include "SearchInterval.h"

#include <chrono>

// ------------------------------------------------------------------------------------------------
Trial* Trial::GetLeftPoint()
{
  if (this->leftInterval != 0)
  {
    if (this->leftInterval->LeftPoint != 0)
    {
      return this->leftInterval->LeftPoint;
    }
  }
  return 0;
}

// ------------------------------------------------------------------------------------------------
Trial* Trial::GetRightPoint()
{
  if (this->rightInterval != 0)
  {
    if (this->rightInterval->RightPoint != 0)
    {
      return this->rightInterval->RightPoint;
    }
  }
  return 0;
}

// ------------------------------------------------------------------------------------------------
double TimeSeconds() 
{
  auto now = std::chrono::system_clock::now();
  auto duration = now.time_since_epoch();
  return std::chrono::duration<double>(duration).count();
}

// ------------------------------------------------------------------------------------------------
Trial::Trial()
{
  discreteValuesIndex = 0;

  x = 0.0;
  index = -2;
  K = 0;

  for (int i = 0; i < MaxNumOfFunc; i++)
    FuncValues[i] = MaxDouble;
  
  for (int i = 0; i < MaxDim; ++i) {
      y[i] = 0.0;
  }

  leftInterval = 0;
  rightInterval = 0;

  lowAndUpPoints = 0;

  TypeColor = 0;

  generatedTask = 0;

  creationTime = TimeSeconds();
  iterationNumber = parameters.iterationNumber;
}

// ------------------------------------------------------------------------------------------------
Trial::Trial(const Trial& trial)
{
  this->discreteValuesIndex = trial.discreteValuesIndex;
  this->x = trial.x;
  memcpy(this->y, trial.y, MaxDim * sizeof(double));
  memcpy(this->FuncValues, trial.FuncValues, MaxNumOfFunc * sizeof(double));

  this->index = trial.index;
  this->K = trial.K;
  this->leftInterval = trial.leftInterval;
  this->rightInterval = trial.rightInterval;
  this->lowAndUpPoints = trial.lowAndUpPoints;
  this->TypeColor = trial.TypeColor;
  this->generatedTask = trial.generatedTask;

  creationTime = TimeSeconds();
  this->iterationNumber = trial.iterationNumber;
}

// ------------------------------------------------------------------------------------------------
Trial* Trial::Clone()
{
  Trial* res = new Trial();

  res->discreteValuesIndex = discreteValuesIndex;
  res->x = x;
  memcpy(res->y, y, MaxDim * sizeof(double));
  memcpy(res->FuncValues, FuncValues, MaxNumOfFunc * sizeof(double));

  res->index = index;
  res->K = K;
  res->leftInterval = leftInterval;
  res->rightInterval = rightInterval;
  res->lowAndUpPoints = lowAndUpPoints;
  res->TypeColor = TypeColor;
  res->generatedTask = generatedTask;
  res->iterationNumber = iterationNumber;
  return res;
}

// ------------------------------------------------------------------------------------------------
Trial::~Trial()
{
  discreteValuesIndex = 0;

  x = 0.0;
  index = -2;
  K = 0;

  leftInterval = 0;
  rightInterval = 0;

  lowAndUpPoints = 0;

  TypeColor = 0;

  generatedTask = 0;
}

// ------------------------------------------------------------------------------------------------
void Trial::SetX(Extended d)
{
  x = d;
}

// ------------------------------------------------------------------------------------------------
Trial& Trial::operator = (Extended d)
{
  SetX(d);
  return *this;
}

// ------------------------------------------------------------------------------------------------
Extended  Trial::X()
{
  return x;
}

// ------------------------------------------------------------------------------------------------
double Trial::GetFloor()
{
  return this->discreteValuesIndex;
}

// ------------------------------------------------------------------------------------------------
double Trial::GetValue()
{
  if (index < 0 || index >= MaxNumOfFunc)
    return FuncValues[0];
  else
    return FuncValues[index];
}

// ------------------------------------------------------------------------------------------------
Trial& Trial::operator = (const Trial& trial)
{
  if (this != &trial)
  {
    this->discreteValuesIndex = trial.discreteValuesIndex;
    this->x = trial.x;
    memcpy(this->y, trial.y, MaxDim * sizeof(double));
    memcpy(this->FuncValues, trial.FuncValues, MaxNumOfFunc * sizeof(double));

    this->index = trial.index;
    this->K = trial.K;
    this->leftInterval = trial.leftInterval;
    this->rightInterval = trial.rightInterval;
    this->lowAndUpPoints = trial.lowAndUpPoints;
    this->TypeColor = trial.TypeColor;
    this->generatedTask = trial.generatedTask;
  }
  return *this;
}

// ------------------------------------------------------------------------------------------------
bool Trial::operator == (Trial& t)
{
  return (x == t.x) && (discreteValuesIndex == t.discreteValuesIndex);
}

// ------------------------------------------------------------------------------------------------
bool Trial::operator > (Trial& t)
{
  if (discreteValuesIndex > t.discreteValuesIndex)
    return true;
  else if (discreteValuesIndex < t.discreteValuesIndex)
    return false;
  else
    return x > t.x;
}

bool Trial::operator < (Trial& t)
{
  if (discreteValuesIndex < t.discreteValuesIndex)
    return true;
  else if (discreteValuesIndex > t.discreteValuesIndex)
    return false;
  else
    return x < t.x;
}
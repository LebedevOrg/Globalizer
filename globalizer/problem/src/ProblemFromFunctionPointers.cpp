#include "ProblemFromFunctionPointers.h"

// ------------------------------------------------------------------------------------------------
ProblemFromFunctionPointers::ProblemFromFunctionPointers(int dimention, std::vector<double> lower_, std::vector<double> upper_,
  std::vector<std::function<double(const double*)>> function_,
  bool isSetOptimum_, double optimumValue_, std::vector<double> optimumCoordinate_)
{
  this->mDim = dimention;
  this->mOwner = this;
  this->mMinDimension = 1;
  this->mMaxDimension = 50;
  this->mNumberOfConstraints = 0;
  this->mLeftBorder = -1.0;
  this->mRightBorder = 1.0;
  this->mNumberOfCriterions = 1;
  isUseAllFunction = false;
  functionsOfPriblem = function_;
  lowerBounds = lower_;
  upperBounds = upper_;
  isSetOptimum = isSetOptimum_;
  this->mNumberOfConstraints = function_.size() - this->mNumberOfCriterions;

  if (isSetOptimum)
  {
    optimumValue = optimumValue_;
    if (optimumCoordinate_.size() != 0)
    {
      optimumCoordinate.resize(dimention);
      for (int i = 0; i < dimention; i++)
        optimumCoordinate[i] = optimumCoordinate_[i];
    }
  }

}

// ------------------------------------------------------------------------------------------------
ProblemFromFunctionPointers::ProblemFromFunctionPointers(int dimention, std::vector<double> lower_, std::vector<double> upper_,
  std::function<double(const double*, int)> function_, int functionCount_,
  bool isSetOptimum_, double optimumValue_, std::vector<double> optimumCoordinate_)
{
  this->mDim = dimention;
  this->mOwner = this;
  this->mMinDimension = 1;
  this->mMaxDimension = 50;
  this->mNumberOfConstraints = 0;
  this->mLeftBorder = -1.0;
  this->mRightBorder = 1.0;
  this->mNumberOfCriterions = 1;
  isUseAllFunction = true;
  allFunction = function_;
  lowerBounds = lower_;
  upperBounds = upper_;
  isSetOptimum = isSetOptimum_;
  this->mNumberOfConstraints = functionCount_ - this->mNumberOfCriterions;

  if (isSetOptimum)
  {
    optimumValue = optimumValue_;
    if (optimumCoordinate_.size() != 0)
    {
      optimumCoordinate.resize(dimention);
      for (int i = 0; i < dimention; i++)
        optimumCoordinate[i] = optimumCoordinate_[i];
    }
  }

}

// Новый конструктор для задач с частично целочисленными
ProblemFromFunctionPointers::ProblemFromFunctionPointers(int dimension, int numberOfDiscreteVariables_,
std::vector<double> lower_, std::vector<double> upper_,
std::vector<int> discreteValues_,
std::vector<std::function<double(const double*)>> function_,
bool isSetOptimum_, double optimumValue_, std::vector<double> optimumCoordinate_)
{
  mIsInit = false;
  BaseProblem<ProblemFromFunctionPointers>::Init(0, 0, false);

  this->mDim = dimension;
  this->mOwner = this;
  this->mMinDimension = 1;
  this->mMaxDimension = 50;
  this->mNumberOfConstraints = 0;
  this->mLeftBorder = -1.0;
  this->mRightBorder = 1.0;
  this->mNumberOfCriterions = 1;
  isUseAllFunction = false;
  functionsOfPriblem = function_;
  lowerBounds = lower_;
  upperBounds = upper_;
  isSetOptimum = isSetOptimum_;
  this->mNumberOfConstraints = function_.size() - this->mNumberOfCriterions;

  this->NumberOfDiscreteVariable = numberOfDiscreteVariables_;
  this->mDefNumberOfValues = discreteValues_.empty() ? -1 : discreteValues_[0];

  this->CheckValue();

  if (numberOfDiscreteVariables_ > 0 && !discreteValues_.empty())
  {
    for (int i = 0; i < std::min(numberOfDiscreteVariables_, (int)discreteValues_.size()); i++)
    {
      this->mNumberOfValues[i] = discreteValues_[i];
    }
  }

  if (isSetOptimum)
  {
    optimumValue = optimumValue_;
    if (optimumCoordinate_.size() != 0)
    {
      optimumCoordinate.resize(dimension);
      for (int i = 0; i < dimension; i++)
        optimumCoordinate[i] = optimumCoordinate_[i];
    }
  }
  mIsInit = true;
}

// ------------------------------------------------------------------------------------------------
/// Инициализация параметров
void ProblemFromFunctionPointers::Init(int argc, char* argv[], bool isMPIInit)
{
  //mIsInit = false;
  //BaseProblem<ProblemFromFunctionPointers>::Init(argc, argv, false);

  //mIsInit = true;
}

// ------------------------------------------------------------------------------------------------
int ProblemFromFunctionPointers::GetOptimumValue(double& value) const
{
  if (!mIsInit || !isSetOptimum)
    return IProblem::UNDEFINED;

  value = optimumValue;

  return IProblem::OK;
}

// ------------------------------------------------------------------------------------------------
int ProblemFromFunctionPointers::GetOptimumPoint(double* point) const
{
  if (!mIsInit || !isSetOptimum)
    return IProblem::UNDEFINED;

  for (int i = 0; i < mDim; i++)
    point[i] = optimumCoordinate[i];

  return IProblem::OK;
}

/** Метод возвращает границы области поиска
*/
void ProblemFromFunctionPointers::GetBounds(double* lower, double* upper)
{
  if (this->mIsInit)
    for (int i = 0; i < Dimension; i++)
    {
      lower[i] = lowerBounds[i];
      upper[i] = upperBounds[i];
    }
}

double ProblemFromFunctionPointers::CalculateFunctionals(const double* x, int fNumber)
{
  if (isUseAllFunction)
  {
    return allFunction(x, fNumber);
  }
  else
  {
    if (fNumber >= functionsOfPriblem.size())
      throw EXCEPTION("Error function number");
    return functionsOfPriblem[fNumber](x);
  }
}
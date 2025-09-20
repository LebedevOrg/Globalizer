#pragma once

#include "Problem.h"


/** ����� ����� ����������� ������� ��� ���������
*/
class ProblemFromFunctionPointers : public Problem<ProblemFromFunctionPointers>
{
#undef OWNER_NAME
#define OWNER_NAME ProblemFromFunctionPointers
protected:
  std::vector<std::function<double(const double*)>> function;
  double optimumValue = 0;
  std::vector<double> optimumCoordinate;
  std::vector<double> lowerBounds;
  std::vector<double> upperBounds;
  bool isSetOptimum;
public:

  // ------------------------------------------------------------------------------------------------
  ProblemFromFunctionPointers(int dimention, std::vector<double> lower_, std::vector<double> upper_,
    std::vector<std::function<double(const double*)>> function_,
    bool isSetOptimum_ = false, double optimumValue_ = 0, double* optimumCoordinate_ = nullptr)
  {
    this->mDim = dimention;
    this->mOwner = this;
    this->mMinDimension = 1;
    this->mMaxDimension = 50;
    this->mNumberOfConstraints = 0;
    this->mLeftBorder = -1.0;
    this->mRightBorder = 1.0;
    this->mNumberOfCriterions = 1;
    function = function_;
    lowerBounds = lower_;
    upperBounds = upper_;
    isSetOptimum = isSetOptimum_;
    this->mNumberOfConstraints = function_.size() - this->mNumberOfCriterions;

    if (isSetOptimum)
    {
      optimumValue = optimumValue_;
      if (optimumCoordinate_ != nullptr)
      {
        optimumCoordinate.resize(dimention);
        for (int i = 0; i < dimention; i++)
          optimumCoordinate[i] = optimumCoordinate_[i];
      }
    }

  }

  // ------------------------------------------------------------------------------------------------
  /// ������������� ����������
  void Init(int argc, char* argv[], bool isMPIInit)
  {
    mIsInit = false;
    BaseProblem<ProblemFromFunctionPointers>::Init(argc, argv, false);

    mIsInit = true;
  }

  // ------------------------------------------------------------------------------------------------
  int GetOptimumValue(double& value) const
  {
    if (!mIsInit || !isSetOptimum)
      return IProblem::UNDEFINED;

    value = optimumValue;

    return IProblem::OK;
  }

  // ------------------------------------------------------------------------------------------------
  int GetOptimumPoint(double* point) const
  {
    if (!mIsInit || !isSetOptimum)
      return IProblem::UNDEFINED;

    for (int i = 0; i < mDim; i++)
      point[i] = optimumCoordinate[i];

    return IProblem::OK;
  }
  /** ����� ���������� ������� ������� ������
*/
  virtual void GetBounds(double* lower, double* upper)
  {
    if (this->mIsInit)
      for (int i = 0; i < Dimension; i++)
      {
        lower[i] = lowerBounds[i];
        upper[i] = upperBounds[i];
      }
  }

  virtual double CalculateFunctionals(const double* x, int fNumber)
  {
    if (fNumber >= function.size())
      throw EXCEPTION("Error function number");
    return function[fNumber](x);
  }

};


#ifdef _GLOBALIZER_BENCHMARKS
#include "IGlobalOptimizationProblem.h"
/** ����� ������ ������� ��� ���������� IGlobalOptimizationProblem
*/
class GlobalizerBenchmarksProblem : public Problem<GlobalizerBenchmarksProblem>
{
#undef OWNER_NAME
#define OWNER_NAME GlobalizerBenchmarksProblem

protected:
  /// ����� ������
  IGlobalOptimizationProblem* problem;

  /// ��� ���������� ���������� ����������
  std::vector< std::vector<std::string>> DiscreteVariableValues;
  /// ����� ������� ������� �����������
  std::vector<double> A;
  /// ������ ������� ������� �����������
  std::vector<double> B;
  /// ��������� ����� ��������
  std::vector<double> optPoint;
  /// ������ �������� ���������� ����������
  std::vector <std::vector<double>> discreteValues;

  /** �����, �������� �� ����������� ����������� ���������� � ������� ���������� ��� ������� � ������������ � ����������� ����������� ��������������

  \param[in] x ���������� �����, ������� ����������� ����������, ����� ������ ���������� ����������
  \param[out] y ����������� ���������� �����, � ������� ���������� ��������� ��������
  \param[out] u ������������ ���������� �����, � ������� ���������� ��������� ��������
  */
  void XtoYU(const double* x, std::vector<double>& y, std::vector<std::string>& u) const
  {
    y.resize(problem->GetNumberOfContinuousVariable());
    int i = 0;
    for (; i < problem->GetNumberOfContinuousVariable(); i++)
    {
      y[i] = x[i];
    }
    for (int j = 0; j < problem->GetNumberOfDiscreteVariable(); j++, i++)
    {
      if (x[i] < 0 || x[i] >  DiscreteVariableValues[j].size())
        throw - 1;
      else
      {
        u[j] = DiscreteVariableValues[j][int(x[i])];
      }
    }
  }

  /** �����, ����������� ������� � ������������ � ����������� ����������� ������������ ������ � ������������

  \param[in] y ����������� ���������� �����, � ������� ���������� ��������� ��������
  \param[in] u ������������ ���������� �����, � ������� ���������� ��������� ��������
  \param[out] x ���������� �����, ������� ����������� ����������, ����� ������ ���������� ����������
  */
  void YUtoX(std::vector<double>& y, std::vector<std::string>& u, double* x) const
  {
    int i = 0;
    for (; i < problem->GetNumberOfContinuousVariable(); i++)
    {
      x[i] = y[i];
    }
    for (int j = 0; j < problem->GetNumberOfDiscreteVariable(); j++, i++)
    {
      bool f = false;
      for (int k = 0; k < DiscreteVariableValues[j].size(); k++)
      {
        if (DiscreteVariableValues[j][k] == u[j])
        {
          f = true;
          x[i] = k;
          break;
        }
      }
      if (!f)
        throw - 1;
    }
  }

public:

  GlobalizerBenchmarksProblem(IGlobalOptimizationProblem* _problem) : problem(_problem)
  {
    mDim = problem->GetDimension();
    mMaxDimension = MaxDim;
    mMinDimension = 1;
    mNumberOfCriterions = problem->GetNumberOfCriterions();
    mNumberOfConstraints = problem->GetNumberOfConstraints();

    NumberOfDiscreteVariable = problem->GetNumberOfDiscreteVariable();

    std::vector<double> x(this->mDim);
    A.resize(this->mDim);
    B.resize(this->mDim);
    problem->GetBounds(A, B);

    std::vector< std::vector<std::string>> values;
    problem->GetDiscreteVariableValues(DiscreteVariableValues);
    if (NumberOfDiscreteVariable > 0)
      mDefNumberOfValues = DiscreteVariableValues[0].size();

    mNumberOfValues = new int[GetNumberOfDiscreteVariable()];
    discreteValues.resize(GetNumberOfDiscreteVariable());
    for (int i = 0; i < GetNumberOfDiscreteVariable(); i++)
    {
      mNumberOfValues[i] = DiscreteVariableValues[i].size();
      discreteValues[i].resize(mNumberOfValues[i]);
      for (int j = 0; j < mNumberOfValues[i]; j++)
      {
        discreteValues[i][j] = j;
      }
    }

    mLeftBorder = A[0];
    mRightBorder = B[0];

  }
  /** ����� ���������� �������� ������� ������� � ����� ����������� ��������
  \param[out] value ����������� ��������
  \return ��� ������ (#OK ��� #UNDEFINED)
  */
  virtual int GetOptimumValue(double& value) const
  {
    return problem->GetOptimumValue(value);
  }
  /** ����� ���������� ���������� ����� ����������� �������� ������� �������
  \param[out] x �����, � ������� ����������� ����������� ��������
  \return ��� ������ (#OK ��� #UNDEFINED)
  */
  virtual int GetOptimumPoint(double* x) const
  {
    std::vector<double> y(problem->GetNumberOfContinuousVariable());
    std::vector<std::string> u(problem->GetNumberOfContinuousVariable());

    int err = problem->GetOptimumPoint(y, u);

    YUtoX(y, u, x);

    return err;
  }
  /** �����, ����������� ������� ������

  \param[in] x �����, � ������� ���������� ��������� ��������
  \param[in] fNumber ����� ����������� �������. 0 ������������� ������� �����������,
  #GetNumberOfFunctions() - 1 -- ���������� ��������
  \return �������� ������� � ��������� �������
  */
  virtual double CalculateFunctionals(const double* x, int fNumber)
  {
    std::vector<double> y(problem->GetNumberOfContinuousVariable());
    std::vector<std::string> u(problem->GetNumberOfContinuousVariable());

    XtoYU(x, y, u);

    return problem->CalculateFunctionals(y, u, fNumber);
  }

  /** ������� ���� �� ����������������� �����

  ������ ����� ����� ���������� ����� #Initialize
  \param[in] configPath ������, ���������� ���� � ����������������� ����� ������
  \return ��� ������
  */
  virtual int SetConfigPath(const std::string& configPath)
  {
    return problem->SetConfigPath(configPath);
  }

  /** ����� ����� ����������� ������

  ������ ����� ������ ���������� ����� #Initialize. ����������� ������ ���� �
  ������ ��������������.
  \param[in] dimension ����������� ������
  \return ��� ������
  */
  virtual int SetDimension(int dimension)
  {
    int err = problem->SetDimension(dimension);
    Dimension = problem->GetDimension();
    return err;
  }

  ///���������� ����������� ������, ����� �������� ����� #Initialize
  virtual int GetDimension() const
  {
    return problem->GetDimension();
  }

  /** ����� ���������� ������� ������� ������
  */
  virtual void GetBounds(double* lower, double* upper)
  {
    std::vector<double> lower_(mDim);
    std::vector<double> upper_(mDim);
    problem->GetBounds(lower_, upper_);
    for (int i = 0; i < mDim; i++)
    {
      lower[i] = lower_[i];
      upper[i] = upper_[i];

    }
  }

  /**
  ���������� ����� �������� ����������� ��������� discreteVariable.
  GetDimension() ���������� ����� ����� ����������.
  (GetDimension() - GetNumberOfDiscreteVariable()) - ����� ��������� ���������� ����������
  ��� �� ���������� ���������� == -1
  */
  virtual int GetNumberOfValues(int discreteVariable)
  {
    if ((discreteVariable > GetDimension()) ||
      (discreteVariable < (GetDimension() - GetNumberOfDiscreteVariable())))
      return -1;
    if (mNumberOfValues == 0)
      return -1;
    return mNumberOfValues[discreteVariable - (GetDimension() - GetNumberOfDiscreteVariable())];
  }

  /// ������� ����� �������� �������� ��� ����������� ���������
  virtual void ClearCurrentDiscreteValueIndex(int** mCurrentDiscreteValueIndex)
  {
    if (NumberOfDiscreteVariable > 0)
    {
      if (*mCurrentDiscreteValueIndex != 0)
        delete[] * mCurrentDiscreteValueIndex;

      *mCurrentDiscreteValueIndex = new int[NumberOfDiscreteVariable];
      for (int i = 0; i < NumberOfDiscreteVariable; i++)
        (*mCurrentDiscreteValueIndex)[i] = 0;
    }
  }

  /**
  ���������� �������� ����������� ��������� � ������� discreteVariable
  ���������� ��� ������.
  \param[out] values ������, � ������� ����� ��������� �������� ����������� ���������
  */
  virtual int GetAllDiscreteValues(int discreteVariable, double* values)
  {
    if ((discreteVariable > GetDimension()) ||
      (discreteVariable < (GetDimension() - GetNumberOfDiscreteVariable())))
      return IIntegerProgrammingProblem::ERROR_DISCRETE_VALUE;
    int* mCurrentDiscreteValueIndex = 0;
    ClearCurrentDiscreteValueIndex(&mCurrentDiscreteValueIndex);

    // ���������� �������� ������� �������� �������� � ������ ����� �������
    GetNextDiscreteValues(mCurrentDiscreteValueIndex, values[0], discreteVariable, -1);
    int numVal = GetNumberOfValues(discreteVariable);
    // ���������� ��� ��������� ��������
    for (int i = 1; i < numVal; i++)
    {
      GetNextDiscreteValues(mCurrentDiscreteValueIndex, values[i], discreteVariable);
    }
    return IProblem::OK;
  }
  /**
  ���������� �������� ����������� ��������� � ������� discreteVariable ����� ������ previousNumber
  ���������� ��� ������.
  \param[in] previousNumber - ����� �������� ����� �������� ������������ ��������
  -2 - �������� �� ���������, ���������� ��������� ��������
  -1 - ���������� ����� -1, �.�. ����� ������� �������
  \param[out] value ���������� � ������� ����������� �������� ����������� ���������
  */
  virtual int GetNextDiscreteValues(int* mCurrentDiscreteValueIndex, double& value, int discreteVariable, int previousNumber = -2)
  {
    if ((discreteVariable > GetDimension()) ||
      (discreteVariable < (GetDimension() - GetNumberOfDiscreteVariable())) ||
      (mCurrentDiscreteValueIndex == 0) ||
      (mNumberOfValues == 0))
      return IIntegerProgrammingProblem::ERROR_DISCRETE_VALUE;
    // ���� -1 �� ���������� �������� �������� ������
    if (previousNumber == -1)
    {
      mCurrentDiscreteValueIndex[discreteVariable - GetNumberOfDiscreteVariable()] = 0;
      value = 0;
      return IProblem::OK;
    }
    else if (previousNumber == -2)
    {
      mCurrentDiscreteValueIndex[discreteVariable - GetNumberOfDiscreteVariable()]++;
      value = mCurrentDiscreteValueIndex[discreteVariable - GetNumberOfDiscreteVariable()];
      return IProblem::OK;
    }
    else
    {
      mCurrentDiscreteValueIndex[discreteVariable - GetNumberOfDiscreteVariable()] =
        previousNumber;
      mCurrentDiscreteValueIndex[discreteVariable - GetNumberOfDiscreteVariable()]++;
      value = mCurrentDiscreteValueIndex[discreteVariable -
        GetNumberOfDiscreteVariable()];
      return IProblem::OK;
    }
  }

};
#endif

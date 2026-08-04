#ifdef _GLOBALIZER_BENCHMARKS
#include "GlobalizerBenchmarksProblem.h"

GlobalizerBenchmarksProblem::GlobalizerBenchmarksProblem(IGlobalOptimizationProblem* _problem) : problem(_problem)
{
  mDim = problem->GetDimension();
  mMaxDimension = MaxDim;
  mMinDimension = 1;
  mNumberOfCriterions = problem->GetNumberOfCriterions();
  mNumberOfConstraints = problem->GetNumberOfConstraints();

  NumberOfDiscreteVariable = problem->GetNumberOfDiscreteVariable();

  std::vector<double> x(this->mDim);
  A.resize(problem->GetNumberOfContinuousVariable());
  B.resize(problem->GetNumberOfContinuousVariable());
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

void GlobalizerBenchmarksProblem::XtoYU(const double* x, std::vector<double>& y, std::vector<std::string>& u) const
{
  y.resize(problem->GetNumberOfContinuousVariable());
  u.resize(problem->GetNumberOfDiscreteVariable());
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

void GlobalizerBenchmarksProblem::YUtoX(std::vector<double>& y, std::vector<std::string>& u, double* x) const
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

/** Метод возвращает значение целевой функции в точке глобального минимума
\param[out] value оптимальное значение
\return Код ошибки (#OK или #UNDEFINED)
*/
int GlobalizerBenchmarksProblem::GetOptimumValue(double& value) const
{
  return problem->GetOptimumValue(value);
}

/** Метод возвращает координаты точки глобального минимума целевой функции
\param[out] x точка, в которой достигается оптимальное значение
\return Код ошибки (#OK или #UNDEFINED)
*/
int GlobalizerBenchmarksProblem::GetOptimumPoint(double* x) const
{
  std::vector<double> y(problem->GetNumberOfContinuousVariable());
  std::vector<std::string> u(problem->GetNumberOfDiscreteVariable());

  int err = problem->GetOptimumPoint(y, u);

  if (err == IGlobalOptimizationProblem::PROBLEM_OK)
    YUtoX(y, u, x);

  return err;
}

/** Метод, вычисляющий функции задачи

\param[in] x Точка, в которой необходимо вычислить значение
\param[in] fNumber Номер вычисляемой функции. 0 соответствует первому ограничению,
#GetNumberOfFunctions() - 1 -- последнему критерию
\return Значение функции с указанным номером
*/
double GlobalizerBenchmarksProblem::CalculateFunctionals(const double* x, int fNumber)
{
  std::vector<double> y(problem->GetNumberOfContinuousVariable());
  std::vector<std::string> u(problem->GetNumberOfDiscreteVariable());

  XtoYU(x, y, u);

  return problem->CalculateFunctionals(y, u, fNumber);
}

/** Задание пути до конфигурационного файла

Данный метод должн вызываться перед #Initialize
\param[in] configPath строка, содержащая путь к конфигурационному файлу задачи
\return Код ошибки
*/
int GlobalizerBenchmarksProblem::SetConfigPath(const std::string& configPath)
{
  return problem->SetConfigPath(configPath);
}


/** Метод задаёт размерность задачи

Данный метод должен вызываться перед #Initialize. Размерность должна быть в
списке поддерживаемых.
\param[in] dimension размерность задачи
\return Код ошибки
*/
int GlobalizerBenchmarksProblem::SetDimension(int dimension)
{
  int err = problem->SetDimension(dimension);
  Dimension = problem->GetDimension();
  return err;
}

///Возвращает размерность задачи, можно вызывать после #Initialize
int GlobalizerBenchmarksProblem::GetDimension() const
{
  return problem->GetDimension();
}

/** Метод возвращает границы области поиска
*/
void GlobalizerBenchmarksProblem::GetBounds(double* lower, double* upper)
{
  std::vector<double> lower_(problem->GetNumberOfContinuousVariable());
  std::vector<double> upper_(problem->GetNumberOfContinuousVariable());
  problem->GetBounds(lower_, upper_);
  for (int i = 0; i < problem->GetNumberOfContinuousVariable(); i++)
  {
    lower[i] = lower_[i];
    upper[i] = upper_[i];
  }
}

/**
Возвращает число значений дискретного параметра discreteVariable.
GetDimension() возвращает общее число параметров.
(GetDimension() - GetNumberOfDiscreteVariable()) - номер начальной дискретной переменной
Для не дискретных переменных == -1
*/
int GlobalizerBenchmarksProblem::GetNumberOfValues(int discreteVariable)
{
  if ((discreteVariable > GetDimension()) ||
    (discreteVariable < (GetDimension() - GetNumberOfDiscreteVariable())))
    return -1;
  if (mNumberOfValues == 0)
    return -1;
  return mNumberOfValues[discreteVariable - (GetDimension() - GetNumberOfDiscreteVariable())];
}

/// Очищает номер текущего значения для дискретного параметра
void GlobalizerBenchmarksProblem::ClearCurrentDiscreteValueIndex(int** mCurrentDiscreteValueIndex)
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
Определяет значения дискретного параметра с номером discreteVariable
Возвращает код ошибки.
\param[out] values массив, в который будут сохранены значения дискретного параметра
*/
int GlobalizerBenchmarksProblem::GetAllDiscreteValues(int discreteVariable, double* values)
{
  if ((discreteVariable > GetDimension()) ||
    (discreteVariable < (GetDimension() - GetNumberOfDiscreteVariable())))
    return IIntegerProgrammingProblem::ERROR_DISCRETE_VALUE;
  int* mCurrentDiscreteValueIndex = 0;
  ClearCurrentDiscreteValueIndex(&mCurrentDiscreteValueIndex);

  // сбрасываем значение индекса текущего значения и задаем левую границу
  GetNextDiscreteValues(mCurrentDiscreteValueIndex, values[0], discreteVariable, -1);
  int numVal = GetNumberOfValues(discreteVariable);
  // определяем все остальные значения
  for (int i = 1; i < numVal; i++)
  {
    GetNextDiscreteValues(mCurrentDiscreteValueIndex, values[i], discreteVariable);
  }
  return IProblem::OK;
}

std::vector<std::vector<std::string>> GlobalizerBenchmarksProblem::GetDiscretesValues()
{
  std::vector< std::vector<std::string>> values;

  problem->GetDiscreteVariableValues(values);

  return values;
}

/**
Определяет значения дискретного параметра с номером discreteVariable после номера previousNumber
Возвращает код ошибки.
\param[in] previousNumber - номер значения после которого возвращается значение
-2 - значение по умолчанию, возвращает следующее значение -1 - возвращает после -1, т.е. левую границу области
\param[out] value переменная в которую сохраняется значение дискретного параметра
*/
int GlobalizerBenchmarksProblem::GetNextDiscreteValues(int* mCurrentDiscreteValueIndex, double& value, int discreteVariable, int previousNumber)
{
  if ((discreteVariable > GetDimension()) ||
    (discreteVariable < (GetDimension() - GetNumberOfDiscreteVariable())) ||
    (mCurrentDiscreteValueIndex == 0) ||
    (mNumberOfValues == 0))
    return IIntegerProgrammingProblem::ERROR_DISCRETE_VALUE;
  // если -1 то сбрасываем значение текущего номера
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

#endif
#include "MCOTask.h"

#include "Evolvent.h"

#include <algorithm>

// ------------------------------------------------------------------------------------------------
MCOTask::MCOTask(IProblem* _problem, const MCOParameters& _mcoParameters) :
  Task(_problem, 0), mcoParameters(_mcoParameters), currentLambda(0), isMinMaxSet(false)
{
  numberOfConstraints = _problem->GetNumberOfConstraints();
  numberOfCriteria = _problem->GetNumberOfCriterions();

  if (numberOfCriteria < 1)
    throw EXCEPTION("MCOTask: problem has no criteria");
  // Значения критериев хранятся в испытании сразу за сверткой
  if (numberOfConstraints + 1 + numberOfCriteria > MaxNumOfFunc)
    throw EXCEPTION("MCOTask: too many functions, increase MAX_NUMBER_OF_FUNCTION");
  if (GetNumberOfDiscreteVariable() != 0)
    throw EXCEPTION("MCOTask: discrete variables are not supported");

  NumOfFunc = numberOfConstraints + 1;
  minValues.assign(numberOfCriteria, 0.0);
  maxValues.assign(numberOfCriteria, 0.0);
  InitLambdas();
}

// ------------------------------------------------------------------------------------------------
void MCOTask::InitLambdas()
{
  if (!mcoParameters.lambdas.empty())
  {
    for (const auto& lambda : mcoParameters.lambdas)
      if ((int)lambda.size() != numberOfCriteria)
        throw EXCEPTION("MCOTask: lambda size must match the number of criteria");
    lambdas = mcoParameters.lambdas;
    return;
  }

  const int tau = mcoParameters.numberOfLambdas;
  if (tau < 1)
    throw EXCEPTION("MCOTask: numberOfLambdas must be positive");

  for (int i = 0; i < tau; i++)
  {
    const double t = (tau > 1) ? double(i) / (tau - 1) : 0.0;
    if (numberOfCriteria == 1)
      lambdas.push_back({ 1.0 });
    else if (numberOfCriteria == 2)
      lambdas.push_back({ t, 1.0 - t });
    else
    {
      // Для трех и более критериев веса - нормированные точки развертки
      std::vector<double> lower(numberOfCriteria, 0.0);
      std::vector<double> upper(numberOfCriteria, 1.0);
      std::vector<double> lambda(numberOfCriteria);
      Evolvent evolvent(numberOfCriteria);
      evolvent.SetBounds(lower.data(), upper.data());
      evolvent.GetImage(Extended(t), lambda.data());

      double sum = 0.0;
      for (double value : lambda)
        sum += value;
      for (double& value : lambda)
        value /= sum;
      lambdas.push_back(lambda);
    }
  }
}

// ------------------------------------------------------------------------------------------------
double MCOTask::CalculateFuncs(const double* y, int fNumber)
{
  if (fNumber < numberOfConstraints)
    return Task::CalculateFuncs(y, fNumber);

  std::vector<double> criteria(numberOfCriteria);
  CalculateCriteria(y, criteria.data());

  // Task возвращает MaxDouble, если критерий не вычислился: такую точку нельзя сворачивать,
  // иначе при нулевом весе она станет обычной
  for (double value : criteria)
    if (value >= MaxDouble)
      return MaxDouble;

  const double convolution = Convolution(criteria.data());

  std::lock_guard<std::mutex> lock(calculatedCriteriaMutex);
  calculatedCriteria[std::vector<double>(y, y + GetN())] = criteria;
  return convolution;
}

// ------------------------------------------------------------------------------------------------
void MCOTask::CalculateCriteria(const double* y, double* criteria)
{
  for (int i = 0; i < numberOfCriteria; i++)
    criteria[i] = Task::CalculateFuncs(y, numberOfConstraints + i);
}

// ------------------------------------------------------------------------------------------------
double MCOTask::Convolution(const double* criteria) const
{
  const std::vector<double>& lambda = GetLambda();
  double value = -MaxDouble;
  for (int i = 0; i < numberOfCriteria; i++)
  {
    double scale = 1.0;
    if (mcoParameters.isScaling)
    {
      scale = maxValues[i] - minValues[i];
      if (scale < 1e-6)
        scale = 1.0;
    }
    value = std::max(value, (criteria[i] - minValues[i]) / scale * lambda[i]);
  }
  return value;
}

// ------------------------------------------------------------------------------------------------
bool MCOTask::UpdateMinMax(const double* criteria)
{
  if (!isMinMaxSet)
  {
    minValues.assign(criteria, criteria + numberOfCriteria);
    maxValues = minValues;
    isMinMaxSet = true;
    return true;
  }

  bool isChanged = false;
  for (int i = 0; i < numberOfCriteria; i++)
  {
    if (minValues[i] > criteria[i])
    {
      minValues[i] = criteria[i];
      isChanged = true;
    }
    if (maxValues[i] < criteria[i])
    {
      maxValues[i] = criteria[i];
      isChanged = true;
    }
  }
  return isChanged;
}

// ------------------------------------------------------------------------------------------------
bool MCOTask::TakeCalculatedCriteria(const double* y, double* criteria)
{
  std::lock_guard<std::mutex> lock(calculatedCriteriaMutex);
  const auto it = calculatedCriteria.find(std::vector<double>(y, y + GetN()));
  if (it == calculatedCriteria.end())
    return false;
  std::copy(it->second.begin(), it->second.end(), criteria);
  calculatedCriteria.erase(it);
  return true;
}

// ------------------------------------------------------------------------------------------------
double* MCOTask::getMin()
{
  return minValues.data();
}

// ------------------------------------------------------------------------------------------------
double* MCOTask::getMax()
{
  return maxValues.data();
}
// - end of file ----------------------------------------------------------------------------------

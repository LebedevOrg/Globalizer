#include "PYProblem.h"
#include "PYLogger.h"
#include <chrono>
#include <cmath>
#include <limits>
#include <algorithm>
#include <stdexcept>

// ------------------------------------------------------------------------------------------------
// Разбор строки дискретного значения в число ("-1" -> -1.0, "0.5" -> 0.5)
static double ParseDiscreteValue(const std::string& s, int var, int j)
{
  try
  {
    size_t pos = 0;
    double v = std::stod(s, &pos);
    return v;
  }
  catch (...)
  {
    throw std::invalid_argument("discrete_variable_values[" + std::to_string(var) + "][" +
      std::to_string(j) + "] = '" + s + "' is not a number");
  }
}

// ------------------------------------------------------------------------------------------------
PYProblem::PYProblem(py::object data)
{
  mIsInit = false;

  /// Параметры по умолчанию
  this->mOwner = this;
  this->mMinDimension = 1;
  this->mMaxDimension = 50;
  this->mNumberOfConstraints = 0;
  this->mLeftBorder = -1.0;
  this->mRightBorder = 1.0;
  this->mNumberOfCriterions = 1;

  // ---------------------------------------------------------------------------
  // 1. Читаем ВСЁ из Python в собственные поля (они не затираются Init/CheckValue)
  // ---------------------------------------------------------------------------
  int dim = 0;
  if (py::hasattr(data, "_dimension") && !data.attr("_dimension").is_none())
    dim = data.attr("_dimension").cast<int>();

  if (py::hasattr(data, "discrete_variable_values"))
  {
    py::list discrete_vals = data.attr("discrete_variable_values");
    for (size_t i = 0; i < discrete_vals.size(); i++)
    {
      py::list vals = discrete_vals[i];
      std::vector<double> numeric;
      for (size_t j = 0; j < vals.size(); j++)
      {
        std::string s = py::str(vals[j]).cast<std::string>();
        discreteValues.push_back(s);
        numeric.push_back(ParseDiscreteValue(s, (int)i, (int)j));
      }
      if (numeric.empty())
        throw std::invalid_argument("discrete variable " + std::to_string(i) + " has no values");
      mDiscreteNumeric.push_back(numeric);
    }
  }
  mNumDiscrete = (int)mDiscreteNumeric.size();

  if (py::hasattr(data, "number_of_discrete_variables"))
  {
    int n = data.attr("number_of_discrete_variables").cast<int>();
    if (n != mNumDiscrete)
      throw std::invalid_argument("number_of_discrete_variables (" + std::to_string(n) +
        ") != len(discrete_variable_values) (" + std::to_string(mNumDiscrete) + ")");
  }

  if (py::hasattr(data, "_lower_bounds"))
    for (auto item : py::list(data.attr("_lower_bounds")))
      lowerBounds.push_back(item.cast<double>());
  if (py::hasattr(data, "_upper_bounds"))
    for (auto item : py::list(data.attr("_upper_bounds")))
      upperBounds.push_back(item.cast<double>());

  if (lowerBounds.size() != upperBounds.size())
    throw std::invalid_argument("_lower_bounds and _upper_bounds have different sizes");

  if (dim <= 0)
    dim = (int)lowerBounds.size() + mNumDiscrete;

  mNumContinuous = dim - mNumDiscrete;
  if (mNumContinuous < 0)
    throw std::invalid_argument("dimension < number of discrete variables");
  // Допускаем, что границы заданы для всех dim координат — берём только непрерывные
  if ((int)lowerBounds.size() < mNumContinuous)
    throw std::invalid_argument("bounds are given for " + std::to_string(lowerBounds.size()) +
      " variables, but problem has " + std::to_string(mNumContinuous) + " continuous variables");
  lowerBounds.resize(mNumContinuous);
  upperBounds.resize(mNumContinuous);

  if (py::hasattr(data, "_functions"))
  {
    py::list functionsList = data.attr("_functions");
    for (auto item : functionsList)
    {
      py::function py_func = py::reinterpret_borrow<py::function>(item);
      functionsOfProblem.push_back(
        [py_func, dim](const double* x) -> double
        {
          py::gil_scoped_acquire gil;
          py::list args;
          for (int i = 0; i < dim; ++i)
            args.append(x[i]);
          return py_func(args).cast<double>();
        });
    }
  }

  isSetOptimum = py::hasattr(data, "_isSetOptimum") ? data.attr("_isSetOptimum").cast<bool>() : false;
  if (isSetOptimum && py::hasattr(data, "_optimumValue"))
    optimumValue = data.attr("_optimumValue").cast<double>();
  if (isSetOptimum && py::hasattr(data, "_optimumPoint"))
    for (auto item : py::list(data.attr("_optimumPoint")))
      optimumCoordinate.push_back(item.cast<double>());

  if (py::hasattr(data, "_num_crit"))
    this->mNumberOfCriterions = data.attr("_num_crit").cast<int>();
  this->mNumberOfConstraints = (int)functionsOfProblem.size() - this->mNumberOfCriterions;

  // ---------------------------------------------------------------------------
  // 2. Инициализация базового класса — в том же порядке, что и в
  //    ProblemFromFunctionPointers (Init -> NumberOfDiscreteVariable -> CheckValue)
  // ---------------------------------------------------------------------------
  this->mDim = dim;                                   // как в ProblemFromFunctionPointers
  BaseProblem<PYProblem>::Init(0, 0, false);          // создаёт параметры (NDV = 0 по умолч.)
  this->NumberOfDiscreteVariable = mNumDiscrete;      // только ПОСЛЕ Init
  this->CheckValue();                                 // выделит/заполнит mNumberOfValues

  mIsInit = true;

  PY_LOG_INFO("[PYProblem] dim=" << GetDimension() << " continuous=" << mNumContinuous
    << " discrete=" << mNumDiscrete << " constraints=" << mNumberOfConstraints
    << " criterions=" << mNumberOfCriterions);
}

// ------------------------------------------------------------------------------------------------
PYProblem::~PYProblem()
{
  if (mNumberOfValues != nullptr)
  {
    delete[] mNumberOfValues;
    mNumberOfValues = nullptr;
  }
}

// ------------------------------------------------------------------------------------------------
/// Решатель вызывает problem->Initialize() -> Init(). Базовый Init заново
/// создаёт параметры и сбрасывает NumberOfDiscreteVariable в 0 — поэтому
/// (как в ProblemFromFunctionPointers) здесь ничего не переинициализируем.
void PYProblem::Init(int argc, char* argv[], bool isMPIInit)
{
  mIsInit = true;
}

// ------------------------------------------------------------------------------------------------
void PYProblem::SyncNumberOfValues()
{
  if (mNumDiscrete <= 0)
    return;
  if (mNumberOfValues != nullptr)
    delete[] mNumberOfValues;
  mNumberOfValues = new int[mNumDiscrete];
  for (int i = 0; i < mNumDiscrete; i++)
    mNumberOfValues[i] = (int)mDiscreteNumeric[i].size();
}

// ------------------------------------------------------------------------------------------------
int PYProblem::CheckValue(int index)
{
  // Базовая проверка параметров (без BaseProblem::CheckValue, который заполняет
  // mNumberOfValues значением mDefNumberOfValues = -1)
  BaseParameters<PYProblem>::CheckValue(index);

  if ((Dimension < mMinDimension) || (Dimension > mMaxDimension))
    Dimension = mMinDimension;

  // NumberOfDiscreteVariable здесь НЕ присваиваем (CheckValue — callback свойства,
  // присваивание внутри него может вызвать рекурсию). Источник истины — mNumDiscrete.
  SyncNumberOfValues();
  return 0;
}

// ------------------------------------------------------------------------------------------------
int PYProblem::DiscreteIndex(int variable) const
{
  int di = variable - (mDim - mNumDiscrete);
  if (di < 0 || di >= mNumDiscrete)
    return -1;
  return di;
}

// ------------------------------------------------------------------------------------------------
bool PYProblem::FindInCache(const double* y, int fNumber, double& result) const
{
  CacheKey key;
  key.fNumber = fNumber;
  key.point.assign(y, y + this->GetDimension());

  std::lock_guard<std::mutex> lock(cache_mutex_);
  auto it = function_cache_.find(key);
  if (it == function_cache_.end())
    return false;
  result = it->second;
  return true;
}

// ------------------------------------------------------------------------------------------------
void PYProblem::AddToCache(const double* y, int fNumber, double value) const
{
  CacheKey key;
  key.fNumber = fNumber;
  key.point.assign(y, y + this->GetDimension());

  std::lock_guard<std::mutex> lock(cache_mutex_);
  if (function_cache_.size() >= MAX_CACHE_SIZE)
    function_cache_.clear();
  function_cache_.emplace(std::move(key), value);
}

// ------------------------------------------------------------------------------------------------
/// Заполняет ВСЕ GetDimension() координат: непрерывные — из _lower/_upper_bounds,
/// дискретные — [min, max] их допустимых значений.
void PYProblem::GetBounds(double* lower, double* upper)
{
  for (int i = 0; i < mNumContinuous; i++)
  {
    lower[i] = lowerBounds[i];
    upper[i] = upperBounds[i];
  }
  for (int d = 0; d < mNumDiscrete; d++)
  {
    const auto& v = mDiscreteNumeric[d];
    lower[mNumContinuous + d] = *std::min_element(v.begin(), v.end());
    upper[mNumContinuous + d] = *std::max_element(v.begin(), v.end());
  }
}

// ------------------------------------------------------------------------------------------------
/// В y дискретные координаты уже содержат РЕАЛЬНЫЕ значения (их выдаёт
/// GetNextDiscreteValues), поэтому никакой трансляции не нужно — y передаётся
/// в Python как есть.
double PYProblem::CalculateFunctionals(const double* y, int fNumber)
{
  if (fNumber < 0 || fNumber >= static_cast<int>(functionsOfProblem.size()))
    throw EXCEPTION("Error function number");

  double cached = 0.0;
  if (FindInCache(y, fNumber, cached))
    return cached;

  double temp = 0.0;
  try
  {
    py::gil_scoped_acquire gil;
    temp = functionsOfProblem[fNumber](y);
  }
  catch (const py::error_already_set& e)
  {
    PY_LOG_ERROR("PYTHON ERROR: " << e.what());
    throw;
  }
  catch (const std::exception& e)
  {
    PY_LOG_ERROR("C++ EXCEPTION: " << e.what());
    throw;
  }

  AddToCache(y, fNumber, temp);
  return temp;
}

// ------------------------------------------------------------------------------------------------
int PYProblem::GetOptimumValue(double& value) const
{
  if (!isSetOptimum)
    return IProblem::UNDEFINED;
  value = optimumValue;
  return IProblem::OK;
}

// ------------------------------------------------------------------------------------------------
int PYProblem::GetOptimumPoint(double* point) const
{
  if (!isSetOptimum || (int)optimumCoordinate.size() != mDim)
    return IProblem::UNDEFINED;
  for (int i = 0; i < mDim; i++)
    point[i] = optimumCoordinate[i];
  return IProblem::OK;
}

// ------------------------------------------------------------------------------------------------
int PYProblem::GetNumberOfDiscreteVariable()
{
  return mNumDiscrete;
}

// ------------------------------------------------------------------------------------------------
int PYProblem::GetNumberOfValues(int discreteVariable)
{
  int di = DiscreteIndex(discreteVariable);
  if (di < 0)
    return -1;
  return (int)mDiscreteNumeric[di].size();
}

// ------------------------------------------------------------------------------------------------
int PYProblem::GetAllDiscreteValues(int discreteVariable, double* values)
{
  int di = DiscreteIndex(discreteVariable);
  if (di < 0 || values == nullptr)
    return IIntegerProgrammingProblem::ERROR_DISCRETE_VALUE;

  const auto& v = mDiscreteNumeric[di];
  for (size_t i = 0; i < v.size(); i++)
    values[i] = v[i];
  return IProblem::OK;
}

// ------------------------------------------------------------------------------------------------
/// Семантика как в BaseProblem::GetNextDiscreteValues:
///   previousNumber == -1 : сброс, value = значение №0
///   previousNumber == -2 : следующее за текущим
///   previousNumber >= 0  : значение №(previousNumber + 1)
/// mCurrentDiscreteValueIndex — массив размера NumberOfDiscreteVariable,
/// индексируется номером ДИСКРЕТНОЙ переменной di (в старой версии было
/// discreteVariable - NumberOfDiscreteVariable -> выход за границы массива).
int PYProblem::GetNextDiscreteValues(int* mCurrentDiscreteValueIndex, double& value,
  int discreteVariable, int previousNumber)
{
  int di = DiscreteIndex(discreteVariable);
  if (di < 0 || mCurrentDiscreteValueIndex == nullptr)
    return IIntegerProgrammingProblem::ERROR_DISCRETE_VALUE;

  const auto& v = mDiscreteNumeric[di];
  const int n = (int)v.size();

  int idx;
  if (previousNumber == -1)
    idx = 0;
  else if (previousNumber == -2)
    idx = mCurrentDiscreteValueIndex[di] + 1;
  else
    idx = previousNumber + 1;

  if (idx < 0 || idx >= n)
    return IIntegerProgrammingProblem::ERROR_DISCRETE_VALUE;

  mCurrentDiscreteValueIndex[di] = idx;
  value = v[idx];
  return IProblem::OK;
}

// ------------------------------------------------------------------------------------------------
bool PYProblem::IsPermissibleValue(double value, int discreteVariable)
{
  int di = DiscreteIndex(discreteVariable);
  if (di < 0)
    return false;
  for (double v : mDiscreteNumeric[di])
    if (std::fabs(v - value) < AccuracyDouble)
      return true;
  return false;
}
// - end of file ----------------------------------------------------------------------------------

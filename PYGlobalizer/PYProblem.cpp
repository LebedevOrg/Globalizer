#include "PYProblem.h"
#include <chrono>

/// Реализация конструктора
PYProblem::PYProblem(py::object data) {
  /// Задание параметров по умолчанию
  this->mOwner = this;
  this->mMinDimension = 1;
  this->mMaxDimension = 50;
  this->mNumberOfConstraints = 0;
  this->mLeftBorder = -1.0;
  this->mRightBorder = 1.0;
  this->mNumberOfCriterions = 1;

  /// Задание размерности из поля "_dimension" переданного Python-объекта
  if (py::hasattr(data, "_dimension")) {
    SetDimension(data.attr("_dimension").cast<int>());
  }

  /// Задание количества дискретных параметров
  if (py::hasattr(data, "number_of_discrete_variables")) {
      this->NumberOfDiscreteVariable = data.attr("number_of_discrete_variables").cast<int>();
  }

  //if (py::hasattr(data, "discrete_variable_names")) {
  //    //get here!!! их пока нет
  //    this->
  //}

  if (py::hasattr(data, "discrete_variable_values")) {
      py::list discrete_vals = data.attr("discrete_variable_values");

      for (int i = 0; i < discrete_vals.size(); i++) {
          py::list val = discrete_vals[i];

          //std::vector<std::string> temp;
          for (int j = 0; j < val.size(); j++) {
              std::string value = val[j].cast<std::string>();
              discreteValues.push_back(value);
          }
      }

      std::cout << "DEBUG: discrete variable values: " << std::endl;
      for (int i = 0; i < discreteValues.size(); i++) {
          std::cout << discreteValues[i] << std::endl;
      }

      std::cout << std::endl;
  }

  /// Задание нижней границы из поля "_lower_bounds" переданного Python-объекта
  if (py::hasattr(data, "_lower_bounds")) {
    py::list lowerList = data.attr("_lower_bounds");
    for (auto item : lowerList) {
      lowerBounds.push_back(item.cast<double>());
    }
  }
  /// Задание верхней границы из поля "_upper_bounds" переданного Python-объекта
  if (py::hasattr(data, "_upper_bounds")) {
    py::list upperList = data.attr("_upper_bounds");
    for (auto item : upperList) {
      upperBounds.push_back(item.cast<double>());
    }
  }

  /// Задание вектора функций из поля "_functions" переданного Python-объекта
  if (py::hasattr(data, "_functions")) {
    py::list functionsList = data.attr("_functions");

    for (auto item : functionsList) {
      py::function py_func = py::reinterpret_borrow<py::function>(item);

      functionsOfProblem.push_back(
        [py_func, mDim = this->GetDimension()](const double* x) -> double {
        py::gil_scoped_acquire gil;

        py::list args;
        for (int i = 0; i < mDim; ++i) {
          args.append(x[i]);
        }

        return py_func(args).cast<double>();
      }
      );
    }
  }

  if (py::hasattr(data, "_isSetOptimum")) {
    isSetOptimum = data.attr("_isSetOptimum").cast<bool>();
  }
  else {
    isSetOptimum = false;
  }

  if (py::hasattr(data, "_num_crit")) {
    this->mNumberOfCriterions = data.attr("_num_crit").cast<int>();
  }

  this->mNumberOfConstraints = functionsOfProblem.size() - this->mNumberOfCriterions;
  std::cout << "Number of constraints: " << this->mNumberOfConstraints << std::endl;
  std::cout << "Number of criterions: " << this->mNumberOfCriterions << std::endl;

  if (isSetOptimum && py::hasattr(data,"_optimumValue"))
  {
    optimumValue = data.attr("_optimumValue").cast<double>();
    /*if (optimumCoordinate_.size() != 0)
    {
      optimumCoordinate.resize(mDim);
      for (int i = 0; i < mDim; i++)
        optimumCoordinate[i] = optimumCoordinate_[i];
    }*/
  }
}

/// Реализация метода получения границ поиска
void PYProblem::GetBounds(double* lower, double* upper) {
  for (int i = 0; i < Dimension; i++)
  {
    lower[i] = lowerBounds[i];
    upper[i] = upperBounds[i];
  }
}

/// Реализация метода, вычисляющего значение функции y из вектора функций с номером fNumber
double PYProblem::CalculateFunctionals(const double* y, int fNumber) {
  py::gil_scoped_acquire gil;
  std::cout << "GIL aquaried, CalculateFunctionals() started" << std::endl;
  if (fNumber >= functionsOfProblem.size())
    throw EXCEPTION("Error function number");

  double temp = 0.0;

  /// Дополнительная проверка на корректность получения функций
  try {
    std::cout << "fNumber: " << fNumber << std::endl;
    std::cout << "functionsOfProblem.size() = " << functionsOfProblem.size() << std::endl;
    std::cout << "Calculate in point: " << *y << std::endl;
    auto start = std::chrono::steady_clock::now();
    temp = functionsOfProblem[fNumber](y);
    auto finish = std::chrono::steady_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
    std::cout << "Time took: " << elapsedTime.count() << std::endl;
  }
  catch (const py::error_already_set& e) {
    std::cerr << "PYTHON ERROR: " << e.what() << std::endl;
    PyErr_Print();
    throw;
  }
  catch (const std::exception& e) {
    std::cerr << "C++ EXCEPTION: " << e.what() << std::endl;
    throw;
  }
  catch (...) {
    std::cerr << "UNKNOWN EXCEPTION occurred while calling Python function" << std::endl;
    throw;
  }

  std::cout << "CalculateFunctionals() finished" << std::endl;
  std::cout << "Result = " << temp << std::endl;

  return temp;
}

int PYProblem::GetNumberOfDiscreteVariable() {
    return NumberOfDiscreteVariable;
}

int PYProblem::GetNumberOfValues(int discreteVariable) {
    //взято из Problem.h 357!!!
    if ((discreteVariable > GetDimension()) ||
        (discreteVariable < (GetDimension() - GetNumberOfDiscreteVariable())))
        return -1;
    if (mNumberOfValues == 0)
        return -1;
    return mNumberOfValues[discreteVariable - (GetDimension() - GetNumberOfDiscreteVariable())];
}

int PYProblem::GetAllDiscreteValues(int discreteVariable, double* values) {
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

int PYProblem::GetNextDiscreteValues(int* mCurrentDiscreteValueIndex, double& value, int discreteVariable, int previousNumber) {
    if ((discreteVariable > GetDimension()) ||
        (discreteVariable < (GetDimension() - GetNumberOfDiscreteVariable())) ||
        (mCurrentDiscreteValueIndex == 0) ||
        (mNumberOfValues == 0))
        return IIntegerProgrammingProblem::ERROR_DISCRETE_VALUE;
    // если -1 то сбрасываем значение текущего номера
    if (previousNumber == -1)
    {
        mCurrentDiscreteValueIndex[discreteVariable - GetNumberOfDiscreteVariable()] = 0;
        value = mLeftBorder;
        return IProblem::OK;
    }
    else if (previousNumber == -2)
    {
        double d = (mRightBorder - mLeftBorder) /
            (mNumberOfValues[discreteVariable - (GetDimension() - GetNumberOfDiscreteVariable())] - 1);
        mCurrentDiscreteValueIndex[discreteVariable - GetNumberOfDiscreteVariable()]++;
        value = mLeftBorder + d *
            mCurrentDiscreteValueIndex[discreteVariable - GetNumberOfDiscreteVariable()];
        return IProblem::OK;
    }
    else
    {
        double d = (mRightBorder - mLeftBorder) /
            (mNumberOfValues[discreteVariable - (GetDimension() - GetNumberOfDiscreteVariable())] - 1);
        mCurrentDiscreteValueIndex[discreteVariable - GetNumberOfDiscreteVariable()] =
            previousNumber;
        mCurrentDiscreteValueIndex[discreteVariable - GetNumberOfDiscreteVariable()]++;
        value = mLeftBorder + d * mCurrentDiscreteValueIndex[discreteVariable -
            GetNumberOfDiscreteVariable()];
        return IProblem::OK;
    }
}

bool PYProblem::IsPermissibleValue(double value, int discreteVariable) {
    if ((discreteVariable > GetDimension()) ||
        (discreteVariable < (GetDimension() - GetNumberOfDiscreteVariable())) ||
        (mNumberOfValues == 0))
        return false;
    double d = (mRightBorder - mLeftBorder) /
        (mNumberOfValues[discreteVariable - (GetDimension() - GetNumberOfDiscreteVariable())] - 1);
    double v = 0;
    for (int i = 0; i < mNumberOfValues[discreteVariable - (GetDimension() - GetNumberOfDiscreteVariable())]; i++)
    {
        v = mLeftBorder + d * i;
        if (fabs(v - value) < AccuracyDouble)
        {
            return true;
        }
    }
    return false;
}
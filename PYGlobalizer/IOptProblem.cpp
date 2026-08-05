#include "IOptProblem.h"

/// Реализация конструктора
IOptProblem::IOptProblem(py::object data) {
	if (py::hasattr(data, "name") && py::hasattr(data, "dimension") && py::hasattr(data, "number_of_float_variables")
	&& py::hasattr(data, "number_of_discrete_variables") && py::hasattr(data, "number_of_objectives")
	&& py::hasattr(data, "number_of_constraints") && py::hasattr(data, "float_variable_names") 
	&& py::hasattr(data, "discrete_variable_names") && py::hasattr(data, "lower_bound_of_float_variables")
	&& py::hasattr(data, "upper_bound_of_float_variables") && py::hasattr(data, "discrete_variable_values")
	/* && py::hasattr(data, "known_optimum")*/) {
		pName = data.attr("name").cast<std::string>();
		this->mDim = data.attr("dimension").cast<int>();
		this->mNumberOfConstraints = data.attr("number_of_constraints").cast<int>();
		this->mNumberOfCriterions = data.attr("number_of_objectives").cast<int>();
		/// Задание нижней границы из поля "lower_bound_of_float_variables" переданного Python-объекта
		if (py::hasattr(data, "lower_bound_of_float_variables")) {
			py::list lowerList = data.attr("lower_bound_of_float_variables");
			for (auto item : lowerList) {
				lowerBounds.push_back(item.cast<double>());
			}
		}
		/// Задание верхней границы из поля "upper_bound_of_float_variables" переданного Python-объекта
		if (py::hasattr(data, "upper_bound_of_float_variables")) {
			py::list upperList = data.attr("upper_bound_of_float_variables");
			for (auto item : upperList) {
				upperBounds.push_back(item.cast<double>());
			}
		}
		if (py::hasattr(data, "discrete_variable_values")) {
			py::list outer_list = data.attr("discrete_variable_values");

			std::vector<std::vector<std::string>> discrete_values;

			for (auto inner_list : outer_list) {
				py::list inner = inner_list.cast<py::list>();
				std::vector<std::string> row;

				for (auto item : inner) {
					row.push_back(item.cast<std::string>());
				}
				discrete_values.push_back(row);
			}

			this->discreteValues = discrete_values;
		}
		//known optimum here!!! (optional)

		//functionsOfProblem.push_back(data.attr("calculate").cast<std::function>());
	}
}
/// Реализация метода, возвращающего границы поиска
void IOptProblem::GetBounds(double* lower, double* upper) {
	for (int i = 0; i < Dimension; i++)
	{
		lower[i] = lowerBounds[i];
		upper[i] = upperBounds[i];
	}
}
/// Реализация метода, вычисляющего значение функции в точке
double IOptProblem::CalculateFunctionals(const double* y, int fNumber) {
	std::cout << "fNumber: " << fNumber << std::endl;
	std::cout << "functionsOfProblem.size() = " << functionsOfProblem.size() << std::endl;
	if (fNumber >= functionsOfProblem.size())
		throw EXCEPTION("Error function number");

	double temp = 0.0;

	/// Дополнительная проверка на корректность получения функций
	try {
		temp = functionsOfProblem[fNumber](y);
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

	return temp;
}
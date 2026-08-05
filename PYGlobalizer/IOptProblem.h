/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2026 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      PYProblem.h                                                 //
//                                                                         //
//  Purpose:   Header file for parsing problem from Python and storing it  //
//                                                                         //
//  Author(s): Egorov K.                                                   //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

/**
\file PYProblem.h
\authors Егоров К.С.
\date 2026
\copyright ННГУ им. Н.И. Лобачевского
\brief Класс задач из Python
\details Реализация парсинга задачи из Python и её хранение
*/

#pragma once
//#include <iostream>
//#include <Python.h>
//#include <pybind11/pybind11.h>
//#include <pybind11/functional.h>
//
//#include "Globalizer.h"
//#include "Problem.h"
#include "PYProblem.h"

namespace py = pybind11;

/**
Класс, реализующий функционал хранения задачи и её передачи из Python
*/
class IOptProblem :public Problem<IOptProblem> {
#undef OWNER_NAME
#define OWNER_NAME IOptProblem
private:
	std::string pName;
	int numObjections, numConstraints, numFloatVars, numDiscVars;

	/// Вектор функций задачи (ограничение + целевые функции)
	std::vector<std::function<double(const double*)>> functionsOfProblem;
	/// Нижняя граница поиска
	std::vector<double> lowerBounds;
	/// Верхняя граница поиска
	std::vector<double> upperBounds;
	std::vector<std::vector<std::string>> discreteValues;	//!!!
	std::vector<std::string> floatNames;
	std::vector<std::string> discreteNames;

	bool isSetOptimum;

	double optimumValue = 0;
	std::vector<double> optimumCoordinate;
	//known optimum here
public:
	/// Конструктор - принимает в качестве параметра Python-объект
	IOptProblem(py::object data);
	/// Метод, возвращающий границы поиска
	virtual void GetBounds(double* lower, double* upper);
	/// Вычисление значения функции y с номером fNumber из вектора функций
	virtual double CalculateFunctionals(const double* y, int fNumber);
};
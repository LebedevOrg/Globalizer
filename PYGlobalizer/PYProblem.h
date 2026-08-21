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
#include <iostream>
#include <Python.h>
#include <pybind11/pybind11.h>
#include <pybind11/functional.h>

#include "Globalizer.h"
#include "Problem.h"

namespace py = pybind11;

/**
Класс, реализующий функционал хранения задачи и её передачи из Python
*/
class PYProblem :public Problem<PYProblem> 
{
#undef OWNER_NAME
#define OWNER_NAME PYProblem
private:
  /// Вектор функций задачи (ограничение + целевые функции)
  std::vector<std::function<double(const double*)>> functionsOfProblem;
  /// Нижняя граница поиска
  std::vector<double> lowerBounds;
  /// Верхняя граница поиска
  std::vector<double> upperBounds;

  bool isSetOptimum;

  double optimumValue = 0;
  std::vector<double> optimumCoordinate;
  int numOfSecreteVariables;
  std::vector<std::string> discreteValues;
public:
  /// Конструктор - принимает в качестве параметра Python-объект
  PYProblem(py::object data);
  /// Метод, возвращающий границы поиска
  virtual void GetBounds(double* lower, double* upper);
  /// Вычисление значения функции y с номером fNumber из вектора функций
  virtual double CalculateFunctionals(const double* y, int fNumber);
  virtual int GetNumberOfDiscreteVariable();
  virtual int GetNumberOfValues(int discreteVariable);
  virtual int GetAllDiscreteValues(int discreteVariable, double* values);
  virtual int GetNextDiscreteValues(int* mCurrentDiscreteValueIndex, double& value, int discreteVariable, int previousNumber = -2);
  virtual bool IsPermissibleValue(double value, int discreteVariable);
};
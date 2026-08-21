/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2026 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      PYGlobalizer.cpp                                            //
//                                                                         //
//  Purpose:   Source code file with implementation of Python interface    //
//  module for Globalizer using pybind11                                   //
//                                                                         //
//  Author(s): Egorov K.                                                   //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

/**
\file PYGlobalizer.cpp
\authors Егоров К.С.
\date 2026
\copyright ННГУ им. Н.И. Лобачевского
\brief Реализация модуля интерфейса для Python
\details Реализация модуля интерфейса для Python с использованием pybind11
*/

#include <iostream>
#include <Python.h>
#include <pybind11\pybind11.h>

#include "Globalizer.h"
#include "PYProblem.h"

namespace py = pybind11;

/// Функция, реализующая приём задачи из Python и вызов решателя Globalizer
void solve(py::object& data, int maxParams = 50, double r = 5, bool localRefineSolution = false, int numThreads = 2)
{
  /// Инициализация Globalizer
  GlobalizerInitialization(0, nullptr, false, true);

  //проверка параметров: maxParams: 2 - 1000000, r > 1

  parameters.MaxNumOfPoints = maxParams;
  parameters.r = r;
  //parameters.LocalRefineSolution = FinalStart; //сделать флажок    
  parameters.NumPoints = numThreads; //и сделать :
  parameters.NumThread = parameters.NumPoints;

  IProblem* problem;

  py::object PYProblem_class = py::module_::import("PYProblem").attr("PYProblem");

  if (py::isinstance(data, PYProblem_class)) {
      /// Создание экземпляра класса задач, получаемых из Python
      problem = new PYProblem(data);
      std::cout << "PYProblem created" << std::endl;
  }

  /// инициализация задачи
  problem->Initialize();
  std::cout << "Problem itialized" << std::endl;
  /// Установка размерности в параметры
  parameters.Dimension = problem->GetDimension();
  std::cout << "Dimension: " << parameters.Dimension << std::endl;
  /// Создание решателя
  Solver solver(problem);
  std::cout << "Solver created" << std::endl;
  /// Запуск решателя с проверкой
  if (solver.Solve() != SYSTEM_OK)
    throw EXCEPTION("Error: solver.Solve crash!!!");

  std::cout << "Solver finished" << std::endl;
}

/// Добавление модуля
PYBIND11_MODULE(PYGlobalizer, m) 
{
  /// Добавление справки
  m.doc() = "Python interface for Globalizer";
  /// Добавление вызываемой функции
  m.def("solve", &solve, "Solve method");
}
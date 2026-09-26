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
#include <vector>
#include <functional>
#include <mutex>
#include <unordered_map>
#ifdef USE_PYTHON
#ifndef WIN32
#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
#endif
#else
#ifdef _DEBUG
#undef _DEBUG
#include "python.h"
#define _DEBUG
#else
#include "python.h"
#endif
#endif
#endif
#include <pybind11/pybind11.h>
#include <pybind11/functional.h>

#include "Globalizer.h"
#include "Problem.h"

namespace py = pybind11;

/**
Класс, реализующий функционал хранения задачи и её передачи из Python.

Исправления относительно исходной версии:
  - functionsOfProblem: тип изменён с vector<py::object> на
    vector<std::function<double(const double*)>>, что позволяет хранить
    C++-лямбды, захватывающие py::function.
  - Все методы объявлены без inline-тел — реализации вынесены в PYProblem.cpp,
    чтобы устранить ошибки "функция уже имеет текст реализации".
*/
class PYProblem : public Problem<PYProblem>
{
#undef OWNER_NAME
#define OWNER_NAME PYProblem

private:
  // Хранилище вычислимых функционалов задачи (критерии + ограничения).
  // Тип std::function<double(const double*)> совместим с C++-лямбдами,
  // захватывающими py::function — в отличие от vector<py::object>.
  std::vector<std::function<double(const double*)>> functionsOfProblem;

  // Границы поиска
  std::vector<double> lowerBounds;
  std::vector<double> upperBounds;

  // Строковые представления дискретных значений (для отладки / будущего API)
  std::vector<std::string> discreteValues;

  // Информация об оптимуме (опционально)
  bool   isSetOptimum;
  double optimumValue;
  std::vector<double> optimumCoordinate;

  // Кэш вычислений — снижает число вызовов Python при повторных обращениях
  struct CacheKey
  {
    std::vector<double> point;
    int fNumber;
    bool operator==(const CacheKey& o) const
    {
      return fNumber == o.fNumber && point == o.point;
    }
  };
  struct CacheKeyHash
  {
    std::size_t operator()(const CacheKey& k) const
    {
      std::size_t h = std::hash<int>{}(k.fNumber);
      for (double v : k.point)
        h ^= std::hash<double>{}(v)+0x9e3779b9 + (h << 6) + (h >> 2);
      return h;
    }
  };
  mutable std::mutex cache_mutex_;
  mutable std::unordered_map<CacheKey, double, CacheKeyHash> function_cache_;
  static constexpr size_t MAX_CACHE_SIZE = 5000;

  bool   FindInCache(const double* y, int fNumber, double& result) const;
  void   AddToCache(const double* y, int fNumber, double value)  const;

public:
  // Конструктор принимает Python-объект задачи (py::object по значению —
  // pybind11 не допускает привязку неконстантных lvalue-ссылок из Python).
  explicit PYProblem(py::object data);

  // --- Обязательные методы интерфейса IProblem / IIntegerProgrammingProblem ---

  /// Заполняет массивы нижних и верхних границ поиска
  void GetBounds(double* lower, double* upper) override;

  /// Вычисляет значение функционала с номером fNumber в точке y
  double CalculateFunctionals(const double* y, int fNumber) override;

  /// Возвращает число дискретных переменных
  int GetNumberOfDiscreteVariable() override;

  /// Возвращает число допустимых значений дискретной переменной с номером idx
  int GetNumberOfValues(int discreteVariable) override;

  /// Заполняет массив values всеми допустимыми значениями переменной
  int GetAllDiscreteValues(int discreteVariable, double* values) override;

  /// Возвращает следующее допустимое значение дискретной переменной
  int GetNextDiscreteValues(int* mCurrentDiscreteValueIndex,
    double& value,
    int discreteVariable,
    int previousNumber = -2) override;

  /// Проверяет, является ли value допустимым значением дискретной переменной
  bool IsPermissibleValue(double value, int discreteVariable) override;
};

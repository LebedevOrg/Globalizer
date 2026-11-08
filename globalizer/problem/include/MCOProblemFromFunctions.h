/**
\file MCOProblemFromFunctions.h
\brief Многокритериальная задача, заданная функциями
*/

#pragma once

#include "ProblemFromFunctionPointers.h"

/// Задача с несколькими критериями и ограничениями вида g(y) <= 0
class MCOProblemFromFunctions : public ProblemFromFunctionPointers
{
public:
  typedef std::function<double(const double*)> Function;

  MCOProblemFromFunctions(int dimension, std::vector<double> lower, std::vector<double> upper,
    const std::vector<Function>& criteria, const std::vector<Function>& constraints = {}) :
    ProblemFromFunctionPointers(dimension, lower, upper, Join(constraints, criteria))
  {
    this->mNumberOfCriterions = (int)criteria.size();
    this->mNumberOfConstraints = (int)constraints.size();
  }

private:
  /// В Globalizer сначала идут ограничения, затем критерии
  static std::vector<Function> Join(const std::vector<Function>& constraints, const std::vector<Function>& criteria)
  {
    std::vector<Function> functions(constraints);
    functions.insert(functions.end(), criteria.begin(), criteria.end());
    return functions;
  }
};
// - end of file ----------------------------------------------------------------------------------

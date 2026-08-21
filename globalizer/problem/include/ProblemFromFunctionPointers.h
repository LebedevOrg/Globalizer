#pragma once

#include "Problem.h"

/** Класс задач принимающий функции как параметры
*/
class GLOBALIZER_API ProblemFromFunctionPointers : public Problem<ProblemFromFunctionPointers>
{
#undef OWNER_NAME
#define OWNER_NAME ProblemFromFunctionPointers
protected:
  std::vector<std::function<double(const double*)>> functionsOfPriblem;
  std::function<double(const double*, int)> allFunction;
  bool isUseAllFunction;

  double optimumValue = 0;
  std::vector<double> optimumCoordinate;
  std::vector<double> lowerBounds;
  std::vector<double> upperBounds;
  bool isSetOptimum;

public:
  ProblemFromFunctionPointers(int dimention, std::vector<double> lower_, std::vector<double> upper_,
    std::vector<std::function<double(const double*)>> function_,
    bool isSetOptimum_ = false, double optimumValue_ = 0, std::vector<double> optimumCoordinate_ = {});

  ProblemFromFunctionPointers(int dimention, std::vector<double> lower_, std::vector<double> upper_,
    std::function<double(const double*, int)> function_, int functionCount_,
    bool isSetOptimum_ = false, double optimumValue_ = 0, std::vector<double> optimumCoordinate_ = {});

  ProblemFromFunctionPointers(int dimension, int numberOfDiscreteVariables_,
    std::vector<double> lower_, std::vector<double> upper_,
    std::vector<int> discreteValues_,
    std::vector<std::function<double(const double*)>> function_,
    bool isSetOptimum_ = false, double optimumValue_ = 0, std::vector<double> optimumCoordinate_ = {});

  void Init(int argc, char* argv[], bool isMPIInit);

  int GetOptimumValue(double& value) const;

  int GetOptimumPoint(double* point) const;

  virtual void GetBounds(double* lower, double* upper);

  virtual double CalculateFunctionals(const double* x, int fNumber);
};
/**
\file MCOTask.h
\brief Многокритериальная задача: минимаксная свертка критериев
\details Свертка со сдвигом на минимум и порядок весов перенесены из iOpt
(MinMaxConvolution, MCOOptimizationTask, MCOMethodManyLambdas). Отличие от iOpt:
критерии берутся по их настоящим номерам, поэтому задачи с ограничениями считаются верно.
*/

#ifndef __MCOTASK_H__
#define __MCOTASK_H__

#include "Task.h"

#include <map>
#include <mutex>
#include <vector>

/// Настройки многокритериального поиска
struct MCOParameters
{
  /// Число сверток; не используется, если веса заданы явно в #lambdas
  int numberOfLambdas = 10;
  /// Веса сверток, заданные явно
  std::vector<std::vector<double>> lambdas;
  /// Локальный параметр надежности метода с двумя r; 0 - метод с одним r (parameters.r)
  double rLocal = 0.0;
  /// Делить ли критерии на разброс значений (max - min)
  bool isScaling = false;
};

/// Испытание многокритериальной задачи
struct MCOPoint
{
  std::vector<double> y;
  /// Значения критериев; пусто, если точка нарушает ограничения
  std::vector<double> criteria;
  /// Номер свертки, при которой проведено испытание
  int lambdaIndex = 0;
};

/// Итог решения одной подзадачи (одной свертки)
struct MCOSubproblem
{
  std::vector<double> lambda;
  /// Число новых испытаний
  int newTrials = 0;
  /// Лучшая точка свертки; пустая, если допустимых точек еще нет
  MCOPoint solution;
};

/**
Задача, в которой метод видит ограничения и одну функцию - свертку критериев.
Функции исходной задачи: сначала ограничения, затем критерии.
*/
class MCOTask : public Task
{
protected:
  int numberOfConstraints;
  int numberOfCriteria;
  MCOParameters mcoParameters;
  std::vector<std::vector<double>> lambdas;
  int currentLambda;

  std::vector<double> minValues;
  std::vector<double> maxValues;
  bool isMinMaxSet;

  /// Критерии точек, вычисленных в CalculateFuncs и еще не записанных в испытания
  std::map<std::vector<double>, std::vector<double>> calculatedCriteria;
  std::mutex calculatedCriteriaMutex;

  std::vector<MCOSubproblem> subproblems;

  /// Строит веса сверток так же, как MCOMethodManyLambdas.init_lambdas в iOpt
  void InitLambdas();

public:
  MCOTask(IProblem* _problem, const MCOParameters& _mcoParameters);

  /// Для ограничения - его значение, для номера свертки - свертка всех критериев в точке y
  virtual double CalculateFuncs(const double* y, int fNumber);

  virtual double* getMin();
  virtual double* getMax();

  /// Номер свертки среди функций, которые видит метод
  int GetConvolutionIndex() const { return numberOfConstraints; }
  int GetNumberOfCriteria() const { return numberOfCriteria; }
  const MCOParameters& GetMCOParameters() const { return mcoParameters; }

  int GetNumberOfLambdas() const { return (int)lambdas.size(); }
  int GetCurrentLambdaIndex() const { return currentLambda; }
  const std::vector<double>& GetLambda() const { return lambdas[currentLambda]; }
  /// Переходит к следующей свертке
  void NextLambda() { currentLambda++; }

  /// Вычисляет все критерии в точке y
  void CalculateCriteria(const double* y, double* criteria);
  /// Значение свертки при текущих весах и минимумах
  double Convolution(const double* criteria) const;
  /// Учитывает значения критериев, возвращает true, если минимум или максимум изменились
  bool UpdateMinMax(const double* criteria);
  /// Забирает критерии, вычисленные в точке y; false, если их нет
  bool TakeCalculatedCriteria(const double* y, double* criteria);

  void AddSubproblem(const MCOSubproblem& subproblem) { subproblems.push_back(subproblem); }
  const std::vector<MCOSubproblem>& GetSubproblems() const { return subproblems; }
};

#endif
// - end of file ----------------------------------------------------------------------------------

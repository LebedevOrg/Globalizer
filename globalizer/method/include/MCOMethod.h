/**
\file MCOMethod.h
\brief Метод решения серии сверток с повторным использованием поисковой информации
\details Схема перенесена из iOpt (MCOMethod, MCOMethodManyLambdas). Метод с двумя параметрами
надежности перенесен из DualLipschitzOptimizer (MethodStrongina, ветка feature/second_r).
*/

#ifndef __MCOMETHOD_H__
#define __MCOMETHOD_H__

#include "MCOTask.h"
#include "Method.h"

/**
Решает все свертки задачи #MCOTask за один проход поиска. Когда свертка решена с заданной
точностью, метод меняет веса, пересчитывает свертку во всех накопленных испытаниях без новых
вычислений критериев и продолжает поиск с этой поисковой информацией.
*/
class MCOMethod : public Method
{
protected:
  MCOTask& mcoTask;
  int convolutionIndex;

  /// Нужно ли пересчитать свертку во всех испытаниях
  bool isRecalcConvolution;
  /// Число проведенных испытаний
  int trialCount;
  /// Число испытаний к концу предыдущей свертки
  int previousTrialCount;

  double rLocal;
  double rho;
  bool isDualR;

  /// Значения критериев записываются в испытание сразу за сверткой
  double* Criteria(Trial* trial) const { return trial->FuncValues + convolutionIndex + 1; }

  /// Пересчитывает свертку в испытаниях, лучшую точку и оценку константы
  virtual void RecalcAllConvolution();
  /// Запоминает итог текущей свертки
  virtual void SaveSubproblem();
  /// Переходит к следующей свертке
  virtual void ChangeLambda();
  /// Предел числа испытаний к концу текущей свертки
  virtual int GetTrialLimit() const;

  /// Характеристика интервала с равными индексами концов при заданном r
  double CalculateR(SearchInterval* p, double r) const;
  /// Выигрывает ли в интервале локальная характеристика
  bool IsLocalR(SearchInterval* p) const;

  /// Характеристика метода с двумя r: max(rho * R(r_loc), R(r_glob))
  virtual double CalculateGlobalR(SearchInterval* p);
  /// Точка нового испытания с тем r, который выиграл в интервале
  virtual void CalculateCurrentPoint(Trial& pCurTrialsj, SearchInterval* BestIntervalsj);
  /// Кроме оценки оптимума обновляет критерии испытания и их минимумы
  virtual bool UpdateOptimumEstimation(Trial& trial);

public:
  MCOMethod(Task& _pTask, SearchData& _pData, Calculation& _Calculation, IEvolvent& _Evolvent);

  virtual void CalculateIterationPoints();
  /// Поиск заканчивается, когда решена последняя свертка
  virtual bool CheckStopCondition();
  /// Локальное уточнение не используется: его точки не содержат критериев
  virtual void LocalSearch() {}
};

#endif
// - end of file ----------------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      ParallelHookeJeevesMethod.h                                 //
//                                                                         //
//  Purpose:   Header file for parallel Hooke-Jeeves local method          //
//                                                                         //
//  Author(s): Sovrasov V.                                                 //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PARALLEL_HOOKE_JEEVES_METHOD_H__
#define __PARALLEL_HOOKE_JEEVES_METHOD_H__

#include "Parameters.h"
#include "Task.h"
#include "Trial.h"
#include "Common.h"
#include <vector>

#include "LocalMethod.h"
#include "Calculation.h"

/**
 Параллельный метод Хука-Дживса: на этапе исследования окрестности точки
 пробные шаги по координатам собираются в пакеты по parameters.NumPoints
 точек и вычисляются вычислителем Calculation параллельно.
*/
class ParallelHookeJeevesMethod : public LocalMethod
{
protected:

  Calculation& calculation;
  /// Входные данные для вычислителя (пакет из parameters.NumPoints испытаний)
  InformationForCalculation inputSet;
  /// Выходные данные вычислителя
  TResultForCalculation outputSet;

  virtual double MakeResearch(OBJECTIV_TYPE*);

  /// Возвращает 0, если точка лежит в области поиска и лимит испытаний не исчерпан, иначе HUGE_VAL
  double CheckCoordinate(const OBJECTIV_TYPE* x);

  virtual double EvaluateObjectiveFunctiuon(const OBJECTIV_TYPE*);

  /// Создаёт испытание в точке x (непрерывные координаты), дискретные берутся из стартовой точки
  Trial* CreateTrialAt(const OBJECTIV_TYPE* x);

  /// Значение критерия испытания; для недопустимого или ошибочного испытания — HUGE_VAL
  double ObjectiveValue(const Trial* trial) const;

public:

  ParallelHookeJeevesMethod(Task* _pTask, Trial _StartPoint, Calculation& _calculation, int logPoints = 0);
};

#endif //__PARALLEL_HOOKE_JEEVES_METHOD_H__
// - end of file ----------------------------------------------------------------------------------

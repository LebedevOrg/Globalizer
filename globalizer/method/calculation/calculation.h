/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      calculation.h                                               //
//                                                                         //
//  Purpose:   Header file for calculation base class                      //
//                                                                         //
//  Author(s): Lebedev I.                                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __CALCULATION_H__
#define __CALCULATION_H__

#include "common.h"
#include "parameters.h"
#include "task.h"
#include "InformationForCalculation.h"
#include "Trial.h"
#include "SearchData.h"


/**
Базовый класс для проведения испытаний
*/
class TCalculation
{
protected:

  /// Указатель на решаемую задачу
  TTask* pTask;

  /// Данные по решению
  TSearchData* pData;


  /// Количество вычислений за итерацию
  static int countCalculation;

  static bool isStartComputingAway;

  static TInformationForCalculation inputCalculation;

  static TResultForCalculation resultCalculation;

public:

  /// Вычислитель который вызывался изначально
  static TCalculation* firstCalculation;
  /// Вычислитель который вызывался на листьях
  static TCalculation* leafCalculation;


  TCalculation(TTask& _pTask);

  void SetCountCalculation(int c);

  virtual ~TCalculation() {}

  virtual void ContinueComputing() 
  {};

  /// Вычисляет функции fNumber и индекс невыполненного ограничения по координатам
  virtual void Calculate(TInformationForCalculation& inputSet,
    TResultForCalculation& outputSet) = 0;
  /// Задасть используемую задачу
  void SetTask(TTask* _pTask);

  void SetSearchData(TSearchData* _pData);

  //Сброс данных
  virtual void Reset();
};

#endif
// - end of file ----------------------------------------------------------------------------------

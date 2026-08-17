/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2025 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      MixedIntegerMethod.h                                        //
//                                                                         //
//  Purpose:   Header file for method class                                //
//                                                                         //
//  Author(s):  Lebedev.i                                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


/**
\file MixedIntegerMethod.h

\authors  Лебедев И.
\date 2025-2026
\copyright ННГУ им. Н.И. Лобачевского

\brief Объявление класса #MixedIntegerMethod

\details Объявление класса #MixedIntegerMethod и сопутствующих типов данных
*/


#ifndef __MIXED_INTEGER_METHOD_H__
#define __MIXED_INTEGER_METHOD_H__

#include "Method.h"


/**
Базовый класс, реализующий алгоритм глобального поиска.

В классе #Method реализованы основные функции, определяющие работу алгоритма глобального поиска.
*/
class MixedIntegerMethod : public Method
{
protected:


  /**
  Количество дискретных значений
  Произведение числа значений всех дискретных переменных.
  Равно числу интервалов.
  */
  int mDiscreteValuesCount;
  /// Значения дискретных параметров
  std::vector< std::vector< double > > mDiscreteValues;
  /// Индекс первого дискретного параметра
  int startDiscreteVariable;


  /// найденные локальные минимумы
  std::vector<Trial*> localMinimumPoints;

  /// Вычисление координат точек испытания для основной\единственной развертки
  virtual void CalculateCurrentPoint(Trial& pCurTrialsj, SearchInterval* BestIntervalsj);


  /// Задать значения дискретного параметра
  virtual void SetDiscreteValue(int u, std::vector< std::vector <double> > dvs);

  /// Получаем поисковую информацию, важно для адаптивного метода
  virtual SearchData* GetSearchData(Trial* trial);


public:

  MixedIntegerMethod(Task& _pTask, SearchData& _pData,
    Calculation& _Calculation, IEvolvent& _Evolvent);
  virtual ~MixedIntegerMethod();

  /** Функция выполняет первую итерацию метода
  */
  virtual void FirstIteration();

  /** Вычисления точек очередной итерации

Вычисленные точки очередной итерации записываются в массив #iteration.pCurTrials
*/
  virtual void CalculateIterationPoints();



};

#endif
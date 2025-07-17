/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      method.h                                                    //
//                                                                         //
//  Purpose:   Header file for method class                                //
//                                                                         //
//  Author(s):                                   //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

/**
\file

\authors
\date
\copyright ННГУ им. Н.И. Лобачевского

\brief

\details
*/


#ifndef __INFORMATION_FOR_CALCULATION_H__
#define __INFORMATION_FOR_CALCULATION_H__

#include <vector>
#include <fstream>
#include <algorithm>
#include "Trial.h"
#include "AdaptiveTask.h"

// ------------------------------------------------------------------------------------------------
/**

*/
class TInformationForCalculation
{
public: 
  /** Указатель на массив испытаний, выполняемых на данной итерации

  В зависимости от типа метода в данном массиве может быть:
  - одна компонпонента (последовательный алгоритм)
  - #NumPoints компонент (параллельный синхронный и асинхронный алгоритмы)
  */
  std::vector<TTrial*> trials;

  /// Массив указателей
  std::vector<TTask*> tasks;

  TTrial*& operator [] (int i)
  {
    if (i >= 0 && i < trials.size())
      return trials[i];
    else
      throw "operator [] (int i)";
  }

  void SetAllTasks(TTask* task)
  {
    for (unsigned int i = 0; i < tasks.size(); i++)
    {
      tasks[i] = task;
    }
  }

  void ToZero()
  {
    for (unsigned int i = 0; i < tasks.size(); i++)
    {
      tasks[i] = 0;
      trials[i] = 0;
    }
  }

  TInformationForCalculation()
  {
    trials.reserve(1);
    tasks.reserve(1);
    ToZero();
  }

  int GetSize()
  {
    return (int)trials.size();
  }



  void Resize(size_t n)
  {
    for (unsigned int i = 0; i < tasks.size(); i++)
      if (tasks[i] != 0)
      {
        delete tasks[i];
    //tasks[i] = 0;
      }

    Clear();

    trials.resize(n);
    tasks.resize(n);

    ToZero();
  }

  void Clear()
  {
    trials.clear();
    tasks.clear();
  }
};

struct TResultForCalculation
{
  /** Указатель на массив испытаний, выполняемых на данной итерации

  В зависимости от типа метода в данном массиве может быть:
  - одна компонпонента (последовательный алгоритм)
  - #NumPoints компонент (параллельный синхронный и асинхронный алгоритмы)
  */
  std::vector<TTrial*> trials;

  /// Массив указателей на задачи
  std::vector<TTask*> tasks;

  /// Количество вычислений каждой функции
  std::vector<int> countCalcTrials;

  /// Уровень точки
  std::vector<int> procLevel;

  std::vector<TTrial*> NeighboursAdditionalPoints;
  std::vector<int> NeighboursAdditionalProcLevel;

  TResultForCalculation()
  {
    trials.reserve(1);
    tasks.reserve(1);
    countCalcTrials.reserve(1);
    procLevel.reserve(1);
    NeighboursAdditionalPoints.reserve(1);
    NeighboursAdditionalProcLevel.reserve(1);
  }

  void Resize(size_t n)
  {
    trials.resize(n);
    tasks.resize(n);
    countCalcTrials.resize(n);
    procLevel.resize(n);
    NeighboursAdditionalPoints.clear();
    NeighboursAdditionalProcLevel.clear();
  }

  void Clear()
  {
    trials.clear();
    tasks.clear();
    countCalcTrials.clear();
    procLevel.clear();
    NeighboursAdditionalPoints.clear();
    NeighboursAdditionalProcLevel.clear();
  }
};

#endif
// - end of file ----------------------------------------------------------------------------------
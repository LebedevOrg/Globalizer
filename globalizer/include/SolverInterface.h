/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:                                                         //
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

#include <vector>
#include "Trial.h"


#ifndef __SOLVER_INTERFACE_H__
#define __SOLVER_INTERFACE_H__

// ------------------------------------------------------------------------------------------------
/**
Интерфейс, базового класса
*/
class ISolver
{
public:
  /// Виртуальный деструктор
  virtual ~ISolver() {}
  /// Решить задачу
  virtual int Solve() = 0;
  /// Добавляет точки испытаний
  virtual void SetPoint(std::vector<Trial*>& points) = 0;
  /// Возврящает все имеющиеся точки испытаний
  virtual std::vector<Trial*>& GetAllPoint() = 0;
  /// Возвращает полученное решение
  virtual SolutionResult* GetSolutionResult() = 0;
  /// Выполняет автоматическую настройку параметров решателя
  virtual void AutoConfig() {};
};

#endif
// - end of file ----------------------------------------------------------------------------------
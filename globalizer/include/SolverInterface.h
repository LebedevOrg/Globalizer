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


#ifndef __SOLVER_INTERFACE_H__
#define __SOLVER_INTERFACE_H__

// ------------------------------------------------------------------------------------------------
/**
Интерфейс, базового класса
*/
class ISolver
{
public:
  /// Решить задачу
  virtual int Solve() = 0;
};

#endif
// - end of file ----------------------------------------------------------------------------------
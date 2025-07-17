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


#ifndef __SEARCH_ITERATION_H__
#define __SEARCH_ITERATION_H__

#include <vector>
#include <fstream>
#include <algorithm>

#include "Trial.h"
#include "SearchInterval.h"

// ------------------------------------------------------------------------------------------------


/**

*/
struct TSearchIteration
{
  /// номер выполненных итераций
  int IterationCount;

  /** Указатель на массив испытаний, выполняемых на данной итерации

  В зависимости от типа метода в данном массиве может быть:
  - одна компонпонента (последовательный алгоритм)
  - #NumPoints компонент (параллельный синхронный и асинхронный алгоритмы)
  */
  std::vector<TTrial*> pCurTrials;

};

#endif
// - end of file ----------------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      method_factory.h                                            //
//                                                                         //
//  Purpose:   Header file for method factory class                        //
//                                                                         //
//  Author(s): Lebedev I.                                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __TASK_FACTORY_H__
#define __TASK_FACTORY_H__

#include "task.h"

class TTaskFactory
{
public:
  static int num;
  static std::vector<int> permutations;
  static TTask* CreateTask(int _N, int _FreeN, IProblem* _problem, int _ProcLevel);
  static TTask* CreateTask();
  static TTask* CreateTask(TTask* t);
};

#endif
// - end of file ----------------------------------------------------------------------------------

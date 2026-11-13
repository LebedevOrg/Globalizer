/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      method_factory.cpp                                          //
//                                                                         //
//  Purpose:   Source file for method factory class                        //
//                                                                         //
//  Author(s): Lebedev I.                                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "TaskFactory.h"


int TaskFactory::num = 0;
std::vector<int> TaskFactory::permutations;

// ------------------------------------------------------------------------------------------------
Task* TaskFactory::CreateTask(IProblem* _problem, int _ProcLevel)
{
  Task* res = new Task(_problem, 0);
  res->num = num++;
  return res;
}

// ------------------------------------------------------------------------------------------------
Task * TaskFactory::CreateTask()
{
  Task* res = new Task();
  res->num = num++;
  return res;
}

// ------------------------------------------------------------------------------------------------
Task* TaskFactory::CreateTask(Task* t)
{
  Task* res = t->CloneWithNewData();
  return res;
}

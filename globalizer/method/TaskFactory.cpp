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
#include "AdaptiveTask.h"
#include "ShuffledVarsTask.h"

int TTaskFactory::num = 0;
std::vector<int> TTaskFactory::permutations;

// ------------------------------------------------------------------------------------------------

TTask* TTaskFactory::CreateTask(int _N, int _FreeN, IProblem* _problem, int _ProcLevel)
{
  TTask* res = 0;

  if (parameters.TypeSolver == SeparableSearch || parameters.TypeSolver == SeparationVariables) 
  {
    if (permutations.size() < parameters.Dimension)
    {
      permutations.resize(parameters.Dimension);

      for (int i = 0; i < parameters.Dimension; i++)
      {
        permutations[i] = i;
      }
    }
    res = new TShuffledVarsTask(parameters.Dimension, parameters.DimInTaskLevel[0], _problem, 0, permutations);
  }
  else if (parameters.TypeMethod == MCO_Method)
  {
    res = 0; /// Доделать для MKO
  }
  else if ((parameters.TypeMethod == AdaptivMethod) ||
    (parameters.TypeMethod == MultievolventsMethod) ||
    (parameters.TypeMethod == ParallelMultievolventsMethod))
  {
    res = new TAdaptiveTask(parameters.Dimension, parameters.DimInTaskLevel[0], _problem, 0);
  }
  else
    res = new TTask(parameters.Dimension, parameters.DimInTaskLevel[0], _problem, 0);
  res->num = num++;
  return res;
}

TTask * TTaskFactory::CreateTask()
{
  TTask* res = 0;

  if (parameters.TypeSolver == SeparableSearch || parameters.TypeSolver == SeparationVariables) 
  {
    TShuffledVarsTask* res2 = new TShuffledVarsTask();

    if (permutations.size() < parameters.Dimension)
    {
      permutations.resize(parameters.Dimension);

      for (int i = 0; i < parameters.Dimension; i++)
      {
        permutations[i] = i;
      }
    }

    res2->SetPermutation(permutations);
    res = res2;
  }
  else if (parameters.TypeMethod == MCO_Method)
  {
    res = 0; /// Доделать для MKO
  }
  else if ((parameters.TypeMethod == AdaptivMethod) ||
    (parameters.TypeMethod == MultievolventsMethod) ||
    (parameters.TypeMethod == ParallelMultievolventsMethod))
  {
    res = new TAdaptiveTask();
  }
  else
    res = new TTask();
  res->num = num++;
  return res;
}

TTask* TTaskFactory::CreateTask(TTask* t)
{
  TTask* res = t->CloneWithNewData();
  return res;
}

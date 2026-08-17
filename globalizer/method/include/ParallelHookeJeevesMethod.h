/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      local_method.h                                              //
//                                                                         //
//  Purpose:   Header file for local method class                          //
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

class ParallelHookeJeevesMethod : public LocalMethod
{
protected:

  Calculation& calculation;
  /// Входные данные для вычислителя, формирубтся в CalculateFunctionals()
  InformationForCalculation inputSet;
  /// Выходные данные вычислителя, обрабатывается в CalculateFunctionals()
  TResultForCalculation outputSet;
  
  virtual double MakeResearch(OBJECTIV_TYPE*);

  //virtual double EvaluateObjectiveFunctiuon(const OBJECTIV_TYPE*);

  double CheckCoordinate(const OBJECTIV_TYPE* x);

  virtual double EvaluateObjectiveFunctiuon(const OBJECTIV_TYPE*);

public:

  ParallelHookeJeevesMethod(Task* _pTask, Trial _StartPoint, Calculation& _calculation, int logPoints = 0);

//  virtual Trial StartOptimization();
};

#endif //__PARALLEL_HOOKE_JEEVES_METHOD_H__
// - end of file ----------------------------------------------------------------------------------

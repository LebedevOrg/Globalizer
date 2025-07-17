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
#include "method_factory.h"

#include "CalculationFactory.h"

#include "method_calculating.h"
#include "method_global_calculating.h"

#include "AdaptiveMethod.h"

// ------------------------------------------------------------------------------------------------
IMethod* TMethodFactory::CreateMethod(TTask& _pTask, TSearchData& _pData,
  TCalculation& _Calculation, TEvolvent& _Evolvent)
{
  IMethod* pMethod = 0;
  if ((parameters.TypeMethod == StandartMethod) ||
    (parameters.TypeMethod == HybridMethod) ||
    (parameters.TypeMethod == ManyNumPointMethod))
    pMethod = new TMethod(_pTask, _pData, _Calculation, _Evolvent);
  else if (parameters.TypeMethod == AdaptivMethod)
    pMethod = new TAdaptiveMethod(_pTask, _pData, _Calculation, _Evolvent);
  return pMethod;
}
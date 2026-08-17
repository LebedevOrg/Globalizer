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
#include "MethodFactory.h"

#include "CalculationFactory.h"

#include "MixedIntegerMethod.h"



// ------------------------------------------------------------------------------------------------
IMethod* MethodFactory::CreateMethod(Task& _pTask, SearchData& _pData,
  Calculation& _Calculation, IEvolvent& _Evolvent)
{
  IMethod* pMethod = 0;
  if (parameters.TypeMethod == StandartMethod)
    pMethod = new Method(_pTask, _pData, _Calculation, _Evolvent);
  if (parameters.TypeMethod == IntegerMethod)
    pMethod = new MixedIntegerMethod(_pTask, _pData, _Calculation, _Evolvent);
  if (parameters.TypeMethod == RSAMethod)
      pMethod = new Method_RSA(_pTask, _pData, _Calculation, _Evolvent);


  return pMethod;
}
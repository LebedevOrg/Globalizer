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

#ifndef __METHOD_FACTORY_H__
#define __METHOD_FACTORY_H__

#include "Method.h"
#include "RSAMethod.hpp"

class MethodFactory
{
public:
  static IMethod* CreateMethod(Task& _pTask, SearchData& _pData,
    Calculation& _Calculation, IEvolvent& _Evolvent);
};

#endif
// - end of file ----------------------------------------------------------------------------------

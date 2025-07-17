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

#include "method.h"

class TMethodFactory
{
public:
  static IMethod* CreateMethod(TTask& _pTask, TSearchData& _pData,
    TCalculation& _Calculation, TEvolvent& _Evolvent);
};

#endif
// - end of file ----------------------------------------------------------------------------------

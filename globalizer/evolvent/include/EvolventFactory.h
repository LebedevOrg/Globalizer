/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2026 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      evolvent_factory.h                                          //
//                                                                         //
//  Purpose:   Header file for evolvent factory class                      //
//                                                                         //
//  Author(s): Zaitsev A.                                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __EVOLVENT_FACTORY_H__
#define __EVOLVENT_FACTORY_H__

#include "Common.h"
#include "Parameters.h"
#include "Evolvent.h"

class EvolventFactory
{
public:
  static IEvolvent* CreateEvolvent(int _N, int _m);
};

#endif
// - end of file ----------------------------------------------------------------------------------

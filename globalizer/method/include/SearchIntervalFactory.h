/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      data.h                                                      //
//                                                                         //
//  Purpose:   Header file for search data classes                         //
//                                                                         //
//  Author(s): Sysoyev A., Barkalov K., Sovrasov V.                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __SEARCH_INTERVAL_FACTORY_H__
#define __SEARCH_INTERVAL_FACTORY_H__

#include "SearchInterval.h"
#include "AdaptiveSearchInterval.h"



// ------------------------------------------------------------------------------------------------
class TSearchInterval;
class TSearchIntervalFactory
{
public:
  static TSearchInterval* CreateSearchInterval(TSearchInterval& interval);
  static TSearchInterval* CreateSearchInterval();
};



#endif
// - end of file ----------------------------------------------------------------------------------

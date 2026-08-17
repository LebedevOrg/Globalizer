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

#ifndef __TRIAL_FACTORY_H__
#define __TRIAL_FACTORY_H__

#include "Parameters.h"
//#include "Trial.h"


class Trial;
class TrialFactory
{
public:
  //static Trial* CreateTrial(Trial& interval);
  static Trial* CreateTrial()
  {
    return new Trial();
  }

  static Trial* CreateTrial(const OBJECTIV_TYPE* StartPoint)
  {
    Trial* res;
    res = new Trial();
    memcpy(res->y, StartPoint, parameters.Dimension * sizeof(double));

    return res;
  }


  static Trial* CreateTrial(Trial* point)
  {
    return new Trial(*point);
  }
};



#endif
// - end of file ----------------------------------------------------------------------------------

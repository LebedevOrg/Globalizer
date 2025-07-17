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

#include "parameters.h"
//#include "Trial.h"


class TTrial;
class TMultievolventsTrial;
class TTrialFactory
{
public:
  //static TTrial* CreateTrial(TTrial& interval);
  static TTrial* CreateTrial()
  {
    if ((parameters.TypeMethod == MultievolventsMethod) || (parameters.TypeMethod == ParallelMultievolventsMethod))
      return new TMultievolventsTrial();
    else
      return new TTrial();
  }

  static TTrial* CreateTrial(const OBJECTIV_TYPE* startPoint)
  {
    TTrial* res;
    if ((parameters.TypeMethod == MultievolventsMethod) || (parameters.TypeMethod == ParallelMultievolventsMethod))
      res = new TMultievolventsTrial();
    else
      res = new TTrial();
    memcpy(res->y, startPoint, parameters.Dimension * sizeof(double));

    return res;
  }

  static TTrial* CreateTrial(TMultievolventsTrial* point)
  {    
    return new TMultievolventsTrial(*point);
  }

  static TTrial* CreateTrial(TTrial* point)
  {
    return new TTrial(*point);
  }
};


//// ------------------------------------------------------------------------------------------------
//TTrial* TTrialFactory::CreateTrial(TTrial& interval)
//{
//  if (parameters.TypeMethod == MultievolventsMethod)
//    return new TMultievolventsTrial((TSearchInterval&)(interval));
//  else
//    return new TSearchInterval(interval);
//}

//// ------------------------------------------------------------------------------------------------
//TTrial* TTrialFactory::CreateTrial()
//{
//  if (parameters.TypeMethod == MultievolventsMethod)
//    return new TMultievolventsTrial();
//  else
//    return new TTrial();
//}


#endif
// - end of file ----------------------------------------------------------------------------------

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

#ifndef __BASE_INTERVAL_H__
#define __BASE_INTERVAL_H__

#include "queue_common.h"

class TQueueBaseData
{
protected:
  /// Элемент очереди хранящий этот интервал
  TQueueElement* queueElementa;
public:
  virtual void SetQueueElementa(TQueueElement* q)
  {
    queueElementa = q;
  }
  virtual TQueueElement* GetQueueElementa()
  {
    return queueElementa;
  }
};

#endif //__BASE_INTERVAL_H__
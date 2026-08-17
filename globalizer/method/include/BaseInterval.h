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

#include "QueueCommon.h"

class QueueBaseData
{
protected:

  /// Элемент очереди, хранящий этот интервал
  QueueElement* queueElementa;
public:

  /// Виртуальный деструктор
  virtual ~QueueBaseData() {}

  virtual void SetQueueElementa(QueueElement* q)
  {
    queueElementa = q;
  }
  virtual QueueElement* GetQueueElementa()
  {
    return queueElementa;
  }
};

#endif //__BASE_INTERVAL_H__

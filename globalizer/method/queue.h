/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      Queue.h                                                     //
//                                                                         //
//  Purpose:   Header file for priority queue class                        //
//                                                                         //
//  Author(s): Sysoyev A., Barkalov K., Sovrasov V.                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "minmaxheap.h"
#include "common.h"
#include "queue_common.h"

class TPriorityQueue : public TPriorityQueueCommon
{
protected:
  int MaxSize;
  int CurSize;
  MinMaxHeap< TQueueElement, _less >* pMem;

  int GetIndOfMinElem();
  void DeleteMinElem();

public:
  TPriorityQueue(int _MaxSize = DefaultQueueSize); // _MaxSize must be qual to 2^k - 1
  ~TPriorityQueue();

  int GetSize() const;
  int GetMaxSize() const;
  bool IsEmpty() const;
  bool IsFull() const;

  //localKey value is not really used by the Push and PushWithPriority methods
  TQueueElement* Push(double globalKey, double localKey, void *value);
  TQueueElement* PushWithPriority(double globalKey, double localKey, void *value);
  void Pop(double *key, void **value);
  void DeleteByValue(void *value);

  /// Удаляет элемент
  void DeleteElement(TQueueElement * item);

  void Clear();
  void Resize(int size);

  TQueueElement& FindMax();
  void TrickleUp(TQueueElement * item);
  void TrickleDown(TQueueElement * item)
  {
    pMem->TrickleDown(item);
  }
};
#endif
// - end of file ----------------------------------------------------------------------------------
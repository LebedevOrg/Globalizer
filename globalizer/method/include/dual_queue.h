/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      dual_queue.h                                                //
//                                                                         //
//  Purpose:   Header file for priority dual queue class                   //
//                                                                         //
//  Author(s): Sysoyev A., Barkalov K., Sovrasov V.                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __DUAL_QUEUE_H__
#define __DUAL_QUEUE_H__

#include "minmaxheap.h"
#include "common.h"
#include "queue_common.h"

class TPriorityDualQueue : public TPriorityQueueCommon
{
protected:
  int MaxSize;
  int CurLocalSize;
  int CurGlobalSize;

  MinMaxHeap< TQueueElement, _less >* pGlobalHeap;
  MinMaxHeap< TQueueElement, _less >* pLocalHeap;

  void DeleteMinLocalElem();
  void DeleteMinGlobalElem();
  void ClearLocal();
  void ClearGlobal();
public:

  TPriorityDualQueue(int _MaxSize = DefaultQueueSize); // _MaxSize must be qual to 2^k - 1
  ~TPriorityDualQueue();

  int GetLocalSize() const;
  int GetSize() const;
  int GetMaxSize() const;
  bool IsLocalEmpty() const;
  bool IsLocalFull() const;
  bool IsEmpty() const;
  bool IsFull() const;

  TQueueElement* Push(double globalKey, double localKey, void *value);
  TQueueElement* PushWithPriority(double globalKey, double localKey, void *value);
  void Pop(double *key, void **value);
  void DeleteByValue(void *value);
  /// Удаляет элемент
  virtual void DeleteElement(TQueueElement * item);
  void PopFromLocal(double *key, void **value);

  void Clear();
  void Resize(int size);
  virtual TQueueElement& FindMax()
  {
    return pGlobalHeap->findMax();
  }
  void TrickleUp(TQueueElement * item)
  {
    pGlobalHeap->TrickleUp(item);
  }

  void TrickleDown(TQueueElement * item)
  {
    pGlobalHeap->TrickleDown(item);
  }
};
#endif
// - end of file ----------------------------------------------------------------------------------
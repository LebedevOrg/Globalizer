/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      Queue.cpp                                                   //
//                                                                         //
//  Purpose:   Source file for priority queue class                        //
//                                                                         //
//  Author(s): Sysoyev A., Barkalov K., Sovrasov V                         //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "queue.h"
#include "baseInterval.h"
// ------------------------------------------------------------------------------------------------
TPriorityQueue::TPriorityQueue(int _MaxSize)
{
  int tmpPow2 = _MaxSize + 1;
  if (tmpPow2&(tmpPow2 - 1))
  {
    throw EXCEPTION("Max size of queue not divisible by power of two");
  }
  MaxSize = _MaxSize;
  CurSize = 0;
  pMem = new MinMaxHeap< TQueueElement, _less >(MaxSize);
}

// ------------------------------------------------------------------------------------------------
TPriorityQueue::~TPriorityQueue()
{
  delete pMem;
}

// ------------------------------------------------------------------------------------------------
bool TPriorityQueue::IsEmpty() const
{
  return CurSize == 0;
}
// ------------------------------------------------------------------------------------------------
int TPriorityQueue::GetSize() const
{
  return CurSize;
}
// ------------------------------------------------------------------------------------------------
int TPriorityQueue::GetMaxSize() const
{
  return MaxSize;
}

// ------------------------------------------------------------------------------------------------
bool TPriorityQueue::IsFull() const
{
  return CurSize == MaxSize;
}

// ------------------------------------------------------------------------------------------------
TQueueElement* TPriorityQueue::Push(double globalKey, double localKey, void *value)
{
  TQueueElement* a = 0;
  if (!IsFull()) {
    CurSize++;
    a = pMem->push(TQueueElement(globalKey, value));
  }
  else {
    if (globalKey > pMem->findMin().Key)
      DeleteMinElem();
    else
      return a;
    CurSize++;
    a = pMem->push(TQueueElement(globalKey, value));
  }
  return a;
}

// ------------------------------------------------------------------------------------------------
TQueueElement* TPriorityQueue::PushWithPriority(double globalKey, double localKey, void *value)
{
  TQueueElement* a = 0;
  if (!IsEmpty()) {
    if (globalKey >= pMem->findMin().Key) {
      if (IsFull())
        DeleteMinElem();
      CurSize++;
      a = pMem->push(TQueueElement(globalKey, value));
    }
  }
  else {
    CurSize++;
    a = pMem->push(TQueueElement(globalKey, value));
  }
  return a;
}

// ------------------------------------------------------------------------------------------------
void TPriorityQueue::Pop(double *key, void **value)
{
  if (CurSize != 0)
  {
    TQueueElement tmp = pMem->popMax();
    *key = tmp.Key;
    *value = tmp.pValue;
    CurSize--;
  }
  else
  {
    throw EXCEPTION("Cannot pop element from empty queue");
  }
}

// ------------------------------------------------------------------------------------------------
void TPriorityQueue::DeleteByValue(void *value)
{
  TQueueElement* heapMem = pMem->getHeapMemPtr();
  for (int i = 0; i < CurSize; i++)
  {
    if (heapMem[i].pValue == value)
    {
      pMem->deleteElement(heapMem + i);
      CurSize--;

      break;
    }
  }
}

// ------------------------------------------------------------------------------------------------
void TPriorityQueue::DeleteElement(TQueueElement* item)
{
  pMem->deleteElement(item);
  CurSize--;
}

// ------------------------------------------------------------------------------------------------
void TPriorityQueue::TrickleUp(TQueueElement * item)
{
  pMem->TrickleUp(item);
}

// ------------------------------------------------------------------------------------------------
void TPriorityQueue::DeleteMinElem()
{
  TQueueElement tmp = pMem->popMin();
  CurSize--;
}

// ------------------------------------------------------------------------------------------------
void TPriorityQueue::Clear()
{
  pMem->clear();
  CurSize = 0;
}

// ------------------------------------------------------------------------------------------------
void TPriorityQueue::Resize(int size)
{
  CurSize = 0;
  MaxSize = size;
  delete pMem;
  pMem = new MinMaxHeap< TQueueElement, _less >(MaxSize);
}

// ------------------------------------------------------------------------------------------------
TQueueElement& TPriorityQueue::FindMax()
{
  return pMem->findMax();
}
// - end of file ----------------------------------------------------------------------------------

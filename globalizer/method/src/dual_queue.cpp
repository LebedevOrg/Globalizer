/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      dual_queue.cpp                                              //
//                                                                         //
//  Purpose:   Source file for priority queue class                        //
//                                                                         //
//  Author(s): Sysoyev A., Barkalov K., Sovrasov V.                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "dual_queue.h"

// ------------------------------------------------------------------------------------------------
TPriorityDualQueue::TPriorityDualQueue(int _MaxSize)
{
  int tmpPow2 = _MaxSize + 1;
  if (tmpPow2&(tmpPow2 - 1))
  {
    throw EXCEPTION("Max size of queue not divisible by power of two");
  }
  MaxSize = _MaxSize;
  CurGlobalSize = CurLocalSize = 0;
  pLocalHeap = new MinMaxHeap< TQueueElement, _less >(MaxSize);
  pGlobalHeap = new MinMaxHeap< TQueueElement, _less >(MaxSize);
}

// ------------------------------------------------------------------------------------------------
TPriorityDualQueue::~TPriorityDualQueue()
{
  delete pLocalHeap;
  delete pGlobalHeap;
}

// ------------------------------------------------------------------------------------------------
void TPriorityDualQueue::DeleteMinLocalElem()
{
  TQueueElement tmp = pLocalHeap->popMin();
  CurLocalSize--;

  //update linked element in the global queue
  if (tmp.pLinkedElement != NULL)
    tmp.pLinkedElement->pLinkedElement = NULL;
}

// ------------------------------------------------------------------------------------------------
void TPriorityDualQueue::DeleteMinGlobalElem()
{
  TQueueElement tmp = pGlobalHeap->popMin();
  CurGlobalSize--;

  //update linked element in the local queue
  if (tmp.pLinkedElement != NULL)
    tmp.pLinkedElement->pLinkedElement = NULL;
}

// ------------------------------------------------------------------------------------------------
int TPriorityDualQueue::GetLocalSize() const
{
  return CurLocalSize;
}

// ------------------------------------------------------------------------------------------------
int TPriorityDualQueue::GetSize() const
{
  return CurGlobalSize;
}
// ------------------------------------------------------------------------------------------------
int TPriorityDualQueue::GetMaxSize() const
{
  return MaxSize;
}

// ------------------------------------------------------------------------------------------------
bool TPriorityDualQueue::IsLocalEmpty() const
{
  return CurLocalSize == 0;
}

// ------------------------------------------------------------------------------------------------
bool TPriorityDualQueue::IsLocalFull() const
{
  return CurLocalSize == MaxSize;
}

// ------------------------------------------------------------------------------------------------
bool TPriorityDualQueue::IsEmpty() const
{
  return CurGlobalSize == 0;
}

// ------------------------------------------------------------------------------------------------
bool TPriorityDualQueue::IsFull() const
{
  return CurGlobalSize == MaxSize;
}

// ------------------------------------------------------------------------------------------------
TQueueElement* TPriorityDualQueue::Push(double globalKey, double localKey, void * value)
{
  TQueueElement* pGlobalElem = NULL, *pLocalElem = NULL;
  //push to a global queue
  if (!IsFull()) {
    CurGlobalSize++;
    pGlobalElem = pGlobalHeap->push(TQueueElement(globalKey, value));
  }
  else {
    if (globalKey > pGlobalHeap->findMin().Key) {
      DeleteMinGlobalElem();
      CurGlobalSize++;
      pGlobalElem = pGlobalHeap->push(TQueueElement(globalKey, value));
    }
  }
  //push to a local queue
  if (!IsLocalFull()) {
    CurLocalSize++;
    pLocalElem = pLocalHeap->push(TQueueElement(localKey, value));
  }
  else {
    if (localKey > pLocalHeap->findMin().Key) {
      DeleteMinLocalElem();
      CurLocalSize++;
      pLocalElem = pLocalHeap->push(TQueueElement(localKey, value));
    }
  }
  //link elements
  if (pGlobalElem != NULL && pLocalElem != NULL) {
    pGlobalElem->pLinkedElement = pLocalElem;
    pLocalElem->pLinkedElement = pGlobalElem;
  }

  return pGlobalElem;
}

// ------------------------------------------------------------------------------------------------
TQueueElement* TPriorityDualQueue::PushWithPriority(double globalKey, double localKey, void * value)
{
  TQueueElement* pGlobalElem = NULL, *pLocalElem = NULL;
  //push to a global queue
  if (!IsEmpty()) {
    if (globalKey >= pGlobalHeap->findMin().Key) {
      if (IsFull())
        DeleteMinGlobalElem();
      CurGlobalSize++;
      pGlobalElem = pGlobalHeap->push(TQueueElement(globalKey, value));
    }
  }
  else {
    CurGlobalSize++;
    pGlobalElem = pGlobalHeap->push(TQueueElement(globalKey, value));
  }
  //push to a local queue
  if (!IsLocalEmpty()) {
    if (localKey >= pLocalHeap->findMin().Key) {
      if (IsLocalFull())
        DeleteMinLocalElem();
      CurLocalSize++;
      pLocalElem = pLocalHeap->push(TQueueElement(localKey, value));
    }
  }
  else {
    CurLocalSize++;
    pLocalElem = pLocalHeap->push(TQueueElement(localKey, value));
  }
  //link elements
  if (pGlobalElem != NULL && pLocalElem != NULL) {
    pGlobalElem->pLinkedElement = pLocalElem;
    pLocalElem->pLinkedElement = pGlobalElem;
  }

  return pGlobalElem;
}

// ------------------------------------------------------------------------------------------------
void TPriorityDualQueue::PopFromLocal(double * key, void ** value)
{
  if (CurLocalSize != 0)
  {
    TQueueElement tmp = pLocalHeap->popMax();
    *key = tmp.Key;
    *value = tmp.pValue;
    CurLocalSize--;

    //delete linked element from the global queue
    if (tmp.pLinkedElement != NULL) {
      pGlobalHeap->deleteElement(tmp.pLinkedElement);
      CurGlobalSize--;
    }
  }
  else
  {
    throw EXCEPTION("Cannot pop element from empty queue");
  }
}

// ------------------------------------------------------------------------------------------------
void TPriorityDualQueue::Pop(double * key, void ** value)
{
  if (CurGlobalSize != 0)
  {
    TQueueElement tmp = pGlobalHeap->popMax();
    *key = tmp.Key;
    *value = tmp.pValue;
    CurGlobalSize--;

    //delete linked element from the local queue
    if (tmp.pLinkedElement != NULL)
    {
      pLocalHeap->deleteElement(tmp.pLinkedElement);
      CurLocalSize--;
    }
  }
  else
  {
    throw EXCEPTION("Cannot pop element from empty queue");
  }
}

// ------------------------------------------------------------------------------------------------
void TPriorityDualQueue::DeleteByValue(void *value)
{
  TQueueElement* globalHeapMem = pGlobalHeap->getHeapMemPtr();
  for (int i = 0; i < CurGlobalSize; i++)
    if (globalHeapMem[i].pValue == value)
    {
      //delete linked element from the local queue
      if (globalHeapMem[i].pLinkedElement != NULL)
      {
        pLocalHeap->deleteElement(globalHeapMem[i].pLinkedElement);
        CurLocalSize--;
      }
      CurGlobalSize--;
      pGlobalHeap->deleteElement(globalHeapMem + i);
      return;
    }

  //if the value exists only in local queue
  TQueueElement* localHeapMem = pLocalHeap->getHeapMemPtr();
  for (int i = 0; i < CurLocalSize; i++)
    if (localHeapMem[i].pValue == value)
    {
      CurLocalSize--;
      pLocalHeap->deleteElement(localHeapMem + i);
      break;
    }
}

// ------------------------------------------------------------------------------------------------
void TPriorityDualQueue::DeleteElement(TQueueElement* item)
{
  DeleteByValue(item->pValue);
  //TQueueElement* heapMem = pMem->getHeapMemPtr();
  //pMem->deleteElement(item);
  //CurSize--;
}

// ------------------------------------------------------------------------------------------------
void TPriorityDualQueue::Clear()
{
  ClearLocal();
  ClearGlobal();
}

// ------------------------------------------------------------------------------------------------
void TPriorityDualQueue::Resize(int size)
{
  MaxSize = size;
  CurGlobalSize = CurLocalSize = 0;
  delete pLocalHeap;
  delete pGlobalHeap;
  pLocalHeap = new MinMaxHeap< TQueueElement, _less >(MaxSize);
  pGlobalHeap = new MinMaxHeap< TQueueElement, _less >(MaxSize);
}

// ------------------------------------------------------------------------------------------------
void TPriorityDualQueue::ClearLocal()
{
  pLocalHeap->clear();
  CurLocalSize = 0;
}

// ------------------------------------------------------------------------------------------------
void TPriorityDualQueue::ClearGlobal()
{
  pGlobalHeap->clear();
  CurGlobalSize = 0;
}
// - end of file ----------------------------------------------------------------------------------
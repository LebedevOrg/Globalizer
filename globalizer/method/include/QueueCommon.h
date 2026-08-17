/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      queue_common.h                                              //
//                                                                         //
//  Purpose:   Header file for abstract queue class                        //
//                                                                         //
//  Author(s): Sysoyev A., Barkalov K., Sovrasov V.                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __QUEUECOMMON_H__
#define __QUEUECOMMON_H__

struct QueueElement
{
  QueueElement *pLinkedElement = nullptr;
  double Key = 0;
  void *pValue = nullptr;

  QueueElement()
  {}
  QueueElement(double _Key, void *_pValue) :
    Key(_Key), pValue(_pValue), pLinkedElement(0)
  {}
  QueueElement(double _Key, void *_pValue, QueueElement* _pLinkedElement) :
    Key(_Key), pValue(_pValue), pLinkedElement(_pLinkedElement)
  {}
};

template<class _Arg1,
  class _Arg2,
  class _Result>
  struct _binary_function
{	// base class for binary functions
  typedef _Arg1 first_argument_type;
  typedef _Arg2 second_argument_type;
  typedef _Result result_type;
};

struct _less : public _binary_function<QueueElement, QueueElement, bool>
{	// functor for operator<
  bool operator()(const QueueElement& _Left, const QueueElement& _Right) const
  {	// apply operator< to operands
    return (_Left.Key < _Right.Key);
  }
};

class PriorityQueueCommon
{
public:
  virtual ~PriorityQueueCommon() {}

  virtual int GetSize() const = 0;
  virtual int GetMaxSize() const = 0;
  virtual bool IsEmpty() const = 0;
  virtual bool IsFull() const = 0;

  virtual QueueElement* Push(double globalKey, double localKey, void *value) = 0;
  virtual QueueElement* PushWithPriority(double globalKey, double localKey, void *value) = 0;
  virtual void Pop(double *key, void **value) = 0;
  virtual void DeleteByValue(void *value) = 0;
  /// Удаляет элемент
  virtual void DeleteElement(QueueElement * item) = 0;
  virtual void Clear() = 0;
  virtual void Resize(int size) = 0;
  virtual QueueElement& FindMax() = 0;
  virtual void TrickleUp(QueueElement * item) = 0;
  virtual void TrickleDown(QueueElement * item) = 0;
};

#endif
// - end of file ----------------------------------------------------------------------------------
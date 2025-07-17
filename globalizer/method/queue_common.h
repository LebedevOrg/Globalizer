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

struct TQueueElement
{
  TQueueElement *pLinkedElement;
  double Key;
  void *pValue;

  TQueueElement() {}
  TQueueElement(double _Key, void *_pValue) :
    Key(_Key), pValue(_pValue), pLinkedElement(0)
  {}
  TQueueElement(double _Key, void *_pValue, TQueueElement* _pLinkedElement) :
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

struct _less : public _binary_function<TQueueElement, TQueueElement, bool>
{	// functor for operator<
  bool operator()(const TQueueElement& _Left, const TQueueElement& _Right) const
  {	// apply operator< to operands
    return (_Left.Key < _Right.Key);
  }
};

class TPriorityQueueCommon
{
public:
  virtual ~TPriorityQueueCommon() {}

  virtual int GetSize() const = 0;
  virtual int GetMaxSize() const = 0;
  virtual bool IsEmpty() const = 0;
  virtual bool IsFull() const = 0;

  virtual TQueueElement* Push(double globalKey, double localKey, void *value) = 0;
  virtual TQueueElement* PushWithPriority(double globalKey, double localKey, void *value) = 0;
  virtual void Pop(double *key, void **value) = 0;
  virtual void DeleteByValue(void *value) = 0;
  /// Удаляет элемент
  virtual void DeleteElement(TQueueElement * item) = 0;
  virtual void Clear() = 0;
  virtual void Resize(int size) = 0;
  virtual TQueueElement& FindMax() = 0;
  virtual void TrickleUp(TQueueElement * item) = 0;
  virtual void TrickleDown(TQueueElement * item) = 0;
};

#endif
// - end of file ----------------------------------------------------------------------------------
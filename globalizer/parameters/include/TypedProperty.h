/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2016 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      TypedProperty.h                                                  //
//                                                                         //
//  Purpose:   Header file for random generator class                      //
//                                                                         //
//  Author(s): Lebedev I.                                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __TYPED_PROPERTY_H__
#define __TYPED_PROPERTY_H__

/**
\file TypedProperty.h

\authors ������� �.
\date 2015-2016
\copyright ���� ��. �.�. ������������

\brief ���������� ������ ������� ��� �������

*/

#include "BaseProperty.h"



/* ======================================================================== *\
**  ���������� �������                                                      **
\* ======================================================================== */


/**
������� ����� ��� �������
*/
template <class Type, class Owner>
class TTypedProperty : public TBaseProperty<Owner>
{
protected:
  typedef typename TBaseProperty<Owner>::tCheckValue tCheckValue;

  /// ��� ������ ������������� ������� ������
  typedef Type(Owner::*tGetter)() const;
  /// ��� ������ ��������� ������� ������
  typedef void (Owner::*tSetter)(Type);

  /// ����� ��������
  int mIndex;

  /// ����� ������������ ������� ������
  tGetter mGetter;
  /// ����� �������� ������� ������
  tSetter mSetter;
  /// ������� �������� ������
  tCheckValue mCheckValue;
  /// ���������� ������
  Type mValue;
  /// ����� ������ ������������ ���������� ��� �������
  bool mIsHaveValue;
  /// ���� �� �������� �������� ����� �������������
  bool mIsChange;
  /// �������� �� �������� ������, � �������� �������� �� ���������
  bool mIsFlag;

  /// ����������� ���������� ��������
  virtual void CopyValue(Type value);
public:

  /// ������ ������������ �� ���������� ������ ��� �������
  void SetIsHaveValue(bool isHaveValue);
  /// ����� ������ ������������, ���������� ��� �������
  bool GetIsHaveValue();

  /// ������ ������ �������� ��� ��������
  void SetIndex(int index);
  /// ���������� ������ �������� ��� ��������
  int GetIndex();

  /// ������������� �������� ���������� ����, ������
  virtual operator Type() const;
  /// ������������� �������� ������������, ������
  virtual void operator =(Type data);

  /// ���������� ��������� ���������� ���� ��������
  virtual void operator = (TTypedProperty<Type, Owner>& data);
  /// ���������� ��������� ���������� ��� ���������
  virtual void operator = (TTypedProperty<Type, Owner>* data);

  /// �������� ������ �� ��������� � ���� ������
  virtual void Copy(void* data);
  /// ������ ������ ������� � void* � ���� �������
  virtual void SetValue(void* data);
  /// ���������� ��������� �� ������ ���������� � �������
  virtual void* GetValue();
  /// ������������� �������
  virtual void Init(Owner * owner, tGetter getMethod, tSetter setMethod, tCheckValue checkMethod);

  /// ���������� ������ � ������������ � ���������
  virtual Type GetData();
  /// ���������� ������������� ������� ������
  virtual Type GetAvailableData() const;
  /// ���� �� �������� �������� ����� �������������
  virtual bool GetIsChange();

  TTypedProperty();
  TTypedProperty(Type value);
  TTypedProperty(Owner * owner, tGetter getMethod, tSetter setMethod, tCheckValue checkMethod, Type value);
  virtual ~TTypedProperty();

  /// ������ ������� ������
  virtual void SetSetter(tSetter setter)
  {
    mSetter = setter;

    if ((mGetter != 0) || (mSetter != 0))
      mIsHaveValue = false;
    else
      mIsHaveValue = true;
  }
  ///// ���������� ������� ������
  virtual tSetter GetSetter()
  {
    return mSetter;
  }
  /// ������ ������� ������
  virtual void SetGetter(tGetter getter)
  {
    mGetter = getter;

    if ((mGetter != 0) || (mSetter != 0))
      mIsHaveValue = false;
    else
      mIsHaveValue = true;
  }
  ///// ���������� ������� ������
  virtual tGetter GetGetter() const
  {
    return mGetter;
  }
  /// ������ ������� ����������� ��������
  virtual void SetCheckValue(tCheckValue checkValue)
  {
    mCheckValue = checkValue;
  }
  /// ���������� ������� ����������� ��������
  virtual tCheckValue GetCheckValue()
  {
    return mCheckValue;
  }

  /// �������� �� �������� ������, � �������� �������� �� ���������.
  virtual bool IsFlag()
  {
    return mIsFlag;
  }

  /// ��������� ������������ ��������
  virtual int CheckValue()
  {
    int err = 0;
    if (mCheckValue != 0)
      err = (this->mOwner->*mCheckValue)(mIndex);
    return err;
  }

};


/* ======================================================================== *\
**  ���������� ������� ������     TTypedProperty                                 **
\* ======================================================================== */

// ------------------------------------------------------------------------------------------------
/// ����������� ���������� ��������
template <class Type, class Owner>
void TTypedProperty<Type, Owner>::CopyValue(Type value)
{
  mValue = value;
}

// ------------------------------------------------------------------------------------------------
/// ������ ������������ �� ���������� ������ ��� �������
template <class Type, class Owner>
void TTypedProperty<Type, Owner>::SetIsHaveValue(bool isHaveValue)
{
  mIsHaveValue = isHaveValue;
}

// ------------------------------------------------------------------------------------------------
/// ����� ������ ������������, ���������� ��� �������
template <class Type, class Owner>
bool TTypedProperty<Type, Owner>::GetIsHaveValue()
{
  return mIsHaveValue;
}

// ------------------------------------------------------------------------------------------------
/// ������ ������ �������� ��� ��������
template <class Type, class Owner>
void TTypedProperty<Type, Owner>::SetIndex(int index)
{
  mIndex = index;
}

// ------------------------------------------------------------------------------------------------
/// ���������� ������ �������� ��� ��������
template <class Type, class Owner>
int TTypedProperty<Type, Owner>::GetIndex()
{
  return mIndex;
}

// ------------------------------------------------------------------------------------------------
/// ������������� �������� ���������� ����, ������
template <class Type, class Owner>
TTypedProperty<Type, Owner>::operator Type() const
{
  if ((mIsHaveValue) || (mGetter == 0))
    return mValue;
  else
    return (this->mOwner->*mGetter)();
}

// ------------------------------------------------------------------------------------------------
/// ������������� �������� ������������, ������
template <class Type, class Owner>
void TTypedProperty<Type, Owner>::operator =(Type data)
{
  Type oldVal = mValue;
  mIsChange = true;
  if ((mIsHaveValue) || (mSetter == 0))
    CopyValue(data);
  else
    (this->mOwner->*mSetter)(data);
  int err = 0;
  if (mCheckValue != 0)
    err = (this->mOwner->*mCheckValue)(mIndex);
  if (err != 0)
  {
    CopyValue(oldVal);
  }
}

// ------------------------------------------------------------------------------------------------
/// ���������� ��������� ���������� ���� ��������
template <class Type, class Owner>
void TTypedProperty<Type, Owner>::operator = (TTypedProperty<Type, Owner>& data)
{
  CopyValue(data.mValue);
  mIsHaveValue = data.mIsHaveValue;
  mGetter = data.mGetter;
  mSetter = data.mSetter;
  mCheckValue = data.mCheckValue;
  this->mOwner = data.mOwner;
  mIndex = data.mIndex;
  mIsChange = data.mIsChange;
}

// ------------------------------------------------------------------------------------------------
/// ���������� ��������� ���������� ��� ���������
template <class Type, class Owner>
void TTypedProperty<Type, Owner>::operator = (TTypedProperty<Type, Owner>* data)
{
  CopyValue(data->mValue);
  mIsHaveValue = data->mIsHaveValue;
  mGetter = data->mGetter;
  mSetter = data->mSetter;
  mCheckValue = data->mCheckValue;
  this->mOwner = data->mOwner;
  mIndex = data->mIndex;
  mIsChange = data->mIsChange;
}

// ------------------------------------------------------------------------------------------------
/// �������� ������ �� ��������� � ���� ������
template <class Type, class Owner>
void TTypedProperty<Type, Owner>::Copy(void* data)
{
  operator = ((TTypedProperty<Type, Owner>*)data);
}

// ------------------------------------------------------------------------------------------------
/// ������ ������ ������� � void* � ���� �������
template <class Type, class Owner>
void TTypedProperty<Type, Owner>::SetValue(void* data)
{
  *this = *((Type*)(data));
}

// ------------------------------------------------------------------------------------------------
/// ���������� ��������� �� ������ ���������� � �������
template <class Type, class Owner>
void* TTypedProperty<Type, Owner>::GetValue()
{
  if ((mIsHaveValue) || (mGetter == 0))
    return (void*)&mValue;
  else
  {
    Type* val = new Type((this->mOwner->*mGetter)());
    return (void*)val;
  }
}

// ------------------------------------------------------------------------------------------------
/// ������������� �������
template <class Type, class Owner>
void TTypedProperty<Type, Owner>::Init(Owner * owner, tGetter getMethod, tSetter setMethod, tCheckValue checkMethod)
{
  this->mOwner = owner;
  mGetter = getMethod;
  mSetter = setMethod;
  mCheckValue = checkMethod;
  if ((getMethod != 0) || (setMethod != 0))
    mIsHaveValue = false;
  else
    mIsHaveValue = true;
  mIsChange = false;
};

// ------------------------------------------------------------------------------------------------
/// ���������� ������ � ������������ � ���������
template <class Type, class Owner>
Type TTypedProperty<Type, Owner>::GetData()
{
  return operator Type();
}

// ------------------------------------------------------------------------------------------------
/// ���������� ������������� ������� ������
template <class Type, class Owner>
Type TTypedProperty<Type, Owner>::GetAvailableData() const
{
  return mValue;
}

// ------------------------------------------------------------------------------------------------
/// ���� �� �������� �������� ����� �������������
template <class Type, class Owner>
bool TTypedProperty<Type, Owner>::GetIsChange()
{
  return mIsChange;
}

// ------------------------------------------------------------------------------------------------
template <class Type, class Owner>
TTypedProperty<Type, Owner>::TTypedProperty() : TBaseProperty<Owner>(0), mGetter(0), mSetter(0), mCheckValue(0)
{
  mIsHaveValue = true;
  mIndex = 0;
  mIsChange = false;
  mIsFlag = false;
  this->mIsPreChange = false;
}

// ------------------------------------------------------------------------------------------------
template <class Type, class Owner>
TTypedProperty<Type, Owner>::TTypedProperty(Type value) : TBaseProperty<Owner>(0), mGetter(0), mSetter(0), mCheckValue(0)
{
  CopyValue(value);
  mIsHaveValue = true;
  mIndex = 0;
  mIsChange = false;
  mIsFlag = false;
  this->mIsPreChange = false;
}

// ------------------------------------------------------------------------------------------------
template <class Type, class Owner>
TTypedProperty<Type, Owner>::TTypedProperty(Owner * owner, tGetter getMethod, tSetter setMethod, tCheckValue checkMethod, Type value) :
  TBaseProperty<Owner>(owner), mGetter(getMethod), mSetter(setMethod), mCheckValue(checkMethod)
{
  CopyValue(value);
  if ((getMethod != 0) || (setMethod != 0))
    mIsHaveValue = false;
  else
    mIsHaveValue = true;
  mIsChange = false;
  mIndex = 0;
  mIsFlag = false;
  this->mIsPreChange = false;
}

// ------------------------------------------------------------------------------------------------
template <class Type, class Owner>
TTypedProperty<Type, Owner>::~TTypedProperty()
{}

#endif //__TYPED_PROPERTY_H__
// - end of file ----------------------------------------------------------------------------------

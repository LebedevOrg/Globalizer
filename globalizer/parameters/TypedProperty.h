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

\authors Лебедев И.
\date 2015-2016
\copyright ННГУ им. Н.И. Лобачевского

\brief Объявление базоых классов для свойств

*/

#include "BaseProperty.h"



/* ======================================================================== *\
**  Объявление классов                                                      **
\* ======================================================================== */


/**
Базовый класс для свойств
*/
template <class Type, class Owner>
class TTypedProperty : public TBaseProperty<Owner>
{
protected:
  typedef typename TBaseProperty<Owner>::tCheckValue tCheckValue;

  /// Тип метода возвращающего внешние данные
  typedef Type(Owner::*tGetter)() const;
  /// Тип метода задающего внешние данные
  typedef void (Owner::*tSetter)(Type);

  /// Индек свойства
  int mIndex;

  /// Метод возвращающий внешние данные
  tGetter mGetter;
  /// Метод задающий внешние данные
  tSetter mSetter;
  /// Внешняя проверка данных
  tCheckValue mCheckValue;
  /// Внутренные данные
  Type mValue;
  /// Какие данные использовать анутренние или внешние
  bool mIsHaveValue;
  /// Было ли изменено свойство после инициализации
  bool mIsChange;
  /// Является ли свойство флагом, и задавать значение не требуется
  bool mIsFlag;

  /// Копирование внутренних значений
  virtual void CopyValue(Type value);
public:

  /// Задает использовать ли внутренние данные или внешние
  void SetIsHaveValue(bool isHaveValue);
  /// Какие данные используются, внутренние или внешние
  bool GetIsHaveValue();

  /// Задает индекс свойства для проверки
  void SetIndex(int index);
  /// Возвращает индекс свойства для проверки
  int GetIndex();

  /// Перегруженный оператор приведения типа, геттер
  virtual operator Type() const;
  /// Перегруженный оператор присваивания, сеттер
  virtual void operator =(Type data);

  /// Перегрузка оператора копрования двух объектов
  virtual void operator = (TTypedProperty<Type, Owner>& data);
  /// Перегрузка оператора копрования для указателя
  virtual void operator = (TTypedProperty<Type, Owner>* data);

  /// Копирует данные из указателя в этот объект
  virtual void Copy(void* data);
  /// Задает данные приводя к void* к типу объекта
  virtual void SetValue(void* data);
  /// Возвращает указатель на данные хранящиеся в объекте
  virtual void* GetValue();
  /// Инициализация методов
  virtual void Init(Owner * owner, tGetter getMethod, tSetter setMethod, tCheckValue checkMethod);

  /// Возвращиет данные в соответствии с правилами
  virtual Type GetData();
  /// Возвращает принадлежащие объекту данные
  virtual Type GetAvailableData() const;
  /// Было ли изменено свойство после инициализации
  virtual bool GetIsChange();

  TTypedProperty();
  TTypedProperty(Type value);
  TTypedProperty(Owner * owner, tGetter getMethod, tSetter setMethod, tCheckValue checkMethod, Type value);
  virtual ~TTypedProperty();

  /// Задает функцию сеттер
  virtual void SetSetter(tSetter setter)
  {
    mSetter = setter;

    if ((mGetter != 0) || (mSetter != 0))
      mIsHaveValue = false;
    else
      mIsHaveValue = true;
  }
  ///// Возвращает функцию сеттер
  virtual tSetter GetSetter()
  {
    return mSetter;
  }
  /// Задает функцию геттер
  virtual void SetGetter(tGetter getter)
  {
    mGetter = getter;

    if ((mGetter != 0) || (mSetter != 0))
      mIsHaveValue = false;
    else
      mIsHaveValue = true;
  }
  ///// Возвращает функцию геттер
  virtual tGetter GetGetter() const
  {
    return mGetter;
  }
  /// Задает функцию проверяющую значение
  virtual void SetCheckValue(tCheckValue checkValue)
  {
    mCheckValue = checkValue;
  }
  /// Возвращает функцию проверяющую значения
  virtual tCheckValue GetCheckValue()
  {
    return mCheckValue;
  }

  /// Является ли свойство флагом, и задавать значение не требуется.
  virtual bool IsFlag()
  {
    return mIsFlag;
  }

  /// Проверяет правильность значения
  virtual int CheckValue()
  {
    int err = 0;
    if (mCheckValue != 0)
      err = (this->mOwner->*mCheckValue)(mIndex);
    return err;
  }

};


/* ======================================================================== *\
**  Реализация методов класса     TTypedProperty                                 **
\* ======================================================================== */

// ------------------------------------------------------------------------------------------------
/// Копирование внутренних значений
template <class Type, class Owner>
void TTypedProperty<Type, Owner>::CopyValue(Type value)
{
  mValue = value;
}

// ------------------------------------------------------------------------------------------------
/// Задает использовать ли внутренние данные или внешние
template <class Type, class Owner>
void TTypedProperty<Type, Owner>::SetIsHaveValue(bool isHaveValue)
{
  mIsHaveValue = isHaveValue;
}

// ------------------------------------------------------------------------------------------------
/// Какие данные используются, внутренние или внешние
template <class Type, class Owner>
bool TTypedProperty<Type, Owner>::GetIsHaveValue()
{
  return mIsHaveValue;
}

// ------------------------------------------------------------------------------------------------
/// Задает индекс свойства для проверки
template <class Type, class Owner>
void TTypedProperty<Type, Owner>::SetIndex(int index)
{
  mIndex = index;
}

// ------------------------------------------------------------------------------------------------
/// Возвращает индекс свойства для проверки
template <class Type, class Owner>
int TTypedProperty<Type, Owner>::GetIndex()
{
  return mIndex;
}

// ------------------------------------------------------------------------------------------------
/// Перегруженный оператор приведения типа, геттер
template <class Type, class Owner>
TTypedProperty<Type, Owner>::operator Type() const
{
  if ((mIsHaveValue) || (mGetter == 0))
    return mValue;
  else
    return (this->mOwner->*mGetter)();
}

// ------------------------------------------------------------------------------------------------
/// Перегруженный оператор присваивания, сеттер
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
/// Перегрузка оператора копрования двух объектов
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
/// Перегрузка оператора копрования для указателя
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
/// Копирует данные из указателя в этот объект
template <class Type, class Owner>
void TTypedProperty<Type, Owner>::Copy(void* data)
{
  operator = ((TTypedProperty<Type, Owner>*)data);
}

// ------------------------------------------------------------------------------------------------
/// Задает данные приводя к void* к типу объекта
template <class Type, class Owner>
void TTypedProperty<Type, Owner>::SetValue(void* data)
{
  *this = *((Type*)(data));
}

// ------------------------------------------------------------------------------------------------
/// Возвращает указатель на данные хранящиеся в объекте
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
/// Инициализация методов
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
/// Возвращиет данные в соответствии с правилами
template <class Type, class Owner>
Type TTypedProperty<Type, Owner>::GetData()
{
  return operator Type();
}

// ------------------------------------------------------------------------------------------------
/// Возвращает принадлежащие объекту данные
template <class Type, class Owner>
Type TTypedProperty<Type, Owner>::GetAvailableData() const
{
  return mValue;
}

// ------------------------------------------------------------------------------------------------
/// Было ли изменено свойство после инициализации
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

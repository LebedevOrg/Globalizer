/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2016 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      BaseProperty.h                                              //
//                                                                         //
//  Purpose:   Header file for random generator class                      //
//                                                                         //
//  Author(s): Lebedev I.                                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __BASE_PROPERTY_H__
#define __BASE_PROPERTY_H__

/**
\file BaseProperty.h

\authors Лебедев И.
\date 2015-2016
\copyright ННГУ им. Н.И. Лобачевского

\brief Объявление базоых классов для свойств

*/

#include "property.h"

//

/* ======================================================================== *\
**  Объявление классов                                                      **
\* ======================================================================== */


/**
Базовый класс имеющий владельца
*/
template <class Owner>
class TBaseProperty : public IBaseValueClass
{
protected:

  /// Тип метода внешней проверки данных, принимает номер свойства возвращает код ошибки
  typedef int (Owner::*tCheckValue)(int);

  /// Владелец объекта
  Owner* mOwner;
  /// Прочиталось ли значение параметра из аргументов консоли или конфиг файла
  bool mIsReadValue;
  /// Свойство изменено до инициализации
  bool mIsPreChange;
public:
  /// Перегрузка оператора копирования двух объектов
  virtual void operator = (TBaseProperty<Owner>& data);
  /// Перегрузка оператора копирования для указателя
  virtual void operator = (TBaseProperty<Owner>* data);

  /// Создает объект data
  virtual void Clone(TBaseProperty<Owner>** data) = 0;

  TBaseProperty(Owner* owner) : mOwner(owner), mIsReadValue(false), mIsPreChange(false) {}
  virtual ~TBaseProperty() {}

  /** Инициализация свойства
  \param[in] owner - класс владелец свойства
  \param[in] checkMethod - метод проверки правильности введенных данных
  \param[in] index - номер свойства
  \param[in] separator - разделитель элементов массива
  \param[in] size - размер массива значений, для типов данных не являющимися масивами - всегда равен 1
  \param[in] name - имя свойства
  \param[in] help - выводимая на консоль справка
  \param[in] link - короткая строка для запуска
  \param[in] defValue - значение по умолчанию
  */
  virtual void InitializationParameterProperty(Owner * owner,
    tCheckValue checkMethod, int index, std::string separator, int size, std::string name,
    std::string help, std::string link, std::string defValue) = 0;

  /// Возвращает прочиталось ли значение параметра из аргументов консоли или конфиг файла
  virtual bool GetIsReadValue()
  {
    return mIsReadValue;
  }
  /// Задает прочиталось ли значение параметра из аргументов консоли или конфиг файла
  virtual void SetIsReadValue(bool isReadValue)
  {
    mIsReadValue = isReadValue;
  }

  /// задать что свойство было изменено до инициализации
  virtual void SetIsPreChange(bool val)
  {
    mIsPreChange = val;
  }

  /// Было ли свойство изменено до инициализации
  virtual bool IsPreChange()
  {
    return mIsPreChange;
  }


};



/* ======================================================================== *\
**  Реализация методов класса     TBaseProperty                                 **
\* ======================================================================== */

// ------------------------------------------------------------------------------------------------
///Перегрузка оператора копрования двух объектов
template <class Owner>
void TBaseProperty<Owner>::operator = (TBaseProperty<Owner>& data)
{
  mOwner = data.mOwner;
}

// ------------------------------------------------------------------------------------------------
/// Перегрузка оператора копрования для указателя
template <class Owner>
void TBaseProperty<Owner>::operator = (TBaseProperty<Owner>* data)
{
  mOwner = data->mOwner;
}

#endif //__BASE_PROPERTY_H__
// - end of file ----------------------------------------------------------------------------------

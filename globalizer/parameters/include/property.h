/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2016 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      property.h                                                  //
//                                                                         //
//  Purpose:   Header file for random generator class                      //
//                                                                         //
//  Author(s): Lebedev I.                                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_WARNINGS

#ifndef __PROPERTY_H__
#define __PROPERTY_H__

/**
\file property.h

\authors Лебедев И.
\date 2015-2016
\copyright ННГУ им. Н.И. Лобачевского

\brief Объявление базоых классов для свойств

*/

#include <string>

//

/* ======================================================================== *\
**  Объявление классов                                                      **
\* ======================================================================== */


/**
Интерфей базового класса имеющего имя и значение
*/

class IBaseValueClass
{
public:

  /// Копирует данные из указателя в этот объект
  virtual void Copy(void* data) = 0;
  /// Задает данные приводя void* к типу объекта
  virtual void SetValue(void* data) = 0;
  /// Возвращает указатель на данные хранящиеся в объекте
  virtual void* GetValue() = 0;
  /// Приводит к строке
  virtual std::string ToString() = 0;
  /// Получение значения из строки
  virtual void FromString(std::string val) = 0;

  virtual ~IBaseValueClass() {}

  /// Задает разделитель элементов массива
  virtual void SetSeparator(std::string separator) = 0;
  /// Возвращает разделитель элементов массива
  virtual std::string GetSeparator() = 0;

  /// Задает размер массива элементов
  virtual void SetSize(int size) = 0;
  /// Возвращает Размер массива элементов
  virtual int GetSize() = 0;

  /// Имя свойства
  virtual std::string GetName() = 0;
  /// Текст справки
  virtual std::string GetHelp() = 0;
  /// Короткое имя для домандной строки
  virtual std::string GetLink() = 0;

  /// Имя свойства
  virtual void SetName(std::string name) = 0;
  /// Текст справки
  virtual void SetHelp(std::string help) = 0;
  /// Короткое имя для домандной строки
  virtual void SetLink(std::string link) = 0;
  /// Возвращает справку по свойству
  virtual std::string GetHelpString() = 0;
  /// Возвращает текущего состояния параметра
  virtual std::string GetCurrentStringValue() = 0;
  /// Совпадает ли имя параметра со введенной строкой
  virtual bool IsNameEqual(std::string name) = 0;

  /// Возвращает прочиталось ли значение параметра из аргументов консоли или конфиг файла
  virtual bool GetIsReadValue() = 0;
  /// Задает прочиталось ли значение параметра из аргументов консоли или конфиг файла
  virtual void SetIsReadValue(bool isReadValue) = 0;

  /// Является ли свойство флагом, и задавать значение не требуется.
  virtual bool IsFlag() = 0;
  /// Проверяет правильность значения
  virtual int CheckValue() = 0;

  /// задать что свойство было изменено до инициализации
  virtual void SetIsPreChange(bool val) = 0;
  /// Было ли свойство изменено до инициализации
  virtual bool IsPreChange() = 0;

  /// будет ли редактироваться (зависит от конфигурации)
  bool mIsEdit = false;
};

#endif //__TYPES_H__
// - end of file ----------------------------------------------------------------------------------

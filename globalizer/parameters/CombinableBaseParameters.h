
/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2016 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      CombinableBaseParameter.h                                                  //
//                                                                         //
//  Purpose:   Header file for random generator class                      //
//                                                                         //
//  Author(s): Lebedev I.                                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __COMBINABLE_BASE_PARAMETERS_H__
#define __COMBINABLE_BASE_PARAMETERS_H__

/**
\file CombinableBaseParameter.h

\authors Лебедев И.
\date 2015-2016
\copyright ННГУ им. Н.И. Лобачевского

\brief Объявление базоых классов для свойств

*/

#include "types.h"

/**
Базовый класс параметров
При создание наследника необходимо переопределить констану OWNER_NAME
и в конструкторе задать mOwner
(mOwner = this;)
*/
class TCombinableBaseParameters
{
protected:
  /// Количество арументов командной строки
  int mArgumentCount;
  /// Сами аргументы командной строки
  char** mAargumentValue;
  /// Был ли инициализирован MPI
  bool mIsMPIInit;
public:

  /// Задает начальные параметры
  virtual void SetInitParam(int argc = 0, char* argv[] = 0, bool isMPIInit = false)
  {
    mArgumentCount = argc;
    mAargumentValue = argv;
    mIsMPIInit = isMPIInit;
  }

  /// Объединяет все параметры двух классов в обоих
  virtual void CombineOptions(IBaseValueClass** otherOptions, int count) = 0;
  /// Возвращает имеющиеся свойства
  virtual IBaseValueClass** GetOptions() = 0;
  /// Возвращает количество опций
  virtual int GetOptionsCount() = 0;

  virtual void SetVal(std::string name, std::string val) = 0;
  /// Задание параметру с именем name значения val
  virtual void SetVal(std::string name, void* val) = 0;
  /// Возвращает строку с значением параметра с именем name
  virtual std::string GetStringVal(std::string name) = 0;
  /// Возвращает значение параметра с именем name
  virtual void* GetVal(std::string name) = 0;

  TCombinableBaseParameters()
  {
    mArgumentCount = 0;
    mAargumentValue = 0;
    mIsMPIInit = false;
  }

  /// Инициализировать данные задачи по параметрам системы
  virtual void InitDataByParameters()
  {};
};

#endif //__COMBINABLE_BASE_PARAMETERS_H__
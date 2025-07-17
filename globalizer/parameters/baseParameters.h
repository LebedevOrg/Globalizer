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

#ifndef __BASE_PARAMETERS_H__
#define __BASE_PARAMETERS_H__

/**
\file baseParameters.h

\authors Лебедев И.
\date 2015-2016
\copyright ННГУ им. Н.И. Лобачевского

\brief Объявление базоых классов для свойств

*/

#include "types.h"
#include "pugixml.hpp"
#include "CombinableBaseParameters.h"

/**
Базовый класс параметров
При создание наследника необходимо переопределить констану OWNER_NAME
и в конструкторе задать mOwner
(mOwner = this;)
*/
template <class Owner>
class TBaseParameters : public TCombinableBaseParameters
{
#undef OWNER_NAME
#define OWNER_NAME Owner

protected:
  /// Массив свойств определенных в других классах
  IBaseValueClass** mOtherOptions;
  /// Массив свойств определенных в этом классе
  TBaseProperty<Owner>** mOptions;
  /// Массив свойств определенных в этом классе приведенные к базовому типу
  IBaseValueClass** mBaseTypeOptions;
  /// Количество опций
  int mOptionsCount;
  /// Количество опций из других классов
  int mOtherOptionsCount;
  /// Максимальный размер массива параметров
  int mOptionsSize;
  /// Максимальный размер массива параметров  определенных в других классах
  int mOtherOptionsSize;

  /// Печатать справку
  bool mIsPrintHelp;
  /// Имеются аргументы командной строки
  bool mIsHaveArguments;
  /// Владелец этого класса
  Owner* mOwner;
  /// Проинициализированы ли параметры
  bool mIsInit;
  /// Путь по умолчанию до кофигурационного файла
  std::string mConfigPath;

  /// Проверка правильности после окончания чтения параметров
  virtual int CheckValue(int index = -1);

  /**
  Инициализация параметра
  \param[in] option - параметр который инициализируем
  \param[in] sizeVal - размер массива значений, для типов данных не являющимися масивами - всегда равен 1
  \param[in] name - имя свойства
  \param[in] help - выводимая на консоль справка
  \param[in] com - короткая строка для запуска
  \param[in] defVal - значение по умолчанию
  */
  virtual void InitializationOption(TBaseProperty<Owner>* option, std::string name, std::string defVal,
    std::string com, std::string help, int sizeVal);
  /// Добавляет опцию в общий список
  virtual void AddOption(TBaseProperty<Owner>* option);
  /// Задание значений по умолчанию базовых параметров
  virtual void SetBaseDefaultParameters();
  /**
  Задание значений по умолчанию для всех параметров
  Пример:
  InitOption(имя параметра, значение по умолчанию, "короткая команда", "справка по параметру", кол-во элементов);
  *кол-во элементов для не массивов всегда равно 1.
  InitOption(Separator,_, "-Separator", "eparator", 1);
  */
  virtual void SetDefaultParameters() = 0;
  /// Чтение параметров из файла ConfigPath
  virtual void ReadConfigFile();
  /// Чтение параметров командной строки
  virtual void ReadParameters(int argc, char* argv[]);
  /// Задать разделитель массива для всех опций
  void SetSeparator();


public:
  ///Печатать или нет справку при пустой командной строке
  TBool<TBaseParameters<Owner>> IsPrintHelpWithoutArguments;
  /// Запускать ли при пустой командной строке
  TBool<TBaseParameters<Owner>> IsStartWithoutArguments;
  /// Разделитель элементов массива
  TString<TBaseParameters<Owner>> Separator;
  /// Путь до конфиг файла программы
  TString<TBaseParameters<Owner>> ConfigPath;
  /// Печать справки по параметрам
  void PrintHelp();
  /// Печать текущих значений параметров
  void PrintParameters();
  /// Запускать ли работу программы
  bool IsStart();

  /**
  Проверка правильности при изменение параметров
  При переопределение необходимо вызвать вызвать метод базового класса!
  */
  virtual int CheckValueParameters(int index = 0);

  /// Печать значения параметра с именем name
  void PrintParameter(std::string name);
  /// Задание параметру с именем name значения val
  void SetVal(std::string name, std::string val);
  /// Задание параметру с именем name значения val
  void SetVal(std::string name, void* val);
  /// Возвращает строку с значением параметра с именем name
  std::string GetStringVal(std::string name);
  /// Возвращает значение параметра с именем name
  void* GetVal(std::string name);

  /**
  Инициализация параметра
  \param[in] pt - тип параметра
  \param[in] sizeVal - размер массива значений, для типов данных не являющимися масивами - всегда равен 1
  \param[in] name - имя свойства
  \param[in] help - выводимая на консоль справка
  \param[in] com - короткая строка для запуска
  \param[in] defVal - значение по умолчанию
  */
  virtual void AddOption(EParameterType pt, std::string name, std::string defVal,
    std::string com, std::string help, int sizeVal);

  /// Объединяет все параметры двух классов в обоих
  virtual void CombineOptions(IBaseValueClass** otherOptions, int count);
  /// Возвращает имеющиеся свойства
  virtual IBaseValueClass** GetOptions();
  /// Возвращает дополнительные свойствя взятые из других классов параметров
  virtual IBaseValueClass** GetOtherOptions();
  /// Возвращает количество опций
  virtual int GetOptionsCount();
  /// Возвращает количество дополнительных свойств взятые из других классов параметров
  virtual int GetOtherOptionsCount();

  /// Инициализация параметров
  virtual void Init(int argc, char* argv[], bool isMPIInit = false);
  TBaseParameters();
  TBaseParameters(TBaseParameters& _parameters);
  virtual ~TBaseParameters();
  /// Является ли класс задачей
  virtual bool IsProblem();
};

// ------------------------------------------------------------------------------------------------
/// Проверка правильности
template <class Owner>
int TBaseParameters<Owner>::CheckValue(int index)
{

  if (Separator.ToString().length() < 1)
  {
    Separator = std::string("_");
  }

  return 0;
}

// ------------------------------------------------------------------------------------------------
/// Инициализация параметра
template <class Owner>
void TBaseParameters<Owner>::InitializationOption(TBaseProperty<Owner>* option, std::string name, std::string defVal,
  std::string com, std::string help, int sizeVal)
{
  option->InitializationParameterProperty(mOwner, &Owner::CheckValueParameters, mOptionsCount, Separator, sizeVal, name, help, com, defVal);
  AddOption(option);

  if (IsProblem())
    option->mIsEdit = true;
}

// ------------------------------------------------------------------------------------------------
/// Добавляет опцию в общий список
template <class Owner>
void TBaseParameters<Owner>::AddOption(TBaseProperty<Owner>* option)
{
  for (int i = 0; i < mOptionsCount; i++)
    if (mOptions[i]->IsNameEqual(option->GetName()))
      return;

  mOptions[mOptionsCount] = option;
  mBaseTypeOptions[mOptionsCount] = (IBaseValueClass*)option;
  mOptionsCount++;


  if (mOptionsCount >= mOptionsSize)
  {
    TBaseProperty<Owner>** bufOptions = new TBaseProperty<Owner>*[mOptionsSize];
    for (int i = 0; i < mOptionsSize; i++)
    {
      bufOptions[i] = mOptions[i];
    }

    delete[] mOptions;
    delete[] mBaseTypeOptions;

    mOptions = new TBaseProperty<Owner>*[mOptionsSize * 2];
    mBaseTypeOptions = new IBaseValueClass*[mOptionsSize * 2];

    for (int i = 0; i < mOptionsSize * 2; i++)
    {
      mOptions[i] = 0;
      mBaseTypeOptions[i] = 0;
    }

    for (int i = 0; i < mOptionsSize; i++)
    {
      mOptions[i] = bufOptions[i];
      mBaseTypeOptions[i] = (IBaseValueClass*)mOptions[i];
    }
    mOptionsSize = mOptionsSize * 2;

    delete[] bufOptions;
  }
}

// ------------------------------------------------------------------------------------------------
/// Задание значений по умолчанию базовых параметров
template <class Owner>
void TBaseParameters<Owner>::SetBaseDefaultParameters()
{
  int sizeVal = 1;

  Separator.InitializationParameterProperty(this, &TBaseParameters::CheckValue, mOptionsCount, Separator, sizeVal,
    "Separator", "eparator", "-Separator", "_");
  AddOption((TBaseProperty<Owner>*)(&Separator));

  IsPrintHelpWithoutArguments.InitializationParameterProperty(this, &TBaseParameters::CheckValue, mOptionsCount, Separator, sizeVal,
    "IsPrintHelpWithoutArguments", "Is print help without console arguments", "-PHWA", "false");
  AddOption((TBaseProperty<Owner>*)(&IsPrintHelpWithoutArguments));

  IsStartWithoutArguments.InitializationParameterProperty(this, &TBaseParameters::CheckValue, mOptionsCount, Separator, sizeVal,
    "IsStartWithoutArguments", "Is start without console arguments", "-SWA", "true");
  AddOption((TBaseProperty<Owner>*)(&IsStartWithoutArguments));

  ConfigPath.InitializationParameterProperty(this, &TBaseParameters::CheckValue, mOptionsCount, Separator, sizeVal,
    "ConfigPath", "The path to the configuration file of the program", "-CP", mConfigPath);
  AddOption((TBaseProperty<Owner>*)(&ConfigPath));

}

// ------------------------------------------------------------------------------------------------
/// Чтение параметров из файла ConfigPath
template <class Owner>
void TBaseParameters<Owner>::ReadConfigFile()
{
  if (mConfigPath != "")
    ConfigPath = mConfigPath;
  if (ConfigPath.operator std::string() != "")
  {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(ConfigPath.ToString().c_str());
    if (result.status != pugi::status_ok)
      return;
    pugi::xml_node config = doc.child("config");
    for (pugi::xml_node iter = config.first_child(); iter != 0; iter = iter.next_sibling())
    {
      std::string name = iter.name();
      for (int i = 0; i < mOptionsCount; i++)
      {
        if (mOptions[i]->IsNameEqual(name))
        {
          if (mOptions[i]->mIsEdit)
          {
            if (!mOptions[i]->IsFlag())
            {
              std::string value = iter.child_value();
              mOptions[i]->FromString(value);
              mOptions[i]->SetIsReadValue(true);
              break;

            }
            else
            {
              mOptions[i]->FromString("1");
              mOptions[i]->SetIsReadValue(true);
              break;
            }
          }
        }
      }
    }
  }
}

// ------------------------------------------------------------------------------------------------
/// Чтение параметров командной строки
template <class Owner>
void TBaseParameters<Owner>::ReadParameters(int argc, char* argv[])
{
  for (int i = 1; i < argc; i++)
  {
    std::string argument = argv[i];
    for (int j = 0; j < mOptionsCount; j++)
    {
      if (mOptions[j]->IsNameEqual(argument))
      {
        if (!mOptions[j]->IsPreChange())
        {
          if (mOptions[j]->mIsEdit)
          {
            if (!mOptions[j]->IsFlag())
            {
              i++;
              if (i < argc)
              {
                std::string value = argv[i];
                mOptions[j]->FromString(value);
                mOptions[j]->SetIsReadValue(true);
                break;
              }
            }
            else
            {
              mOptions[j]->FromString("1");
              mOptions[j]->SetIsReadValue(true);
              break;
            }
          }
        }
      }
    }
  }
}

// ------------------------------------------------------------------------------------------------
/// Задать разделитель массива для всех опций
template <class Owner>
void TBaseParameters<Owner>::SetSeparator()
{
  for (int i = 0; i < mOptionsCount; i++)
  {
    mOptions[i]->SetSeparator(Separator);
  }
}

// ------------------------------------------------------------------------------------------------
/// Печать справки по параметрам
template <class Owner>
void TBaseParameters<Owner>::PrintHelp()
{
  printf("\n\nHelp:\n");
  for (int i = 0; i < mOptionsCount; i++)
  {
    if (mOptions[i]->mIsEdit)
      printf("%s\n", mOptions[i]->GetHelpString().c_str());
  }
  for (int i = 0; i < mOtherOptionsCount; i++)
  {
    if (mOtherOptions[i]->mIsEdit)
      printf("%s\n", mOtherOptions[i]->GetHelpString().c_str());
  }
}

// ------------------------------------------------------------------------------------------------
template <class Owner>
void TBaseParameters<Owner>::PrintParameters()
{
  for (int i = 0; i < mOptionsCount; i++)
  {
    if (mOptions[i]->mIsEdit)
      printf("%s\n", mOptions[i]->GetCurrentStringValue().c_str());
  }
  for (int i = 0; i < mOtherOptionsCount; i++)
  {
    if (mOtherOptions[i]->mIsEdit)
      printf("%s\n", mOtherOptions[i]->GetCurrentStringValue().c_str());
  }
}

// ------------------------------------------------------------------------------------------------
/// Запускать ли работу программы
template <class Owner>
bool TBaseParameters<Owner>::IsStart()
{
  if ((mIsHaveArguments == false) && (IsStartWithoutArguments == false))
    return false;
  return true;
}

// ------------------------------------------------------------------------------------------------
/**
Проверка правильности при изменение параметров
При переопределение необходимо вызвать вызвать метод базового класса!
*/
template <class Owner>
int TBaseParameters<Owner>::CheckValueParameters(int index)
{
  if (mIsInit)
  {
    CheckValue(index);
    mIsInit = false;
    for (int i = 0; i < mOtherOptionsCount; i++)
    {
      for (int j = 0; j < mOptionsCount; j++)
      {
        if (mOtherOptions[i]->IsNameEqual(mOptions[j]->GetName()))
        {
          std::string oldBaseParameterValue = mOptions[j]->ToString();
          std::string oldOtherParameterValue = mOtherOptions[i]->ToString();

          if (oldBaseParameterValue != oldOtherParameterValue)
          {
            mOtherOptions[i]->FromString(oldBaseParameterValue);

            std::string newOtherParameterValue = mOtherOptions[i]->ToString();

            if (newOtherParameterValue == oldOtherParameterValue)
            {
              mOptions[j]->FromString(oldOtherParameterValue);
            }
          }
          break;
        }
      }
    }
    mIsInit = true;
  }
  return 0;
}

// ------------------------------------------------------------------------------------------------
/// Печать значения параметра с именем name
template <class Owner>
void TBaseParameters<Owner>::PrintParameter(std::string name)
{
  for (int i = 0; i < mOptionsCount; i++)
  {
    if (mOptions[i]->IsNameEqual(name))
    {
      printf("%s\n", mOptions[i]->GetCurrentStringValue().c_str());
      break;
    }
  }
  for (int i = 0; i < mOtherOptionsCount; i++)
  {
    if (mOtherOptions[i]->IsNameEqual(name))
    {
      printf("%s\n", mOtherOptions[i]->GetCurrentStringValue().c_str());
      break;
    }
  }
}

// ------------------------------------------------------------------------------------------------
/// Задание параметру с именем name значения val
template <class Owner>
void TBaseParameters<Owner>::SetVal(std::string name, std::string val)
{
  for (int i = 0; i < mOptionsCount; i++)
  {
    if (mOptions[i]->IsNameEqual(name))
    {
      mOptions[i]->FromString(val);
      break;
    }
  }

  for (int i = 0; i < mOtherOptionsCount; i++)
  {
    if (mOtherOptions[i]->IsNameEqual(name))
    {
      mOtherOptions[i]->FromString(val);
      break;
    }
  }
}

// ------------------------------------------------------------------------------------------------
/// Задание параметру с именем name значения val
template <class Owner>
void TBaseParameters<Owner>::SetVal(std::string name, void* val)
{
  for (int i = 0; i < mOptionsCount; i++)
  {
    if (mOptions[i]->IsNameEqual(name))
    {
      mOptions[i]->SetValue(val);
      break;
    }
  }
  for (int i = 0; i < mOtherOptionsCount; i++)
  {
    if (mOtherOptions[i]->IsNameEqual(name))
    {
      mOtherOptions[i]->SetValue(val);
      break;
    }
  }
}

// ------------------------------------------------------------------------------------------------
/// Возвращает строку с значением параметра с именем name
template <class Owner>
std::string TBaseParameters<Owner>::GetStringVal(std::string name)
{
  for (int i = 0; i < mOptionsCount; i++)
  {
    if (mOptions[i]->IsNameEqual(name))
    {
      return mOptions[i]->ToString();
    }
  }

  for (int i = 0; i < mOtherOptionsCount; i++)
  {
    if (mOtherOptions[i]->IsNameEqual(name))
    {
      return mOtherOptions[i]->ToString();
    }
  }

  return std::string("");
}

// ------------------------------------------------------------------------------------------------
/// Возвращает значение параметра с именем name
template <class Owner>
void* TBaseParameters<Owner>::GetVal(std::string name)
{
  for (int i = 0; i < mOptionsCount; i++)
  {
    if (mOptions[i]->IsNameEqual(name))
    {
      return mOptions[i]->GetValue();
    }
  }

  for (int i = 0; i < mOtherOptionsCount; i++)
  {
    if (mOtherOptions[i]->IsNameEqual(name))
    {
      return mOtherOptions[i]->GetValue();
    }
  }

  return NULL;
}

// ------------------------------------------------------------------------------------------------
/**
Инициализация параметра
\param[in] pt - тип параметра
\param[in] sizeVal - размер массива значений, для типов данных не являющимися масивами - всегда равен 1
\param[in] name - имя свойства
\param[in] help - выводимая на консоль справка
\param[in] com - короткая строка для запуска
\param[in] defVal - значение по умолчанию
*/
template <class Owner>
void TBaseParameters<Owner>::AddOption(EParameterType pt, std::string name, std::string defVal,
  std::string com, std::string help, int sizeVal)
{
  TBaseProperty<Owner>* option = 0;

  if (pt == Pint)
    option = new TInt<Owner>();
  else if (pt == Pdouble)
    option = new TDouble<Owner>();
  else if (pt == Pstring)
    option = new TString<Owner>();
  else if (pt == PETypeMethod)
    option = new TETypeMethod<Owner>();
  else if (pt == PETypeProcess)
    option = new TETypeProcess<Owner>();
  else if (pt == PETypeCalculation)
    option = new TETypeCalculation<Owner>();
  else if (pt == PELocalMethodScheme)
    option = new TELocalMethodScheme<Owner>();
  else if (pt == PESeparableMethodType)
    option = new TESeparableMethodType<Owner>();
  else if (pt == PEStopCondition)
    option = new TEStopCondition<Owner>();
  else if (pt == Pbool)
    option = new TBool<Owner>();
  else if (pt == Pints)
    option = new TInts<Owner>();
  else if (pt == Pdoubles)
    option = new TDoubles<Owner>();
  else if (pt == PEMapType)
    option = new TEMapType<Owner>();

  option->InitializationParameterProperty(mOwner, 0, mOptionsCount, Separator, sizeVal,
    name, help, com, defVal);
  AddOption(option);
}

// ------------------------------------------------------------------------------------------------
/// Объединяет все параметры двух классов в обоих
template <class Owner>
void TBaseParameters<Owner>::CombineOptions(IBaseValueClass** otherOptions, int count)
{
  int newOtherOptionsCount = mOtherOptionsCount + count;
  if (newOtherOptionsCount >= mOtherOptionsSize)
  {
    IBaseValueClass** bufOptions = new IBaseValueClass*[mOtherOptionsSize];
    for (int i = 0; i < mOtherOptionsSize; i++)
    {
      bufOptions[i] = mOtherOptions[i];
    }

    delete[] mOtherOptions;

    mOtherOptions = new IBaseValueClass*[mOtherOptionsSize * 2];

    for (int i = 0; i < mOtherOptionsSize * 2; i++)
    {
      mOtherOptions[i] = 0;
    }

    for (int i = 0; i < mOtherOptionsSize; i++)
    {
      mOtherOptions[i] = bufOptions[i];
    }
    mOtherOptionsSize = mOtherOptionsSize * 2;

    delete[] bufOptions;
  }

  for (int i = 0; i < count; i++)
  {
    mOtherOptions[mOtherOptionsCount] = otherOptions[i];

    for (int j = 0; j < mOptionsCount; j++)
    {
      if (mOtherOptions[mOtherOptionsCount]->IsNameEqual(mOptions[j]->GetName()))
      {
        std::string oldBaseParameterValue = mOptions[j]->ToString();
        std::string oldOtherParameterValue = mOtherOptions[mOtherOptionsCount]->ToString();
        if (!mOptions[j]->IsPreChange() && !mOtherOptions[mOtherOptionsCount]->IsPreChange())
        {
          if (oldBaseParameterValue != oldOtherParameterValue)
          {
            mOptions[j]->FromString(oldOtherParameterValue);
            std::string newBaseParameterValue = mOptions[j]->ToString();

            if (newBaseParameterValue == oldBaseParameterValue)
            {
              mOtherOptions[mOtherOptionsCount]->FromString(oldBaseParameterValue);
            }
          }
        }
        break;
      }
    }

    mOtherOptionsCount++;
  }
}

// ------------------------------------------------------------------------------------------------
/// Возвращает имеющиеся свойства
template <class Owner>
IBaseValueClass** TBaseParameters<Owner>::GetOptions()
{
  return mBaseTypeOptions;
}

// ------------------------------------------------------------------------------------------------
/// Возвращает дополнительные свойства взятые из других классов параметров
template <class Owner>
IBaseValueClass** TBaseParameters<Owner>::GetOtherOptions()
{
  return mOtherOptions;
}

// ------------------------------------------------------------------------------------------------
/// Возвращает количество опций
template <class Owner>
int TBaseParameters<Owner>::GetOptionsCount()
{
  return mOptionsCount;
}

// ------------------------------------------------------------------------------------------------
/// Возвращает количество дополнительных свойств взятые из других классов параметров
template <class Owner>
int TBaseParameters<Owner>::GetOtherOptionsCount()
{
  return mOtherOptionsCount;
}

// ------------------------------------------------------------------------------------------------
template <class Owner>
void TBaseParameters<Owner>::Init(int argc, char* argv[], bool isMPIInit)
{
  mIsMPIInit = isMPIInit;
  mIsInit = false;
  mOptionsCount = 0;
  mOtherOptionsCount = 0;
  mOptions = 0;

  mOptionsSize = 200;
  mOtherOptionsSize = 200;

  mOptions = new TBaseProperty<Owner>*[mOptionsSize];
  mOtherOptions = new IBaseValueClass*[mOtherOptionsSize];
  mBaseTypeOptions = new IBaseValueClass*[mOptionsSize];
  for (int i = 0; i < mOptionsSize; i++)
  {
    mOptions[i] = 0;
    mBaseTypeOptions[i] = 0;
  }

  for (int i = 0; i < mOtherOptionsSize; i++)
  {
    mOtherOptions[i] = 0;
  }

  // Определяем есть ли параметры консоли

  if (argc > 0)
  {
    mArgumentCount = argc;
    mAargumentValue = argv;
  }

  if (mArgumentCount <= 1)
    mIsHaveArguments = false;
  else
    mIsHaveArguments = true;

  // Инициализация базовых параметров по умолчанию
  SetBaseDefaultParameters();
  // Инициализация рабочих параметров
  SetDefaultParameters();
  // Задать разделитель для массивов
  SetSeparator();
#ifndef EXAMIN_LITE
  // Определяем параметры из консоли
  ReadParameters(mArgumentCount, mAargumentValue);
#endif // DEBUG

  // Определяем параметры из файлов
  ReadConfigFile();
  // Проверка параметров
  CheckValue();

  if ((mIsHaveArguments == false) && (IsPrintHelpWithoutArguments == false))
    mIsPrintHelp = false;
  else if (mIsHaveArguments == true)
    mIsPrintHelp = false;
  else
    mIsPrintHelp = true;
  mIsInit = true;

  CheckValueParameters();
}

// ------------------------------------------------------------------------------------------------
template <class Owner>
TBaseParameters<Owner>::TBaseParameters()
{
#ifdef EXAMIN_LITE
  // Определяем параметры из консоли
  mConfigPath = "Globalizer_conf.xml";
#else
  mConfigPath = "";
#endif // DEBUG

  mOwner = 0;
  mIsInit = false;
}

// ------------------------------------------------------------------------------------------------
template <class Owner>
TBaseParameters<Owner>::TBaseParameters(TBaseParameters& _parameters) : TCombinableBaseParameters()
{
  mIsInit = false;
  mOptionsCount = 0;
  mOtherOptionsCount = 0;
  mIsMPIInit = _parameters.mIsMPIInit;
  mOptionsSize = _parameters.mOptionsSize;
  mOtherOptionsSize = _parameters.mOtherOptionsSize;
  mOwner = 0;
  mIsHaveArguments = _parameters.mIsHaveArguments;
  IsPrintHelpWithoutArguments = _parameters.IsPrintHelpWithoutArguments;
  IsStartWithoutArguments = _parameters.IsStartWithoutArguments;
  mIsPrintHelp = _parameters.mIsPrintHelp;
  mOtherOptionsCount = _parameters.mOtherOptionsCount;

  mOptions = new TBaseProperty<Owner>*[mOptionsSize];
  mOtherOptions = new IBaseValueClass*[mOtherOptionsSize];
  mBaseTypeOptions = new IBaseValueClass*[mOptionsSize];
  for (int i = 0; i < mOptionsSize; i++)
  {
    mOptions[i] = 0;

    mBaseTypeOptions[i] = 0;
  }

  for (int i = 0; i < mOtherOptionsSize; i++)
  {
    mOtherOptions[i] = 0;
  }

  // Инициализация базовых параметров по умолчанию
  SetBaseDefaultParameters();

  for (int i = 0; i < mOptionsCount; i++)
  {
    *mOptions[i] = *_parameters.mOptions[i];
  }

  for (int i = 0; i < mOtherOptionsCount; i++)
  {
    mOtherOptions[i] = _parameters.mOtherOptions[i];
  }

}

// ------------------------------------------------------------------------------------------------
template <class Owner>
TBaseParameters<Owner>::~TBaseParameters()
{
  delete[] mOptions;
  mOptions = 0;
}

/// Является ли класс задачей
template <class Owner>
bool TBaseParameters<Owner>::IsProblem()
{
  return true;
}

#endif //__BASE_PARAMETERS_H__
// - end of file ----------------------------------------------------------------------------------

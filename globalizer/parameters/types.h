/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2016 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      types.h                                                     //
//                                                                         //
//  Purpose:   Header file for random generator class                      //
//                                                                         //
//  Author(s): Lebedev I.                                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////



#ifndef __TYPES_H__
#define __TYPES_H__

/**
\file types.h

\authors Лебедев И.
\date 2015-2016
\copyright ННГУ им. Н.И. Лобачевского

\brief Объявление класса #TInteger

\details Объявление класса #TInteger и его реализация
*/
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string>
#include <algorithm>
#include <cstring>

#include "ParameterProperty.h"

//



/* ======================================================================== *\
**  Объявление классов                                                      **
\* ======================================================================== */



/**
Класс логического чисел
*/
template <class Owner>
class TBool : public TParameterProperty<bool, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TBool, bool);

  TBool(bool value = 0) :TParameterProperty<bool, Owner>(value) {}

  /// Парсер строки
  virtual void operator = (std::string data);

  /// Приведение к строке
  virtual operator std::string();
};


/**
Класс логического чисел
*/
template <class Owner>
class TFlag : public TBool<Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TFlag, bool);

  TFlag(bool value = 0) :TBool<Owner>(value)
  {
    this->mIsFlag = true;
  }

  /// Парсер строки
  virtual void operator = (std::string data);

  /// Приведение к строке
  virtual operator std::string();
};

/**
Класс целых чисел
*/
template <class Owner>
class TInt : public TParameterProperty<int, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TInt, int);

  TInt(int value = 0) :TParameterProperty<int, Owner>(value) {}

  /// Парсер строки
  virtual void operator = (std::string data);

  /// Приведение к строке
  virtual operator std::string();


};

/**
Класс действительных чисел
*/
template <class Owner>
class TDouble : public TParameterProperty<double, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TDouble, double);

  TDouble(double value = 0) :TParameterProperty<double, Owner>(value) {}

  /// Парсер строки
  virtual void operator = (std::string data);

  /// Приведение к строке
  virtual operator std::string();
};

/**
Класс строк
*/
template <class Owner>
class TString : public TParameterProperty<std::string, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TString, std::string);

  TString(std::string value = "") :TParameterProperty<std::string, Owner>(value) {}

  /// Приведение к строке
  virtual operator std::string();
};

/**
Класс массива целых чисел
*/
template <class Owner>
class TStrings : public TParameterProperty<std::string*, Owner>
{
protected:

  /// Копирование внутренних значений
  virtual void CopyValue(std::string* value);
public:

  /// Задает размер массива элементов
  virtual void SetSize(int size);
  /// Возвращает Размер массива элементов
  virtual int GetSize();

  TStrings() : TParameterProperty<std::string*, Owner>(0) { }
  TStrings(std::string* value, int size) : TParameterProperty<std::string*, Owner>(0)
  {
    if (size > 0)
    {
      this->mSize = size;
      this->mValue = new std::string[this->mSize];
      std::copy(value, value + this->mSize, this->mValue);
    }
  }

  virtual ~TStrings();

  /// Базовые переопределения
  BasicMethods(TStrings, std::string*);

  /// Парсер строки
  virtual void operator = (std::string data);
  /// Приведение к строке
  virtual operator std::string();
  /// Перегрузка оператора индексации
  virtual std::string& operator [] (int index);
};

/**
Класс массива целых чисел
*/
template <class Owner>
class TInts : public TParameterProperty<int*, Owner>
{
protected:

  /// Копирование внутренних значений
  virtual void CopyValue(int* value);
public:

  /// Задает размер массива элементов
  virtual void SetSize(int size);
  /// Возвращает Размер массива элементов
  virtual int GetSize();

  TInts() : TParameterProperty<int*, Owner>(0) { }

  TInts(int* value, int size) : TParameterProperty<int*, Owner>(0)
  {
    if (size > 0)
    {
      this->mSize = size;
      this->mValue = new int[this->mSize];
      std::copy(value, value + this->mSize, this->mValue);
    }
  }

  virtual ~TInts();

  /// Базовые переопределения
  BasicMethods(TInts, int*);

  /// Парсер строки
  virtual void operator = (std::string data);
  /// Приведение к строке
  virtual operator std::string();
  /// Перегрузка оператора индексации
  virtual int& operator [] (int index);
};

/**
Класс массива целых чисел
*/
template <class Owner>
class TDoubles : public TParameterProperty<double*, Owner>
{
protected:

  /// Копирование внутренних значений
  virtual void CopyValue(double* value);
public:

  /// Задает размер массива элементов
  virtual void SetSize(int size);
  /// Возвращает Размер массива элементов
  virtual int GetSize();

  TDoubles() : TParameterProperty<double*, Owner>(0) { }

  TDoubles(double* value, int size) : TParameterProperty<double*, Owner>(0)
  {
    if (size > 0)
    {
      this->mSize = size;
      this->mValue = new double[this->mSize];
      std::copy(value, value + this->mSize, this->mValue);
    }
  }

  virtual ~TDoubles();

  /// Базовые переопределения
  BasicMethods(TDoubles, double*);


  /// Парсер строки
  virtual void operator = (std::string data);
  /// Приведение к строке
  virtual operator std::string();
  /// Перегрузка оператора индексации
  virtual double& operator [] (int index);
};

/**
Класс для перечисления типа методов
*/
template <class Owner>
class TETypeMethod : public TParameterProperty<ETypeMethod, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TETypeMethod, ETypeMethod);

  TETypeMethod(ETypeMethod value = StandartMethod) :
    TParameterProperty<ETypeMethod, Owner>(value) {}

  /// Парсер строки
  virtual void operator = (std::string data);

  /// Приведение к строке
  virtual operator std::string();
};

/**
Класс для перечисления типов сепарабельного поиска
*/
template <class Owner>
class TESeparableMethodType : public TParameterProperty<ESeparableMethodType, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TESeparableMethodType, ESeparableMethodType);

  TESeparableMethodType(ESeparableMethodType value = Off) :
    TParameterProperty<ESeparableMethodType, Owner>(value) {}

  /// Парсер строки
  virtual void operator = (std::string data);

  /// Приведение к строке
  virtual operator std::string();
};



/**
Класс для перечисления типов запуска локального поиска
*/
template <class Owner>
class TELocalMethodScheme : public TParameterProperty<ELocalMethodScheme, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TELocalMethodScheme, ELocalMethodScheme);

  TELocalMethodScheme(ELocalMethodScheme value = None) :
    TParameterProperty<ELocalMethodScheme, Owner>(value) {}

  /// Парсер строки
  virtual void operator = (std::string data);

  /// Приведение к строке
  virtual operator std::string();
};

/**
Класс для перечисления типов критерия остановки
*/
template <class Owner>
class TEStopCondition : public TParameterProperty<EStopCondition, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TEStopCondition, EStopCondition);

  TEStopCondition(EStopCondition value = Accuracy) :
    TParameterProperty<EStopCondition, Owner>(value) {}

  /// Парсер строки
  virtual void operator = (std::string data);

  /// Приведение к строке
  virtual operator std::string();
};

/**
Класс для перечисления типов способов вычисления
*/
template <class Owner>
class TETypeCalculation : public TParameterProperty<ETypeCalculation, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TETypeCalculation, ETypeCalculation);

  TETypeCalculation(ETypeCalculation value = OMP) :
    TParameterProperty<ETypeCalculation, Owner>(value) {}

  /// Парсер строки
  virtual void operator = (std::string data);

  /// Приведение к строке
  virtual operator std::string();
};

/**
Класс для перечисления типов процесса
*/
template <class Owner>
class TETypeProcess : public TParameterProperty<ETypeProcess, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TETypeProcess, ETypeProcess);

  TETypeProcess(ETypeProcess value = SynchronousProcess) :
    TParameterProperty<ETypeProcess, Owner>(ETypeProcess(value)) {}

  /// Парсер строки
  virtual void operator = (std::string data);

  /// Приведение к строке
  virtual operator std::string();
};

/**
Класс для перечисления типов решателей
*/
template <class Owner>
class TETypeSolver : public TParameterProperty<ETypeSolver, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TETypeSolver, ETypeSolver);

  TETypeSolver(ETypeSolver value = SingleSearch) :
    TParameterProperty<ETypeSolver, Owner>(ETypeSolver(value)) {}

  /// Парсер строки
  virtual void operator = (std::string data);

  /// Приведение к строке
  virtual operator std::string();
};

/**
Класс для перечисления типов разверток
*/
template <class Owner>
class TEMapType : public TParameterProperty<EMapType, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TEMapType, EMapType);

  TEMapType(EMapType value = mpRotated) :
    TParameterProperty<EMapType, Owner>(value) {}

  /// Парсер строки
  virtual void operator = (std::string data);

  /// Приведение к строке
  virtual operator std::string();
};

/**
Класс для перечисления типов распределения начальных точек
*/
template <class Owner>
class TETypeDistributionStartingPoints :
  public TParameterProperty<ETypeDistributionStartingPoints, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TETypeDistributionStartingPoints, ETypeDistributionStartingPoints);

  TETypeDistributionStartingPoints(ETypeDistributionStartingPoints value = Adaptively) :
    TParameterProperty<ETypeDistributionStartingPoints, Owner>(value) {}

  /// Парсер строки
  virtual void operator = (std::string data);

  /// Приведение к строке
  virtual operator std::string();
};

/**
Класс для перечисления типов локальных методов
*/
template <class Owner>
class TETypeLocalMethod :
  public TParameterProperty<ETypeLocalMethod, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TETypeLocalMethod, ETypeLocalMethod);

  TETypeLocalMethod(ETypeLocalMethod value = HookeJeeves) :
    TParameterProperty<ETypeLocalMethod, Owner>(value) {}

  /// Парсер строки
  virtual void operator = (std::string data);

  /// Приведение к строке
  virtual operator std::string();
};


/**
Класс для перечисления типов старта локального уточнения
*/
template <class Owner>
class TETypeStartLocalMethod :
  public TParameterProperty<ETypeStartLocalMethod, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TETypeStartLocalMethod, ETypeStartLocalMethod);

  TETypeStartLocalMethod(ETypeStartLocalMethod value = EqualNumberOfPoints) :
    TParameterProperty<ETypeStartLocalMethod, Owner>(value) {}

  /// Парсер строки
  virtual void operator = (std::string data);

  /// Приведение к строке
  virtual operator std::string();
};

/**
Класс для перечисления типов добавления точек локального поиска
*/
template <class Owner>
class TETypeAddLocalPoint :
  public TParameterProperty<ETypeAddLocalPoint, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TETypeAddLocalPoint, ETypeAddLocalPoint);

  TETypeAddLocalPoint(ETypeAddLocalPoint value = RegularPoints) :
    TParameterProperty<ETypeAddLocalPoint, Owner>(value) {}

  /// Парсер строки
  virtual void operator = (std::string data);

  /// Приведение к строке
  virtual operator std::string();
};


/**
Класс определяющий выбора интервала хранящего локальный минимум
*/
template <class Owner>
class TETypeLocalMinInterval :
  public TParameterProperty<ETypeLocalMinInterval, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TETypeLocalMinInterval, ETypeLocalMinInterval);

  TETypeLocalMinInterval(ETypeLocalMinInterval value = NPoints) :
    TParameterProperty<ETypeLocalMinInterval, Owner>(value) {}

  /// Парсер строки
  virtual void operator = (std::string data);

  /// Приведение к строке
  virtual operator std::string();
};


/**
Класс определяющий тип локального уточнения: 0 - без него; 1 - минимаксное; 2 - адаптивное; 3 - адаптивно-минимаксное
*/
template <class Owner>
class TELocalTuningType :
  public TParameterProperty<ELocalTuningType, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TELocalTuningType, ELocalTuningType);

  TELocalTuningType(ELocalTuningType value = WithoutLocalTuning) :
    TParameterProperty<ELocalTuningType, Owner>(value) {}

  /// Парсер строки
  virtual void operator = (std::string data);

  /// Приведение к строке
  virtual operator std::string();
};

/* ======================================================================== *\
**  Служебные функции                                                       **
\* ======================================================================== */

// ------------------------------------------------------------------------------------------------
/// Шаблонная функция для реализации перегрузки операции индексации
template<class Type, class Owner>
Type& Indexer(int index, Type* mValue, int mSize, TBaseProperty<Owner>* obj)
{
  if (((index >= 0) && (index < mSize)) && (mValue != 0))
    return mValue[index];
  else
  {
    if (mValue == 0)
    {
      obj->SetSize(1);
    }
    return (*((Type**)obj->GetValue()))[0];
  }
}


/* ======================================================================== *\
**  Реализация методов класса     TBool                                     **
\* ======================================================================== */

// ------------------------------------------------------------------------------------------------
/// Парсер строки
template <class Owner>
void TBool<Owner>::operator = (std::string data)
{
  if ((data == "false") || (data == "0"))
    *this = false;
  if ((data == "true") || (data == "1"))
    *this = true;
}

// ------------------------------------------------------------------------------------------------
/// Приведение к строке
template <class Owner>
TBool<Owner>::operator std::string()
{
  std::string result;
  if (this->GetData() == false)
    result = "false";
  if (this->GetData() == true)
    result = "true";
  return result;
}

/* ======================================================================== *\
**  Реализация методов класса     TFlag                                     **
\* ======================================================================== */

// ------------------------------------------------------------------------------------------------
/// Парсер строки
template <class Owner>
void TFlag<Owner>::operator = (std::string data)
{
  TBool<Owner>::operator = (data);
}

// ------------------------------------------------------------------------------------------------
/// Приведение к строке
template <class Owner>
TFlag<Owner>::operator std::string()
{
  return TBool<Owner>::operator std::string();
}

/* ======================================================================== *\
**  Реализация методов класса     TInt                                     **
\* ======================================================================== */

// ------------------------------------------------------------------------------------------------
template <class Owner>
void TInt<Owner>::operator = (std::string data)
{
  int val = 0;
  int errCode = sscanf(data.data(), "%d", &val);
  *this = val;
}

// ------------------------------------------------------------------------------------------------
/// Приведение к строке
template <class Owner>
TInt<Owner>::operator std::string()
{
  std::string result;
  char ch[256];
  int errCode = sprintf(ch, "%d", this->GetData());
  result = ch;
  return result;
}

/* ======================================================================== *\
**  Реализация методов класса     TDouble                                   **
\* ======================================================================== */

// ------------------------------------------------------------------------------------------------
/// Парсер строки
template <class Owner>
void TDouble<Owner>::operator = (std::string data)
{
  double val = 0;
  int errCode = sscanf(data.data(), "%lf", &val);
  *this = val;
}

// ------------------------------------------------------------------------------------------------
/// Приведение к строке
template <class Owner>
TDouble<Owner>::operator std::string()
{
  std::string result;
  char ch[256];
  sprintf(ch, "%lf", this->GetData());
  result = ch;
  return result;
}

/* ======================================================================== *\
**  Реализация методов класса     TString                                   **
\* ======================================================================== */

// ------------------------------------------------------------------------------------------------
/// Приведение к строке
template <class Owner>
TString<Owner>::operator std::string()
{
  return this->mValue;
}


/* ======================================================================== *\
**  Реализация методов класса     TStrings                                   **
\* ======================================================================== */

// ------------------------------------------------------------------------------------------------
/// Копирование внутренних значений
template <class Owner>
void TStrings<Owner>::CopyValue(std::string* value)
{
  if (this->mValue != 0)
    delete[] this->mValue;
  this->mValue = 0;
  if (this->mSize > 0)
  {
    this->mValue = new std::string[this->mSize];
    for (int i = 0; i < this->mSize; i++)
      this->mValue[i] = value[i];
  }
}

// ------------------------------------------------------------------------------------------------
/// Задает размер массива элементов
template <class Owner>
void TStrings<Owner>::SetSize(int size)
{
  if (size > 0)
  {
    this->mIsChange = true;
    std::string* buf = 0;
    if (this->mValue != 0)
    {
      buf = new std::string[this->mSize];
      for (int i = 0; i < this->mSize; i++)
      {
        buf[i] = this->mValue[i];
      }

      delete[] this->mValue;
      this->mValue = 0;
    }
    else
    {
      this->mSize = 0;
    }

    this->mValue = new std::string[size];

    for (int i = 0; i < size; i++)
      this->mValue[i] = "";

    if (buf != 0)
    {
      for (int i = 0; i < std::min(this->mSize, size); i++)
        this->mValue[i] = buf[i];
      delete[] buf;
    }
    this->mSize = size;
  }

}

// ------------------------------------------------------------------------------------------------
/// Возвращает Размер массива элементов
template <class Owner>
int TStrings<Owner>::GetSize()
{
  return this->mSize;
}

// ------------------------------------------------------------------------------------------------
template <class Owner>
TStrings<Owner>::~TStrings()
{
  if (this->mValue != 0)
    delete[] this->mValue;
  this->mValue = 0;
  this->mSize = 0;
}

// ------------------------------------------------------------------------------------------------
/// Парсер строки
template <class Owner>
void TStrings<Owner>::operator = (std::string data)
{
  //sscanf(data.data(), "%d", &mValue);

  int l = 0;
  char *s = new char[data.size() + 1];

  strcpy(s, data.c_str());

  char *pp = strtok(s, this->mSeparator.c_str());
  std::string tt[100];
  std::string t = "";
  while (pp != 0)
  {
    char b[256];
    sscanf(pp, "%s", b);
    t = b;
    tt[l] = t;
    pp = strtok(NULL, this->mSeparator.c_str());
    l++;
  }

  SetSize(l);
  *this = tt;

  delete[] s;

}

// ------------------------------------------------------------------------------------------------
/// Приведение к строке
template <class Owner>
TStrings<Owner>::operator std::string()
{
  std::string result;

  for (int i = 0; i < this->mSize - 1; i++)
  {
    result += this->mValue[i] + this->mSeparator;
  }

  if ((this->mSize - 1) >= 0)
  {
    result += this->mValue[this->mSize - 1];
  }

  return result;
}

// ------------------------------------------------------------------------------------------------
/// Перегрузка оператора индексации
template <class Owner>
std::string& TStrings<Owner>::operator [] (int index)
{
  return Indexer(index, this->mValue, this->mSize, this);
}


/* ======================================================================== *\
**  Реализация методов класса     TInts                                   **
\* ======================================================================== */

// ------------------------------------------------------------------------------------------------
/// Копирование внутренних значений
template <class Owner>
void TInts<Owner>::CopyValue(int* value)
{
  if (this->mValue != 0)
    delete[] this->mValue;
  this->mValue = 0;
  if (this->mSize > 0)
  {
    this->mValue = new int[this->mSize];
    for (int i = 0; i < this->mSize; i++)
      this->mValue[i] = value[i];
  }
}

// ------------------------------------------------------------------------------------------------
/// Задает размер массива элементов
template <class Owner>
void TInts<Owner>::SetSize(int size)
{
  if (size > 0)
  {
    this->mIsChange = true;
    int* buf = 0;
    if (this->mValue != 0)
    {
      buf = new int[this->mSize];
      for (int i = 0; i < this->mSize; i++)
      {
        buf[i] = this->mValue[i];
      }

      delete[] this->mValue;
      this->mValue = 0;
    }
    else
    {
      this->mSize = 0;
    }

    this->mValue = new int[size];

    for (int i = 0; i < size; i++)
      this->mValue[i] = 0;

    if (buf != 0)
    {
      for (int i = 0; i < GLOBALIZER_MIN(this->mSize, size); i++)
        this->mValue[i] = buf[i];
      delete[] buf;
    }
    this->mSize = size;
  }

}

// ------------------------------------------------------------------------------------------------
/// Возвращает Размер массива элементов
template <class Owner>
int TInts<Owner>::GetSize()
{
  return this->mSize;
}

// ------------------------------------------------------------------------------------------------
template <class Owner>
TInts<Owner>::~TInts()
{
  if (this->mValue != 0)
    delete[] this->mValue;
  this->mValue = 0;
  this->mSize = 0;
}

// ------------------------------------------------------------------------------------------------
/// Парсер строки
template <class Owner>
void TInts<Owner>::operator = (std::string data)
{
  //sscanf(data.data(), "%d", &mValue);

  int l = 0;
  char *s = new char[data.size() + 1];

  strcpy(s, data.c_str());

  char *pp = strtok(s, this->mSeparator.c_str());
  int tt[100];
  int t = 0;
  while (pp != 0)
  {
    int errCode = sscanf(pp, "%d", &t);
    tt[l] = t;
    pp = strtok(NULL, this->mSeparator.c_str());
    l++;
  }

  SetSize(l);
  *this = tt;

  delete[] s;

}

// ------------------------------------------------------------------------------------------------
/// Приведение к строке
template <class Owner>
TInts<Owner>::operator std::string()
{
  std::string result;
  char ch[256];

  for (int i = 0; i < this->mSize - 1; i++)
  {
    memset(ch, 0, 256);
    sprintf(ch, "%d", this->mValue[i]);
    result += ch + this->mSeparator;
  }

  if ((this->mSize - 1) >= 0)
  {
    memset(ch, 0, 256);
    sprintf(ch, "%d", this->mValue[this->mSize - 1]);
    result += ch;
  }

  return result;
}

// ------------------------------------------------------------------------------------------------
/// Перегрузка оператора индексации
template <class Owner>
int& TInts<Owner>::operator [] (int index)
{
  return Indexer(index, this->mValue, this->mSize, this);
}

/* ======================================================================== *\
**  Реализация методов класса     TDoubles                                   **
\* ======================================================================== */

// ------------------------------------------------------------------------------------------------
/// Копирование внутренних значений
template <class Owner>
void TDoubles<Owner>::CopyValue(double* value)
{
  if (this->mValue != 0)
    delete[] this->mValue;
  this->mValue = 0;
  if (this->mSize > 0)
  {
    this->mValue = new double[this->mSize];
    for (int i = 0; i < this->mSize; i++)
      this->mValue[i] = value[i];
  }
}

// ------------------------------------------------------------------------------------------------
/// Задает размер массива элементов
template <class Owner>
void TDoubles<Owner>::SetSize(int size)
{
  if (size > 0)
  {
    this->mIsChange = true;
    double* buf = 0;
    if (this->mValue != 0)
    {
      buf = new double[this->mSize];
      for (int i = 0; i < this->mSize; i++)
      {
        buf[i] = this->mValue[i];
      }

      delete[] this->mValue;
      this->mValue = 0;
    }
    else
    {
      this->mSize = 0;
    }

    this->mValue = new double[size];

    for (int i = 0; i < size; i++)
      this->mValue[i] = 0;
    if (buf != 0)
    {
      for (int i = 0; i < GLOBALIZER_MIN(this->mSize, size); i++)
        this->mValue[i] = buf[i];
      delete[] buf;
    }
    this->mSize = size;
  }
}

// ------------------------------------------------------------------------------------------------
/// Возвращает Размер массива элементов
template <class Owner>
int TDoubles<Owner>::GetSize()
{
  return this->mSize;
}

// ------------------------------------------------------------------------------------------------
template <class Owner>
TDoubles<Owner>::~TDoubles()
{
  if (this->mValue != 0)
    delete[] this->mValue;
  this->mValue = 0;
  this->mSize = 0;
}

// ------------------------------------------------------------------------------------------------
/// Парсер строки
template <class Owner>
void TDoubles<Owner>::operator = (std::string data)
{
  //sscanf(data.data(), "%d", &mValue);

  int l = 0;
  char *s = new char[data.size() + 1];

  strcpy(s, data.c_str());

  char *pp = strtok(s, this->mSeparator.c_str());
  double tt[100];
  double t = 0;
  while (pp != 0)
  {
    int errCode = sscanf(pp, "%lf", &t);
    tt[l] = t;
    pp = strtok(NULL, this->mSeparator.c_str());
    l++;
  }

  SetSize(l);
  *this = tt;
  //  delete[] s;
}

// ------------------------------------------------------------------------------------------------
/// Приведение к строке
template <class Owner>
TDoubles<Owner>::operator std::string()
{
  std::string result;
  char ch[256];

  for (int i = 0; i < this->mSize - 1; i++)
  {
    memset(ch, 0, 256);
    sprintf(ch, "%lf", this->mValue[i]);
    result += ch + this->mSeparator;
  }

  if ((this->mSize - 1) >= 0)
  {
    memset(ch, 0, 256);
    sprintf(ch, "%lf", this->mValue[this->mSize - 1]);
    result += ch;
  }

  return result;
}

// ------------------------------------------------------------------------------------------------
/// Перегрузка оператора индексации
template <class Owner>
double& TDoubles<Owner>::operator [] (int index)
{
  return Indexer(index, this->mValue, this->mSize, this);
}

/* ======================================================================== *\
**  Реализация методов класса     TETypeMethod                                   **
\* ======================================================================== */

// ------------------------------------------------------------------------------------------------
/// Парсер строки
template <class Owner>
void TETypeMethod<Owner>::operator = (std::string data)
{
  if ((data == "StandartMethod") || (data == "0"))
    *this = StandartMethod;
  if ((data == "HybridMethod") || (data == "1"))
    *this = HybridMethod;
  if ((data == "ManyNumPointMethod") || (data == "2"))
    *this = ManyNumPointMethod;
  if ((data == "UniformSearchMethod") || (data == "3"))
    *this = UniformSearchMethod;
  if ((data == "GlobalSearchMethod") || (data == "4"))
    *this = GlobalSearchMethod;
  if ((data == "MCO_Method") || (data == "5"))
    *this = MCO_Method;
  if ((data == "MultievolventsMethod") || (data == "6"))
    *this = MultievolventsMethod;
  if ((data == "ParallelMultievolventsMethod") || (data == "7"))
    *this = ParallelMultievolventsMethod;
  if ((data == "IntegerMethod") || (data == "8"))
    *this = IntegerMethod;
  if ((data == "AdaptivMethod") || (data == "9"))
    *this = AdaptivMethod;
}

// ------------------------------------------------------------------------------------------------
/// Приведение к строке
template <class Owner>
TETypeMethod<Owner>::operator std::string()
{
  std::string s;
  if (this->mValue == StandartMethod)
    s = "StandartMethod";
  if (this->mValue == HybridMethod)
    s = "HybridMethod";
  if (this->mValue == ManyNumPointMethod)
    s = "ManyNumPointMethod";
  if (this->mValue == UniformSearchMethod)
    s = "UniformSearchMethod";
  if (this->mValue == GlobalSearchMethod)
    s = "GlobalSearchMethod";
  if (this->mValue == MCO_Method)
    s = "MCO_Method";
  if (this->mValue == MultievolventsMethod)
    s = "MultievolventsMethod";
  if (this->mValue == ParallelMultievolventsMethod)
    s = "ParallelMultievolventsMethod";
  if (this->mValue == IntegerMethod)
    s = "IntegerMethod";
  if (this->mValue == AdaptivMethod)
    s = "AdaptivMethod";
  return s;
}

/* ======================================================================== *\
**  Реализация методов класса     TESeparableMethodType                                   **
\* ======================================================================== */

// ------------------------------------------------------------------------------------------------
/// Парсер строки
template <class Owner>
void TESeparableMethodType<Owner>::operator = (std::string data)
{
  if ((data == "Off") || (data == "0"))
    *this = Off;
  if ((data == "GridSearch") || (data == "1"))
    *this = GridSearch;
  if ((data == "GlobalMethod") || (data == "2"))
    *this = GlobalMethod;
}

// ------------------------------------------------------------------------------------------------
/// Приведение к строке
template <class Owner>
TESeparableMethodType<Owner>::operator std::string()
{
  std::string s;
  if (this->mValue == Off)
    s = "Off";
  if (this->mValue == GridSearch)
    s = "GridSearch";
  if (this->mValue == GlobalMethod)
    s = "GlobalMethod";
  return s;
}

/* ======================================================================== *\
**  Реализация методов класса     TELocalMethodScheme                                   **
\* ======================================================================== */

// ------------------------------------------------------------------------------------------------
/// Парсер строки
template <class Owner>
void TELocalMethodScheme<Owner>::operator = (std::string data)
{
  if ((data == "None") || (data == "0"))
    *this = None;
  if ((data == "FinalStart") || (data == "1"))
    *this = FinalStart;
  if ((data == "UpdatedMinimum") || (data == "2"))
    *this = UpdatedMinimum;
}

// ------------------------------------------------------------------------------------------------
/// Приведение к строке
template <class Owner>
TELocalMethodScheme<Owner>::operator std::string()
{
  std::string s;
  if (this->mValue == FinalStart)
    s = "FinalStart";
  if (this->mValue == None)
    s = "None";
  if (this->mValue == UpdatedMinimum)
    s = "UpdatedMinimum";
  return s;
}

/* ======================================================================== *\
**  Реализация методов класса     TEStopCondition                                   **
\* ======================================================================== */

// ------------------------------------------------------------------------------------------------
/// Парсер строки InLocalArea
template <class Owner>
void TEStopCondition<Owner>::operator = (std::string data)
{
  if (data == "Accuracy" || data == "0")
    *this = Accuracy;
  if (data == "OptimumVicinity" || data == "1")
    *this = OptimumVicinity;
  if (data == "OptimumVicinity2" || data == "2")
    *this = OptimumVicinity2;
  if (data == "OptimumValue" || data == "3")
    *this = OptimumValue;
  if (data == "AccuracyWithCheck" || data == "4")
    *this = AccuracyWithCheck;
  if (data == "InLocalArea" || data == "5")
    *this = InLocalArea;
}

// ------------------------------------------------------------------------------------------------
/// Приведение к строке
template <class Owner>
TEStopCondition<Owner>::operator std::string()
{
  std::string s;
  if (this->mValue == Accuracy)
    s = "Accuracy";
  if (this->mValue == OptimumVicinity)
    s = "OptimumVicinity";
  if (this->mValue == OptimumVicinity2)
    s = "OptimumVicinity2";
  if (this->mValue == OptimumValue)
    s = "OptimumValue";
  if (this->mValue == OptimumValue)
    s = "AccuracyWithCheck";
  if (this->mValue == InLocalArea)
    s = "InLocalArea";

  return s;
}

/* ======================================================================== *\
**  Реализация методов класса     TETypeCalculation                                   **
\* ======================================================================== */

// ------------------------------------------------------------------------------------------------
/// Парсер строки
template <class Owner>
void TETypeCalculation<Owner>::operator = (std::string data)
{
  if ((data == "OMP") || (data == "0"))
    *this = OMP;
  if ((data == "CUDA") || (data == "1"))
    *this = CUDA;
  if ((data == "PHI") || (data == "2"))
    *this = PHI;
  if ((data == "BlockScheme") || (data == "3"))
    *this = BlockScheme;
  if ((data == "Adaptiv") || (data == "4"))
    *this = Adaptiv;
  if ((data == "Multievolvents") || (data == "5"))
    *this = Multievolvents;
  if ((data == "ParallelBlockScheme") || (data == "6"))
    * this = ParallelBlockScheme;
  if ((data == "MPICalculation") || (data == "7"))
    *this = MPICalculation;
  if ((data == "AsyncMPI") || (data == "8"))
    *this = AsyncMPI;
  if ((data == "ApproximationScheme") || (data == "9"))
    * this = ApproximationScheme;
  if ((data == "OneApi") || (data == "10"))
    *this = OneApi;
}

// ------------------------------------------------------------------------------------------------
/// Приведение к строке
template <class Owner>
TETypeCalculation<Owner>::operator std::string()
{
  std::string s;
  if (this->mValue == OMP)
    s = "OMP";
  if (this->mValue == CUDA)
    s = "CUDA";
  if (this->mValue == PHI)
    s = "PHI";
  if (this->mValue == BlockScheme)
    s = "BlockScheme";
  if (this->mValue == Adaptiv)
    s = "Adaptiv";
  if (this->mValue == Multievolvents)
    s = "Multievolvents";
  if (this->mValue == ParallelBlockScheme)
    s = "ParallelBlockScheme";
  if (this->mValue == MPICalculation)
    s = "MPICalculation";
  if (this->mValue == AsyncMPI)
    s = "AsyncMPI";
  if (this->mValue == ApproximationScheme)
    s = "ApproximationScheme";
  if (this->mValue == OneApi)
    s = "OneApi";
  return s;
}

/* ======================================================================== *\
**  Реализация методов класса     TETypeProcess                                   **
\* ======================================================================== */

// ------------------------------------------------------------------------------------------------
/// Парсер строки
template <class Owner>
void TETypeProcess<Owner>::operator = (std::string data)
{
  if ((data == "SynchronousProcess") || (data == "0"))
    *this = SynchronousProcess;
  if ((data == "SynchronousProcessNew") || (data == "1"))
    *this = SynchronousProcessNew;
  if ((data == "AsynchronousProcess") || (data == "2"))
    *this = AsynchronousProcess;
}

// ------------------------------------------------------------------------------------------------
/// Приведение к строке
template <class Owner>
TETypeProcess<Owner>::operator std::string()
{
  std::string s;
  if (this->mValue == SynchronousProcess)
    s = "SynchronousProcess";
  if (this->mValue == SynchronousProcessNew)
    s = "SynchronousProcessNew";
  if (this->mValue == AsynchronousProcess)
    s = "AsynchronousProcess";
  return s;
}

/* ======================================================================== *\
**  Реализация методов класса     TETypeSolver                                   **
\* ======================================================================== */

template <class Owner>
void TETypeSolver<Owner>::operator = (std::string data)
{
  if ((data == "SingleSearch") || (data == "0"))
    *this = SingleSearch;
  if ((data == "SequentialSearch") || (data == "1"))
    *this = SequentialSearch;
  if ((data == "SeparableSearch") || (data == "2"))
    *this = SeparableSearch;
  if ((data == "SeparationVariables") || (data == "3"))
    *this = SeparationVariables;
}

// ------------------------------------------------------------------------------------------------
/// Приведение к строке
template <class Owner>
TETypeSolver<Owner>::operator std::string()
{
  std::string s;
  if (this->mValue == SeparableSearch)
    s = "SeparableSearch";
  if (this->mValue == SequentialSearch)
    s = "SequentialSearch";
  if (this->mValue == SingleSearch)
    s = "SingleSearch";
  if (this->mValue == SeparationVariables)
    s = "SeparationVariables";
  return s;
}

/* ======================================================================== *\
**  Реализация методов класса     TEMapType                                   **
\* ======================================================================== */

// ------------------------------------------------------------------------------------------------
/// Парсер строки
template <class Owner>
void TEMapType<Owner>::operator = (std::string data)
{
  if ((data == "mpRotated") || (data == "0"))
    *this = mpRotated;
  if ((data == "mpShifted") || (data == "1"))
    *this = mpShifted;
  if ((data == "mpBase") || (data == "2"))
    *this = mpBase;
}

// ------------------------------------------------------------------------------------------------
/// Приведение к строке
template <class Owner>
TEMapType<Owner>::operator std::string()
{
  std::string s;
  if (this->mValue == mpRotated)
    s = "mpRotated";
  if (this->mValue == mpShifted)
    s = "mpShifted";
  if (this->mValue == mpBase)
    s = "mpBase";
  return s;
}

/* ======================================================================== *\
**  Реализация методов класса     TETypeDistributionStartingPoints                                   **
\* ======================================================================== */

// ------------------------------------------------------------------------------------------------
/// Парсер строки
template <class Owner>
void TETypeDistributionStartingPoints<Owner>::operator = (std::string data)
{
  if ((data == "Evenly") || (data == "0"))
    *this = Evenly;
  if ((data == "Adaptively") || (data == "1"))
    *this = Adaptively;
}

// ------------------------------------------------------------------------------------------------
/// Приведение к строке
template <class Owner>
TETypeDistributionStartingPoints<Owner>::operator std::string()
{
  std::string s;
  if (this->mValue == Evenly)
    s = "Evenly";
  if (this->mValue == Adaptively)
    s = "Adaptively";
  return s;
}


/* ======================================================================== *\
**  Реализация методов класса     TETypeLocalMethod                                   **
\* ======================================================================== */

// ------------------------------------------------------------------------------------------------
/// Парсер строки
template <class Owner>
void TETypeLocalMethod<Owner>::operator = (std::string data)
{
  if ((data == "HookeJeeves") || (data == "0"))
    *this = HookeJeeves;
  if ((data == "LeastSquareMethod") || (data == "1"))
    *this = LeastSquareMethod;
  if ((data == "L_BFGS_B") || (data == "2"))
    *this = L_BFGS_B;
  if ((data == "ParallelHookeJeeves") || (data == "3"))
    *this = ParallelHookeJeeves;
  if ((data == "Parallel_L_BFGS_B") || (data == "4"))
      *this = Parallel_L_BFGS_B;
}

// ------------------------------------------------------------------------------------------------
/// Приведение к строке
template <class Owner>
TETypeLocalMethod<Owner>::operator std::string()
{
  std::string s;
  if (this->mValue == HookeJeeves)
    s = "HookeJeeves";
  if (this->mValue == LeastSquareMethod)
    s = "LeastSquareMethod";
  if (this->mValue == L_BFGS_B)
    s = "L_BFGS_B";
  if (this->mValue == ParallelHookeJeeves)
    s = "ParallelHookeJeeves";
  if (this->mValue == Parallel_L_BFGS_B)
      s = "Parallel_L_BFGS_B";
  return s;
}


/* ======================================================================== *\
**  Реализация методов класса     TETypeStartLocalMethod                                   **
\* ======================================================================== */

// ------------------------------------------------------------------------------------------------
/// Парсер строки
template <class Owner>
void TETypeStartLocalMethod<Owner>::operator = (std::string data)
{
  if ((data == "AnyPoints") || (data == "0"))
    *this = AnyPoints;
  if ((data == "EqualNumberOfPoints") || (data == "1"))
    *this = EqualNumberOfPoints;
}

// ------------------------------------------------------------------------------------------------
/// Приведение к строке
template <class Owner>
TETypeStartLocalMethod<Owner>::operator std::string()
{
  std::string s;
  if (this->mValue == AnyPoints)
    s = "AnyPoints";
  if (this->mValue == EqualNumberOfPoints)
    s = "EqualNumberOfPoints";
  return s;
}


/* ======================================================================== *\
**  Реализация методов класса     TETypeAddLocalPoint                                   **
\* ======================================================================== */

// ------------------------------------------------------------------------------------------------
/// Парсер строки
template <class Owner>
void TETypeAddLocalPoint<Owner>::operator = (std::string data)
{
  if ((data == "RegularPoints") || (data == "0"))
    *this = RegularPoints;
  if ((data == "NotTakenIntoAccountInStoppingCriterion") || (data == "1"))
    *this = NotTakenIntoAccountInStoppingCriterion;
  if ((data == "IntegratedOnePoint") || (data == "2"))
    *this = IntegratedOnePoint;
  if ((data == "IntegratedAllPoint") || (data == "3"))
    *this = IntegratedAllPoint;
  if ((data == "IntegratedBestPath") || (data == "4"))
    *this = IntegratedBestPath;
}

// ------------------------------------------------------------------------------------------------
/// Приведение к строке
template <class Owner>
TETypeAddLocalPoint<Owner>::operator std::string()
{
  std::string s;
  if (this->mValue == RegularPoints)
    s = "RegularPoints";
  if (this->mValue == NotTakenIntoAccountInStoppingCriterion)
    s = "NotTakenIntoAccountInStoppingCriterion";
  if (this->mValue == IntegratedOnePoint)
    s = "IntegratedOnePoint";
  if (this->mValue == IntegratedAllPoint)
    s = "IntegratedAllPoint";
  if (this->mValue == IntegratedBestPath)
    s = "IntegratedBestPath";
  return s;
}

/* ======================================================================== *\
**  Реализация методов класса     TETypeLocalMinInterval                                   **
\* ======================================================================== */

// ------------------------------------------------------------------------------------------------
/// Парсер строки
template <class Owner>
void TETypeLocalMinInterval<Owner>::operator = (std::string data)
{
  if ((data == "NPoints") || (data == "0"))
    *this = NPoints;
  if ((data == "DecisionTrees") || (data == "1"))
    *this = DecisionTrees;
  if ((data == "AllMinimum") || (data == "2"))
    *this = AllMinimum;
}

// ------------------------------------------------------------------------------------------------
/// Приведение к строке
template <class Owner>
TETypeLocalMinInterval<Owner>::operator std::string()
{
  std::string s;
  if (this->mValue == NPoints)
    s = "NPoints";
  if (this->mValue == DecisionTrees)
    s = "DecisionTrees";
  if (this->mValue == AllMinimum)
    s = "AllMinimum";
  return s;
}


/* ======================================================================== *\
**  Реализация методов класса     TELocalTuningType                                   **
\* ======================================================================== */

// ------------------------------------------------------------------------------------------------
/// Парсер строки
template <class Owner>
void TELocalTuningType<Owner>::operator = (std::string data)
{
  if ((data == "WithoutLocalTuning") || (data == "0"))
    *this = WithoutLocalTuning;
  if ((data == "MiniMax") || (data == "1"))
    *this = MiniMax;
  if ((data == "Adaptive") || (data == "2"))
    *this = Adaptive;
  if ((data == "AdaptiveMiniMax") || (data == "3"))
    *this = AdaptiveMiniMax;
}

// ------------------------------------------------------------------------------------------------
/// Приведение к строке
template <class Owner>
TELocalTuningType<Owner>::operator std::string()
{
  std::string s;
  if (this->mValue == WithoutLocalTuning)
    s = "WithoutLocalTuning";
  if (this->mValue == MiniMax)
    s = "MiniMax";
  if (this->mValue == Adaptive)
    s = "Adaptive";
  if (this->mValue == AdaptiveMiniMax)
    s = "AdaptiveMiniMax";
  return s;
}

#endif //__TYPES_H__
// - end of file ----------------------------------------------------------------------------------

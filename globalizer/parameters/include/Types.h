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
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

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
class TBool : public ParameterProperty<bool, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TBool, bool);

  TBool(bool value = 0) :ParameterProperty<bool, Owner>(value) {}

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
class TInt : public ParameterProperty<int, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TInt, int);

  TInt(int value = 0) :ParameterProperty<int, Owner>(value) {}

  /// Парсер строки
  virtual void operator = (std::string data);

  /// Приведение к строке
  virtual operator std::string();


};

/**
Класс действительных чисел
*/
template <class Owner>
class TDouble : public ParameterProperty<double, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TDouble, double);

  TDouble(double value = 0) :ParameterProperty<double, Owner>(value) {}

  /// Парсер строки
  virtual void operator = (std::string data);

  /// Приведение к строке
  virtual operator std::string();
};

/**
Класс строк
*/
template <class Owner>
class TString : public ParameterProperty<std::string, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TString, std::string);

  TString(std::string value = "") :ParameterProperty<std::string, Owner>(value) {}

  /// Приведение к строке
  virtual operator std::string();
};

/**
Класс массива целых чисел
*/
template <class Owner>
class TStrings : public ParameterProperty<std::string*, Owner>
{
protected:

  /// Копирование внутренних значений
  virtual void CopyValue(std::string* value);
public:

  /// Задает размер массива элементов
  virtual void SetSize(int size);
  /// Возвращает Размер массива элементов
  virtual int GetSize();

  TStrings() : ParameterProperty<std::string*, Owner>(0) 
  { 
    this->mSize = 0; 
  }
  TStrings(std::string* value, int size) : ParameterProperty<std::string*, Owner>(0)
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
class TInts : public ParameterProperty<int*, Owner>
{
protected:

  /// Копирование внутренних значений
  virtual void CopyValue(int* value);
public:

  /// Задает размер массива элементов
  virtual void SetSize(int size);
  /// Возвращает Размер массива элементов
  virtual int GetSize();

  TInts() : ParameterProperty<int*, Owner>(0) 
  {
    this->mSize = 0;
  }

  TInts(int* value, int size) : ParameterProperty<int*, Owner>(0)
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
class TDoubles : public ParameterProperty<double*, Owner>
{
protected:

  /// Копирование внутренних значений
  virtual void CopyValue(double* value);
public:

  /// Задает размер массива элементов
  virtual void SetSize(int size);
  /// Возвращает Размер массива элементов
  virtual int GetSize();

  TDoubles() : ParameterProperty<double*, Owner>(0) 
  {
    this->mSize = 0;
  }

  TDoubles(double* value, int size) : ParameterProperty<double*, Owner>(0)
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
class TETypeMethod : public ParameterProperty<ETypeMethod, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TETypeMethod, ETypeMethod);

  TETypeMethod(ETypeMethod value = StandartMethod) :
    ParameterProperty<ETypeMethod, Owner>(value) {}

  /// Парсер строки
  virtual void operator = (std::string data);

  /// Приведение к строке
  virtual operator std::string();
};

/**
Класс для перечисления типов сепарабельного поиска
*/
template <class Owner>
class TESeparableMethodType : public ParameterProperty<ESeparableMethodType, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TESeparableMethodType, ESeparableMethodType);

  TESeparableMethodType(ESeparableMethodType value = Off) :
    ParameterProperty<ESeparableMethodType, Owner>(value) {}

  /// Парсер строки
  virtual void operator = (std::string data);

  /// Приведение к строке
  virtual operator std::string();
};



/**
Класс для перечисления типов запуска локального поиска
*/
template <class Owner>
class TELocalMethodScheme : public ParameterProperty<ELocalMethodScheme, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TELocalMethodScheme, ELocalMethodScheme);

  TELocalMethodScheme(ELocalMethodScheme value = None) :
    ParameterProperty<ELocalMethodScheme, Owner>(value) {}

  /// Парсер строки
  virtual void operator = (std::string data);

  /// Приведение к строке
  virtual operator std::string();
};

/**
Класс для перечисления типов критерия остановки
*/
template <class Owner>
class TEStopCondition : public ParameterProperty<EStopCondition, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TEStopCondition, EStopCondition);

  TEStopCondition(EStopCondition value = Accuracy) :
    ParameterProperty<EStopCondition, Owner>(value) {}

  /// Парсер строки
  virtual void operator = (std::string data);

  /// Приведение к строке
  virtual operator std::string();
};

/**
Класс для перечисления типов способов вычисления
*/
template <class Owner>
class TETypeCalculation : public ParameterProperty<ETypeCalculation, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TETypeCalculation, ETypeCalculation);

  TETypeCalculation(ETypeCalculation value = OMP) :
    ParameterProperty<ETypeCalculation, Owner>(value) {}

  /// Парсер строки
  virtual void operator = (std::string data);

  /// Приведение к строке
  virtual operator std::string();
};

/**
Класс для перечисления типов процесса
*/
template <class Owner>
class TETypeProcess : public ParameterProperty<ETypeProcess, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TETypeProcess, ETypeProcess);

  TETypeProcess(ETypeProcess value = SynchronousProcess) :
    ParameterProperty<ETypeProcess, Owner>(ETypeProcess(value)) {}

  /// Парсер строки
  virtual void operator = (std::string data);

  /// Приведение к строке
  virtual operator std::string();
};

/**
Класс для перечисления типов решателей
*/
template <class Owner>
class TETypeSolver : public ParameterProperty<ETypeSolver, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TETypeSolver, ETypeSolver);

  TETypeSolver(ETypeSolver value = SingleSearch) :
    ParameterProperty<ETypeSolver, Owner>(ETypeSolver(value)) {}

  /// Парсер строки
  virtual void operator = (std::string data);

  /// Приведение к строке
  virtual operator std::string();
};

/**
Класс для перечисления типов разверток
*/
template <class Owner>
class TEMapType : public ParameterProperty<EMapType, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TEMapType, EMapType);

  TEMapType(EMapType value = mpBase) :
    ParameterProperty<EMapType, Owner>(value) {}

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
  public ParameterProperty<ETypeDistributionStartingPoints, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TETypeDistributionStartingPoints, ETypeDistributionStartingPoints);

  TETypeDistributionStartingPoints(ETypeDistributionStartingPoints value = Evenly) :
    ParameterProperty<ETypeDistributionStartingPoints, Owner>(value) {}

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
  public ParameterProperty<ETypeLocalMethod, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TETypeLocalMethod, ETypeLocalMethod);

  TETypeLocalMethod(ETypeLocalMethod value = HookeJeeves) :
    ParameterProperty<ETypeLocalMethod, Owner>(value) {}

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
  public ParameterProperty<ETypeStartLocalMethod, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TETypeStartLocalMethod, ETypeStartLocalMethod);

  TETypeStartLocalMethod(ETypeStartLocalMethod value = EqualNumberOfPoints) :
    ParameterProperty<ETypeStartLocalMethod, Owner>(value) {}

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
  public ParameterProperty<ETypeAddLocalPoint, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TETypeAddLocalPoint, ETypeAddLocalPoint);

  TETypeAddLocalPoint(ETypeAddLocalPoint value = RegularPoints) :
    ParameterProperty<ETypeAddLocalPoint, Owner>(value) {}

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
  public ParameterProperty<ETypeLocalMinInterval, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TETypeLocalMinInterval, ETypeLocalMinInterval);

  TETypeLocalMinInterval(ETypeLocalMinInterval value = NPoints) :
    ParameterProperty<ETypeLocalMinInterval, Owner>(value) {}

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
  public ParameterProperty<ELocalTuningType, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TELocalTuningType, ELocalTuningType);

  TELocalTuningType(ELocalTuningType value = WithoutLocalTuning) :
    ParameterProperty<ELocalTuningType, Owner>(value) {}

  /// Парсер строки
  virtual void operator = (std::string data);

  /// Приведение к строке
  virtual operator std::string();
};

/** тип визуализации целевой функции
доступные режимы :
0 - LevelLayers - линии уровней
1 - Surface - поверхность
*/
template <class Owner>
class TEFigureTypes :
  public ParameterProperty<FigureTypes, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TEFigureTypes, FigureTypes);

  TEFigureTypes(FigureTypes value = LevelLayers) :
    ParameterProperty<FigureTypes, Owner>(value) {}

  /// Парсер строки
  virtual void operator = (std::string data);

  /// Приведение к строке
  virtual operator std::string();
};

/** тип вычислений значений для визуализации целевой функции
доступные режимы:
0 - ObjectiveFunction - строит линии уровня / поверхность по сетке 100 * 100,
1 - Approximation - строит аппроксимацию линий уровня / поверхности по имеющейся поисковой информации,
2 - Interpolation - строит интерполяцию линий уровня / поверхности по имеющейся поисковой информации,
3 - ByPoints - строит поверхность путем "натягивагия" ее на точки поисковой информации без сглаживания,
4 - OnlyPoints - отображает только распределение точек поисковой информации в области поиска.
*/
template <class Owner>
class TECalcsTypes :
  public ParameterProperty<CalcsTypes, Owner>
{
public:
  /// Базовые переопределения
  BasicMethods(TECalcsTypes, CalcsTypes);

  TECalcsTypes(CalcsTypes value = ObjectiveFunction) :
    ParameterProperty<CalcsTypes, Owner>(value) {}

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
Type& Indexer(int index, Type* mValue, int mSize, BaseProperty<Owner>* obj)
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
  char* s = new char[data.size() + 1];

  strcpy(s, data.c_str());

  char* pp = strtok(s, this->mSeparator.c_str());
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
  char* s = new char[data.size() + 1];

  strcpy(s, data.c_str());

  char* pp = strtok(s, this->mSeparator.c_str());
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
  char* s = new char[data.size() + 1];

  strcpy(s, data.c_str());

  char* pp = strtok(s, this->mSeparator.c_str());
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
  if ((data == "IntegerMethod") || (data == "1"))
    *this = IntegerMethod;
  if ((data == "RSAMethod") || (data == "2"))
      *this = RSAMethod;
}

// ------------------------------------------------------------------------------------------------
/// Приведение к строке
template <class Owner>
TETypeMethod<Owner>::operator std::string()
{
  std::string s;
  if (this->mValue == StandartMethod)
    s = "StandartMethod";
  if (this->mValue == IntegerMethod)
    s = "IntegerMethod";
  if (this->mValue == RSAMethod)
      s = "RSAMethod";
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
  if (data == "MaxIterWithoutImprovement" || data == "4")
    *this = MaxIterWithoutImprovement;
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
  if ((data == "MPI_calc") || (data == "2"))
    *this = MPI_calc;
  if ((data == "AsyncMPI") || (data == "3"))
    *this = AsyncMPI;
  if ((data == "OneApi") || (data == "4"))
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
  if (this->mValue == MPI_calc)
    s = "MPI_calc";
  if (this->mValue == AsyncMPI)
    s = "AsyncMPI";
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
}

// ------------------------------------------------------------------------------------------------
/// Приведение к строке
template <class Owner>
TETypeProcess<Owner>::operator std::string()
{
  std::string s;
  if (this->mValue == SynchronousProcess)
    s = "SynchronousProcess";
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
  if ((data == "HDSearch") || (data == "2"))
    *this = HDSearch;
}

// ------------------------------------------------------------------------------------------------
/// Приведение к строке
template <class Owner>
TETypeSolver<Owner>::operator std::string()
{
  std::string s;
  if (this->mValue == HDSearch)
    s = "HDSearch";
  if (this->mValue == SingleSearch)
    s = "SingleSearch";
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
  if ((data == "mpBase") || (data == "2"))
    *this = mpBase;
}

// ------------------------------------------------------------------------------------------------
/// Приведение к строке
template <class Owner>
TEMapType<Owner>::operator std::string()
{
  std::string s;
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
}

// ------------------------------------------------------------------------------------------------
/// Приведение к строке
template <class Owner>
TETypeDistributionStartingPoints<Owner>::operator std::string()
{
  std::string s;
  if (this->mValue == Evenly)
    s = "Evenly";
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
  if ((data == "ParallelHookeJeeves") || (data == "3"))
    *this = ParallelHookeJeeves;
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
  if (this->mValue == ParallelHookeJeeves)
    s = "ParallelHookeJeeves";
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


/* ======================================================================== *\
**  Реализация методов класса     TEFigureTypes                                   **
\* ======================================================================== */

// ------------------------------------------------------------------------------------------------
/// Парсер строки
template <class Owner>
void TEFigureTypes<Owner>::operator = (std::string data)
{
  if ((data == "LevelLayers") || (data == "0"))
    *this = LevelLayers;
  if ((data == "Surface") || (data == "1"))
    *this = Surface;
}

// ------------------------------------------------------------------------------------------------
/// Приведение к строке
template <class Owner>
TEFigureTypes<Owner>::operator std::string()
{
  std::string s;
  if (this->mValue == LevelLayers)
    s = "LevelLayers";
  if (this->mValue == Surface)
    s = "Surface";
  return s;
}


/* ======================================================================== *\
**  Реализация методов класса     TECalcsTypes                                 **
\* ======================================================================== */

// ------------------------------------------------------------------------------------------------
/// Парсер строки
template <class Owner>
void TECalcsTypes<Owner>::operator = (std::string data)
{
  if ((data == "ObjectiveFunction") || (data == "0"))
    *this = ObjectiveFunction;
  if ((data == "Approximation") || (data == "1"))
    *this = Approximation;
  if ((data == "Interpolation") || (data == "2"))
    *this = Interpolation;
  if ((data == "ByPoints") || (data == "3"))
    *this = ByPoints;
  if ((data == "OnlyPoints") || (data == "4"))
    *this = OnlyPoints;
}

// ------------------------------------------------------------------------------------------------
/// Приведение к строке
template <class Owner>
TECalcsTypes<Owner>::operator std::string()
{
  std::string s;
  if (this->mValue == ObjectiveFunction)
    s = "ObjectiveFunction";
  if (this->mValue == Approximation)
    s = "Approximation";
  if (this->mValue == Interpolation)
    s = "Interpolation";
  if (this->mValue == ByPoints)
    s = "ByPoints";
  if (this->mValue == OnlyPoints)
    s = "OnlyPoints";
  return s;
}

#endif //__TYPES_H__
// - end of file ----------------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2025 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      Problem.h                                                   //
//                                                                         //
//  Purpose:   Header file for Globalizer problem                          //
//                                                                         //
//                                                                         //
//  Author(s): Lebedev I.                                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

/**
\file problem_interface.h
\authors Лебедев И.Г.
\date 2025
\copyright ННГУ им. Н.И. Лобачевского
\brief Базовые классы для задач Globalizer
\details Базовые классы для задач Globalizer
*/

#ifndef __PROBLEM_H__
#define __PROBLEM_H__

#include <vector>
#include <string>
#include <cmath>
#include <functional>
#include <iostream>

#include "Common.h"
#include "ProblemPar.hpp"
#include "ProblemInterface.h"
#include "BaseParameters.h"

#include "Exception.h"


/**
Базовый класс реализующий основные методы задач
*/
template <class Owner>
class BaseProblem : virtual public IIntegerProgrammingProblem, public BaseParameters<Owner>, virtual public ProblemPar
{
#undef OWNER_NAME
#define OWNER_NAME BaseProblem
protected:

  /// Максимальная допустимая размерность
  int mMaxDimension;
  /// минимальная допустимая размерность
  int mMinDimension;


  /** Левая граница области поиска,
  используется если левая граница одна и таже для всех размерностей
  */
  double mLeftBorder;
  /** Правая граница области поиска,
  используется если правая граница одна и таже для всех размерностей
  */
  double mRightBorder;

  /// Число значений дискретного параметра
  int* mNumberOfValues;
  /// Значение по умолчанию для числа значений дискретного параметра
  int mDefNumberOfValues;

  /// Очищает номер текущего значения для дискретного параметра
  virtual void ClearCurrentDiscreteValueIndex(int** mCurrentDiscreteValueIndex)
  {
    if (NumberOfDiscreteVariable > 0)
    {
      if (*mCurrentDiscreteValueIndex != 0)
        delete[] * mCurrentDiscreteValueIndex;

      *mCurrentDiscreteValueIndex = new int[NumberOfDiscreteVariable];
      for (int i = 0; i < NumberOfDiscreteVariable; i++)
        (*mCurrentDiscreteValueIndex)[i] = 0;
    }
  }

  /// Проверка правильности после окончания чтения параметров
  virtual int CheckValue(int index = -1)
  {

    BaseParameters<Owner>::CheckValue(index);
    if ((Dimension < mMinDimension) || (Dimension > mMaxDimension))
    {
      Dimension = mMinDimension;
    }

    if (NumberOfDiscreteVariable > 0)
    {
      if (mNumberOfValues != nullptr)
      {
        delete[] mNumberOfValues;
        mNumberOfValues = nullptr;
      }

      mNumberOfValues = new int[NumberOfDiscreteVariable];
      for (int i = 0; i < NumberOfDiscreteVariable; i++)
      {
        mNumberOfValues[i] = mDefNumberOfValues;
      }
    }

    return 0;
  }

  /// Задание значений по умолчанию базовых параметров
  virtual void SetBaseDefaultParameters()
  {
    BaseParameters<Owner>::SetBaseDefaultParameters();

    int sizeVal = 1;
    int bd = mDim;
    Owner* aqqq = (Owner*)this;
    Dimension.InitializationParameterProperty(aqqq, &BaseProblem::GetDim, &BaseProblem::SetDim,
      &BaseProblem::CheckValue,
      this->mOptionsCount, this->Separator, sizeVal, "Dimension", "Problem dimension", "-N", "0");
    if (bd != 0)
      Dimension = bd;
    else
      Dimension = mMinDimension;
    mDim = Dimension;
    this->AddOption((BaseProperty<Owner>*)(&Dimension));

    IsPrintParameters.InitializationParameterProperty(aqqq, &BaseProblem::CheckValue,
      this->mOptionsCount, this->Separator, sizeVal,
      "IsPrintParameters", "Is Print Parameters", "-IsPrintParameters", "false");
    this->AddOption((BaseProperty<Owner>*)(&IsPrintParameters));

    NumberOfDiscreteVariable.InitializationParameterProperty(aqqq, &BaseProblem::CheckValue, this->mOptionsCount,
      this->Separator, sizeVal,
      "NumberOfDiscreteVariable", "Number Of Discrete Variable", "-NDV", "0");
    this->AddOption((BaseProperty<Owner>*)(&NumberOfDiscreteVariable));

    IsMultInt.InitializationParameterProperty(aqqq, &BaseProblem::CheckValue, this->mOptionsCount,
      this->Separator, sizeVal,
      "IsMultInt", "IsMultInt", "-IMI", "true");
    this->AddOption((BaseProperty<Owner>*)(&IsMultInt));

    Dimension.mIsEdit = true;
    IsPrintParameters.mIsEdit = true;
    NumberOfDiscreteVariable.mIsEdit = true;
    IsMultInt.mIsEdit = true;

  }
  /**
  Задание значений по умолчанию для всех параметров
  Пример:
  InitOption(имя параметра, значение по умолчанию, "короткая команда", "справка по параметру", кол-во элементов);
  *кол-во элементов для не массивов всегда равно 1.
  InitOption(Separator,_, "-Separator", "eparator", 1);
  */
  virtual  void SetDefaultParameters()
  {
  }
public:

  /// Используемая размерность
  TInt<Owner> Dimension;
  /// Печатать ли параметры задачи на консоль
  TBool<Owner> IsPrintParameters;
  /// Число дискретных параметров
  TInt<Owner> NumberOfDiscreteVariable;
  /// Умножать ли на пароболоидцелевую функцию
  TBool<Owner> IsMultInt;

  /// Метод возвращающий размерность
  int GetDim() const
  {
    return mDim;
  }
  /// Метод задачющий размерность
  void SetDim(int dim)
  {
    mDim = dim;
  }

  ///// Код ошибки, возвращаемый, если операция завершена успешно
  // static const int OK = 0;
  // /** Код ошибки, возвращаемый методами #GetOptimumValue и #GetOptimumPoint,
  // если соответствующие параметры задачи не определены,
  // */
  // static const int UNDEFINED = -1;
  // /// Код ошибки, возвращаемый, если операция не выполнена
  static const int PROBLEM_ERROR = -2;

  BaseProblem() : BaseParameters<Owner>()
  {
    this->mIsInit = false;
    mMaxDimension = MaxDim;
    mMinDimension = 2;
    mNumberOfCriterions = 1;
    mNumberOfConstraints = 0;
    std::string configPath = "";
    mLeftBorder = -1.8;
    mRightBorder = 2.2;
    //mCurrentDiscreteValueIndex = 0;
    mNumberOfValues = 0;
    mDim = 0;
    mDefNumberOfValues = -1;
  }
  /** Задание пути до конфигурационного файла
  Данный метод должн вызываться перед #Initialize
  \param[in] configPath строка, содержащая путь к конфигурационному файлу задачи
  \return Код ошибки
  */
  virtual int SetConfigPath(const std::string& configPath)
  {
    if (this->mIsInit)
    {
      this->ConfigPath = configPath;
    }
    else
    {
      this->mConfigPath = configPath;
    }
    return IProblem::OK;
  }

  /** Метод задаёт размерность задачи
  Данный метод должен вызываться перед #Initialize. Размерность должна быть в
  списке поддерживаемых.
  \param[in] dimension размерность задачи
  \return Код ошибки
  */
  virtual int SetDimension(int dimension)
  {
    if (!this->mIsInit)
    {
      mDim = dimension;
      Dimension.SetIsPreChange(true);
    }
    //return BaseProblem::PROBLEM_ERROR;
    if (dimension >= mMinDimension && dimension <= mMaxDimension)
    {
      Dimension = dimension;
      return IProblem::OK;
    }
    else
      return BaseProblem::PROBLEM_ERROR;
  }
  ///Возвращает размерность задачи, можно вызывать после #Initialize
  virtual int GetDimension() const
  {
    return mDim;
  }

  /// Инициализация параметров
  virtual void Init(int argc, char* argv[], bool isMPIInit = false)
  {
    BaseParameters<Owner>::Init(argc, argv, false);
    if (IsPrintParameters)
    {
      this->PrintParameters();
    }
  }

  ///Инициализация задачи
  virtual int Initialize(int argc, char* argv[], bool isMPIInit = false)
  {
    Init(argc, argv, false);
    return IProblem::OK;
  }

  ///Инициализация задачи
  virtual int Initialize()
  {
    Init(0, 0, false);
    return IProblem::OK;
  }

  /** Метод возвращает границы области поиска
  */
  virtual void GetBounds(double* lower, double* upper)
  {
    if (this->mIsInit)
      for (int i = 0; i < Dimension; i++)
      {
        lower[i] = mLeftBorder;
        upper[i] = mRightBorder;
      }
  }
  /** Метод возвращает значение целевой функции в точке глобального минимума
  \param[out] value оптимальное значение
  \return Код ошибки (#OK или #UNDEFINED)
  */
  virtual int GetOptimumValue(double& value) const
  {
    return BaseProblem<Owner>::UNDEFINED;
  }
  /** Метод возвращает значение функции с номером index в точке глобального минимума
  \param[out] value оптимальное значение
  \return Код ошибки (#OK или #UNDEFINED)
  */
  virtual int GetOptimumValue(double& value, int index) const
  {
    if (index == GetNumberOfConstraints())
      return GetOptimumValue(value);
    return IProblem::UNDEFINED;
  }


  /** Метод возвращает число общее функций в задаче (оно равно число ограничений + число критериев)
  \return Число функций
  */
  virtual int GetNumberOfFunctions() const
  {
    return mNumberOfCriterions + mNumberOfConstraints;
  }

  /** Метод возвращает число ограничений в задаче
  \return Число ограничений
  */
  virtual int GetNumberOfConstraints() const
  {
    return mNumberOfConstraints;
  }
  /** Метод возвращает число критериев в задаче
  \return Число критериев
  */
  virtual int GetNumberOfCriterions() const
  {
    return mNumberOfCriterions;
  }

  ///Деструктор
  virtual ~BaseProblem()
  {
  }

  virtual std::string ProblemName()
  {
    return std::string("This Problem\n");
  }

  /// Возвращает число дискретных параметров, дискретные параметры всегда последние в векторе y
  virtual int GetNumberOfDiscreteVariable()
  {
    return NumberOfDiscreteVariable;
  }
  /**
  Возвращает число значений дискретного параметра discreteVariable.
  GetDimension() возвращает общее число параметров.
  (GetDimension() - GetNumberOfDiscreteVariable()) - номер начальной дискретной переменной
  Для не дискретных переменных == -1
  */
  virtual int GetNumberOfValues(int discreteVariable)
  {
    if ((discreteVariable > GetDimension()) ||
      (discreteVariable < (GetDimension() - GetNumberOfDiscreteVariable())))
      return -1;
    if (mNumberOfValues == 0)
      return -1;
    return mNumberOfValues[discreteVariable - (GetDimension() - GetNumberOfDiscreteVariable())];
  }
  /**
  Определяет значения дискретного параметра с номером discreteVariable
  Возвращает код ошибки.
  \param[out] values массив, в который будут сохранены значения дискретного параметра
  */
  virtual int GetAllDiscreteValues(int discreteVariable, double* values)
  {
    if ((discreteVariable > GetDimension()) ||
      (discreteVariable < (GetDimension() - GetNumberOfDiscreteVariable())))
      return IIntegerProgrammingProblem::ERROR_DISCRETE_VALUE;
    int* mCurrentDiscreteValueIndex = 0;
    ClearCurrentDiscreteValueIndex(&mCurrentDiscreteValueIndex);

    // сбрасываем значение индекса текущего значения и задаем левую границу
    GetNextDiscreteValues(mCurrentDiscreteValueIndex, values[0], discreteVariable, -1);
    int numVal = GetNumberOfValues(discreteVariable);
    // определяем все остальные значения
    for (int i = 1; i < numVal; i++)
    {
      GetNextDiscreteValues(mCurrentDiscreteValueIndex, values[i], discreteVariable);
    }
    return IProblem::OK;
  }
  /**
  Определяет значения дискретного параметра с номером discreteVariable после номера previousNumber
  Возвращает код ошибки.
  \param[in] previousNumber - номер значения после которого возвращается значение
  -2 - значение по умолчанию, возвращает следующее значение
  -1 - возвращает после -1, т.е. левую границу области
  \param[out] value переменная в которую сохраняется значение дискретного параметра
  */
  virtual int GetNextDiscreteValues(int* mCurrentDiscreteValueIndex, double& value, int discreteVariable, int previousNumber = -2)
  {
    if ((discreteVariable > GetDimension()) ||
      (discreteVariable < (GetDimension() - GetNumberOfDiscreteVariable())) ||
      (mCurrentDiscreteValueIndex == 0) ||
      (mNumberOfValues == 0))
      return IIntegerProgrammingProblem::ERROR_DISCRETE_VALUE;

    // Единый корректный индекс дискретной переменной в массивах размера NumberOfDiscreteVariable
    const int di = discreteVariable - (GetDimension() - GetNumberOfDiscreteVariable());

    // если -1 то сбрасываем значение текущего номера
    if (previousNumber == -1)
    {
      mCurrentDiscreteValueIndex[di] = 0;
      value = mLeftBorder;
      return IProblem::OK;
    }
    else if (previousNumber == -2)
    {
      double d = (mRightBorder - mLeftBorder) /
        (mNumberOfValues[di] - 1);
      mCurrentDiscreteValueIndex[di]++;
      value = mLeftBorder + d * mCurrentDiscreteValueIndex[di];
      return IProblem::OK;
    }
    else
    {
      double d = (mRightBorder - mLeftBorder) /
        (mNumberOfValues[di] - 1);
      mCurrentDiscreteValueIndex[di] = previousNumber;
      mCurrentDiscreteValueIndex[di]++;
      value = mLeftBorder + d * mCurrentDiscreteValueIndex[di];
      return IProblem::OK;
    }
  }


  /// Проверяет является ли value допустимым значением для параметра с номером discreteVariable
  virtual bool IsPermissibleValue(double value, int discreteVariable)
  {
    if ((discreteVariable > GetDimension()) ||
      (discreteVariable < (GetDimension() - GetNumberOfDiscreteVariable())) ||
      (mNumberOfValues == 0))
      return false;
    double d = (mRightBorder - mLeftBorder) /
      (mNumberOfValues[discreteVariable - (GetDimension() - GetNumberOfDiscreteVariable())] - 1);
    double v = 0;
    for (int i = 0; i < mNumberOfValues[discreteVariable - (GetDimension() - GetNumberOfDiscreteVariable())]; i++)
    {
      v = mLeftBorder + d * i;
      if (fabs(v - value) < AccuracyDouble)
      {
        return true;
      }
    }
    return false;
  }
};

/** Основной класс для работы с задачами
*/
template <class Owner>
class Problem : public BaseProblem<Owner>
{
public:
  /** Метод возвращает координаты точки глобального минимума целевой функции
\param[out] y точка, в которой достигается оптимальное значение
\return Код ошибки (#OK или #UNDEFINED)
*/
  virtual int GetOptimumPoint(double* y) const
  {
    return IProblem::UNDEFINED;
  }

  /// Возвращает точку глобального оптимума для функции fNumber
  virtual int GetConstraintOptimumPoint(double* point, int fNumber)
  {
    return IProblem::UNDEFINED;
  }

  template <class ClassType, class Type>
  void ProblemParameterCheckSize2(ClassType& par, Type defVal, Type left, Type right, int size, bool isChackVal = false)
  {
    if (par.GetSize() < size)
    {
      if (par.GetSize() == 1)
      {
        Type d = defVal;
        if (par.GetSize() > 0)
          d = par[0];

        if (isChackVal)
          if ((d > right) || (d < left))
            d = defVal;
        par.SetSize(size);
        for (int i = 0; i < size; i++)
          par[i] = d;
      }
      else
      {
        int oldSize = par.GetSize();
        Type d = defVal;
        if (par.GetSize() > 0)
          d = par[0];

        if (isChackVal)
          if ((d > right) || (d < left))
            d = defVal;
        par.SetSize(size);
        for (int i = oldSize; i < size; i++)
          (par.GetData())[i] = d;
      }
    }
  }
};



#endif
// - end of file ----------------------------------------------------------------------------------

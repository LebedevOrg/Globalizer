/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      task.h                                                      //
//                                                                         //
//  Purpose:   Header file for optimization task class                     //
//                                                                         //
//  Author(s): Sysoyev A., Barkalov K.                                     //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __TASK_H__
#define __TASK_H__

#include "parameters.h"
#include "common.h"
#include "problem_interface.h"
#include "exception.h"
#include "baseInterval.h"

// ------------------------------------------------------------------------------------------------
class TTask: public TQueueBaseData
{
protected:
  /// полная размерность задачи
  int        N;
  /// левая граница области поиска
  double     A[MaxDim];
  /// правая граница области поиска
  double     B[MaxDim];
  /// число функционалов (последний - критерий)
  int        NumOfFunc;
  /// указатель на саму задачу оптимизации
  IProblem*  pProblem;
  /// размерность подзадачи
  int        FreeN;
  /**
  число фиксированных размерностей
  чем "ниже" уровень задача в дереве задач, тем больше FixedN
  */
  int        FixedN;
  /**
  значения фиксированных переменных
  включая значения переменных, зафиксированных на уровнях выше
  */
  double     FixedY[MaxDim];
  /// оптимальное значение целевой функции (определено, если известно из задачи)
  double     OptimumValue;
  /// координаты глобального минимума целевой функции (определено, если известно)
  double     OptimumPoint[MaxDim];
  /// true, если в задаче известно оптимальное значение критерия
  bool       IsOptimumValueDefined;
  /// true, если в задаче известна точка глобального минимума
  bool       IsOptimumPointDefined;

  /// уровень процесса в дереве процессов
  int ProcLevel;

  bool isInit;

public:
  int num;
  TTask(int _N, int _FreeN, IProblem* _problem, int _ProcLevel);
  TTask();
  virtual ~TTask();
  virtual TTask* Clone();
  virtual TTask* CloneWithNewData()
  {
    return Clone();
  }
  virtual void Init(int _N, int _FreeN, IProblem* _problem, int _ProcLevel);
  /// Задает фиксированные переменные
  virtual void SetFixed(int _FixedN, double *_FixedY);
  /// Возвращает общую рамерность задачи
  virtual int GetN() const { return N; }
  /// Возвращает уровень процесса в дереве процессов
  virtual int GetProcLevel() { return ProcLevel; }
  /// Возвращает число свободных переменных
  virtual int GetFreeN() const { return FreeN; }
  /// Возвращает число фиксированных переменных
  virtual int GetFixedN() const { return FixedN; }

  /// Возвращает левую границу области поиска
  virtual const double* GetA() const { return A; }
  /// Возвращает правую границу области поиска
  virtual const double* GetB() const { return B; }

  /// Возвращает априори известное значение глобального минимума
  virtual double GetOptimumValue() const { return OptimumValue; }
  /// Определяет априори известные координаты точки глобального минимума
  virtual void resetOptimumPoint()
  {
    pProblem->GetOptimumPoint(OptimumPoint);
  }
  /**
  Возвращает априори известные координаты точки глобального минимума
  Перед первым вызовом нужно вызвать resetOptimumPoint()
  */
  virtual const double* GetOptimumPoint() const { return OptimumPoint; }
  /// Возвращает известно ли для задачи значение глобального минимума
  virtual bool GetIsOptimumValueDefined() const { return IsOptimumValueDefined; }
  /// Возвращает известны ли для задачи координаты глобального минимума
  virtual bool GetIsOptimumPointDefined() const { return IsOptimumPointDefined; }
  /// Возвращает текущую задачу
  virtual IProblem* getProblem() { return pProblem; }

  ///Возвращает фиксированные координаты
  virtual const double* GetFixedY() const { return FixedY; }
  /// Возвращает число функций, сначала ограничения, потом критерии
  virtual int GetNumOfFunc() const { return NumOfFunc; }
  /// Задает число функций
  virtual void SetNumofFunc(int nf)
  {
    NumOfFunc = nf;
  }

  virtual int GetNumOfFuncAtProblem() const {return NumOfFunc; }
  /// Вычисляет значение функции с номером fNumber в точке y
  virtual double CalculateFuncs(const double *y, int fNumber)
  {
    double multInLevel = parameters.functionSignMultiplier[GetProcLevel()];
    double result = multInLevel * pProblem->CalculateFunctionals(y, fNumber);
    return result;
  }
  /**
  Вычисляет numPoints значений функции с номером fNumber, в координатах y в массив values
  Работает только если задача является наследником IGPUProblem
  */
  virtual void CalculateFuncsInManyPoints(double* y, int fNumber, int numPoints, double* values)
  {
    IGPUProblem* newProblem = dynamic_cast<IGPUProblem*>(pProblem);
    if (newProblem != 0)
    {
      newProblem->CalculateFunctionals(y, fNumber, numPoints, values);
      //double multInLevel = parameters.functionSignMultiplier[GetProcLevel()];

      //if (multInLevel != 1)
      //{
      //  for (size_t i = 0; i < size_t(numPoints); i++)
      //  {
      //    values[i] = values[i] = multInLevel;
      //  }
      //}
    }
  }

  /**
  Возвращает число дискретных параметров, дискретные параметры всегда последние в векторе y
  */
  virtual int GetNumberOfDiscreteVariable()
  {
    if (IsLeaf())
    {
      IIntegerProgrammingProblem* newProblem = dynamic_cast<IIntegerProgrammingProblem*>(pProblem);
      if (newProblem != 0)
      {
        return newProblem->GetNumberOfDiscreteVariable();
      }
    }
    return 0;
  }
  /**
  Возвращает число значений дискретного параметра discreteVariable.
  GetDimension() возвращает общее число параметров.
  (GetDimension() - GetNumberOfDiscreteVariable()) - номер начальной дискретной переменной
  Для не дискретных переменных == -1
  */
  virtual int GetNumberOfValues(int discreteVariable)
  {
    if (IsLeaf())
    {
      IIntegerProgrammingProblem* newProblem = dynamic_cast<IIntegerProgrammingProblem*>(pProblem);
      if (newProblem != 0)
      {
        return newProblem->GetNumberOfValues(discreteVariable);
      }
    }
    return -1;
  }
  /**
  Определяет значения дискретного параметра с номером discreteVariable
  Возвращает код ошибки.
  \param[out] values массив, в который будут сохранены значения дискретного параметра
  нулевой элемент это левая граница, поледний элемент это правая граница.
  */
  virtual int GetAllDiscreteValues(int discreteVariable, double* values)
  {
    if (IsLeaf())
    {
      IIntegerProgrammingProblem* newProblem = dynamic_cast<IIntegerProgrammingProblem*>(pProblem);
      if (newProblem != 0)
      {
        return newProblem->GetAllDiscreteValues(discreteVariable, values);
      }
    }
    return IProblem::ERROR;
  }
  ///**
  //Определяет значения дискретного параметра с номером discreteVariable после номера previousNumber
  //Возвращает код ошибки.
  //\param[in] previousNumber - номер значения после которого возвращается значение
  //-2 - значение по умолчанию, возвращает следующее значение
  //-1 - возвращает после -1, т.е. левую границу области
  //\param[out] value переменная в которую сохраняется значение дискретного параметра
  //*/
  //virtual int GetNextDiscreteValues(double& value, int discreteVariable, int previousNumber = -2)
  //{
  //  IIntegerProgrammingProblem* newProblem = dynamic_cast<IIntegerProgrammingProblem*>(pProblem);
  //  if (newProblem != 0)
  //  {
  //    return newProblem->GetNextDiscreteValues(value, discreteVariable, previousNumber);
  //  }
  //  return IProblem::ERROR;
  //}
  /**
  Проверяет является ли value допустимым значением для параметра с номером discreteVariable
  */
  virtual bool IsPermissibleValue(double value, int discreteVariable)
  {
    if (IsLeaf())
    {
      IIntegerProgrammingProblem* newProblem = dynamic_cast<IIntegerProgrammingProblem*>(pProblem);
      if (newProblem != 0)
      {
        return newProblem->IsPermissibleValue(value, discreteVariable);
      }
    }
    return false;
  }
  /**
  Возвращает максимальные значения функций
  Только для многокритериальной оптимизации
  */
  virtual double * getMin() { return NULL; }
  /**
  Возвращает максимальное значения функций
  Только для многокритериальной оптимизации
  */
  virtual double * getMax() { return NULL; }


  virtual bool IsInit()
  {
    return isInit;
  }

  virtual bool IsLeaf()
  {
    return ProcLevel == (parameters.NumOfTaskLevels - 1);
  }
};

#endif
// - end of file ----------------------------------------------------------------------------------

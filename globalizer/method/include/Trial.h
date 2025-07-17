/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      data.h                                                      //
//                                                                         //
//  Purpose:   Header file for search data classes                         //
//                                                                         //
//  Author(s): Sysoyev A., Barkalov K., Sovrasov V.                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __TRIAL_H__
#define __TRIAL_H__

#include "common.h"
#include "extended.h"

#include "task.h"

#include <cstring>



class TSearchInterval;
// ------------------------------------------------------------------------------------------------
class TTrial
{
protected:
  /// точка на одномерном отрезке
  Extended  x;

public:
  /// Индекс значения дискретного параметра
  int discreteValuesIndex;
  /// точка в многомерном пространстве
  double y[MaxDim];
  /// значения функций задачи, вычисленные до первого нарушенного
  double FuncValues[MaxNumOfFunc];
  /// индекс точки
  int index;
  /// число "вложенных" итераций
  int K;
  /// Метка точки: потенциальный локальный минимум, убывающая, возрастающая, потенциальный локальный максимум
  enum Status
  {
    low_inflection,
    low,
    up,
    up_inflection,
    local_min
  };

  /// Метка точки: потенциальный локальный минимум, убывающая, возрастающая, потенциальный локальный максимум
  Status pointStatus;

  ///Количество нужных для локального метода точек (обновляется только в случае потенциального локального минимума)
  int lowAndUpPoints;

  /// Задача порождаемая точкой при адаптивной схеме редукции
  TTask* generatedTask;

  
  /// Интервал слева, эта точка для него правая
  TSearchInterval* leftInterval;
  /// Правый интервал, эта точка для него левая(главная)
  TSearchInterval* rightInterval;
  /// Цвет рисования точки
  int TypeColor;
  /// Нужно ли пересчитать координаты
  bool isNeedRecalculateCoordinates;

  TTrial()
  {
    discreteValuesIndex = 0;

    x = 0.0;
    index = -2;
    K = 0;

    for (int i = 0; i < MaxNumOfFunc; i++)
      FuncValues[i] = MaxDouble;
    memset(y, 0, MaxDim * sizeof(*y));

    leftInterval = 0;
    rightInterval = 0;

    pointStatus = up_inflection;
    lowAndUpPoints = 0;

    TypeColor = 0;

    generatedTask = 0;

    isNeedRecalculateCoordinates = false;
  }

  TTrial(const TTrial& trial)
  {
    this->discreteValuesIndex = trial.discreteValuesIndex;
    this->x = trial.x;
    memcpy(this->y, trial.y, MaxDim * sizeof(double));
    memcpy(this->FuncValues, trial.FuncValues, MaxNumOfFunc * sizeof(double));

    this->index = trial.index;
    this->K = trial.K;
    this->leftInterval = trial.leftInterval;
    this->rightInterval = trial.rightInterval;
    this->pointStatus = trial.pointStatus;
    this->lowAndUpPoints = trial.lowAndUpPoints;
    this->TypeColor = trial.TypeColor;
    this->generatedTask = trial.generatedTask;
    this->isNeedRecalculateCoordinates = trial.isNeedRecalculateCoordinates;
  }

  virtual TTrial* Clone()
  {
    TTrial* res = new TTrial();

    res->discreteValuesIndex = discreteValuesIndex;
    res->x = x;
    memcpy(res->y, y, MaxDim * sizeof(double));
    memcpy(res->FuncValues, FuncValues, MaxNumOfFunc * sizeof(double));

    res->index = index;
    res->K = K;
    res->leftInterval = leftInterval;
    res->rightInterval = rightInterval;
    res->pointStatus = pointStatus;
    res->lowAndUpPoints = lowAndUpPoints;
    res->TypeColor = TypeColor;
    res->generatedTask = generatedTask;
    res->isNeedRecalculateCoordinates = isNeedRecalculateCoordinates;
    return res;
  }

  ~TTrial()
  {
    discreteValuesIndex = 0;

    x = 0.0;
    index = -2;
    K = 0;

    leftInterval = 0;
    rightInterval = 0;

    pointStatus = up_inflection;
    lowAndUpPoints = 0;

    TypeColor = 0;

    generatedTask = 0;
  }

  void SetX(Extended d)
  {
    x = d;
  }

  virtual TTrial& operator = (Extended d)
  {
    SetX(d);
    return *this;
  }

  virtual Extended  X()
  {
    return x;
  }

  virtual double GetFloor()
  {
    return this->discreteValuesIndex;
  }

  virtual double GetValue()
  {
    if (index < 0 || index >= MaxNumOfFunc)
      return FuncValues[0];
    else
      return FuncValues[index];
  }

  virtual TTrial* GetLeftPoint();

  virtual TTrial* GetRightPoint();

  virtual TTrial& operator = (const TTrial& trial)
  {
    if (this != &trial)
    {
      this->discreteValuesIndex = trial.discreteValuesIndex;
      this->x = trial.x;
      memcpy(this->y, trial.y, MaxDim * sizeof(double));
      memcpy(this->FuncValues, trial.FuncValues, MaxNumOfFunc * sizeof(double));

      this->index = trial.index;
      this->K = trial.K;
      this->leftInterval = trial.leftInterval;
      this->rightInterval = trial.rightInterval;
      this->pointStatus = trial.pointStatus;
      this->lowAndUpPoints = trial.lowAndUpPoints;
      this->TypeColor = trial.TypeColor;
      this->generatedTask = trial.generatedTask;
    }
    return *this;
  }

  virtual bool operator == (TTrial& t)
  {
    return (x == t.x) && (discreteValuesIndex == t.discreteValuesIndex);
  }

  virtual bool operator > (TTrial& t)
  {
    if (discreteValuesIndex > t.discreteValuesIndex)
      return true;
    else if (discreteValuesIndex < t.discreteValuesIndex)
      return false;
    else
      return x > t.x;
  }

  virtual bool operator < (TTrial& t)
  {
    if (discreteValuesIndex < t.discreteValuesIndex)
      return true;
    else if (discreteValuesIndex > t.discreteValuesIndex)
      return false;
    else
      return x < t.x;
  }


};

// ------------------------------------------------------------------------------------------------
class TMultievolventsTrial: public TTrial
{
public:
  /// Номер развертки
  int evolvent;

  TMultievolventsTrial() : TTrial()
  {
    evolvent = 0;
  }

  TMultievolventsTrial(const TMultievolventsTrial& trial) : TTrial(trial)
  {
    evolvent = trial.evolvent;
  }

  virtual TTrial* Clone()
  {
    TMultievolventsTrial* res = new TMultievolventsTrial();

    res->discreteValuesIndex = discreteValuesIndex;
    res->x = x;
    memcpy(res->y, y, MaxDim * sizeof(double));
    memcpy(res->FuncValues, FuncValues, MaxNumOfFunc * sizeof(double));

    res->index = index;
    res->K = K;
    res->leftInterval = leftInterval;
    res->rightInterval = rightInterval;
    res->evolvent = evolvent;
    res->pointStatus = pointStatus;
    res->lowAndUpPoints = lowAndUpPoints;
    res->TypeColor = TypeColor;
    res->generatedTask = generatedTask;
    return res;
  }

  virtual Extended  X()
  {
    return this->x + evolvent;
  }

  virtual TMultievolventsTrial& operator = (Extended d)
  {
    evolvent = int(d.toDouble());
    x = d - evolvent;

    return *this;
  }

  virtual TTrial& operator = (const TTrial& trial)
  {
    const TTrial* t = &trial;
    TMultievolventsTrial* mt = (TMultievolventsTrial*)(t);
    return *this = *mt;
  }

  virtual TTrial& operator = (const TMultievolventsTrial& trial)
  {
    if (this != &trial)
    {
      this->discreteValuesIndex = trial.discreteValuesIndex;
      this->x = trial.x;
      memcpy(this->y, trial.y, MaxDim * sizeof(double));
      memcpy(this->FuncValues, trial.FuncValues, MaxNumOfFunc * sizeof(double));

      this->index = trial.index;
      this->K = trial.K;
      this->leftInterval = trial.leftInterval;
      this->rightInterval = trial.rightInterval;
      this->evolvent = trial.evolvent;
      this->pointStatus = trial.pointStatus;
      this->lowAndUpPoints = trial.lowAndUpPoints;
      this->TypeColor = trial.TypeColor;
      this->generatedTask = trial.generatedTask;
    }
    return *this;
  }

  virtual bool operator == (TTrial& t)
  {
    TMultievolventsTrial* mt = static_cast<TMultievolventsTrial*>(&t);
    if (mt != 0)
      return *this == *mt;
    else
      return *this == t;
  }
  
  virtual bool operator > (TTrial& t)
  {
    TMultievolventsTrial* mt = static_cast<TMultievolventsTrial*>(&t);
    return *this > *mt;
  }

  virtual bool operator < (TTrial& t)
  {
    TMultievolventsTrial* mt = static_cast<TMultievolventsTrial*>(&t);
    return *this < *mt;
  }

  virtual bool operator == (TMultievolventsTrial& t)
  {
    if (evolvent != t.evolvent)
      return false;
    return x == t.x;
  }

  virtual bool operator > (TMultievolventsTrial& t)
  {
    if (evolvent > t.evolvent)
      return true;
    else if (evolvent < t.evolvent)
      return false;
    else
      return x > t.x;
  }

  virtual bool operator < (TMultievolventsTrial& t)
  {
    if (evolvent < t.evolvent)
      return true;
    else if (evolvent > t.evolvent)
      return false;
    else
      return x < t.x;
  }
};


#endif
// - end of file ----------------------------------------------------------------------------------

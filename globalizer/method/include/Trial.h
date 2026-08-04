/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2025 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      Trial.h                                                     //
//                                                                         //
//  Purpose:   Header file for search data classes                         //
//                                                                         //
//  Author(s): Lebedev I. Sysoyev A., Barkalov K., Sovrasov V.             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __TRIAL_H__
#define __TRIAL_H__

#include "Common.h"
#include "Extended.h"

#include "Task.h"

#include <cstring>



class SearchInterval;
// ------------------------------------------------------------------------------------------------
class Trial
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

  ///Количество нужных для локального метода точек (обновляется только в случае потенциального локального минимума)
  int lowAndUpPoints;

  /// Задача порождаемая точкой при адаптивной схеме редукции
  Task* generatedTask;

  
  /// Интервал слева, эта точка для него правая
  SearchInterval* leftInterval;

  /// Правый интервал, эта точка для него левая(главная)
  SearchInterval* rightInterval;

  /// Цвет рисования точки
  int TypeColor = 0;

  /// Поле для хранения номера итерации
  int iterationNumber = 0;

  /// Время создания точки
  double  creationTime = 0.0;

  /// Создает не вычесленное испытание в координате х=0
  Trial();

  /// Копия точки
  Trial(const Trial& trial);

  /// Создает копию точки
  virtual Trial* Clone();

  ~Trial();

  /** Задаем координату в одномерном пространстве
  перещет в многомерное не производится!

  \param[in] d - новая координата
  */
  void SetX(Extended d);

  /// Присвоение координаты точки в одномерном пространстве
  virtual Trial& operator = (Extended d);

  /// Возвращает координату точки
  virtual Extended  X();

  /// Возвращает левую границу отрезка == 0
  virtual double GetFloor();

  /// Возвращает значение испытания (с учетом индексной схемы)
  virtual double GetValue();

  /// Возврящает соседнюю с лева точку
  virtual Trial* GetLeftPoint();

  /// Возврящает соседнюю с права точку
  virtual Trial* GetRightPoint();

  /// Копирование точки
  virtual Trial& operator = (const Trial& trial);

  /// Сравнение точек в одномерном пространстве
  virtual bool operator == (Trial& t);

  /// Сравнение точек в одномерном пространстве
  virtual bool operator > (Trial& t);

  /// Сравнение точек в одномерном пространстве
  virtual bool operator < (Trial& t);

};



#endif
// - end of file ----------------------------------------------------------------------------------

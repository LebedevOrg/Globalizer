/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      method.h                                                    //
//                                                                         //
//  Purpose:   Header file for method class                                //
//                                                                         //
//  Author(s): Barkalov K., Sysoyev A.                                     //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


/**
\file method.h

\authors Баркалов К., Сысоев А.
\date 2015-2016
\copyright ННГУ им. Н.И. Лобачевского

\brief Объявление класса #TMethod

\details Объявление класса #TMethod и сопутствующих типов данных
*/


#ifndef __METHOD_INTERFACE_H__
#define __METHOD_INTERFACE_H__

#include "common.h"
#include "task.h"
#include "Trial.h"
#include "evolvent.h"
#include "parameters.h"

// ------------------------------------------------------------------------------------------------

/**
Базовый класс, реализующий алгоритм глобального поиска.

В классе #TMethod реализованы основные функции, определяющие работу алгоритма глобального поиска.
*/
class IMethod
{
public:
  /** Функция выполняет первую итерацию метода
  */
  virtual void FirstIteration() = 0;

  /** Вычисления точек очередной итерации

  Вычисленные точки очередной итерации записываются в массив #pCurTrials
  */
  virtual void CalculateIterationPoints() = 0;

  /** Вычисление функций задачи

  Проводятся испытания в точках из массива #pCurTrials, результаты проведенных испытаний
  записываются в тот же массив
  */
  virtual void CalculateFunctionals() = 0;

  /** Обновление поисковой информации
  */
  virtual void RenewSearchData() = 0;

  /** Проверка выполнения критерия остановки метода

  Метод прекращает работу в следующих случаях:
  - число испытаний превысило максимально допустимое значение
  - если решается одна задача и выполнен критерий \f$ x_t - x_{t-1} < \epsilon \f$
  - если решается серия задач и выполнен критерий \f$ \| y^k - y^\ast \| < \epsilon \f$

  \return истина, если критерий остановки выполнен; ложь - в противном случае.
  */
  virtual bool CheckStopCondition() = 0;

  /** Оценить текущее значение оптимума

  \return истина, если оптимум изменился; ложь - в противном случае
  */
  virtual bool EstimateOptimum() = 0;

  /** Функция вызывается в конце проведения итерации
  */
  virtual void FinalizeIteration() = 0;

  /** Получить число испытаний

  \return число испытаний
  */
  virtual int GetIterationCount() = 0;

  /** Получить текущую оценку оптимума

  \return испытание, соответствующее текущему оптимуму
  */
  virtual TTrial* GetOptimEstimation() = 0;

  /**Сбор статистики

  Функция возвращает общее число испытаний, выполненных при решении текущей задачи и всех вложенных
  подзадач
  \return общее число испытаний
  */
  virtual int GetNumberOfTrials() = 0;

  ///Возвращает число вычислений каждой функции
  virtual std::vector<int> GetFunctionCalculationCount() = 0;

  /// Возвращает достигнутую точность
  virtual double GetAchievedAccuracy() = 0;


  /**Добавляет испытания в поисковую информацию, при этом обновляя константу Гёльдера и
  оценку оптимума

  \param[in] points точки испытаний, которые будут добавлены
  */
  virtual void InsertPoints(const std::vector<TTrial*>& points) = 0;

  /// Печать точек в файл, для отрисовки
  virtual void PrintPoints(const std::string & fileName) = 0;

  /// Запуск локального метода Хука-Дживса
  virtual void LocalSearch() = 0;

  /// Метод сохраняющий точки в статический массив
  virtual void  SavePoints() = 0;

  /// Возвращает число точек полученное от локальныго метода
  virtual int GetLocalPointCount() = 0;
  /// Возвращает число запусков локально метода
  virtual int GetNumberLocalMethodtStart() = 0;
  /// Печатает информацию о сечениях
  virtual void PrintSection() = 0;
};

#endif
// - end of file ----------------------------------------------------------------------------------

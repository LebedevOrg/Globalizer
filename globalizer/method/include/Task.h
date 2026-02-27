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


/**
 * \file task.h
 *
 * \authors Сысоев А., Баркалов К.
 * \date 2015-2016
 * \copyright ННГУ им. Н.И. Лобачевского
 *
 * \brief Объявление класса #Task
 *
 * \details Объявление класса #Task, который представляет собой обертку
 * над решаемой задачей оптимизации (#IProblem).
 */

#ifndef __TASK_H__
#define __TASK_H__

#include "Parameters.h"
#include "Common.h"
#include "ProblemInterface.h"
#include "Exception.h"
#include "BaseInterval.h"

class Trial;

// ------------------------------------------------------------------------------------------------

/**
 * \brief Класс, инкапсулирующий информацию о задаче оптимизации.
 *
 * #Task является оберткой над интерфейсом #IProblem и предоставляет
 * доступ к параметрам задачи, таким как границы поиска, число функций,
 * известные оптимальные значения, а также предоставляет методы для
 * вычисления значений функций.
 */

class Task: public QueueBaseData
{
protected:
  /// Левая граница области поиска
  double     A[MaxDim];
  /// Правая граница области поиска
  double     B[MaxDim];
  /// Число функционалов (последний - критерий)
  int        NumOfFunc;
  /// Указатель на саму задачу оптимизации
  IProblem*  pProblem;
  /// Оптимальное значение целевой функции (определено, если известно из задачи)
  double     OptimumValue;
  /// Координаты глобального минимума целевой функции (определено, если известно)
  double     OptimumPoint[MaxDim];
  /// true, если в задаче известно оптимальное значение критерия
  bool       IsOptimumValueDefined;
  /// true, если в задаче известна точка глобального минимума
  bool       IsOptimumPointDefined;
  /// Уровень процесса в дереве процессов
  int ProcLevel;
  /// Флаг, указывающий, был ли класс инициализирован
  bool isInit;

public:
  int num;

  /**
   * \brief Конструктор.
   * \param[in] _problem Указатель на объект задачи оптимизации.
   * \param[in] _ProcLevel Уровень процесса в дереве процессов.
   */
  Task(IProblem* _problem, int _ProcLevel);

  /**
   * \brief Конструктор по умолчанию.
   * Создает неинициализированный объект.
   */
  Task();

  /**
   * \brief Деструктор.
   */
  virtual ~Task();

  /**
   * \brief Создает клон текущего объекта.
   * \return Указатель на новый объект #Task.
   */
  virtual Task* Clone();

  /**
   * \brief Создает клон текущего объекта с новыми данными (в данной реализации эквивалентно #Clone).
   * \return Указатель на новый объект #Task.
   */
  virtual Task* CloneWithNewData();

  /**
   * \brief Инициализирует объект данными задачи.
   * \param[in] _problem Указатель на объект задачи оптимизации.
   * \param[in] _ProcLevel Уровень процесса в дереве процессов.
   */
  virtual void Init(IProblem* _problem, int _ProcLevel);

  /**
   * \brief Возвращает общую размерность задачи.
   * \return Размерность задачи.
   */
  virtual int GetN() const;

  /**
   * \brief Возвращает левую границу области поиска.
   * \return Указатель на массив со значениями левой границы.
   */
  virtual const double* GetA() const;
  
  /**
   * \brief Возвращает правую границу области поиска.
   * \return Указатель на массив со значениями правой границы.
   */
  virtual const double* GetB() const;

  /**
  * \brief Возвращает априори известное значение глобального минимума.
  * \return Значение глобального минимума.
  */
  virtual double GetOptimumValue() const;
 
  /**
   * \brief Обновляет координаты точки глобального минимума из объекта #IProblem.
   */
  virtual void resetOptimumPoint();
 
  /**
   * \brief Возвращает априори известные координаты точки глобального минимума.
   * \details Перед первым вызовом рекомендуется вызвать #resetOptimumPoint().
   * \return Указатель на массив с координатами точки глобального минимума.
   */
  virtual const double* GetOptimumPoint() const;
  
  /**
   * \brief Проверяет, известно ли для задачи значение глобального минимума.
   * \return true, если значение известно, иначе false.
   */
  virtual bool GetIsOptimumValueDefined() const;
 
  /**
   * \brief Проверяет, известны ли для задачи координаты глобального минимума.
   * \return true, если координаты известны, иначе false.
   */

  virtual bool GetIsOptimumPointDefined() const;
 
  /**
  * \brief Возвращает указатель на текущую задачу.
  * \return Указатель на объект #IProblem.
  */
  virtual IProblem* getProblem();

  /**
   * \brief Возвращает число функций (ограничения и критерии).
   * \return Число функций.
   */
  virtual int GetNumOfFunc() const;

  /**
     * \brief Задает число функций.
     * \param[in] nf Новое число функций.
     */
  virtual void SetNumofFunc(int nf);

  /**
     * \brief Возвращает уровень процесса в дереве процессов.
     * \return Уровень процесса.
     */
  int GetProcLevel();

  /**
   * \brief Возвращает число функций в исходной задаче.
   * \return Число функций.
   */
  virtual int GetNumOfFuncAtProblem() const;

  /**
     * \brief Вычисляет значение функции с номером fNumber в точке y.
     * \param[in] y Указатель на массив с координатами точки.
     * \param[in] fNumber Номер вычисляемой функции.
     * \return Значение функции.
     */
  virtual double CalculateFuncs(const double* y, int fNumber);
  
  /**
 * \brief Вычисляет значения функции в нескольких точках (для GPU).
 * \details Работает только если задача является наследником #IGPUProblem.
 * \param[in] y Указатель на массив координат точек.
 * \param[in] fNumber Номер функции.
 * \param[in] numPoints Количество точек.
 * \param[out] values Массив для записи результатов.
 */
  virtual void CalculateFuncsInManyPoints(double* y, int fNumber, int numPoints, double* values);

  /**
  * \brief Возвращает число дискретных параметров.
  * \details Дискретные параметры всегда последние в векторе y.
  * \return Число дискретных переменных или 0, если задача не является целочисленной.
  */
  virtual int GetNumberOfDiscreteVariable();
 
  /**
  * \brief Возвращает число допустимых значений для дискретного параметра.
  * \param[in] discreteVariable Индекс дискретной переменной.
  * \return Число значений или -1, если задача не является целочисленной.
  */
  virtual int GetNumberOfValues(int discreteVariable);

  /**
     * \brief Определяет все допустимые значения дискретного параметра.
     * \param[in] discreteVariable Индекс дискретной переменной.
     * \param[out] values Массив, в который будут сохранены значения.
     * \return Код ошибки (#IProblem::OK или #IProblem::ERROR).
     */
  virtual int GetAllDiscreteValues(int discreteVariable, double* values);
  
  /**
  * \brief Проверяет, является ли значение допустимым для дискретного параметра.
  * \param[in] value Проверяемое значение.
  * \param[in] discreteVariable Индекс дискретной переменной.
  * \return true, если значение допустимо, иначе false.
  */
  virtual bool IsPermissibleValue(double value, int discreteVariable);
 
  /**
   * \brief Возвращает минимальные значения функций (для многокритериальной оптимизации).
   * \return NULL в текущей реализации.
   */
  virtual double* getMin();
 
  /**
   * \brief Возвращает максимальные значения функций (для многокритериальной оптимизации).
   * \return NULL в текущей реализации.
   */
  virtual double* getMax();

  /**
   * \brief Проверяет, был ли объект инициализирован.
   * \return true, если объект инициализирован, иначе false.
   */
  virtual bool IsInit();

  /**
  * \brief Проверяет, является ли задача листом в дереве процессов.
  * \return true, если ProcLevel не равен 0, иначе false.
  */
  virtual bool IsLeaf();

  /**
* \brief Копирует координаты точки из массива, согласно имеющимся правилам
* \param[in] y имеющисся координаты.
* \param[out] point точка назначения.
* \return true, если значение допустимо, иначе false.
*/
  virtual void CopyPoint(double* y, Trial* point);

  /// <summary>
/// Копируем координаты точки
/// </summary>
/// <param name="resPoint"></param>
/// <param name="y"></param>
  virtual void TransformPoint(double* resPoint, const double* y);
};

#endif
// - end of file ----------------------------------------------------------------------------------

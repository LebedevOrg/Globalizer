#pragma once

#include "Task.h"

class HDTask : public Task
{
protected:

  /// Начало блока переменных
  int startParameterNumber;
  /// Стартовый индекс переменных
  int startIndex;
  /// Флаг для задач со смешанными параметрами
  bool isMixedInteger;


public:
  HDTask(IProblem* _problem, int _ProcLevel, bool _isMixedInteger = true);
  HDTask(bool _isMixedInteger = true);

  /// Создает копию класса
  virtual Task* Clone();

  /// Возвращает левую границу области поиска
  virtual const double* GetA() const;
  /// Возвращает правую границу области поиска
  virtual const double* GetB() const;

  /**
 Возвращает априори известные координаты точки глобального минимума
 Перед первым вызовом нужно вызвать resetOptimumPoint()
 */
  virtual const double* GetOptimumPoint() const;
  /// Вычисляет значение функции с номером fNumber в точке y
  virtual double CalculateFuncs(const double* y, int fNumber);
  /**
  Вычисляет numPoints значений функции с номером fNumber, в координатах y в массив values
  Работает только если задача является наследником IGPUProblem
  */
  virtual void CalculateFuncsInManyPoints(double* y, int fNumber, int numPoints, double* values);

  /// Задает начало блока переменных
  void SetStartParameterNumber(int _startParameterNumber);

  /**
  * \brief Копирует координаты точки из массива, согласно имеющимся правилам
  * \param[in] y имеющиеся координаты.
  * \param[out] point точка назначения.
  * \return true, если значение допустимо, иначе false.
  */
  virtual void CopyPoint(double* y, Trial* point);

  /// <summary>
  /// Копирует координаты точки
  /// </summary>
  /// <param name="resPoint"></param>
  /// <param name="y"></param>
  virtual void TransformPoint(double* resPoint, const double* y);

  /// Возвращает число непрерывных параметров
  virtual int GetNumberOfContinuousVariable();

  /// Возвращает число дискретных параметров
  virtual int GetNumberOfDiscreteVariable();

  /// Задает задачу со смешанными переменными
  void SetMixedInteger();

  /// Задает задачу с непрерывными переменными
  virtual void UnsetMixedInteger();

  /// Задает стартовый индекс параметров задачи
  virtual void SetStartIndex(int);

  /// Задает стартовый индекс дискретных параметров задачи
  virtual int GetStartDiscreteVariable();
};

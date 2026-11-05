/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  File:      DynamicProblem.h                                            //
//                                                                         //
//  Purpose:   Problem with changeable parameter for dynamic optimization  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __DYNAMIC_PROBLEM_H__
#define __DYNAMIC_PROBLEM_H__

#include "Problem.h"
#include <iostream>
#include <vector>
#include <cmath>

/**
 * @brief Задача оптимизации с изменяемым параметром
 * 
 * Наследник BaseProblem для задач, где параметры целевого функционала
 * меняются между итерациями (например, онлайн-оптимизация, адаптивное управление).
 * 
 * Целевая функция: f(x) = x² · data
 */
class DynamicProblem : public Problem<DynamicProblem>
{
#undef OWNER_NAME
#define OWNER_NAME DynamicProblem
private:
  /// Текущее значение изменяемого параметра
  double mData;
  
  /// Границы области поиска (для каждой размерности)
  std::vector<double> mLowerBounds;
  std::vector<double> mUpperBounds;
  
  /// Число функций (критериев)
  int mNumberOfFunctions;
  
  /// Число ограничений
  int mNumberOfConstraints;
  
  
public:
  /**
   * @brief Конструктор
   * @param dimension Размерность задачи
   * @param leftBound Нижняя граница области поиска
   * @param rightBound Верхняя граница области поиска
   * @param initialData Начальное значение параметра data
   */
  DynamicProblem(int dimension, double leftBound, double rightBound, double initialData)
    : Problem<DynamicProblem>()
    , mData(initialData)
    , mLowerBounds(dimension, leftBound)
    , mUpperBounds(dimension, rightBound)
    , mNumberOfFunctions(1)      // один критерий
    , mNumberOfConstraints(0)    // без ограничений
   {
    // Устанавливаем размерность через TypedProperty
    this->Dimension = dimension;
    this->mDim = dimension;
    
    // Устанавливаем границы через внутренние члены базового класса
    this->mLeftBorder = leftBound;
    this->mRightBorder = rightBound;
    
    // Без дискретных переменных
    this->NumberOfDiscreteVariable = 0;
    
    mMinDimension = 1;
  }
  
  /**
   * @brief Обновить значение параметра data
   * @param newData Новое значение параметра
   * 
   * Вызывается между итерациями для изменения задачи оптимизации
   */
  void SetData(double newData)
  {
    mData = newData;
  }
  
  /**
   * @brief Получить текущее значение параметра data
   */
  double GetData() const
  {
    return mData;
  }
  
  /**
   * @brief Получение границ области поиска
   * @param lower Массив для нижних границ (размера Dimension)
   * @param upper Массив для верхних границ (размера Dimension)
   * 
   * Интерфейс BaseProblem/IProblem
   */
  virtual void GetBounds(double* lower, double* upper) override
  {
    int dim = (int)this->Dimension;
    for (int i = 0; i < dim; ++i)
    {
      lower[i] = mLowerBounds[i];
      upper[i] = mUpperBounds[i];
    }
  }
  
  /**
   * @brief Получить число функций (критериев)
   * @return Количество целевых функций (здесь 1)
   */
  virtual int GetNumberOfFunctions() const override
  {
    return mNumberOfFunctions;
  }
  
  /**
   * @brief Получить число ограничений
   * @return Количество ограничений (здесь 0)
   */
  virtual int GetNumberOfConstraints() const override
  {
    return mNumberOfConstraints;
  }

#define FunctionalParametersType double
#define IntParametersType unsigned long long int

  FunctionalParametersType Functional(FunctionalParametersType x)
  {
    return sin(x) * cos(FunctionalParametersType(1.0) / x) + FunctionalParametersType(1.0);
  }

  FunctionalParametersType IntToFunctionalParametersType(IntParametersType t)
  {
    FunctionalParametersType res = 0;
    res = *((FunctionalParametersType*)((char*)(&t)));
    return res;
  }
  
  /**
   * @brief Вычисление значения целевой функции
   * @param y Точка в пространстве поиска (массив размера Dimension)
   * @param fNumber Индекс функции (для многокритериальных задач, здесь всегда 0)
   * @return Значение f(x) = x² · data
   * 
   * Основной метод интерфейса IProblem
   */
  virtual double CalculateFunctionals(const double* y, int fNumber) override
  {
    
    IntParametersType t = (IntParametersType)(y[0]);
    // Для одномерной задачи берём первую координату
    double minVal = 0;
    double minX = 0;
    FunctionalParametersType x = IntToFunctionalParametersType(t + 0);
    double result = Functional(x);
    minVal = result;
    minX = (double)t ;


    return minVal;
  }

  double FinishCalculateFunctionals(double* y, IntParametersType& minT)
  {

    IntParametersType t = (IntParametersType)(y[0]) ;
    // Для одномерной задачи берём первую координату
    double minVal = 0;
    double minX = 0;
    FunctionalParametersType x = IntToFunctionalParametersType(t + 0);
    double result = Functional(x);
    minVal = result;
    minX = (double)t ;
    minT = t;

    y[0] = minX;

    return minVal;
  }


  /**
   * @brief Вычисление функций задачи сразу в нескольких точках (векторизовано)
   *
   * @param[in]  y         Массив последовательно записанных многомерных точек,
   *                       в которых нужно вычислить функционал. Раскладка:
   *                       y[j * Dimension + k] — k-я координата j-й точки.
   * @param[in]  fNumber   Номер вычисляемой функции (здесь единственная, 0).
   * @param[in]  numPoints Количество передаваемых точек в блоке.
   * @param[out] values    Массив длины numPoints для значений функционала:
   *                       values[j] = f(точка_j).
   *
   * Именно этот метод вызывает вычислитель блока точек (CPUMultiPointCalculation
   * / CUDACalculation) через Task::CalculateFuncsInManyPoints(). Размер блока
   * numPoints определяется параметрами Globalizer (SizeInBlock / NumPoints).
   *
   * Для f(x) = x² · data реализовано векторизованно: AVX обрабатывает по 4
   * числа double за такт (случай Dimension == 1, координаты непрерывны),
   * иначе — корректный скалярный путь.
   */
  virtual void CalculateFunctionals(double* y, int fNumber, int& numPoints,
                                    double* values) override
  {
    const int n = numPoints;

    for (size_t j = 0; j < n; ++j)
    {
      IntParametersType t = (IntParametersType)(y[j]);

      FunctionalParametersType x = IntToFunctionalParametersType(t);
      double result = Functional(x);
      values[j] = result;
    }
  }
    
  /**
   * @brief Инициализация задачи (если нужна дополнительная настройка)
   */
  virtual int Initialize() override
  {
    // Вызываем базовую инициализацию
    int result = BaseProblem<DynamicProblem>::Initialize();

    return result;
  }
  
  /**
   * @brief Деструктор
   */
  virtual ~DynamicProblem() {}
};

#endif // __DYNAMIC_PROBLEM_H__
// - end of file ----------------------------------------------------------------------------------

#ifdef _GLOBALIZER_BENCHMARKS
#pragma once

#include "IGlobalOptimizationProblem.h"
#include "Problem.h"

/** Класс задачи обертки для интерфейса IGlobalOptimizationProblem
*/
class GlobalizerBenchmarksProblem : public Problem<GlobalizerBenchmarksProblem>
{
#undef OWNER_NAME
#define OWNER_NAME GlobalizerBenchmarksProblem

protected:
  /// Новая задача
  IGlobalOptimizationProblem* problem;

  /// Все комбинации дискретных параметров
  std::vector< std::vector<std::string>> DiscreteVariableValues;
  /// Левая граница области определения
  std::vector<double> A;
  /// Правая граница области определения
  std::vector<double> B;
  /// Известная точка оптимума
  std::vector<double> optPoint;
  /// Номера значений дискретных параметров
  std::vector <std::vector<double>> discreteValues;

public:

  /** Метод, получает по координатам непрерывных параметров и номерам дискретных два вектора с непрерывными и дискретными параметрами соответственно

  \param[in] x координаты точки, сначала непрерывные координаты, затем номера дискретных параметров
  \param[out] y непрерывные координаты точки, в которой необходимо вычислить значение
  \param[out] u целочисленые координаты точки, в которой необходимо вычислить значение
  */
  void XtoYU(const double* x, std::vector<double>& y, std::vector<std::string>& u) const;

  /** Метод, преобразует вектора с непрерывными и дискретными параметрами объединенный массив с координатами

  \param[in] y непрерывные координаты точки, в которой необходимо вычислить значение
  \param[in] u целочисленые координаты точки, в которой необходимо вычислить значение
  \param[out] x координаты точки, сначала непрерывные координаты, затем номера дискретных параметров
  */
  void YUtoX(std::vector<double>& y, std::vector<std::string>& u, double* x) const;


  GlobalizerBenchmarksProblem(IGlobalOptimizationProblem* _problem);

  /** Метод возвращает значение целевой функции в точке глобального минимума
  \param[out] value оптимальное значение
  \return Код ошибки (#OK или #UNDEFINED)
  */
  virtual int GetOptimumValue(double& value) const;

  /** Метод возвращает координаты точки глобального минимума целевой функции
  \param[out] x точка, в которой достигается оптимальное значение
  \return Код ошибки (#OK или #UNDEFINED)
  */
  virtual int GetOptimumPoint(double* x) const;

  /** Метод, вычисляющий функции задачи

  \param[in] x Точка, в которой необходимо вычислить значение
  \param[in] fNumber Номер вычисляемой функции. 0 соответствует первому ограничению,
  #GetNumberOfFunctions() - 1 -- последнему критерию
  \return Значение функции с указанным номером
  */
  virtual double CalculateFunctionals(const double* x, int fNumber);

  /** Задание пути до конфигурационного файла

  Данный метод должн вызываться перед #Initialize
  \param[in] configPath строка, содержащая путь к конфигурационному файлу задачи
  \return Код ошибки
  */
  virtual int SetConfigPath(const std::string& configPath);

  /** Метод задаёт размерность задачи

  Данный метод должен вызываться перед #Initialize. Размерность должна быть в
  списке поддерживаемых.
  \param[in] dimension размерность задачи
  \return Код ошибки
  */
  virtual int SetDimension(int dimension);

  ///Возвращает размерность задачи, можно вызывать после #Initialize
  virtual int GetDimension() const;

  /** Метод возвращает границы области поиска
  */
  virtual void GetBounds(double* lower, double* upper);

  /**
  Возвращает число значений дискретного параметра discreteVariable.
  GetDimension() возвращает общее число параметров.
  (GetDimension() - GetNumberOfDiscreteVariable()) - номер начальной дискретной переменной
  Для не дискретных переменных == -1
  */
  virtual int GetNumberOfValues(int discreteVariable);

  /// Очищает номер текущего значения для дискретного параметра
  virtual void ClearCurrentDiscreteValueIndex(int** mCurrentDiscreteValueIndex);

  /**
  Определяет значения дискретного параметра с номером discreteVariable
  Возвращает код ошибки.
  \param[out] values массив, в который будут сохранены значения дискретного параметра
  */
  virtual int GetAllDiscreteValues(int discreteVariable, double* values);

  /**
  Возвращает значения дискретных параметров
*/
  virtual std::vector< std::vector<std::string>> GetDiscretesValues();

  /**
  Определяет значения дискретного параметра с номером discreteVariable после номера previousNumber
  Возвращает код ошибки.
  \param[in] previousNumber - номер значения после которого возвращается значение
  -2 - значение по умолчанию, возвращает следующее значение
  -1 - возвращает после -1, т.е. левую границу области
  \param[out] value переменная в которую сохраняется значение дискретного параметра
  */
  virtual int GetNextDiscreteValues(int* mCurrentDiscreteValueIndex, double& value, int discreteVariable, int previousNumber = -2);
};
#endif
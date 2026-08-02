#include "Task.h"
#include "Problem.h"
#include "test_config.h"
#include "ProblemFromFunctionPointers.h"

#include <gtest/gtest.h>
#include <string>
#include <cstdlib>

/**
  Вспомогательный класс, помогающий задать начальную конфигурацию объекта класса #Task,
  которая будет использоваться в тестах
 */
class TaskTest : public ::testing::Test
{
protected:
  /// Размерность задачи
  static const int n = 5;
  /// Размерность подзадачи
  static const int freeN = 2;
  /// Число функционалов
  static const int numOfFunc = 1;
  /// Левая граница области поиска
  double A[MaxDim];
  /// Правая граница области поиска
  double B[MaxDim];
  /// Указатель на задачу
  Task* task;
  //Функционалы
  IProblem* problem;

  void SetUp()
  {
    parameters.Dimension = n;
      problem = new ProblemFromFunctionPointers(n, // размерность задачи
          std::vector<double>(parameters.Dimension, -2.2), // верхняя граница
          std::vector<double>(parameters.Dimension, 1.8), // нижняя граница
          std::vector<std::function<double(const double*)>>(1, [](const double* y)
              {
                  double pi_ = 3.14159265358979323846;
                  double sum = 0.;
                  for (int j = 0; j < parameters.Dimension; j++)
                      sum += y[j] * y[j] - 10. * cos(2.0 * pi_ * y[j]) + 10.0;
                  return sum;
              }), // критерий
          true, // определен ли оптимум
          0, // значение глобального оптимума
          std::vector<double>(parameters.Dimension, 0) // координаты глобального минимума

      );

      task = new Task(problem, 0);

  }

  void TearDown()
  {
    delete task;
  }
};

// Определения статических переменных
const int TaskTest::n;
const int TaskTest::freeN;
const int TaskTest::numOfFunc;

/**
 * Проверка параметра размерности задачи N
 * 1<= N <= MaxDim
 */
TEST_F(TaskTest, throws_when_create_with_negative_N)
{
  int oldN = parameters.Dimension;
  parameters.Dimension = -1;
  ASSERT_ANY_THROW(Task testTask( problem, 0));
  parameters.Dimension = oldN;
}

TEST_F(TaskTest, throws_when_create_with_null_N)
{
  int oldN = parameters.Dimension;
  parameters.Dimension = 0;
  ASSERT_ANY_THROW(Task testTask( problem, 0));
  parameters.Dimension = oldN;
}

TEST_F(TaskTest, throws_when_create_with_too_large_N)
{
  int oldN = parameters.Dimension;
  parameters.Dimension = 500;
  ASSERT_ANY_THROW(Task testTask(problem, 0));
  parameters.Dimension = oldN;
}

/**
 * Создание задачи с корректными входными параметрами
 */
TEST_F(TaskTest, can_create_with_correct_values)
{
  ASSERT_NO_THROW(Task testTask( problem, 0));
}

/**
 * Тестирует метод SetNumofFunc.
 */
TEST_F(TaskTest, SetNumOfFuncUpdatesValue)
{
    task->SetNumofFunc(10);
    EXPECT_EQ(task->GetNumOfFunc(), 10);
}

/**
 * Тестирует методы, проверяющие состояние объекта: IsInit и IsLeaf.
 */
TEST_F(TaskTest, StateCheckMethods)
{
    EXPECT_TRUE(task->IsInit());
    EXPECT_FALSE(task->IsLeaf());

    Task leafTask(problem, 1);
    EXPECT_TRUE(leafTask.IsLeaf());
}

/**
 * Тестирует клонирование объекта.
 */
TEST_F(TaskTest, CloneCreatesIdenticalCopy)
{
    Task* clone = task->Clone();
    ASSERT_NE(clone, nullptr);
    ASSERT_NE(clone, task);

    EXPECT_EQ(clone->IsInit(), task->IsInit());
    EXPECT_EQ(clone->GetN(), task->GetN());
    EXPECT_EQ(clone->GetNumOfFunc(), task->GetNumOfFunc());
    EXPECT_EQ(clone->getProblem(), task->getProblem());

    delete clone;
}

/**
 * Тестирует вычисление функции и учет множителя.
 */
TEST_F(TaskTest, CalculateFuncsAppliesMultiplier)
{
    double y[n] = { 1.0, 1.0, 1.0, 1.0, 1.0 };
    double baseValue = 5.0; // f(y) = 5 * (1 - 10*cos(2pi) + 10) = 5

    parameters.FunctionSignMultiplier[0] = 1.0;
    EXPECT_DOUBLE_EQ(task->CalculateFuncs(y, 0), baseValue);

    parameters.FunctionSignMultiplier[0] = -2.0;
    EXPECT_DOUBLE_EQ(task->CalculateFuncs(y, 0), baseValue * -2.0);

    parameters.FunctionSignMultiplier[0] = 1.0;
}

/**
 * Проверяет, что getMin() и getMax() возвращают NULL.
 */
TEST_F(TaskTest, GetMinMaxReturnNull)
{
    EXPECT_EQ(task->getMin(), nullptr);
    EXPECT_EQ(task->getMax(), nullptr);
}

/* ======================================================================== *\
**  НОВЫЕ ТЕСТЫ                                                             **
\* ======================================================================== */

/* ------------------------------------------------------------------------ *\
**  Конструктор по умолчанию (неинициализированный Task)                    **
\* ------------------------------------------------------------------------ */

/// Конструктор по умолчанию создаёт неинициализированный объект
TEST_F(TaskTest, default_ctor_creates_uninitialized)
{
  Task empty;
  EXPECT_FALSE(empty.IsInit());
  EXPECT_EQ(0, empty.GetNumOfFunc());
  EXPECT_EQ(nullptr, empty.getProblem());
  EXPECT_FALSE(empty.GetIsOptimumValueDefined());
  EXPECT_FALSE(empty.GetIsOptimumPointDefined());
}

/// Неинициализированный Task не является листом (ProcLevel == 0)
TEST_F(TaskTest, default_ctor_is_not_leaf)
{
  Task empty;
  EXPECT_FALSE(empty.IsLeaf());
  EXPECT_EQ(0, empty.GetProcLevel());
}

/* ------------------------------------------------------------------------ *\
**  Размерность и границы поиска                                            **
\* ------------------------------------------------------------------------ */

/// GetN возвращает размерность из параметров
TEST_F(TaskTest, get_n_returns_dimension)
{
  EXPECT_EQ(parameters.Dimension, task->GetN());
  EXPECT_EQ(n, task->GetN());
}

/// GetN отражает изменение parameters.Dimension (размерность берётся из параметров)
TEST_F(TaskTest, get_n_follows_parameters_dimension)
{
  int oldN = parameters.Dimension;
  parameters.Dimension = 3;
  EXPECT_EQ(3, task->GetN());
  parameters.Dimension = oldN; // восстановить глобальное состояние
}

/// GetA возвращает не-nullptr указатель на левую границу
TEST_F(TaskTest, get_a_not_null)
{
  ASSERT_NE(nullptr, task->GetA());
}

/// GetB возвращает не-nullptr указатель на правую границу
TEST_F(TaskTest, get_b_not_null)
{
  ASSERT_NE(nullptr, task->GetB());
}

/// Границы поиска соответствуют заданным в задаче (lower/upper)
/// В ProblemFromFunctionPointers GetBounds заполняет lower=lowerBounds, upper=upperBounds.
/// В SetUp переданы -2.2 и 1.8 (в порядке аргументов конструктора: lower_, upper_).
TEST_F(TaskTest, bounds_match_problem_definition)
{
  const double* a = task->GetA();
  const double* b = task->GetB();
  for (int i = 0; i < task->GetN(); i++)
  {
    // A/B заполняются через problem->GetBounds(A, B) в конструкторе Task.
    // Значения должны совпадать с массивами границ задачи.
    EXPECT_NEAR(-2.2, a[i], 1e-9) << "index " << i;
    EXPECT_NEAR(1.8, b[i], 1e-9) << "index " << i;
  }
}

/// Левая граница всегда меньше правой во всех координатах
TEST_F(TaskTest, lower_bound_less_than_upper)
{
  const double* a = task->GetA();
  const double* b = task->GetB();
  for (int i = 0; i < task->GetN(); i++)
    EXPECT_LT(a[i], b[i]) << "index " << i;
}

/* ------------------------------------------------------------------------ *\
**  Оптимум задачи                                                          **
\* ------------------------------------------------------------------------ */

/// Для задачи задан оптимум -> флаги определены как true
TEST_F(TaskTest, optimum_flags_defined_when_set)
{
  // В SetUp isSetOptimum = true, значит оба флага должны быть true.
  EXPECT_TRUE(task->GetIsOptimumValueDefined());
  EXPECT_TRUE(task->GetIsOptimumPointDefined());
}

/// Значение оптимума совпадает с заданным (0)
TEST_F(TaskTest, optimum_value_matches)
{
  EXPECT_DOUBLE_EQ(0.0, task->GetOptimumValue());
}

/// resetOptimumPoint обновляет точку оптимума из задачи без исключений
TEST_F(TaskTest, reset_optimum_point_no_throw)
{
  ASSERT_NO_THROW(task->resetOptimumPoint());
}

/// После resetOptimumPoint координаты оптимума соответствуют заданным (все 0)
TEST_F(TaskTest, optimum_point_matches_after_reset)
{
  task->resetOptimumPoint();
  const double* pt = task->GetOptimumPoint();
  ASSERT_NE(nullptr, pt);
  for (int i = 0; i < task->GetN(); i++)
    EXPECT_NEAR(0.0, pt[i], 1e-9) << "index " << i;
}

/* ------------------------------------------------------------------------ *\
**  getProblem / число функций                                             **
\* ------------------------------------------------------------------------ */

/// getProblem возвращает тот же указатель, что передан в конструктор
TEST_F(TaskTest, get_problem_returns_same_pointer)
{
  EXPECT_EQ(problem, task->getProblem());
}

/// NumOfFunc инициализируется из задачи (1 критерий, 0 ограничений)
TEST_F(TaskTest, num_of_func_from_problem)
{
  EXPECT_EQ(problem->GetNumberOfFunctions(), task->GetNumOfFunc());
  EXPECT_EQ(1, task->GetNumOfFunc());
}

/// GetNumOfFuncAtProblem возвращает то же, что и GetNumOfFunc
TEST_F(TaskTest, num_of_func_at_problem_consistent)
{
  EXPECT_EQ(task->GetNumOfFunc(), task->GetNumOfFuncAtProblem());
}

/// SetNumofFunc влияет и на GetNumOfFuncAtProblem (общее поле NumOfFunc)
TEST_F(TaskTest, set_num_of_func_affects_at_problem)
{
  task->SetNumofFunc(7);
  EXPECT_EQ(7, task->GetNumOfFunc());
  EXPECT_EQ(7, task->GetNumOfFuncAtProblem());
}

/* ------------------------------------------------------------------------ *\
**  Уровень процесса / IsLeaf                                               **
\* ------------------------------------------------------------------------ */

/// GetProcLevel возвращает уровень, переданный в конструктор
TEST_F(TaskTest, proc_level_zero)
{
  EXPECT_EQ(0, task->GetProcLevel());
}

/// Task с ненулевым ProcLevel является листом, и уровень сохраняется
TEST_F(TaskTest, leaf_task_keeps_proc_level)
{
  Task leaf(problem, 3);
  EXPECT_TRUE(leaf.IsLeaf());
  EXPECT_EQ(3, leaf.GetProcLevel());
}

/* ------------------------------------------------------------------------ *\
**  Init (переинициализация существующего объекта)                          **
\* ------------------------------------------------------------------------ */

/// Init на объекте, созданном конструктором по умолчанию, инициализирует его
TEST_F(TaskTest, init_makes_default_task_initialized)
{
  Task t; // не инициализирован
  ASSERT_FALSE(t.IsInit());

  t.Init(problem, 0);

  EXPECT_TRUE(t.IsInit());
  EXPECT_EQ(problem, t.getProblem());
  EXPECT_EQ(problem->GetNumberOfFunctions(), t.GetNumOfFunc());
}

/// Init бросает исключение при некорректной размерности
TEST_F(TaskTest, init_throws_on_bad_dimension)
{
  int oldN = parameters.Dimension;
  Task t;

  parameters.Dimension = 0;
  EXPECT_ANY_THROW(t.Init(problem, 0));

  parameters.Dimension = MaxDim + 1;
  EXPECT_ANY_THROW(t.Init(problem, 0));

  parameters.Dimension = oldN;
}

/// Init задаёт уровень процесса
TEST_F(TaskTest, init_sets_proc_level)
{
  Task t;
  t.Init(problem, 2);
  EXPECT_EQ(2, t.GetProcLevel());
  EXPECT_TRUE(t.IsLeaf());
}

/* ------------------------------------------------------------------------ *\
**  Clone / CloneWithNewData                                                **
\* ------------------------------------------------------------------------ */

/// CloneWithNewData эквивалентен Clone (создаёт независимую копию)
TEST_F(TaskTest, clone_with_new_data_creates_copy)
{
  Task* clone = task->CloneWithNewData();
  ASSERT_NE(nullptr, clone);
  ASSERT_NE(task, clone);

  EXPECT_EQ(task->IsInit(), clone->IsInit());
  EXPECT_EQ(task->GetN(), clone->GetN());
  EXPECT_EQ(task->GetNumOfFunc(), clone->GetNumOfFunc());
  EXPECT_EQ(task->getProblem(), clone->getProblem());

  delete clone;
}

/// Клон неинициализированного Task тоже неинициализирован
TEST_F(TaskTest, clone_of_uninitialized_is_uninitialized)
{
  Task empty;
  Task* clone = empty.Clone();
  ASSERT_NE(nullptr, clone);
  EXPECT_FALSE(clone->IsInit());
  delete clone;
}

/// Клон сохраняет уровень процесса (лист остаётся листом)
TEST_F(TaskTest, clone_preserves_leaf_state)
{
  Task leaf(problem, 4);
  Task* clone = leaf.Clone();
  ASSERT_NE(nullptr, clone);
  EXPECT_TRUE(clone->IsLeaf());
  EXPECT_EQ(4, clone->GetProcLevel());
  delete clone;
}

/* ------------------------------------------------------------------------ *\
**  Вычисление функции                                                      **
\* ------------------------------------------------------------------------ */

/// В точке глобального минимума (все 0) значение функции Растригина == 0
TEST_F(TaskTest, calculate_func_at_optimum_is_zero)
{
  double y[MaxDim] = { 0 };
  parameters.FunctionSignMultiplier[0] = 1.0;
  EXPECT_NEAR(0.0, task->CalculateFuncs(y, 0), 1e-9);
}

/// Множитель знака = -1 инвертирует значение функции
TEST_F(TaskTest, calculate_func_sign_multiplier_minus_one)
{
  double y[n] = { 1.0, 1.0, 1.0, 1.0, 1.0 };
  double base = 5.0; // из существующего теста CalculateFuncsAppliesMultiplier

  parameters.FunctionSignMultiplier[0] = -1.0;
  EXPECT_DOUBLE_EQ(-base, task->CalculateFuncs(y, 0));

  parameters.FunctionSignMultiplier[0] = 1.0; // восстановить
}

/// CalculateFuncsInManyPoints: задача распознаётся как IGPUProblem
/// (ProblemFromFunctionPointers наследует IGPUProblem через цепочку интерфейсов),
/// но GPU-перегрузка CalculateFunctionals не реализована -> бросается исключение.
TEST_F(TaskTest, calculate_funcs_in_many_points_throws_when_gpu_overload_absent)
{
  double y[MaxDim] = { 0 };
  double values[1] = { 0.0 };
  EXPECT_ANY_THROW(task->CalculateFuncsInManyPoints(y, 0, 1, values));
}

/* ------------------------------------------------------------------------ *\
**  Дискретные / непрерывные переменные (задача без дискретных)             **
\* ------------------------------------------------------------------------ */

/// Задача без дискретных переменных: число дискретных == 0
TEST_F(TaskTest, discrete_variable_count_zero_for_continuous_problem)
{
  EXPECT_EQ(0, task->GetNumberOfDiscreteVariable());
}

/// Все переменные непрерывные: GetNumberOfContinuousVariable == GetN
TEST_F(TaskTest, continuous_variable_count_equals_n)
{
  EXPECT_EQ(task->GetN(), task->GetNumberOfContinuousVariable());
}

/// Дискретные параметры начинаются с индекса N (их нет)
TEST_F(TaskTest, start_discrete_variable_equals_n_when_no_discrete)
{
  EXPECT_EQ(task->GetN(), task->GetStartDiscreteVariable());
}

/// Для не целочисленной задачи GetNumberOfValues возвращает -1
TEST_F(TaskTest, number_of_values_minus_one_for_non_integer)
{
  EXPECT_EQ(-1, task->GetNumberOfValues(0));
}

/// Для непрерывной задачи (нет дискретных переменных) запрос дискретных значений
/// у переменной 0 возвращает ERROR_DISCRETE_VALUE: индекс 0 не попадает в диапазон
/// дискретных переменных (их диапазон пуст, т.к. NumberOfDiscreteVariable == 0).
TEST_F(TaskTest, all_discrete_values_error_for_non_integer)
{
  double buf[MaxDim] = { 0 };
  EXPECT_EQ(IIntegerProgrammingProblem::ERROR_DISCRETE_VALUE,
    task->GetAllDiscreteValues(0, buf));
}

/// Для не целочисленной задачи IsPermissibleValue возвращает false
TEST_F(TaskTest, is_permissible_value_false_for_non_integer)
{
  EXPECT_FALSE(task->IsPermissibleValue(0.0, 0));
}

/* ------------------------------------------------------------------------ *\
**  Копирование / трансформация точки                                       **
\* ------------------------------------------------------------------------ */

/// TransformPoint копирует координаты один-в-один (Dimension элементов)
TEST_F(TaskTest, transform_point_copies_coordinates)
{
  double src[n] = { 0.1, -0.2, 0.3, -0.4, 0.5 };
  double dst[n] = { 0 };

  task->TransformPoint(dst, src);

  for (int i = 0; i < n; i++)
    EXPECT_DOUBLE_EQ(src[i], dst[i]) << "index " << i;
}

/* ------------------------------------------------------------------------ *\
**  getMin / getMax                                                         **
\* ------------------------------------------------------------------------ */

/// getMin/getMax возвращают NULL и не зависят от инициализации
TEST_F(TaskTest, get_min_max_null_for_uninitialized)
{
  Task empty;
  EXPECT_EQ(nullptr, empty.getMin());
  EXPECT_EQ(nullptr, empty.getMax());
}

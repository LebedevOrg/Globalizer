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
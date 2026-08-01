// ===== FILE START: tests/src/test_method.cpp =====
#include <gtest/gtest.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include "Method.h"
#include "MethodFactory.h"
#include "Common.h"

#include "SearchData.h"
#include "Task.h"
#include "Parameters.h"
#include "Trial.h"
#include "Evolvent.h"
#include "EvolventFactory.h"
#include "Calculation.h"
#include "CalculationFactory.h"

// Задача создаётся напрямую (как в SimpleMain.cpp) — без DLL и адаптеров.
#include "ProblemFromFunctionPointers.h"

#include <string>
#include <vector>
#include <functional>

using namespace std;

/**
 * \brief Фикстура тестов класса Method.
 *
 * \details Ключевое отличие от прежней версии: задача создаётся НАПРЯМУЮ через
 * ProblemFromFunctionPointers (наследник IProblem), точно как в SimpleMain.cpp.
 * Это устраняет:
 *   - загрузку DLL и связанные с ней падения;
 *   - несовместимость vtable (IGlobalOptimizationProblem vs IProblem);
 *   - необходимость в адаптере GlobalizerBenchmarksProblem.
 *
 * parameters — глобальный синглтон; Init вызывается один раз на процесс.
 */
class MethodTest : public ::testing::Test
{
protected:
  IProblem* pProblem;
  Task* pTask;
  SearchData* pData;
  IEvolvent* pEvolvent;
  Calculation* pCalculation;

  static bool sParamsInited;

  /// Задача Растригина как в SimpleMain.cpp (RASTRIGIN).
  static IProblem* CreateRastrigin(int dim)
  {
    return new ProblemFromFunctionPointers(
      dim,                                        // размерность
      std::vector<double>(dim, -2.2),             // нижняя граница
      std::vector<double>(dim, 1.8),              // верхняя граница
      std::vector<std::function<double(const double*)>>(1, [](const double* y)
        {
          const double pi_ = 3.14159265358979323846;
          double sum = 0.0;
          for (int j = 0; j < parameters.Dimension; j++)
            sum += y[j] * y[j] - 10.0 * cos(2.0 * pi_ * y[j]) + 10.0;
          return sum;
        }),
      true,                                       // оптимум определён
      0.0,                                        // значение оптимума
      std::vector<double>(dim, 0.0)               // координаты оптимума
    );
  }

  void SetUp() override
  {
    pProblem = nullptr;
    pTask = nullptr;
    pData = nullptr;
    pEvolvent = nullptr;
    pCalculation = nullptr;

    const int n = 4;

    // Однократная инициализация ГЛОБАЛЬНЫХ параметров (корректный argv).
    if (!sParamsInited)
    {
      static char arg0[] = "tests";
      char* argv[] = { arg0, nullptr };
      int argc = 1;
      parameters.Init(argc, argv, false);
      sParamsInited = true;
    }

    // Валидные параметры перед каждым тестом.
    parameters.Dimension = n;
    parameters.MapType = mpBase;
    parameters.TypeMethod = StandartMethod;
    parameters.TypeCalculation = OMP;
    parameters.Epsilon = 0.01;
    parameters.r = 2.3;
    parameters.rEps = 0.001;
    parameters.rDynamic = 0.0;
    parameters.NumPoints = 1;
    parameters.NumThread = 1;
    parameters.MaxNumOfPoints = 10000;
    parameters.m = 10;
    parameters.LocalRefineSolution = None;
    parameters.LocalTuningType = WithoutLocalTuning;
    parameters.IsCalculationInBorderPoint = false;
    parameters.LocalMix = 0;
    parameters.StopCondition = Accuracy;
    parameters.IsUseStartPoint = false;
    parameters.IsLoadFirstPointFromFile = false;
    parameters.IsSerializeToDashBoard = false;
    parameters.FileSerializer = "";
    parameters.IterPointsSavePath = "";

    // Создаём задачу напрямую (как SimpleMain).
    pProblem = CreateRastrigin(n);
    pProblem->Initialize();

    pTask = new Task(pProblem, 0);
    pData = new SearchData(MaxNumOfFunc, DefaultSearchDataSize);

    pEvolvent = EvolventFactory::CreateEvolvent(pTask->GetN(), parameters.m);
    ASSERT_NE(pEvolvent, nullptr) << "EvolventFactory returned nullptr";

    pCalculation = CalculationFactory::CreateCalculation(*pTask, pEvolvent);
    ASSERT_NE(pCalculation, nullptr) << "CalculationFactory returned nullptr";
  }

  void TearDown() override
  {
    if (pEvolvent) { delete pEvolvent; pEvolvent = nullptr; }
    if (pData) { delete pData;     pData = nullptr; }
    if (pTask) { delete pTask;     pTask = nullptr; }
    if (pProblem) { delete pProblem;  pProblem = nullptr; }
    // pCalculation — синглтон фабрики — не удаляем.
    // Между тестами сбрасываем возможный кэш вычислителя (для чистоты).
    Calculation::leafCalculation = 0;
    Calculation::firstCalculation = 0;
  }

  bool DoIteration(IMethod* method)
  {
    method->CalculateIterationPoints();
    bool isStop = method->CheckStopCondition();
    method->CalculateFunctionals();
    method->EstimateOptimum();
    method->RenewSearchData();
    method->FinalizeIteration();
    return isStop;
  }

  void RunToStop(IMethod* method, int guardLimit = 5000)
  {
    method->FirstIteration();
    bool isStop = false;
    int guard = 0;
    while (!isStop && guard < guardLimit)
    {
      isStop = DoIteration(method);
      guard++;
    }
    ASSERT_LT(guard, guardLimit) << "Method did not stop within guard limit";
  }
};

bool MethodTest::sParamsInited = false;

// ================================================================
// --- Готовность окружения ---
// ================================================================

TEST_F(MethodTest, problem_and_objects_created)
{
  ASSERT_NE(pProblem, nullptr);
  ASSERT_NE(pTask, nullptr);
  ASSERT_NE(pData, nullptr);
  ASSERT_NE(pEvolvent, nullptr);
  ASSERT_NE(pCalculation, nullptr);
  EXPECT_EQ(pTask->GetN(), 4);
  // Rastrigin: 0 ограничений + 1 критерий => 1 функция.
  EXPECT_EQ(pTask->GetNumOfFunc(), 1);
}

// ================================================================
// --- Валидация параметров конструктора Method ---
// ================================================================

TEST_F(MethodTest, throws_when_MaxNumOfPoints_is_not_positive)
{
  parameters.MaxNumOfPoints = 0;
  ASSERT_ANY_THROW(Method m(*pTask, *pData, *pCalculation, *pEvolvent));
}

TEST_F(MethodTest, throws_when_epsilon_is_not_positive)
{
  parameters.Epsilon = 0.0;
  ASSERT_ANY_THROW(Method m(*pTask, *pData, *pCalculation, *pEvolvent));
}

TEST_F(MethodTest, throws_when_r_is_too_low)
{
  parameters.r = 1.0;
  ASSERT_ANY_THROW(Method m(*pTask, *pData, *pCalculation, *pEvolvent));
}

TEST_F(MethodTest, throws_when_reserv_is_negative)
{
  parameters.rEps = -0.001;
  ASSERT_ANY_THROW(Method m(*pTask, *pData, *pCalculation, *pEvolvent));
}

TEST_F(MethodTest, throws_when_reserv_is_too_large)
{
  parameters.rEps = 0.51;
  ASSERT_ANY_THROW(Method m(*pTask, *pData, *pCalculation, *pEvolvent));
}


TEST_F(MethodTest, can_create_with_correct_values)
{
  ASSERT_NO_THROW(Method m(*pTask, *pData, *pCalculation, *pEvolvent));
}

TEST_F(MethodTest, factory_creates_standard_method)
{
  parameters.TypeMethod = StandartMethod;
  IMethod* m = MethodFactory::CreateMethod(*pTask, *pData, *pCalculation, *pEvolvent);
  ASSERT_NE(m, nullptr);
  EXPECT_NE(dynamic_cast<Method*>(m), nullptr);
  delete m;
}

// ================================================================
// --- FirstIteration ---
// ================================================================

TEST_F(MethodTest, first_iteration_sets_iteration_count_to_one)
{
  Method method(*pTask, *pData, *pCalculation, *pEvolvent);
  method.FirstIteration();
  EXPECT_EQ(method.GetIterationCount(), 1);
}

TEST_F(MethodTest, first_iteration_best_trial_not_yet_calculated)
{
  Method method(*pTask, *pData, *pCalculation, *pEvolvent);
  method.FirstIteration();
  Trial* best = method.GetOptimEstimation();
  ASSERT_NE(best, nullptr);
  EXPECT_EQ(best->index, -2);
}

TEST_F(MethodTest, first_iteration_number_of_trials_is_zero)
{
  Method method(*pTask, *pData, *pCalculation, *pEvolvent);
  method.FirstIteration();
  EXPECT_EQ(method.GetNumberOfTrials(), 0);
}

TEST_F(MethodTest, first_iteration_resets_achieved_accuracy)
{
  Method method(*pTask, *pData, *pCalculation, *pEvolvent);
  method.FirstIteration();
  EXPECT_DOUBLE_EQ(method.GetAchievedAccuracy(), 1.0);
}

// ================================================================
// --- FinalizeIteration ---
// ================================================================

TEST_F(MethodTest, finalize_iteration_increments_counter)
{
  Method method(*pTask, *pData, *pCalculation, *pEvolvent);
  method.FirstIteration();
  int count = method.GetIterationCount();
  method.FinalizeIteration();
  EXPECT_EQ(method.GetIterationCount(), count + 1);
}

// ================================================================
// --- Полный цикл / критерий остановки ---
// ================================================================

TEST_F(MethodTest, stops_when_reaches_max_iterations)
{
  parameters.MaxNumOfPoints = 5;
  Method method(*pTask, *pData, *pCalculation, *pEvolvent);
  RunToStop(&method);
  EXPECT_GE(method.GetIterationCount(), 5);
}

TEST_F(MethodTest, number_of_trials_grows_over_iterations)
{
  parameters.MaxNumOfPoints = 50;
  Method method(*pTask, *pData, *pCalculation, *pEvolvent);
  method.FirstIteration();
  int trialsBefore = method.GetNumberOfTrials();

  bool isStop = false;
  int guard = 0;
  while (!isStop && guard < 500)
  {
    isStop = DoIteration(&method);
    guard++;
  }
  EXPECT_GT(method.GetNumberOfTrials(), trialsBefore);
}

TEST_F(MethodTest, function_calculation_count_is_updated)
{
  parameters.MaxNumOfPoints = 30;
  Method method(*pTask, *pData, *pCalculation, *pEvolvent);
  RunToStop(&method);

  std::vector<int> counts = method.GetFunctionCalculationCount();
  ASSERT_FALSE(counts.empty());
  int total = 0;
  for (int c : counts)
  {
    EXPECT_GE(c, 0);
    total += c;
  }
  EXPECT_GT(total, 0);
}

TEST_F(MethodTest, optimum_estimation_is_computed_after_run)
{
  parameters.MaxNumOfPoints = 200;
  Method method(*pTask, *pData, *pCalculation, *pEvolvent);
  RunToStop(&method);

  Trial* best = method.GetOptimEstimation();
  ASSERT_NE(best, nullptr);
  // У Rastrigin индекс целевой функции = 0 (= GetNumOfFunc()-1).
  EXPECT_EQ(best->index, pTask->GetNumOfFunc() - 1);
  EXPECT_LE(method.GetAchievedAccuracy(), 1.0);
}

TEST_F(MethodTest, achieved_accuracy_does_not_increase)
{
  parameters.MaxNumOfPoints = 100;
  Method method(*pTask, *pData, *pCalculation, *pEvolvent);
  method.FirstIteration();
  double acc0 = method.GetAchievedAccuracy();

  bool isStop = false;
  int guard = 0;
  while (!isStop && guard < 500)
  {
    isStop = DoIteration(&method);
    guard++;
  }
  EXPECT_LE(method.GetAchievedAccuracy(), acc0);
}

// ================================================================
// --- Проверка на другой размерности (2D) ---
// ================================================================

TEST_F(MethodTest, works_on_2d_rastrigin)
{
  // Пересобираем окружение под размерность 2.
  delete pTask; delete pData; delete pEvolvent; delete pProblem;

  parameters.Dimension = 2;
  parameters.MaxNumOfPoints = 100;

  pProblem = CreateRastrigin(2);
  pProblem->Initialize();
  pTask = new Task(pProblem, 0);
  pData = new SearchData(MaxNumOfFunc, DefaultSearchDataSize);
  pEvolvent = EvolventFactory::CreateEvolvent(pTask->GetN(), parameters.m);
  pCalculation = CalculationFactory::CreateCalculation(*pTask, pEvolvent);

  ASSERT_EQ(pTask->GetN(), 2);

  Method method(*pTask, *pData, *pCalculation, *pEvolvent);
  RunToStop(&method);

  Trial* best = method.GetOptimEstimation();
  ASSERT_NE(best, nullptr);
  EXPECT_EQ(best->index, 0);
}
// ===== FILE END: tests/src/test_method.cpp =====
// ===== FILE START: tests/src/test_method.cpp =====
#include <gtest/gtest.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include "Method.h"
#include "MethodFactory.h"
#include "Common.h"
#include "test_config.h"

#include "SearchData.h"
#include "Task.h"
#include "Parameters.h"
#include "Trial.h"
#include "Evolvent.h"
#include "EvolventFactory.h"
#include "Calculation.h"
#include "CalculationFactory.h"

// Правильный менеджер и адаптер для задач, экспортирующих IGlobalOptimizationProblem
#include "IGlobalOptimizationProblem.h"
#include "GlobalOptimizationProblemManager.h"
#include "GlobalizerBenchmarksProblem.h"

#include <string>
#include <vector>

using namespace std;

/**
 * \brief Фикстура тестов класса Method.
 *
 * \details Критично: библиотека rastrigin экспортирует объект типа
 * IGlobalOptimizationProblem (create() возвращает RastriginProblem :
 * IGlobalOptimizationProblem). Старый ProblemManager трактует его как IProblem*,
 * из-за чего vtable не совпадает и вызов SetDimension() уходит "не в тот" метод
 * (access violation). Поэтому здесь:
 *   1) грузим задачу через GlobalOptimizationProblemManager
 *      (он знает IGlobalOptimizationProblem);
 *   2) оборачиваем её в GlobalizerBenchmarksProblem — адаптер к IProblem,
 *      именно так делает Solver/HDSolver в боевом коде;
 *   3) только полученный IProblem* передаём в Task/Method.
 *
 * parameters — глобальный синглтон (extern Parameters parameters);
 * Init вызывается один раз на процесс.
 */
class MethodTest : public ::testing::Test
{
protected:
  Task* pTask;
  SearchData* pData;
  IEvolvent* pEvolvent;
  Calculation* pCalculation;

  // Адаптированная задача (IProblem*), создаётся в каждом SetUp.
  IProblem* pProblem;

  // Менеджер и «сырая» задача живут весь процесс — грузим библиотеку один раз.
  static GlobalOptimizationProblemManager* sManager;
  static IGlobalOptimizationProblem* sRawProblem;
  static bool                               sInitialized;
  static bool                               sLoadFailed;

  static void GlobalInitOnce()
  {
    if (sInitialized)
      return;
    sInitialized = true;

    sManager = new GlobalOptimizationProblemManager();

    std::string libPath = std::string(TESTDATA_BIN_PATH) + std::string(LIB_RASTRIGIN);
    if (GlobalOptimizationProblemManager::OK_ != sManager->LoadProblemLibrary(libPath))
    {
      sLoadFailed = true;
      return;
    }

    // Корректный argv (не new char(8)!) и однократная инициализация ГЛОБАЛЬНЫХ параметров.
    static char arg0[] = "tests";
    char* argv[] = { arg0, nullptr };
    int argc = 1;
    parameters.Init(argc, argv, false);

    sRawProblem = sManager->GetProblem();     // IGlobalOptimizationProblem*
    if (sRawProblem == nullptr)
      sLoadFailed = true;
  }

  void SetUp() override
  {
    pTask = nullptr;
    pData = nullptr;
    pEvolvent = nullptr;
    pCalculation = nullptr;
    pProblem = nullptr;

    GlobalInitOnce();
    if (sLoadFailed || sRawProblem == nullptr)
      GTEST_SKIP() << "Problem library not available: " << LIB_RASTRIGIN;

    const int n = 4;

    // Валидные глобальные параметры перед каждым тестом.
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

    // Настраиваем «сырую» задачу через ПРАВИЛЬНЫЙ интерфейс.
    ASSERT_EQ(sRawProblem->SetDimension(n), IGlobalOptimizationProblem::PROBLEM_OK);
    sRawProblem->SetConfigPath(parameters.LibConfigPath);
    ASSERT_EQ(sRawProblem->Initialize(), IGlobalOptimizationProblem::PROBLEM_OK);

    // Оборачиваем в адаптер IProblem — как это делает Solver/HDSolver.
    pProblem = new GlobalizerBenchmarksProblem(sRawProblem);

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
    if (pProblem) { delete pProblem;  pProblem = nullptr; } // адаптер — наш, удаляем
    // sRawProblem / sManager живут до конца процесса — не трогаем.
    // pCalculation — синглтон фабрики — не удаляем.
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

GlobalOptimizationProblemManager* MethodTest::sManager = nullptr;
IGlobalOptimizationProblem* MethodTest::sRawProblem = nullptr;
bool                               MethodTest::sInitialized = false;
bool                               MethodTest::sLoadFailed = false;

// ================================================================
// --- Готовность окружения ---
// ================================================================

TEST_F(MethodTest, problem_loaded_and_objects_created)
{
  ASSERT_NE(pTask, nullptr);
  ASSERT_NE(pData, nullptr);
  ASSERT_NE(pEvolvent, nullptr);
  ASSERT_NE(pCalculation, nullptr);
  EXPECT_EQ(pTask->GetN(), 4);
  EXPECT_GE(pTask->GetNumOfFunc(), 1);
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

TEST_F(MethodTest, throws_when_NumPoints_is_not_positive)
{
  parameters.NumPoints = 0;
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
// ===== FILE END: tests/src/test_method.cpp =====
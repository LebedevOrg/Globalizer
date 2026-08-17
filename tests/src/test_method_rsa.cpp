#include <gtest/gtest.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include "RSAMethod.hpp"          // Method_RSA
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
#include "TrialFactory.h"

// Задача создаётся напрямую (как в SimpleMain.cpp) — без DLL и адаптеров.
#include "ProblemFromFunctionPointers.h"

#include <cstdio>
#include <string>
#include <vector>
#include <functional>

#include "test_reset.h"

using namespace std;

/**
 * \brief Фикстура тестов класса Method_RSA (алгоритм поиска нуля/корня).
 *
 * \details Method_RSA имеет тот же публичный контракт и ту же валидацию
 * параметров в конструкторе, что и Method:
 *   - конструктор (Task&, SearchData&, Calculation&, IEvolvent&);
 *   - проверки MaxNumOfPoints>=1, Epsilon>0, r>1, rEps in [0,0.5];
 *   - одинаковый набор публичных методов итерирования.
 * Создаётся при parameters.TypeMethod == RSAMethod.
 *
 * Как и в test_method.cpp, задача строится напрямую через
 * ProblemFromFunctionPointers (наследник IProblem) — это исключает загрузку DLL
 * и несовместимость vtable. parameters — глобальный синглтон.
 */
class MethodRSATest : public ::testing::Test
{
protected:
  IProblem* pProblem;
  Task* pTask;
  SearchData* pData;
  IEvolvent* pEvolvent;
  Calculation* pCalculation;

  /// Задача Растригина (как RASTRIGIN в SimpleMain.cpp).
  static IProblem* CreateRastrigin(int dim)
  {
    return new ProblemFromFunctionPointers(
      dim,
      std::vector<double>(dim, -2.2),
      std::vector<double>(dim, 1.8),
      std::vector<std::function<double(const double*)>>(1, [](const double* y)
        {
          const double pi_ = 3.14159265358979323846;
          double sum = 0.0;
          for (int j = 0; j < parameters.Dimension; j++)
            sum += y[j] * y[j] - 10.0 * cos(2.0 * pi_ * y[j]) + 10.0;
          return sum;
        }),
      true, 0.0, std::vector<double>(dim, 0.0)
    );
  }

  /// Полностью сбрасывает кэш вычислителей фабрики.
/// ОБЯЗАТЕЛЬНО вызывать перед удалением Task, к которому привязан вычислитель.
  static void ResetCalculationCache()
  {
    if (Calculation::leafCalculation)
    {
      delete Calculation::leafCalculation;
      Calculation::leafCalculation = 0;
    }
    if (Calculation::firstCalculation)
    {
      delete Calculation::firstCalculation;
      Calculation::firstCalculation = 0;
    }
  }

  /// Пересоздаёт Problem/Task/SearchData/Evolvent/Calculation под новую размерность.
  /// Корректно сбрасывает кэш вычислителя ПЕРЕД удалением старого Task.
  void RebuildEnvironment(int dim)
  {
    // 1) Сначала выбросить вычислитель, который держит старый Task.
    ResetCalculationCache();

    // 2) Порядок удаления: сперва данные, затем задача, развёртка, проблема.
    if (pEvolvent) { delete pEvolvent; pEvolvent = nullptr; }
    if (pData) { delete pData;     pData = nullptr; }
    if (pTask) { delete pTask;     pTask = nullptr; }
    if (pProblem) { delete pProblem;  pProblem = nullptr; }

    parameters.Dimension = dim;

    pProblem = CreateRastrigin(dim);
    pProblem->Initialize();

    pTask = new Task(pProblem, 0);
    pData = new SearchData(MaxNumOfFunc, DefaultSearchDataSize);
    pEvolvent = EvolventFactory::CreateEvolvent(pTask->GetN(), parameters.m);

    // leafCalculation == 0 -> фабрика создаст НОВЫЙ вычислитель на новый Task.
    pCalculation = CalculationFactory::CreateCalculation(*pTask, pEvolvent);
  }

  void SetUp() override
  {
    pProblem = nullptr; pTask = nullptr; pData = nullptr;
    pEvolvent = nullptr; pCalculation = nullptr;

    ResetParametersToMethodDefaults(4);
    parameters.TypeMethod = RSAMethod;

    ResetCalculationCache();  // ваш существующий метод (delete + nullptr)

    pProblem = CreateRastrigin(4);
    pProblem->Initialize();
    pTask = new Task(pProblem, 0);
    pData = new SearchData(MaxNumOfFunc, DefaultSearchDataSize);
    pEvolvent = EvolventFactory::CreateEvolvent(pTask->GetN(), parameters.m);
    ASSERT_NE(pEvolvent, nullptr);
    pCalculation = CalculationFactory::CreateCalculation(*pTask, pEvolvent);
    ASSERT_NE(pCalculation, nullptr);
  }

  void TearDown() override
  {
    // Вычислитель держит указатель на pTask — уничтожаем его ПЕРВЫМ.
    ResetCalculationCache();
    pCalculation = nullptr;

    if (pEvolvent) { delete pEvolvent; pEvolvent = nullptr; }
    if (pData) { delete pData;     pData = nullptr; }
    if (pTask) { delete pTask;     pTask = nullptr; }
    if (pProblem) { delete pProblem;  pProblem = nullptr; }
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
    ASSERT_LT(guard, guardLimit) << "Method_RSA did not stop within guard limit";
  }
};


// ================================================================
// --- Готовность окружения ---
// ================================================================

TEST_F(MethodRSATest, problem_and_objects_created)
{
  ASSERT_NE(pProblem, nullptr);
  ASSERT_NE(pTask, nullptr);
  ASSERT_NE(pData, nullptr);
  ASSERT_NE(pEvolvent, nullptr);
  ASSERT_NE(pCalculation, nullptr);
  EXPECT_EQ(pTask->GetN(), 4);
  EXPECT_EQ(pTask->GetNumOfFunc(), 1);
}

// ================================================================
// --- Валидация параметров конструктора Method_RSA ---
// ================================================================

TEST_F(MethodRSATest, throws_when_MaxNumOfPoints_is_not_positive)
{
  parameters.MaxNumOfPoints = 0;
  ASSERT_ANY_THROW(Method_RSA m(*pTask, *pData, *pCalculation, *pEvolvent));
}

TEST_F(MethodRSATest, throws_when_epsilon_is_not_positive)
{
  parameters.Epsilon = 0.0;
  ASSERT_ANY_THROW(Method_RSA m(*pTask, *pData, *pCalculation, *pEvolvent));
}

TEST_F(MethodRSATest, throws_when_r_is_too_low)
{
  parameters.r = 1.0;
  ASSERT_ANY_THROW(Method_RSA m(*pTask, *pData, *pCalculation, *pEvolvent));
}

TEST_F(MethodRSATest, throws_when_reserv_is_negative)
{
  parameters.rEps = -0.001;
  ASSERT_ANY_THROW(Method_RSA m(*pTask, *pData, *pCalculation, *pEvolvent));
}

TEST_F(MethodRSATest, throws_when_reserv_is_too_large)
{
  parameters.rEps = 0.51;
  ASSERT_ANY_THROW(Method_RSA m(*pTask, *pData, *pCalculation, *pEvolvent));
}

TEST_F(MethodRSATest, can_create_with_correct_values)
{
  ASSERT_NO_THROW(Method_RSA m(*pTask, *pData, *pCalculation, *pEvolvent));
}

/**
 * Фабрика создаёт именно Method_RSA при TypeMethod == RSAMethod.
 */
TEST_F(MethodRSATest, factory_creates_rsa_method)
{
  parameters.TypeMethod = RSAMethod;
  IMethod* m = MethodFactory::CreateMethod(*pTask, *pData, *pCalculation, *pEvolvent);
  ASSERT_NE(m, nullptr);
  EXPECT_NE(dynamic_cast<Method_RSA*>(m), nullptr);
  delete m;
}

// ================================================================
// --- FirstIteration ---
// ================================================================

TEST_F(MethodRSATest, first_iteration_sets_iteration_count_to_one)
{
  Method_RSA method(*pTask, *pData, *pCalculation, *pEvolvent);
  method.FirstIteration();
  EXPECT_EQ(method.GetIterationCount(), 1);
}

TEST_F(MethodRSATest, first_iteration_best_trial_not_yet_calculated)
{
  Method_RSA method(*pTask, *pData, *pCalculation, *pEvolvent);
  method.FirstIteration();
  Trial* best = method.GetOptimEstimation();
  ASSERT_NE(best, nullptr);
  EXPECT_EQ(best->index, -2);
}

TEST_F(MethodRSATest, first_iteration_number_of_trials_is_zero)
{
  Method_RSA method(*pTask, *pData, *pCalculation, *pEvolvent);
  method.FirstIteration();
  EXPECT_EQ(method.GetNumberOfTrials(), 0);
}

TEST_F(MethodRSATest, first_iteration_resets_achieved_accuracy)
{
  Method_RSA method(*pTask, *pData, *pCalculation, *pEvolvent);
  method.FirstIteration();
  EXPECT_DOUBLE_EQ(method.GetAchievedAccuracy(), 1.0);
}

// ================================================================
// --- FinalizeIteration ---
// ================================================================

TEST_F(MethodRSATest, finalize_iteration_increments_counter)
{
  Method_RSA method(*pTask, *pData, *pCalculation, *pEvolvent);
  method.FirstIteration();
  int count = method.GetIterationCount();
  method.FinalizeIteration();
  EXPECT_EQ(method.GetIterationCount(), count + 1);
}

// ================================================================
// --- Полный цикл / критерий остановки ---
// ================================================================

TEST_F(MethodRSATest, stops_when_reaches_max_iterations)
{
  parameters.MaxNumOfPoints = 5;
  Method_RSA method(*pTask, *pData, *pCalculation, *pEvolvent);
  RunToStop(&method);
  EXPECT_GE(method.GetIterationCount(), 5);
}

TEST_F(MethodRSATest, number_of_trials_grows_over_iterations)
{
  parameters.MaxNumOfPoints = 50;
  Method_RSA method(*pTask, *pData, *pCalculation, *pEvolvent);
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

TEST_F(MethodRSATest, function_calculation_count_is_updated)
{
  parameters.MaxNumOfPoints = 30;
  Method_RSA method(*pTask, *pData, *pCalculation, *pEvolvent);
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

TEST_F(MethodRSATest, optimum_estimation_is_computed_after_run)
{
  parameters.MaxNumOfPoints = 200;
  Method_RSA method(*pTask, *pData, *pCalculation, *pEvolvent);
  RunToStop(&method);

  Trial* best = method.GetOptimEstimation();
  ASSERT_NE(best, nullptr);
  EXPECT_EQ(best->index, pTask->GetNumOfFunc() - 1);
  EXPECT_LE(method.GetAchievedAccuracy(), 1.0);
}

TEST_F(MethodRSATest, achieved_accuracy_does_not_increase)
{
  parameters.MaxNumOfPoints = 100;
  Method_RSA method(*pTask, *pData, *pCalculation, *pEvolvent);
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
// --- InsertPoints / EstimateOptimum ---
// ================================================================

TEST_F(MethodRSATest, insert_points_updates_optimum_estimation)
{
  Method_RSA method(*pTask, *pData, *pCalculation, *pEvolvent);
  method.FirstIteration();

  Trial* t = TrialFactory::CreateTrial();
  for (int i = 0; i < pTask->GetN(); ++i)
    t->y[i] = 0.0;                 // глобальный минимум Rastrigin
  t->index = 0;
  t->FuncValues[0] = pTask->CalculateFuncs(t->y, 0);
  t->K = 1;

  std::vector<Trial*> pts = { t };
  method.InsertPoints(pts);

  Trial* best = method.GetOptimEstimation();
  ASSERT_NE(best, nullptr);
  EXPECT_EQ(best->index, 0);
  EXPECT_NEAR(best->FuncValues[0], 0.0, 1e-6);
}

TEST_F(MethodRSATest, estimate_optimum_reports_update)
{
  parameters.MaxNumOfPoints = 20;
  Method_RSA method(*pTask, *pData, *pCalculation, *pEvolvent);
  method.FirstIteration();

  method.CalculateIterationPoints();
  method.CheckStopCondition();
  method.CalculateFunctionals();
  bool updated = method.EstimateOptimum();

  EXPECT_TRUE(updated);
}

// ================================================================
// --- Локальный метод: геттеры без запуска ---
// ================================================================

TEST_F(MethodRSATest, local_method_counters_zero_without_local_refine)
{
  parameters.LocalRefineSolution = None;
  parameters.MaxNumOfPoints = 30;

  Method_RSA method(*pTask, *pData, *pCalculation, *pEvolvent);
  RunToStop(&method);

  EXPECT_EQ(method.GetLocalPointCount(), 0);
  EXPECT_EQ(method.GetNumberLocalMethodtStart(), 0);
  EXPECT_NO_THROW(method.LocalSearch());
  EXPECT_EQ(method.GetNumberLocalMethodtStart(), 0);
}

// ================================================================
// --- Печать результатов в файл ---
// ================================================================

TEST_F(MethodRSATest, print_points_writes_file_with_header)
{
  parameters.MaxNumOfPoints = 40;
  Method_RSA method(*pTask, *pData, *pCalculation, *pEvolvent);
  RunToStop(&method);

  const std::string fname = "test_method_rsa_points_out.txt";
  std::remove(fname.c_str());

  method.PrintPoints(fname);

  FILE* pf = std::fopen(fname.c_str(), "r");
  ASSERT_NE(pf, nullptr) << "PrintPoints did not create the file";

  int nPoints = -1, nFuncs = -1;
  int read = std::fscanf(pf, "%d %d", &nPoints, &nFuncs);
  std::fclose(pf);
  std::remove(fname.c_str());

  ASSERT_EQ(read, 2);
  EXPECT_GE(nPoints, 0);
  EXPECT_EQ(nFuncs, pTask->GetNumOfFunc());
}

TEST_F(MethodRSATest, print_level_points_writes_file)
{
  parameters.MaxNumOfPoints = 40;
  Method_RSA method(*pTask, *pData, *pCalculation, *pEvolvent);
  RunToStop(&method);

  const std::string fname = "test_method_rsa_level_points_out.txt";
  std::remove(fname.c_str());

  method.PrintLevelPoints(fname);

  FILE* pf = std::fopen(fname.c_str(), "r");
  ASSERT_NE(pf, nullptr);

  int nPoints = -1, nFuncs = -1;
  int read = std::fscanf(pf, "%d %d", &nPoints, &nFuncs);
  std::fclose(pf);
  std::remove(fname.c_str());

  ASSERT_EQ(read, 2);
  EXPECT_EQ(nFuncs, pTask->GetNumOfFunc());
}

// ================================================================
// --- PrintSection ---
// ================================================================

TEST_F(MethodRSATest, print_section_does_not_crash)
{
  parameters.MaxNumOfPoints = 20;
  Method_RSA method(*pTask, *pData, *pCalculation, *pEvolvent);
  RunToStop(&method);

  parameters.IsPrintSectionPoint = false;
  EXPECT_NO_THROW(method.PrintSection());

  parameters.IsPrintSectionPoint = true;
  EXPECT_NO_THROW(method.PrintSection());
  parameters.IsPrintSectionPoint = false;
}

// ================================================================
// --- Много точек за итерацию (NumPoints > 1) ---
// ================================================================

TEST_F(MethodRSATest, works_with_multiple_points_per_iteration)
{
  parameters.NumPoints = 3;
  parameters.MaxNumOfPoints = 60;

  Method_RSA method(*pTask, *pData, *pCalculation, *pEvolvent);
  RunToStop(&method);

  EXPECT_GT(method.GetNumberOfTrials(), 3);
  Trial* best = method.GetOptimEstimation();
  ASSERT_NE(best, nullptr);
  EXPECT_EQ(best->index, 0);

  parameters.NumPoints = 1;
}

// ================================================================
// --- Другие типы развёрток через фабрику ---
// ================================================================

TEST_F(MethodRSATest, runs_on_linear_evolvent)
{
  delete pEvolvent;
  parameters.MapType = mpLinar;
  pEvolvent = EvolventFactory::CreateEvolvent(pTask->GetN(), parameters.m);
  ASSERT_NE(pEvolvent, nullptr);

  parameters.MaxNumOfPoints = 40;
  Method_RSA method(*pTask, *pData, *pCalculation, *pEvolvent);
  RunToStop(&method);

  EXPECT_GT(method.GetIterationCount(), 1);
  parameters.MapType = mpBase;
}

TEST_F(MethodRSATest, runs_on_shifted_evolvent)
{
  delete pEvolvent;
  parameters.MapType = mpShifted;
  pEvolvent = EvolventFactory::CreateEvolvent(pTask->GetN(), parameters.m);
  ASSERT_NE(pEvolvent, nullptr);

  parameters.MaxNumOfPoints = 40;
  Method_RSA method(*pTask, *pData, *pCalculation, *pEvolvent);
  RunToStop(&method);

  EXPECT_GT(method.GetIterationCount(), 1);
  parameters.MapType = mpBase;
}

// ================================================================
// --- Другая размерность (2D) ---
// ================================================================

TEST_F(MethodRSATest, works_on_2d_rastrigin)
{
  parameters.MaxNumOfPoints = 100;

  // Безопасно пересобираем окружение под размерность 2:
  // сначала сбрасываем кэш вычислителя, потом удаляем старый Task.
  RebuildEnvironment(2);

  ASSERT_EQ(pTask->GetN(), 2);

  Method_RSA method(*pTask, *pData, *pCalculation, *pEvolvent);
  RunToStop(&method);

  Trial* best = method.GetOptimEstimation();
  ASSERT_NE(best, nullptr);
  EXPECT_EQ(best->index, 0);
}

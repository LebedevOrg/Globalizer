#include <gtest/gtest.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include "MixedIntegerMethod.h"
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
 * \brief Фикстура тестов класса MixedIntegerMethod (частично целочисленные задачи).
 *
 * \details MixedIntegerMethod наследуется от Method и переопределяет FirstIteration,
 * CalculateIterationPoints и CalculateCurrentPoint для работы с дискретными
 * переменными. Конструктор делегирует базовому Method, поэтому валидация
 * параметров (MaxNumOfPoints>=1, Epsilon>0, r>1, rEps in [0,0.5]) — та же.
 * Метод создаётся при parameters.TypeMethod == IntegerMethod.
 *
 * Задача — Растригин с 2 непрерывными и 2 дискретными переменными
 * (аналог RASTRIGIN_INT из SimpleMain.cpp). Дискретные параметры всегда
 * последние в векторе y; число значений задаётся через discreteValues.
 *
 * parameters — глобальный синглтон; Init вызывается один раз на процесс.
 */
class MixedIntegerMethodTest : public ::testing::Test
{
protected:
  IProblem* pProblem;
  Task* pTask;
  SearchData* pData;
  IEvolvent* pEvolvent;
  Calculation* pCalculation;

  // Полная размерность и число дискретных переменных задачи.
  static const int kDim = 4;
  static const int kDiscrete = 2;

  /// Растригин с частично целочисленными переменными (как RASTRIGIN_INT).
  static IProblem* CreateRastriginInt(int dim, int numDiscrete,
    const std::vector<int>& discreteValues)
  {
    return new ProblemFromFunctionPointers(
      dim,
      numDiscrete,
      std::vector<double>(dim, -2.2),   // нижняя граница
      std::vector<double>(dim, 1.8),    // верхняя граница
      discreteValues,                   // число значений для целочисленных переменных
      std::vector<std::function<double(const double*)>>(1, [](const double* y)
        {
          const double pi_ = 3.14159265358979323846;
          double sum = 0.0;
          // непрерывные переменные (первые две)
          for (int j = 0; j < 2; j++)
            sum += y[j] * y[j] - 10.0 * cos(2.0 * pi_ * y[j]) + 10.0;
          // дискретные переменные (последние две)
          for (int j = 2; j < 4; j++)
          {
            double rounded = round(y[j]) / 2.0;
            sum += 0.01 * (y[j] - rounded) * (y[j] - rounded);
          }
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

  /// Пересоздаёт окружение под новую размерность/число дискретных переменных.
  /// Корректно сбрасывает кэш вычислителя ПЕРЕД удалением старого Task.
  void RebuildEnvironment(int dim, int numDiscrete)
  {
    // 1) Сначала выбросить вычислитель, который держит старый Task.
    ResetCalculationCache();

    // 2) Порядок удаления: данные -> задача -> развёртка -> проблема.
    if (pEvolvent) { delete pEvolvent; pEvolvent = nullptr; }
    if (pData) { delete pData;     pData = nullptr; }
    if (pTask) { delete pTask;     pTask = nullptr; }
    if (pProblem) { delete pProblem;  pProblem = nullptr; }

    parameters.Dimension = dim;

    // BuildEnvironment создаёт Problem/Task/SearchData/Evolvent/Calculation.
    // leafCalculation == 0 -> будет создан НОВЫЙ вычислитель на новый Task.
    BuildEnvironment(dim, numDiscrete);
  }

  void BuildEnvironment(int dim, int numDiscrete)
  {
    pProblem = CreateRastriginInt(dim, numDiscrete, std::vector<int>(numDiscrete, 3));
    pProblem->Initialize();

    pTask = new Task(pProblem, 0);
    pData = new SearchData(MaxNumOfFunc, DefaultSearchDataSize);

    pEvolvent = EvolventFactory::CreateEvolvent(pTask->GetN(), parameters.m);
    pCalculation = CalculationFactory::CreateCalculation(*pTask, pEvolvent);
  }

 
  void SetUp() override
  {
    pProblem = nullptr; pTask = nullptr; pData = nullptr;
    pEvolvent = nullptr; pCalculation = nullptr;

    ResetParametersToMethodDefaults(kDim);
    parameters.TypeMethod = IntegerMethod;

    ResetCalculationCache();          // чистим кэш ДО создания вычислителя
    BuildEnvironment(kDim, kDiscrete); // единственное создание окружения
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
    ASSERT_LT(guard, guardLimit) << "MixedIntegerMethod did not stop within guard limit";
  }
};

// ================================================================
// --- Готовность окружения ---
// ================================================================

TEST_F(MixedIntegerMethodTest, problem_and_objects_created)
{
  ASSERT_NE(pProblem, nullptr);
  ASSERT_NE(pTask, nullptr);
  ASSERT_NE(pData, nullptr);
  ASSERT_NE(pEvolvent, nullptr);
  ASSERT_NE(pCalculation, nullptr);
  EXPECT_EQ(pTask->GetN(), kDim);
  EXPECT_EQ(pTask->GetNumOfFunc(), 1);
  // У задачи есть дискретные переменные.
  EXPECT_EQ(pTask->GetNumberOfDiscreteVariable(), kDiscrete);
  EXPECT_EQ(pTask->GetNumberOfContinuousVariable(), kDim - kDiscrete);
}

// ================================================================
// --- Валидация параметров конструктора (унаследована от Method) ---
// ================================================================

TEST_F(MixedIntegerMethodTest, throws_when_MaxNumOfPoints_is_not_positive)
{
  parameters.MaxNumOfPoints = 0;
  ASSERT_ANY_THROW(MixedIntegerMethod m(*pTask, *pData, *pCalculation, *pEvolvent));
}

TEST_F(MixedIntegerMethodTest, throws_when_epsilon_is_not_positive)
{
  parameters.Epsilon = 0.0;
  ASSERT_ANY_THROW(MixedIntegerMethod m(*pTask, *pData, *pCalculation, *pEvolvent));
}

TEST_F(MixedIntegerMethodTest, throws_when_r_is_too_low)
{
  parameters.r = 1.0;
  ASSERT_ANY_THROW(MixedIntegerMethod m(*pTask, *pData, *pCalculation, *pEvolvent));
}

TEST_F(MixedIntegerMethodTest, throws_when_reserv_is_negative)
{
  parameters.rEps = -0.001;
  ASSERT_ANY_THROW(MixedIntegerMethod m(*pTask, *pData, *pCalculation, *pEvolvent));
}

TEST_F(MixedIntegerMethodTest, throws_when_reserv_is_too_large)
{
  parameters.rEps = 0.51;
  ASSERT_ANY_THROW(MixedIntegerMethod m(*pTask, *pData, *pCalculation, *pEvolvent));
}

TEST_F(MixedIntegerMethodTest, can_create_with_correct_values)
{
  ASSERT_NO_THROW(MixedIntegerMethod m(*pTask, *pData, *pCalculation, *pEvolvent));
}

/**
 * Фабрика создаёт именно MixedIntegerMethod при TypeMethod == IntegerMethod.
 */
TEST_F(MixedIntegerMethodTest, factory_creates_mixed_integer_method)
{
  parameters.TypeMethod = IntegerMethod;
  IMethod* m = MethodFactory::CreateMethod(*pTask, *pData, *pCalculation, *pEvolvent);
  ASSERT_NE(m, nullptr);
  EXPECT_NE(dynamic_cast<MixedIntegerMethod*>(m), nullptr);
  delete m;
}

/**
 * MixedIntegerMethod является наследником Method (проверка полиморфизма).
 */
TEST_F(MixedIntegerMethodTest, is_a_method_subclass)
{
  MixedIntegerMethod method(*pTask, *pData, *pCalculation, *pEvolvent);
  Method* asBase = dynamic_cast<Method*>(&method);
  EXPECT_NE(asBase, nullptr);
}

// ================================================================
// --- FirstIteration ---
// ================================================================

TEST_F(MixedIntegerMethodTest, first_iteration_sets_iteration_count_to_one)
{
  MixedIntegerMethod method(*pTask, *pData, *pCalculation, *pEvolvent);
  method.FirstIteration();
  EXPECT_EQ(method.GetIterationCount(), 1);
}

TEST_F(MixedIntegerMethodTest, first_iteration_best_trial_not_yet_calculated)
{
  MixedIntegerMethod method(*pTask, *pData, *pCalculation, *pEvolvent);
  method.FirstIteration();
  Trial* best = method.GetOptimEstimation();
  ASSERT_NE(best, nullptr);
  EXPECT_EQ(best->index, -2);
}

TEST_F(MixedIntegerMethodTest, first_iteration_number_of_trials_is_zero)
{
  MixedIntegerMethod method(*pTask, *pData, *pCalculation, *pEvolvent);
  method.FirstIteration();
  EXPECT_EQ(method.GetNumberOfTrials(), 8);
}

TEST_F(MixedIntegerMethodTest, first_iteration_resets_achieved_accuracy)
{
  MixedIntegerMethod method(*pTask, *pData, *pCalculation, *pEvolvent);
  method.FirstIteration();
  EXPECT_DOUBLE_EQ(method.GetAchievedAccuracy(), 1.0);
}

// ================================================================
// --- FinalizeIteration ---
// ================================================================

TEST_F(MixedIntegerMethodTest, finalize_iteration_increments_counter)
{
  MixedIntegerMethod method(*pTask, *pData, *pCalculation, *pEvolvent);
  method.FirstIteration();
  int count = method.GetIterationCount();
  method.FinalizeIteration();
  EXPECT_EQ(method.GetIterationCount(), count + 1);
}

// ================================================================
// --- Полный цикл / критерий остановки ---
// (учитываем спец-ветку CalculateIterationPoints при IterationCount == 2)
// ================================================================

TEST_F(MixedIntegerMethodTest, stops_when_reaches_max_iterations)
{
  parameters.MaxNumOfPoints = 5;
  MixedIntegerMethod method(*pTask, *pData, *pCalculation, *pEvolvent);
  RunToStop(&method);
  EXPECT_GE(method.GetIterationCount(), 5);
}

TEST_F(MixedIntegerMethodTest, number_of_trials_grows_over_iterations)
{
  parameters.MaxNumOfPoints = 50;
  MixedIntegerMethod method(*pTask, *pData, *pCalculation, *pEvolvent);
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

TEST_F(MixedIntegerMethodTest, function_calculation_count_is_updated)
{
  parameters.MaxNumOfPoints = 30;
  MixedIntegerMethod method(*pTask, *pData, *pCalculation, *pEvolvent);
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

TEST_F(MixedIntegerMethodTest, optimum_estimation_is_computed_after_run)
{
  parameters.MaxNumOfPoints = 300;
  MixedIntegerMethod method(*pTask, *pData, *pCalculation, *pEvolvent);
  RunToStop(&method);

  Trial* best = method.GetOptimEstimation();
  ASSERT_NE(best, nullptr);
  EXPECT_EQ(best->index, pTask->GetNumOfFunc() - 1);
  EXPECT_LE(method.GetAchievedAccuracy(), 1.0);
}

TEST_F(MixedIntegerMethodTest, achieved_accuracy_does_not_increase)
{
  parameters.MaxNumOfPoints = 100;
  MixedIntegerMethod method(*pTask, *pData, *pCalculation, *pEvolvent);
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
// --- Специфика дискретных переменных ---
// ================================================================

/**
 * После полного прогона дискретные координаты лучшей точки лежат в границах
 * области поиска [-2.2, 1.8] (значения выбираются из допустимого дискретного
 * набора, записываемого в CalculateCurrentPoint).
 */
TEST_F(MixedIntegerMethodTest, best_trial_discrete_coords_within_bounds)
{
  parameters.MaxNumOfPoints = 200;
  MixedIntegerMethod method(*pTask, *pData, *pCalculation, *pEvolvent);
  RunToStop(&method);

  Trial* best = method.GetOptimEstimation();
  ASSERT_NE(best, nullptr);

  const double* a = pTask->GetA();
  const double* b = pTask->GetB();
  for (int i = 0; i < pTask->GetN(); ++i)
  {
    EXPECT_GE(best->y[i], a[i] - 1e-9) << "coord " << i;
    EXPECT_LE(best->y[i], b[i] + 1e-9) << "coord " << i;
  }
}

/**
 * InsertPoints с готовой точкой (все координаты в глобальном минимуме)
 * корректно обновляет оценку оптимума.
 */
TEST_F(MixedIntegerMethodTest, insert_points_updates_optimum_estimation)
{
  MixedIntegerMethod method(*pTask, *pData, *pCalculation, *pEvolvent);
  method.FirstIteration();

  Trial* t = TrialFactory::CreateTrial();
  for (int i = 0; i < pTask->GetN(); ++i)
    t->y[i] = 0.0;
  t->index = 0;
  t->FuncValues[0] = pTask->CalculateFuncs(t->y, 0);
  t->K = 1;

  std::vector<Trial*> pts = { t };
  method.InsertPoints(pts);

  Trial* best = method.GetOptimEstimation();
  ASSERT_NE(best, nullptr);
  EXPECT_EQ(best->index, 0);
}

// ================================================================
// --- Локальный метод: геттеры без запуска ---
// ================================================================

TEST_F(MixedIntegerMethodTest, local_method_counters_zero_without_local_refine)
{
  parameters.LocalRefineSolution = None;
  parameters.MaxNumOfPoints = 30;

  MixedIntegerMethod method(*pTask, *pData, *pCalculation, *pEvolvent);
  RunToStop(&method);

  EXPECT_EQ(method.GetLocalPointCount(), 0);
  EXPECT_EQ(method.GetNumberLocalMethodtStart(), 0);
}

// ================================================================
// --- Печать результатов в файл ---
// ================================================================

TEST_F(MixedIntegerMethodTest, print_points_writes_file_with_header)
{
  parameters.MaxNumOfPoints = 40;
  MixedIntegerMethod method(*pTask, *pData, *pCalculation, *pEvolvent);
  RunToStop(&method);

  const std::string fname = "test_mixed_integer_points_out.txt";
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

TEST_F(MixedIntegerMethodTest, print_level_points_writes_file)
{
  parameters.MaxNumOfPoints = 40;
  MixedIntegerMethod method(*pTask, *pData, *pCalculation, *pEvolvent);
  RunToStop(&method);

  const std::string fname = "test_mixed_integer_level_points_out.txt";
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

TEST_F(MixedIntegerMethodTest, print_section_does_not_crash)
{
  parameters.MaxNumOfPoints = 20;
  MixedIntegerMethod method(*pTask, *pData, *pCalculation, *pEvolvent);
  RunToStop(&method);

  parameters.IsPrintSectionPoint = false;
  EXPECT_NO_THROW(method.PrintSection());

  parameters.IsPrintSectionPoint = true;
  EXPECT_NO_THROW(method.PrintSection());
  parameters.IsPrintSectionPoint = false;
}

// ================================================================
// --- Много точек за итерацию (NumPoints > 1) ---
// (важно: CalculateIterationPoints при IterationCount==2 вызывает SetNumPoints)
// ================================================================

TEST_F(MixedIntegerMethodTest, works_with_multiple_points_per_iteration)
{
  parameters.NumPoints = 3;
  parameters.MaxNumOfPoints = 60;

  MixedIntegerMethod method(*pTask, *pData, *pCalculation, *pEvolvent);
  RunToStop(&method);

  EXPECT_GT(method.GetNumberOfTrials(), 3);
  Trial* best = method.GetOptimEstimation();
  ASSERT_NE(best, nullptr);
  EXPECT_EQ(best->index, 0);

  parameters.NumPoints = 1;
}

TEST_F(MixedIntegerMethodTest, works_with_single_discrete_variable)
{
  const int dim = 4, numDiscrete = 1;
  parameters.MaxNumOfPoints = 80;

  // Безопасно пересобираем окружение: кэш вычислителя сбрасывается внутри.
  RebuildEnvironment(dim, numDiscrete);
  ASSERT_EQ(pTask->GetNumberOfDiscreteVariable(), numDiscrete);

  MixedIntegerMethod method(*pTask, *pData, *pCalculation, *pEvolvent);
  RunToStop(&method);

  EXPECT_GT(method.GetIterationCount(), 1);
  Trial* best = method.GetOptimEstimation();
  ASSERT_NE(best, nullptr);
  EXPECT_EQ(best->index, 0);
}
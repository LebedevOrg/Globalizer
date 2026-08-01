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

#include "test_reset.h"

// Задача создаётся напрямую (как в SimpleMain.cpp) — без DLL и адаптеров.
#include "ProblemFromFunctionPointers.h"

#include <string>
#include <vector>
#include <functional>


#include <cstdio>     // std::remove, std::fopen
#include "TrialFactory.h"

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
    pProblem = nullptr; pTask = nullptr; pData = nullptr;
    pEvolvent = nullptr; pCalculation = nullptr;

    ResetParametersToMethodDefaults(4);
    parameters.TypeMethod = StandartMethod;

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
    ASSERT_LT(guard, guardLimit) << "Method did not stop within guard limit";
  }

  /// Полностью сбрасывает кэш вычислителей фабрики.
/// ОБЯЗАТЕЛЬНО вызывать перед удалением Task, к которому привязан вычислитель.
  static void ResetCalculationCache()
  {
    // Вычислители создаются фабрикой через new и кэшируются в статике.
    // Их владелец — фабрика; безопасно удалить и обнулить указатели.
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

    // 2) Порядок удаления: сперва то, что ссылается (Method уже разрушен вызывающим),
    //    затем данные, задача, развёртка, проблема.
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
};


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
  parameters.MaxNumOfPoints = 100;

  // Безопасно пересобираем окружение под размерность 2:
  // сначала сбрасываем кэш вычислителя, потом удаляем старый Task.
  RebuildEnvironment(2);

  ASSERT_EQ(pTask->GetN(), 2);

  Method method(*pTask, *pData, *pCalculation, *pEvolvent);
  RunToStop(&method);

  Trial* best = method.GetOptimEstimation();
  ASSERT_NE(best, nullptr);
  EXPECT_EQ(best->index, 0);
}


// ================================================================
// --- InsertPoints / EstimateOptimum / GetNumberOfTrials ---
// ================================================================

/**
 * InsertPoints добавляет готовые испытания: увеличивает счётчики вычислений
 * функций и число интервалов в базе.
 */
TEST_F(MethodTest, insert_points_adds_trials_and_updates_counts)
{
  Method method(*pTask, *pData, *pCalculation, *pEvolvent);
  method.FirstIteration();

  int trialsBefore = method.GetNumberOfTrials();

  // Готовим 3 вычисленные точки внутри области [-2.2, 1.8]^4.
  std::vector<Trial*> pts;
  const double coords[3][4] = {
    { 0.0,  0.0,  0.0,  0.0 },
    { 0.5, -0.5,  0.5, -0.5 },
    {-1.0,  1.0, -1.0,  1.0 }
  };
  for (int k = 0; k < 3; ++k)
  {
    Trial* t = TrialFactory::CreateTrial();
    for (int i = 0; i < pTask->GetN(); ++i)
      t->y[i] = coords[k][i];
    t->index = 0;                                  // целевая функция вычислена
    t->FuncValues[0] = pTask->CalculateFuncs(t->y, 0);
    t->K = 1;
    pts.push_back(t);
  }

  method.InsertPoints(pts);

  // Число интервалов (а значит и испытаний) должно вырасти.
  EXPECT_GT(method.GetNumberOfTrials(), trialsBefore);

  // Счётчик вычислений целевой функции должен увеличиться минимум на 3.
  std::vector<int> counts = method.GetFunctionCalculationCount();
  ASSERT_FALSE(counts.empty());
  EXPECT_GE(counts[0], 3);
}

/**
 * После InsertPoints оценка оптимума становится валидной (index == 0)
 * и её значение конечно.
 */
TEST_F(MethodTest, insert_points_updates_optimum_estimation)
{
  Method method(*pTask, *pData, *pCalculation, *pEvolvent);
  method.FirstIteration();

  Trial* t = TrialFactory::CreateTrial();
  for (int i = 0; i < pTask->GetN(); ++i)
    t->y[i] = 0.0;                 // глобальный минимум Rastrigin -> 0
  t->index = 0;
  t->FuncValues[0] = pTask->CalculateFuncs(t->y, 0);
  t->K = 1;

  std::vector<Trial*> pts = { t };
  method.InsertPoints(pts);

  Trial* best = method.GetOptimEstimation();
  ASSERT_NE(best, nullptr);
  EXPECT_EQ(best->index, 0);
  EXPECT_NEAR(best->FuncValues[0], 0.0, 1e-6); // f(0,..,0) = 0
}

/**
 * EstimateOptimum возвращает true, когда текущая точка улучшает оптимум.
 */
TEST_F(MethodTest, estimate_optimum_reports_update)
{
  parameters.MaxNumOfPoints = 20;
  Method method(*pTask, *pData, *pCalculation, *pEvolvent);
  method.FirstIteration();

  // Первая же полноценная итерация обычно обновляет оптимум с index==-2.
  method.CalculateIterationPoints();
  method.CheckStopCondition();
  method.CalculateFunctionals();
  bool updated = method.EstimateOptimum();

  // После вычисления функционалов оптимум с "невычисленного" точно обновится.
  EXPECT_TRUE(updated);
}

// ================================================================
// --- Критерий остановки: MaxIterWithoutImprovement ---
// ================================================================

/**
 * При StopCondition == MaxIterWithoutImprovement метод останавливается,
 * когда долго нет улучшений (либо по достижению MaxNumOfPoints — как страховка).
 */
TEST_F(MethodTest, stops_by_max_iter_without_improvement)
{
  parameters.StopCondition = MaxIterWithoutImprovement;
  parameters.MaxIterationsWithoutImprovement = 5;
  parameters.MaxNumOfPoints = 1000; // не даём остановиться по числу испытаний слишком рано

  Method method(*pTask, *pData, *pCalculation, *pEvolvent);
  RunToStop(&method, 5000);

  // Метод обязан остановиться в пределах guard-лимита (проверено в RunToStop).
  EXPECT_GT(method.GetIterationCount(), 1);
}

// ================================================================
// --- Печать результатов в файл ---
// ================================================================

/**
 * PrintPoints создаёт файл с корректным заголовком:
 * первая строка = "<число точек> <число функций>".
 */
TEST_F(MethodTest, print_points_writes_file_with_header)
{
  parameters.MaxNumOfPoints = 40;
  Method method(*pTask, *pData, *pCalculation, *pEvolvent);
  RunToStop(&method);

  const std::string fname = "test_method_points_out.txt";
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

/**
 * PrintLevelPoints также создаёт непустой файл с корректным числом функций.
 */
TEST_F(MethodTest, print_level_points_writes_file)
{
  parameters.MaxNumOfPoints = 40;
  Method method(*pTask, *pData, *pCalculation, *pEvolvent);
  RunToStop(&method);

  const std::string fname = "test_method_level_points_out.txt";
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
// --- Локальный метод: геттеры без запуска ---
// ================================================================

/**
 * Без локального уточнения (LocalRefineSolution == None) счётчики
 * локального метода остаются нулевыми, а LocalSearch не приводит к падению.
 */
TEST_F(MethodTest, local_method_counters_zero_without_local_refine)
{
  parameters.LocalRefineSolution = None;
  parameters.MaxNumOfPoints = 30;

  Method method(*pTask, *pData, *pCalculation, *pEvolvent);
  RunToStop(&method);

  EXPECT_EQ(method.GetLocalPointCount(), 0);
  EXPECT_EQ(method.GetNumberLocalMethodtStart(), 0);

  // Явный вызов LocalSearch при None не должен ничего запускать и падать.
  EXPECT_NO_THROW(method.LocalSearch());
  EXPECT_EQ(method.GetNumberLocalMethodtStart(), 0);
}

// ================================================================
// --- PrintSection / SaveCurrentProgress ---
// ================================================================

/**
 * PrintSection не падает ни при выключенной, ни при включённой печати сечений.
 */
TEST_F(MethodTest, print_section_does_not_crash)
{
  parameters.MaxNumOfPoints = 20;
  Method method(*pTask, *pData, *pCalculation, *pEvolvent);
  RunToStop(&method);

  parameters.IsPrintSectionPoint = false;
  EXPECT_NO_THROW(method.PrintSection());

  parameters.IsPrintSectionPoint = true;
  EXPECT_NO_THROW(method.PrintSection());
  parameters.IsPrintSectionPoint = false;
}

/**
 * SaveCurrentProgress с пустым FileSerializer — это ранний выход (no-op).
 */
TEST_F(MethodTest, save_current_progress_noop_when_serializer_empty)
{
  parameters.FileSerializer = "";
  Method method(*pTask, *pData, *pCalculation, *pEvolvent);
  method.FirstIteration();

  // FirstIteration внутри вызывает SaveCurrentProgress — не должно падать,
  // и файл создаваться не должен.
  EXPECT_NO_THROW(method.RenewSearchData());
}

// ================================================================
// --- Много точек за итерацию (NumPoints > 1) ---
// ================================================================

/**
 * Метод корректно работает при NumPoints > 1 (несколько испытаний за итерацию).
 */
TEST_F(MethodTest, works_with_multiple_points_per_iteration)
{
  parameters.NumPoints = 3;
  parameters.MaxNumOfPoints = 60;

  // Пересоздаём объекты, зависящие от NumPoints не требуется — Method читает
  // parameters.NumPoints в конструкторе, поэтому создаём метод уже с NumPoints=3.
  Method method(*pTask, *pData, *pCalculation, *pEvolvent);
  RunToStop(&method);

  EXPECT_GT(method.GetNumberOfTrials(), 3);
  Trial* best = method.GetOptimEstimation();
  ASSERT_NE(best, nullptr);
  EXPECT_EQ(best->index, 0);

  parameters.NumPoints = 1; // восстановление для последующих тестов
}

// ================================================================
// --- Другие типы развёрток через фабрику ---
// ================================================================

/**
 * Метод отрабатывает на линейной развёртке (mpLinar).
 */
TEST_F(MethodTest, runs_on_linear_evolvent)
{
  delete pEvolvent;
  parameters.MapType = mpLinar;
  pEvolvent = EvolventFactory::CreateEvolvent(pTask->GetN(), parameters.m);
  ASSERT_NE(pEvolvent, nullptr);

  parameters.MaxNumOfPoints = 40;
  Method method(*pTask, *pData, *pCalculation, *pEvolvent);
  RunToStop(&method);

  EXPECT_GT(method.GetIterationCount(), 1);
  parameters.MapType = mpBase; // восстановление
}

/**
 * Метод отрабатывает на сдвиговой развёртке (mpShifted).
 */
TEST_F(MethodTest, runs_on_shifted_evolvent)
{
  delete pEvolvent;
  parameters.MapType = mpShifted;
  pEvolvent = EvolventFactory::CreateEvolvent(pTask->GetN(), parameters.m);
  ASSERT_NE(pEvolvent, nullptr);

  parameters.MaxNumOfPoints = 40;
  Method method(*pTask, *pData, *pCalculation, *pEvolvent);
  RunToStop(&method);

  EXPECT_GT(method.GetIterationCount(), 1);
  parameters.MapType = mpBase; // восстановление
}

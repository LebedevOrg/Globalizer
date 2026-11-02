/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//  File:      test_local_method.cpp                                       //
//                                                                         //
//  Purpose:   Модульные тесты локальных методов: LocalMethod (Хук-Дживс)  //
//             и ParallelHookeJeevesMethod.                                 //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include <gtest/gtest.h>

#include <cmath>
#include <memory>

#include "LocalMethod.h"
#include "ParallelHookeJeevesMethod.h"
#include "OMPCalculation.h"
#include "Task.h"
#include "Trial.h"

#include "test_env.h"

using namespace test_env;

class LocalMethodTest : public ::testing::Test
{
protected:
  std::unique_ptr<ProblemFromFunctionPointers> problem;
  std::unique_ptr<Task> task;

  void SetUp() override
  {
    ResetSolverParameters(2);
  }

  void TearDown() override
  {
    task.reset();
    problem.reset();
    ResetSolverParameters(2);
  }

  void Build(ProblemFromFunctionPointers* p)
  {
    problem.reset(p);
    task.reset(new Task(problem.get(), 0));
  }

  /// Стартовая точка с вычисленными функциями.
  Trial StartAt(double y0, double y1)
  {
    Trial t;
    t.y[0] = y0;
    t.y[1] = y1;
    const int last = task->GetNumOfFunc() - 1;
    for (int i = 0; i <= last; i++)
      t.FuncValues[i] = task->CalculateFuncs(t.y, i);
    t.index = last;
    return t;
  }
};

// ================================================================================================
//  LocalMethod
// ================================================================================================

TEST_F(LocalMethodTest, hooke_jeeves_converges_to_minimum)
{
  Build(MakeParaboloid(2));
  Trial start = StartAt(0.8, 0.8);
  LocalMethod lm(task.get(), start);
  lm.SetEps(1e-7);
  lm.SetInitialStep(0.1);
  lm.SetStepMultiplier(2.0);
  lm.SetMaxTrials(10000);

  Trial best = lm.StartOptimization();
  EXPECT_NEAR(best.y[0], 0.3, 1e-4);
  EXPECT_NEAR(best.y[1], -0.2, 1e-4);
  EXPECT_LT(best.FuncValues[0], 1e-7);
  EXPECT_GT(lm.GetTrialsCounter(), 0);
  EXPECT_LE(lm.GetTrialsCounter(), 10000);
  EXPECT_FALSE(lm.GetSearchSequence().empty()) << "improved best point is recorded";
}

TEST_F(LocalMethodTest, max_trials_limits_the_search)
{
  Build(MakeParaboloid(2));
  Trial start = StartAt(0.8, 0.8);
  LocalMethod lm(task.get(), start);
  lm.SetInitialStep(0.01);
  lm.SetMaxTrials(5);
  Trial best = lm.StartOptimization();
  EXPECT_LE(lm.GetTrialsCounter(), 5);
  EXPECT_LE(best.FuncValues[0], start.FuncValues[0]);
}

TEST_F(LocalMethodTest, start_in_minimum_is_not_worsened)
{
  Build(MakeParaboloid(2));
  Trial start = StartAt(0.3, -0.2);
  LocalMethod lm(task.get(), start);
  Trial best = lm.StartOptimization();
  EXPECT_DOUBLE_EQ(best.FuncValues[0], 0.0);
  EXPECT_DOUBLE_EQ(best.y[0], 0.3);
  EXPECT_DOUBLE_EQ(best.y[1], -0.2);
}

TEST_F(LocalMethodTest, logs_all_trial_points)
{
  Build(MakeParaboloid(2));
  Trial start = StartAt(0.8, 0.8);
  LocalMethod lm(task.get(), start, /*logPoints*/ 1);
  lm.StartOptimization();
  // При logPoints == 1 записывается каждое испытание (+ итоговая лучшая точка).
  EXPECT_GE((int)lm.GetSearchSequence().size(), lm.GetTrialsCounter());
}

TEST_F(LocalMethodTest, logs_best_path)
{
  Build(MakeParaboloid(2));
  Trial start = StartAt(0.8, 0.8);
  LocalMethod lm(task.get(), start, /*logPoints*/ 2);
  lm.StartOptimization();
  EXPECT_GT(lm.GetSearchSequence().size(), 1u);
  EXPECT_LT((int)lm.GetSearchSequence().size(), lm.GetTrialsCounter());
}

TEST_F(LocalMethodTest, respects_constraints)
{
  Build(MakeConstrainedProblem());  // y0 >= 0.1, f = y0^2 + y1^2
  Trial start = StartAt(0.6, 0.6);
  LocalMethod lm(task.get(), start);
  lm.SetEps(1e-7);
  lm.SetMaxTrials(10000);
  Trial best = lm.StartOptimization();
  EXPECT_GE(best.y[0], 0.1 - 1e-9);
  EXPECT_NEAR(best.y[0], 0.1, 1e-3);
  EXPECT_NEAR(best.y[1], 0.0, 1e-3);
  EXPECT_NEAR(best.FuncValues[1], 0.01, 1e-3);
}

TEST_F(LocalMethodTest, stays_inside_search_domain)
{
  // Минимум параболоида с центром вне области: (x-2)^2 на [-1,1] -> граница x = 1.
  Build(new ProblemFromFunctionPointers(2, std::vector<double>(2, -1.0), std::vector<double>(2, 1.0),
    std::vector<std::function<double(const double*)>>(1, [](const double* y)
      { return (y[0] - 2.0) * (y[0] - 2.0) + y[1] * y[1]; })));
  Trial start = StartAt(0.0, 0.5);
  LocalMethod lm(task.get(), start);
  lm.SetEps(1e-7);
  Trial best = lm.StartOptimization();
  EXPECT_LE(best.y[0], 1.0);
  EXPECT_NEAR(best.y[0], 1.0, 1e-3);
  EXPECT_NEAR(best.y[1], 0.0, 1e-3);
}

TEST_F(LocalMethodTest, copy_constructor_is_forbidden)
{
  Build(MakeParaboloid(2));
  Trial start = StartAt(0.8, 0.8);
  LocalMethod lm(task.get(), start);
  EXPECT_THROW({ LocalMethod copy(lm); }, Exception);
}

// ================================================================================================
//  ParallelHookeJeevesMethod
// ================================================================================================

class ParallelHookeJeevesTest : public LocalMethodTest,
  public ::testing::WithParamInterface<int> {};

TEST_P(ParallelHookeJeevesTest, converges_with_batch_size)
{
  parameters.NumPoints = GetParam();
  Build(MakeParaboloid(2));
  OMPCalculation calculation(*task);
  Trial start = StartAt(0.8, 0.8);
  ParallelHookeJeevesMethod lm(task.get(), start, calculation);
  lm.SetEps(1e-7);
  lm.SetMaxTrials(20000);
  Trial best = lm.StartOptimization();
  EXPECT_NEAR(best.y[0], 0.3, 1e-3);
  EXPECT_NEAR(best.y[1], -0.2, 1e-3);
  EXPECT_GT(lm.GetTrialsCounter(), 0);
}

INSTANTIATE_TEST_SUITE_P(BatchSizes, ParallelHookeJeevesTest, ::testing::Values(1, 2, 4, 8));

TEST_F(LocalMethodTest, parallel_hooke_jeeves_with_constraints_and_logging)
{
  parameters.NumPoints = 4;
  Build(MakeConstrainedProblem());
  OMPCalculation calculation(*task);
  Trial start = StartAt(0.6, 0.6);
  ParallelHookeJeevesMethod lm(task.get(), start, calculation, /*logPoints*/ 1);
  lm.SetEps(1e-6);
  Trial best = lm.StartOptimization();
  EXPECT_GE(best.y[0], 0.1 - 1e-9);
  EXPECT_NEAR(best.y[0], 0.1, 1e-3);
  EXPECT_NEAR(best.y[1], 0.0, 1e-3);
  EXPECT_NEAR(best.FuncValues[1], 0.01, 1e-3);
  EXPECT_FALSE(lm.GetSearchSequence().empty());
  EXPECT_GE((int)lm.GetSearchSequence().size(), lm.GetTrialsCounter());
}

/// Недопустимые точки (нарушено ограничение) не должны считаться лучшими,
/// даже если значение ограничения меньше значений критерия.
TEST_P(ParallelHookeJeevesTest, infeasible_points_are_never_best)
{
  parameters.NumPoints = GetParam();
  // g = 0.2 - y0 <= 0; f = 10 + (y0 - 0.2)^2 + y1^2 (всегда > 10 > g).
  Build(new ProblemFromFunctionPointers(2, std::vector<double>(2, -1.0), std::vector<double>(2, 1.0),
    [](const double* y, int f)
      { return f == 0 ? 0.2 - y[0] : 10.0 + (y[0] - 0.2) * (y[0] - 0.2) + y[1] * y[1]; },
    2));
  OMPCalculation calculation(*task);
  Trial start = StartAt(0.7, 0.5);
  ParallelHookeJeevesMethod lm(task.get(), start, calculation);
  lm.SetEps(1e-7);
  Trial best = lm.StartOptimization();
  EXPECT_GE(best.y[0], 0.2 - 1e-9);
  EXPECT_NEAR(best.FuncValues[1], 10.0, 1e-4);
}

/// Точка у границы области: часть пробных шагов выходит за область,
/// неполные пакеты вычислителя не должны обращаться к старым испытаниям.
TEST_P(ParallelHookeJeevesTest, stays_inside_domain_near_boundary)
{
  parameters.NumPoints = GetParam();
  Build(new ProblemFromFunctionPointers(2, std::vector<double>(2, -1.0), std::vector<double>(2, 1.0),
    std::vector<std::function<double(const double*)>>(1, [](const double* y)
      { return (y[0] - 2.0) * (y[0] - 2.0) + y[1] * y[1]; })));
  OMPCalculation calculation(*task);
  for (int run = 0; run < 3; run++)  // повторные запуски на одном вычислителе
  {
    Trial start = StartAt(0.95, 0.5);
    ParallelHookeJeevesMethod lm(task.get(), start, calculation, run % 2);
    lm.SetEps(1e-7);
    Trial best = lm.StartOptimization();
    EXPECT_LE(best.y[0], 1.0);
    EXPECT_NEAR(best.y[0], 1.0, 1e-3);
    EXPECT_NEAR(best.y[1], 0.0, 1e-3);
  }
}

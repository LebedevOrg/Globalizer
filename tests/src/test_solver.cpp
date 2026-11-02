/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//  File:      test_solver.cpp                                             //
//                                                                         //
//  Purpose:   Модульные тесты решателя Solver и процесса Process          //
//             (GlobProcess): решение задач в разных режимах вычислителя,  //
//             развёрток, методов, критериев остановки, локального         //
//             уточнения, а также вывод результатов в файлы.               //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include <gtest/gtest.h>

#include <memory>

#include "Solver.h"
#include "Globalizer.h"
#include "SolutionResult.h"
#include "TrialFactory.h"

#include "test_env.h"

using namespace test_env;

// ================================================================================================
//  Фикстура
// ================================================================================================
class SolverTest : public ::testing::Test
{
protected:
  std::unique_ptr<TempWorkDir> workDir;
  IProblem* problem = nullptr;

  void SetUp() override
  {
    workDir.reset(new TempWorkDir());
    ResetSolverParameters(2);
  }

  void TearDown() override
  {
    ResetSolverParameters(2);
    Extended::SetTypeID(etDouble);
    delete problem;
    problem = nullptr;
    workDir.reset();
  }

  /// Решает задачу и возвращает результат (владение — у вызывающего).
  /// При ошибке бросает исключение: gtest засчитывает провал, и тест не
  /// обращается к результату несуществующего процесса.
  std::unique_ptr<SolutionResult> SolveWith(Solver& solver)
  {
    if (solver.Solve() != 0)
      throw std::runtime_error("Solver::Solve() returned an error");
    return std::unique_ptr<SolutionResult>(solver.GetSolutionResult());
  }

  void ExpectNearParaboloidOptimum(const SolutionResult& res, int dim, double tol)
  {
    std::vector<double> c = ParaboloidCenter(dim);
    EXPECT_NEAR(res.GetBestValue(), 0.0, tol * tol * dim);
    for (int i = 0; i < dim; i++)
      EXPECT_NEAR(res.GetBestPoint()[i], c[i], tol) << "coordinate " << i;
  }
};

// ================================================================================================
//  Базовые сценарии
// ================================================================================================

TEST_F(SolverTest, solves_2d_paraboloid)
{
  problem = MakeParaboloid(2);
  Solver solver(problem);
  auto res = SolveWith(solver);

  ExpectNearParaboloidOptimum(*res, 2, 0.05);
  EXPECT_GT(res->GetIterationsCount(), 0);
  EXPECT_GT(res->TrialCount, 0);
  EXPECT_GE(res->SolvingTime, 0.0);
}

TEST_F(SolverTest, solves_1d_paraboloid)
{
  ResetSolverParameters(1);
  problem = MakeParaboloid(1);
  Solver solver(problem);
  auto res = SolveWith(solver);
  ExpectNearParaboloidOptimum(*res, 1, 0.02);
}

TEST_F(SolverTest, accessors_return_internal_objects)
{
  problem = MakeParaboloid(2);
  Solver solver(problem);
  EXPECT_EQ(solver.GetProblem(), problem);
  ASSERT_EQ(solver.Solve(), 0);

  ASSERT_NE(solver.GetTask(), nullptr);
  ASSERT_NE(solver.GetData(), nullptr);
  EXPECT_EQ(solver.GetTask()->GetN(), 2);
  EXPECT_FALSE(solver.GetAllPoint().empty());
  // В списке испытаний есть и граничная точка, не входящая в счётчик интервалов.
  EXPECT_GE((int)solver.GetAllPoint().size(), solver.GetData()->GetCount());

  IProblem* other = MakeParaboloid(2);
  solver.SetProblem(other);
  EXPECT_EQ(solver.GetProblem(), other);
  solver.SetProblem(problem);
  delete other;
}

TEST_F(SolverTest, solve_can_be_called_repeatedly)
{
  problem = MakeParaboloid(2);
  Solver solver(problem);
  auto first = SolveWith(solver);
  auto second = SolveWith(solver);
  EXPECT_NEAR(first->GetBestValue(), second->GetBestValue(), 1e-12);
  EXPECT_EQ(first->GetIterationsCount(), second->GetIterationsCount());
}

TEST_F(SolverTest, stepwise_initialize_and_do_iteration)
{
  problem = MakeParaboloid(2);
  Solver solver(problem);

  bool finished = false;
  EXPECT_EQ(solver.DoIteration(finished), 1) << "DoIteration before Initialize must fail";

  ASSERT_EQ(solver.Initialize(), 0);
  EXPECT_EQ(solver.Initialize(), 0) << "repeated Initialize is a no-op";

  int guard = 0;
  while (!finished && guard < 20000)
  {
    ASSERT_EQ(solver.DoIteration(finished), 0);
    guard++;
  }
  ASSERT_TRUE(finished);

  // После завершения DoIteration сразу сообщает finished.
  bool again = false;
  EXPECT_EQ(solver.DoIteration(again), 0);
  EXPECT_TRUE(again);

  std::unique_ptr<SolutionResult> res(solver.GetSolutionResult());
  ExpectNearParaboloidOptimum(*res, 2, 0.05);
}

TEST_F(SolverTest, print_result_to_console)
{
  problem = MakeParaboloid(2);
  Solver solver(problem);
  solver.PrintResultToConsole();  // без процесса — ничего не делает
  ASSERT_EQ(solver.Solve(), 0);

  ::testing::internal::CaptureStdout();
  solver.PrintResultToConsole();
  parameters.IsUseExtendedConsole = true;
  solver.PrintResultToConsole();
  std::string out = ::testing::internal::GetCapturedStdout();
  EXPECT_FALSE(out.empty());
  EXPECT_FALSE(parameters.IsPrintResultToConsole) << "flag must be restored";
}

TEST_F(SolverTest, set_point_inserts_external_trials)
{
  problem = MakeParaboloid(2);
  Solver solver(problem);

  std::vector<Trial*> points;
  std::vector<double> c = ParaboloidCenter(2);
  Trial* t = TrialFactory::CreateTrial();
  t->y[0] = c[0];
  t->y[1] = c[1];
  t->FuncValues[0] = 0.0;
  t->index = 0;
  points.push_back(t);
  solver.SetPoint(points);

  auto res = SolveWith(solver);
  EXPECT_NEAR(res->GetBestValue(), 0.0, 1e-3);
}

// ================================================================================================
//  Проверка параметров (CheckParameters)
// ================================================================================================

TEST_F(SolverTest, optimum_value_stop_is_rejected_without_known_optimum)
{
  problem = MakeParaboloid(2, /*withOptimum*/ false);
  parameters.StopCondition = OptimumValue;
  ::testing::internal::CaptureStdout();
  Solver solver(problem);
  EXPECT_EQ(solver.Solve(), 1);
  EXPECT_EQ(solver.Initialize(), 1);
  ::testing::internal::GetCapturedStdout();
}

TEST_F(SolverTest, vicinity_stop_falls_back_to_accuracy_without_optimum_point)
{
  problem = MakeParaboloid(2, /*withOptimum*/ false);
  parameters.StopCondition = OptimumVicinity;
  ::testing::internal::CaptureStdout();
  Solver solver(problem);
  auto res = SolveWith(solver);
  ::testing::internal::GetCapturedStdout();
  EXPECT_EQ((int)parameters.StopCondition, (int)Accuracy);
  ExpectNearParaboloidOptimum(*res, 2, 0.05);
}

TEST_F(SolverTest, discrete_problem_switches_to_integer_method)
{
  // y0 — непрерывная, y1 — дискретная с 3 значениями из [-1, 1] (-1, 0, 1).
  problem = new ProblemFromFunctionPointers(2, 1,
    std::vector<double>(2, -1.0), std::vector<double>(2, 1.0), std::vector<int>{3},
    std::vector<std::function<double(const double*)>>(1, [](const double* y)
      { return (y[0] - 0.3) * (y[0] - 0.3) + (y[1] - 1.0) * (y[1] - 1.0); }));
  parameters.TypeMethod = StandartMethod;
  Solver solver(problem);
  auto res = SolveWith(solver);
  EXPECT_EQ((int)parameters.TypeMethod, (int)IntegerMethod);
  EXPECT_NEAR(res->GetBestPoint()[0], 0.3, 0.05);
  EXPECT_NEAR(res->GetBestPoint()[1], 1.0, 1e-9);
}

struct MixedIntegerConfig
{
  const char* name;
  int numPoints;
  bool constrained;
  ELocalMethodScheme localRefine;
};

class SolverMixedIntegerTest : public SolverTest,
  public ::testing::WithParamInterface<MixedIntegerConfig> {};

/**
 y0 — непрерывная на [-1, 1]; y1, y2 — дискретные, по 3 значения (-1, 0, 1).
 f = (y0 - 0.3)^2 + (y1 - 1)^2 + (y2 + 1)^2, минимум 0 в (0.3, 1, -1).
 С ограничением g = 0.5 - y0 <= 0 минимум 0.04 в (0.5, 1, -1).
*/
TEST_P(SolverMixedIntegerTest, solves_mixed_integer_problem)
{
  const MixedIntegerConfig& c = GetParam();
  ResetSolverParameters(3);
  parameters.NumPoints = c.numPoints;
  parameters.LocalRefineSolution = c.localRefine;
  parameters.IterationsCount = 5000;
  parameters.MaxNumOfPoints = 5000;

  std::vector<std::function<double(const double*)>> funcs;
  if (c.constrained)
    funcs.push_back([](const double* y) { return 0.5 - y[0]; });
  funcs.push_back([](const double* y)
    { return (y[0] - 0.3) * (y[0] - 0.3) + (y[1] - 1.0) * (y[1] - 1.0) + (y[2] + 1.0) * (y[2] + 1.0); });

  problem = new ProblemFromFunctionPointers(3, 2,
    std::vector<double>(3, -1.0), std::vector<double>(3, 1.0), std::vector<int>{3, 3}, funcs);
  Solver solver(problem);
  auto res = SolveWith(solver);

  EXPECT_EQ((int)parameters.TypeMethod, (int)IntegerMethod);
  EXPECT_NEAR(res->GetBestPoint()[1], 1.0, 1e-9);
  EXPECT_NEAR(res->GetBestPoint()[2], -1.0, 1e-9);
  const double x0 = c.constrained ? 0.5 : 0.3;
  EXPECT_NEAR(res->GetBestPoint()[0], x0, 0.05);
  EXPECT_NEAR(res->GetBestValue(), c.constrained ? 0.04 : 0.0, 0.01);
}

INSTANTIATE_TEST_SUITE_P(MixedInteger, SolverMixedIntegerTest, ::testing::Values(
  MixedIntegerConfig{ "basic",        1, false, None },
  MixedIntegerConfig{ "parallel3",    3, false, None },
  MixedIntegerConfig{ "constrained",  1, true,  None },
  MixedIntegerConfig{ "parConstr",    2, true,  None },
  MixedIntegerConfig{ "finalLocal",   1, false, FinalStart }),
  [](const ::testing::TestParamInfo<MixedIntegerConfig>& info) { return std::string(info.param.name); });

// ================================================================================================
//  Критерии остановки
// ================================================================================================

class SolverStopConditionTest : public SolverTest,
  public ::testing::WithParamInterface<EStopCondition> {};

TEST_P(SolverStopConditionTest, solves_with_stop_condition)
{
  parameters.StopCondition = GetParam();
  parameters.MaxIterationsWithoutImprovement = 300;
  parameters.IterationsCount = 3000;
  parameters.MaxNumOfPoints = 3000;
  problem = MakeParaboloid(2);
  Solver solver(problem);
  auto res = SolveWith(solver);
  EXPECT_LE(res->GetIterationsCount(), 3000 + 1);
  EXPECT_LT(res->GetBestValue(), 0.05);
}

INSTANTIATE_TEST_SUITE_P(StopConditions, SolverStopConditionTest,
  ::testing::Values(Accuracy, OptimumVicinity, OptimumVicinity2, OptimumValue,
                    MaxIterWithoutImprovement, IterationOnly));

TEST_F(SolverTest, iteration_only_stops_exactly_at_iteration_limit)
{
  parameters.StopCondition = IterationOnly;
  parameters.IterationsCount = 50;
  parameters.MaxNumOfPoints = 50;
  problem = MakeParaboloid(2);
  Solver solver(problem);
  auto res = SolveWith(solver);
  EXPECT_LE(res->GetIterationsCount(), 51);
  EXPECT_GE(res->GetIterationsCount(), 49);
}

// ================================================================================================
//  Типы развёрток
// ================================================================================================

class SolverMapTypeTest : public SolverTest,
  public ::testing::WithParamInterface<EMapType> {};

TEST_P(SolverMapTypeTest, solves_with_map_type)
{
  parameters.MapType = GetParam();
  parameters.IterationsCount = 5000;
  parameters.MaxNumOfPoints = 5000;
  problem = MakeParaboloid(2);
  Solver solver(problem);
  auto res = SolveWith(solver);
  EXPECT_LT(res->GetBestValue(), 0.05);
}

INSTANTIATE_TEST_SUITE_P(MapTypes, SolverMapTypeTest,
  ::testing::Values(mpBase, mpLinar, mpNoninjective, mpRotated, mpShifted, mpSmooth));

TEST_F(SolverTest, solves_1d_with_rotated_map)
{
  ResetSolverParameters(1);
  parameters.MapType = mpRotated;
  problem = MakeParaboloid(1);
  Solver solver(problem);
  auto res = SolveWith(solver);
  ExpectNearParaboloidOptimum(*res, 1, 0.02);
}

// ================================================================================================
//  Вычислители и параллельные точки
// ================================================================================================

TEST_F(SolverTest, omp_with_several_points_per_iteration)
{
  parameters.NumPoints = 4;
  parameters.NumThread = 2;
  problem = MakeParaboloid(2);
  Solver solver(problem);
  auto res = SolveWith(solver);
  ExpectNearParaboloidOptimum(*res, 2, 0.05);
}

TEST_F(SolverTest, cpu_multi_point_calculation)
{
  parameters.TypeCalculation = CPUMultiPoint;
  parameters.NumPoints = 4;
  problem = MakeMultiPointParaboloid(2);
  Solver solver(problem);
  auto res = SolveWith(solver);
  ExpectNearParaboloidOptimum(*res, 2, 0.05);
}

TEST_F(SolverTest, auto_config_with_automatic_parameters)
{
  ResetSolverParameters(4);
  parameters.AutomaticParametersSetting = true;
  parameters.MaxNumOfPoints = 2000;
  parameters.IterationsCount = 2000;
  problem = MakeParaboloid(4);
  Solver solver(problem);
  EXPECT_GE((int)parameters.NumThread, 1);
  EXPECT_EQ((int)parameters.NumPoints, (int)parameters.NumThread);
  auto res = SolveWith(solver);
  EXPECT_LT(res->GetBestValue(), 0.5);
}

// ================================================================================================
//  Задачи с ограничениями и разные схемы метода
// ================================================================================================

TEST_F(SolverTest, solves_constrained_problem)
{
  parameters.IterationsCount = 5000;
  parameters.MaxNumOfPoints = 5000;
  problem = MakeConstrainedProblem();
  Solver solver(problem);
  auto res = SolveWith(solver);
  ASSERT_EQ(res->BestTrial->index, 1) << "best trial must be feasible";
  EXPECT_GE(res->GetBestPoint()[0], 0.1 - 1e-9);
  EXPECT_NEAR(res->GetBestValue(), 0.01, 0.01);
}

class SolverLocalTuningTest : public SolverTest,
  public ::testing::WithParamInterface<ELocalTuningType> {};

TEST_P(SolverLocalTuningTest, solves_with_local_tuning)
{
  parameters.LocalTuningType = GetParam();
  parameters.IterationsCount = 5000;
  parameters.MaxNumOfPoints = 5000;
  problem = MakeConstrainedProblem();
  Solver solver(problem);
  auto res = SolveWith(solver);
  EXPECT_EQ(res->BestTrial->index, 1);
  EXPECT_LT(res->GetBestValue(), 0.05);
}

INSTANTIATE_TEST_SUITE_P(LocalTuning, SolverLocalTuningTest,
  ::testing::Values(WithoutLocalTuning, MiniMax, Adaptive, AdaptiveMiniMax));

TEST_F(SolverTest, solves_with_local_mix)
{
  parameters.LocalMix = 3;
  problem = MakeParaboloid(2);
  Solver solver(problem);
  auto res = SolveWith(solver);
  ExpectNearParaboloidOptimum(*res, 2, 0.05);
}

TEST_F(SolverTest, solves_with_calculation_in_border_points)
{
  parameters.IsCalculationInBorderPoint = true;
  problem = MakeParaboloid(2);
  Solver solver(problem);
  auto res = SolveWith(solver);
  ExpectNearParaboloidOptimum(*res, 2, 0.05);
}

TEST_F(SolverTest, rsa_method)
{
  parameters.TypeMethod = RSAMethod;
  problem = MakeParaboloid(2);
  Solver solver(problem);
  auto res = SolveWith(solver);
  EXPECT_LT(res->GetBestValue(), 0.05);
}

TEST_F(SolverTest, mpfr_extended_precision)
{
#ifdef NO_MPREAL
  GTEST_SKIP() << "built without MPFR";
#endif
  // m*N = 60 > 50: InitAutoPrecision не переключает тип обратно на double.
  ResetSolverParameters(3);
  parameters.m = 20;
  parameters.Epsilon = 0.05;
  Extended::SetTypeID(etMPFR);
  problem = MakeParaboloid(3);
  Solver solver(problem);
  auto res = SolveWith(solver);
  EXPECT_EQ(Extended::GetTypeID(), etMPFR);
  EXPECT_LT(res->GetBestValue(), 0.05);
}

// ================================================================================================
//  Локальное уточнение решения
// ================================================================================================

class SolverLocalRefineTest : public SolverTest,
  public ::testing::WithParamInterface<ETypeLocalMethod> {};

TEST_P(SolverLocalRefineTest, final_local_refinement_improves_solution)
{
  parameters.Epsilon = 0.05;
  problem = MakeParaboloid(2);

  Solver coarse(problem);
  auto coarseRes = SolveWith(coarse);

  parameters.LocalRefineSolution = FinalStart;
  parameters.TypeLocalMethod = GetParam();
  parameters.LocalVerificationEpsilon = 1e-6;
  Solver refined(problem);
  auto refinedRes = SolveWith(refined);

  EXPECT_LE(refinedRes->GetBestValue(), coarseRes->GetBestValue() + 1e-12);
  ExpectNearParaboloidOptimum(*refinedRes, 2, 1e-2);
}

INSTANTIATE_TEST_SUITE_P(LocalMethods, SolverLocalRefineTest,
  ::testing::Values(HookeJeeves, ParallelHookeJeeves));

TEST_F(SolverTest, local_refinement_on_updated_minimum)
{
  parameters.LocalRefineSolution = UpdatedMinimum;
  problem = MakeParaboloid(2);
  Solver solver(problem);
  auto res = SolveWith(solver);
  ExpectNearParaboloidOptimum(*res, 2, 0.05);
}

// ================================================================================================
//  Вывод результатов
// ================================================================================================

TEST_F(SolverTest, writes_optim_and_result_log_files)
{
  parameters.IsPrintFile = true;
  parameters.ResulLog = "result_log.txt";
  parameters.IsPrintResultToConsole = true;
  parameters.StepPrintMessages = 10;
  problem = MakeParaboloid(2);

  ::testing::internal::CaptureStdout();
  Solver solver(problem);
  auto res = SolveWith(solver);
  std::string out = ::testing::internal::GetCapturedStdout();

  EXPECT_TRUE(workDir->Exists("optim.dat"));
  EXPECT_NE(workDir->Read("optim.dat").find("min ="), std::string::npos);
  EXPECT_TRUE(workDir->Exists("result_log.txt"));
  EXPECT_NE(workDir->Read("result_log.txt").find("Iteration ="), std::string::npos);
  EXPECT_NE(out.find("iteration"), std::string::npos);
}

TEST_F(SolverTest, extended_console_output)
{
  parameters.IsPrintResultToConsole = true;
  parameters.IsUseExtendedConsole = true;
  problem = MakeParaboloid(2);
  ::testing::internal::CaptureStdout();
  Solver solver(problem);
  auto res = SolveWith(solver);
  std::string out = ::testing::internal::GetCapturedStdout();
  EXPECT_FALSE(out.empty());
}

TEST_F(SolverTest, saves_iteration_points)
{
  parameters.IterPointsSavePath = "points.txt";
  parameters.StepSavePoint = 20;
  problem = MakeParaboloid(2);
  Solver solver(problem);
  auto res = SolveWith(solver);
  EXPECT_TRUE(workDir->Exists("points.txt"));
  EXPECT_TRUE(workDir->Exists("20_points.txt"));
}

TEST_F(SolverTest, serializes_to_dashboard)
{
  parameters.IsSerializeToDashBoard = true;
  parameters.FileSerializer = "state.json";
  problem = MakeParaboloid(2);
  Solver solver(problem);
  auto res = SolveWith(solver);
  ASSERT_TRUE(workDir->Exists("SerializeToDashBoard_state.json"));
  std::string json = workDir->Read("SerializeToDashBoard_state.json");
  EXPECT_NE(json.find("\"SearchDataItem\""), std::string::npos);
  EXPECT_NE(json.find("\"best_trials\""), std::string::npos);
  parameters.IsSerializeToDashBoard = false;
}

TEST_F(SolverTest, plot_without_python_plotter_prints_warning)
{
#ifndef USE_PYTHON
  parameters.IsPlot = true;
  problem = MakeParaboloid(2);
  ::testing::internal::CaptureStdout();
  Solver solver(problem);
  auto res = SolveWith(solver);
  std::string out = ::testing::internal::GetCapturedStdout();
  EXPECT_NE(out.find("WITHOUT rendering"), std::string::npos);
  EXPECT_EQ(parameters.IterPointsSavePath.ToString(), "Globalizer_IterPointsSavePath.txt");
  parameters.IterPointsSavePath = "";
#else
  GTEST_SKIP() << "built with the Python plotter";
#endif
}

// ================================================================================================
//  Функция-обёртка GlobalizerSolveProblem
// ================================================================================================

TEST_F(SolverTest, globalizer_solve_problem)
{
  problem = MakeParaboloid(2);
  std::unique_ptr<SolutionResult> res(GlobalizerSolveProblem(problem));
  ASSERT_NE(res, nullptr);
  ExpectNearParaboloidOptimum(*res, 2, 0.05);
  EXPECT_EQ((int)parameters.MaxNumOfPoints, (int)parameters.IterationsCount);
}

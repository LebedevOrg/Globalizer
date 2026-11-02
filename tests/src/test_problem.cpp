/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//  File:      test_problem.cpp                                            //
//                                                                         //
//  Purpose:   Модульные тесты задач: ProblemFromFunctionPointers,         //
//             базовый класс BaseProblem/Problem, адаптер                  //
//             GlobalizerBenchmarksProblem, загрузчики ProblemManager и    //
//             InitProblem*.                                                //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>

#include "Solver.h"
#include "ProblemManager.h"
#include "InitProblem.h"
#include "GlobalizerBenchmarksProblem.h"

#include "test_env.h"

using namespace test_env;

// ================================================================================================
//  ProblemFromFunctionPointers и BaseProblem
// ================================================================================================

class ProblemFromFunctionPointersTest : public ::testing::Test
{
protected:
  void SetUp() override { ResetSolverParameters(2); }
  void TearDown() override { ResetSolverParameters(2); }
};

TEST_F(ProblemFromFunctionPointersTest, vector_of_functions_constructor)
{
  std::unique_ptr<ProblemFromFunctionPointers> p(MakeParaboloid(2));
  EXPECT_EQ(p->GetDimension(), 2);
  EXPECT_EQ(p->GetNumberOfFunctions(), 1);
  EXPECT_EQ(p->GetNumberOfConstraints(), 0);
  EXPECT_EQ(p->GetNumberOfCriterions(), 1);
  EXPECT_EQ(p->GetNumberOfDiscreteVariable(), 0);

  double lower[2], upper[2];
  p->GetBounds(lower, upper);
  EXPECT_DOUBLE_EQ(lower[0], -1.0);
  EXPECT_DOUBLE_EQ(upper[1], 1.0);

  double y[2] = { 0.3, -0.2 };
  EXPECT_DOUBLE_EQ(p->CalculateFunctionals(y, 0), 0.0);
  EXPECT_THROW(p->CalculateFunctionals(y, 5), Exception);
}

TEST_F(ProblemFromFunctionPointersTest, optimum_reporting)
{
  std::unique_ptr<ProblemFromFunctionPointers> with(MakeParaboloid(2, true));
  double value = -1;
  double point[2];
  EXPECT_EQ(with->GetOptimumValue(value), (int)IProblem::OK);
  EXPECT_DOUBLE_EQ(value, 0.0);
  EXPECT_EQ(with->GetOptimumPoint(point), (int)IProblem::OK);
  EXPECT_DOUBLE_EQ(point[0], 0.3);
  EXPECT_DOUBLE_EQ(point[1], -0.2);
  // Перегрузка с индексом скрыта в наследнике — вызываем через базовый класс.
  auto& base = static_cast<BaseProblem<ProblemFromFunctionPointers>&>(*with);
  EXPECT_EQ(base.GetOptimumValue(value, 0), (int)IProblem::OK)
    << "index == number of constraints selects the criterion";
  EXPECT_EQ(base.GetOptimumValue(value, 1), (int)IProblem::UNDEFINED);
  EXPECT_EQ(with->GetConstraintOptimumPoint(point, 0), (int)IProblem::UNDEFINED);

  std::unique_ptr<ProblemFromFunctionPointers> without(MakeParaboloid(2, false));
  EXPECT_EQ(without->GetOptimumValue(value), (int)IProblem::UNDEFINED);
  EXPECT_EQ(without->GetOptimumPoint(point), (int)IProblem::UNDEFINED);
}

TEST_F(ProblemFromFunctionPointersTest, indexed_function_constructor_with_constraints)
{
  std::unique_ptr<ProblemFromFunctionPointers> p(MakeConstrainedProblem());
  EXPECT_EQ(p->GetNumberOfFunctions(), 2);
  EXPECT_EQ(p->GetNumberOfConstraints(), 1);
  double y[2] = { 0.5, 0.5 };
  EXPECT_DOUBLE_EQ(p->CalculateFunctionals(y, 0), -0.4);
  EXPECT_DOUBLE_EQ(p->CalculateFunctionals(y, 1), 0.5);
}

TEST_F(ProblemFromFunctionPointersTest, many_points_constructor)
{
  std::unique_ptr<ProblemFromFunctionPointers> p(MakeMultiPointParaboloid(2));
  double y[6] = { 0.3, -0.2, 0.0, 0.0, 1.0, 1.0 };
  double values[3] = { -1, -1, -1 };
  int n = 3;
  p->CalculateFunctionals(y, 0, n, values);
  EXPECT_DOUBLE_EQ(values[0], 0.0);
  EXPECT_DOUBLE_EQ(values[1], 0.09 + 0.04);
  EXPECT_DOUBLE_EQ(values[2], 0.49 + 1.44);
  EXPECT_DOUBLE_EQ(p->CalculateFunctionals(y, 0), 0.0);
}

TEST_F(ProblemFromFunctionPointersTest, base_problem_helpers)
{
  std::unique_ptr<ProblemFromFunctionPointers> p(MakeParaboloid(2));
  EXPECT_FALSE(p->ProblemName().empty());
  EXPECT_EQ(p->Initialize(), (int)IProblem::OK);
  p->Init(0, nullptr, false);
  EXPECT_EQ(p->SetDimension(3), (int)IProblem::OK);
  EXPECT_NE(p->SetDimension(1000), (int)IProblem::OK);
  EXPECT_EQ(p->GetNumberOfValues(0), -1) << "continuous variable has no values";
}

TEST_F(ProblemFromFunctionPointersTest, discrete_variables)
{
  // 2 переменные: y0 — непрерывная, y1 — дискретная с 3 значениями (-1, 0, 1).
  std::unique_ptr<ProblemFromFunctionPointers> p(new ProblemFromFunctionPointers(2, 1,
    std::vector<double>(2, -1.0), std::vector<double>(2, 1.0), std::vector<int>{3},
    std::vector<std::function<double(const double*)>>(1, [](const double* y) { return y[0] + y[1]; }),
    true, -2.0, std::vector<double>{-1.0, -1.0}));

  EXPECT_EQ(p->GetNumberOfDiscreteVariable(), 1);
  EXPECT_EQ(p->GetNumberOfValues(1), 3);
  EXPECT_EQ(p->GetNumberOfValues(0), -1);

  double values[3];
  ASSERT_EQ(p->GetAllDiscreteValues(1, values), (int)IProblem::OK);
  EXPECT_DOUBLE_EQ(values[0], -1.0);
  EXPECT_DOUBLE_EQ(values[1], 0.0);
  EXPECT_DOUBLE_EQ(values[2], 1.0);
  EXPECT_EQ(p->GetAllDiscreteValues(0, values), (int)IIntegerProgrammingProblem::ERROR_DISCRETE_VALUE);

  int* index = nullptr;
  double v = 0;
  EXPECT_EQ(p->GetNextDiscreteValues(index, v, 1), (int)IIntegerProgrammingProblem::ERROR_DISCRETE_VALUE)
    << "index array is not allocated yet";
  // ClearCurrentDiscreteValueIndex — protected; используем через GetAllDiscreteValues выше,
  // а здесь создаём массив вручную.
  index = new int[1]{ 0 };
  EXPECT_EQ(p->GetNextDiscreteValues(index, v, 1, -1), (int)IProblem::OK);
  EXPECT_DOUBLE_EQ(v, -1.0);
  EXPECT_EQ(p->GetNextDiscreteValues(index, v, 1), (int)IProblem::OK);
  EXPECT_DOUBLE_EQ(v, 0.0);
  EXPECT_EQ(p->GetNextDiscreteValues(index, v, 1, 1), (int)IProblem::OK);
  EXPECT_DOUBLE_EQ(v, 1.0);
  delete[] index;

  EXPECT_TRUE(p->IsPermissibleValue(0.0, 1));
  EXPECT_TRUE(p->IsPermissibleValue(1.0, 1));
  EXPECT_FALSE(p->IsPermissibleValue(0.5, 1));
  EXPECT_FALSE(p->IsPermissibleValue(0.0, 0));
}

// ================================================================================================
//  Адаптер GlobalizerBenchmarksProblem
// ================================================================================================

#ifdef _GLOBALIZER_BENCHMARKS

/**
 Тестовая задача в интерфейсе IGlobalOptimizationProblem:
 y — непрерывная на [-1, 1], u — дискретная из {"a", "b", "c"}.
 f(y, u) = (y - 0.25)^2 + (u == "b" ? 0 : 1). Минимум 0 в (0.25, "b").
*/
class FakeMixedProblem : public IGlobalOptimizationProblem
{
public:
  int dimension = 2;
  int initialized = 0;
  std::string config;

  int SetDimension(int d) override
  {
    if (d != 2) return PROBLEM_ERROR;
    dimension = d;
    return PROBLEM_OK;
  }
  int GetDimension() const override { return dimension; }
  int Initialize() override { initialized++; return PROBLEM_OK; }
  void GetBounds(std::vector<double>& lower, std::vector<double>& upper) override
  {
    lower.assign(1, -1.0);
    upper.assign(1, 1.0);
  }
  int GetOptimumValue(double& value) const override { value = 0.0; return PROBLEM_OK; }
  int GetOptimumPoint(std::vector<double>& y, std::vector<std::string>& u) const override
  {
    y.assign(1, 0.25);
    u.assign(1, "b");
    return PROBLEM_OK;
  }
  int GetNumberOfFunctions() const override { return 1; }
  int GetNumberOfConstraints() const override { return 0; }
  int GetNumberOfCriterions() const override { return 1; }
  int GetNumberOfDiscreteVariable() const override { return 1; }
  int GetDiscreteVariableValues(std::vector<std::vector<std::string>>& values) const override
  {
    values.assign(1, std::vector<std::string>{ "a", "b", "c" });
    return PROBLEM_OK;
  }
  int SetConfigPath(const std::string& path) override { config = path; return PROBLEM_OK; }
  double CalculateFunctionals(const std::vector<double>& y, std::vector<std::string>& u, int) override
  {
    return (y[0] - 0.25) * (y[0] - 0.25) + (u[0] == "b" ? 0.0 : 1.0);
  }
};

/// Непрерывная 2D задача в интерфейсе IGlobalOptimizationProblem (для Solver).
class FakeContinuousProblem : public IGlobalOptimizationProblem
{
public:
  int SetDimension(int d) override { return d == 2 ? PROBLEM_OK : PROBLEM_ERROR; }
  int GetDimension() const override { return 2; }
  int Initialize() override { return PROBLEM_OK; }
  void GetBounds(std::vector<double>& lower, std::vector<double>& upper) override
  {
    lower.assign(2, -1.0);
    upper.assign(2, 1.0);
  }
  int GetOptimumValue(double& value) const override { value = 0.0; return PROBLEM_OK; }
  int GetOptimumPoint(std::vector<double>& y, std::vector<std::string>& u) const override
  {
    y = ParaboloidCenter(2);
    u.clear();
    return PROBLEM_OK;
  }
  int GetNumberOfFunctions() const override { return 1; }
  int GetNumberOfConstraints() const override { return 0; }
  int GetNumberOfCriterions() const override { return 1; }
  double CalculateFunctionals(const std::vector<double>& y, std::vector<std::string>&, int) override
  {
    return Paraboloid(y.data(), 2);
  }
};

class GlobalizerBenchmarksProblemTest : public ::testing::Test
{
protected:
  std::unique_ptr<TempWorkDir> workDir;
  void SetUp() override { workDir.reset(new TempWorkDir()); ResetSolverParameters(2); }
  void TearDown() override { ResetSolverParameters(2); workDir.reset(); }
};

TEST_F(GlobalizerBenchmarksProblemTest, wraps_problem_description)
{
  FakeMixedProblem fake;
  GlobalizerBenchmarksProblem p(&fake);
  EXPECT_EQ(p.GetDimension(), 2);
  EXPECT_EQ(p.GetNumberOfFunctions(), 1);
  EXPECT_EQ(p.GetNumberOfDiscreteVariable(), 1);
  EXPECT_EQ(p.GetNumberOfValues(1), 3);
  EXPECT_EQ(p.GetNumberOfValues(0), -1);

  double lower[1], upper[1];
  p.GetBounds(lower, upper);
  EXPECT_DOUBLE_EQ(lower[0], -1.0);
  EXPECT_DOUBLE_EQ(upper[0], 1.0);

  auto values = p.GetDiscretesValues();
  ASSERT_EQ(values.size(), 1u);
  EXPECT_EQ(values[0][2], "c");

  EXPECT_EQ(p.SetConfigPath("cfg.xml"), (int)IGlobalOptimizationProblem::PROBLEM_OK);
  EXPECT_EQ(fake.config, "cfg.xml");
  EXPECT_EQ(p.SetDimension(2), (int)IGlobalOptimizationProblem::PROBLEM_OK);
  EXPECT_NE(p.SetDimension(3), (int)IGlobalOptimizationProblem::PROBLEM_OK);
}

TEST_F(GlobalizerBenchmarksProblemTest, optimum_and_calculation)
{
  FakeMixedProblem fake;
  GlobalizerBenchmarksProblem p(&fake);

  double value = -1;
  EXPECT_EQ(p.GetOptimumValue(value), (int)IGlobalOptimizationProblem::PROBLEM_OK);
  EXPECT_DOUBLE_EQ(value, 0.0);

  double x[2];
  ASSERT_EQ(p.GetOptimumPoint(x), (int)IGlobalOptimizationProblem::PROBLEM_OK);
  EXPECT_DOUBLE_EQ(x[0], 0.25);
  EXPECT_DOUBLE_EQ(x[1], 1.0) << "index of \"b\"";

  EXPECT_DOUBLE_EQ(p.CalculateFunctionals(x, 0), 0.0);
  double other[2] = { 0.25, 2.0 };  // "c"
  EXPECT_DOUBLE_EQ(p.CalculateFunctionals(other, 0), 1.0);
}

TEST_F(GlobalizerBenchmarksProblemTest, x_yu_conversion)
{
  FakeMixedProblem fake;
  GlobalizerBenchmarksProblem p(&fake);

  double x[2] = { -0.5, 0.0 };
  std::vector<double> y;
  std::vector<std::string> u;
  p.XtoYU(x, y, u);
  ASSERT_EQ(y.size(), 1u);
  ASSERT_EQ(u.size(), 1u);
  EXPECT_DOUBLE_EQ(y[0], -0.5);
  EXPECT_EQ(u[0], "a");

  double back[2] = { 0, 0 };
  u[0] = "c";
  p.YUtoX(y, u, back);
  EXPECT_DOUBLE_EQ(back[0], -0.5);
  EXPECT_DOUBLE_EQ(back[1], 2.0);

  u[0] = "unknown";
  EXPECT_ANY_THROW(p.YUtoX(y, u, back));
  double bad[2] = { 0.0, -1.0 };
  EXPECT_ANY_THROW(p.XtoYU(bad, y, u));
}

TEST_F(GlobalizerBenchmarksProblemTest, discrete_values_enumeration)
{
  FakeMixedProblem fake;
  GlobalizerBenchmarksProblem p(&fake);

  double values[3] = { -1, -1, -1 };
  ASSERT_EQ(p.GetAllDiscreteValues(1, values), (int)IProblem::OK);
  EXPECT_DOUBLE_EQ(values[0], 0.0);
  EXPECT_DOUBLE_EQ(values[1], 1.0);
  EXPECT_DOUBLE_EQ(values[2], 2.0);
  EXPECT_EQ(p.GetAllDiscreteValues(0, values), (int)IIntegerProgrammingProblem::ERROR_DISCRETE_VALUE);

  int* index = nullptr;
  double v = 0;
  EXPECT_EQ(p.GetNextDiscreteValues(index, v, 1), (int)IIntegerProgrammingProblem::ERROR_DISCRETE_VALUE);
  p.ClearCurrentDiscreteValueIndex(&index);
  ASSERT_NE(index, nullptr);
  EXPECT_EQ(p.GetNextDiscreteValues(index, v, 1, 0), (int)IProblem::OK);
  EXPECT_DOUBLE_EQ(v, 1.0);
  p.ClearCurrentDiscreteValueIndex(&index);  // повторный вызов освобождает старый массив
  delete[] index;
}

TEST_F(GlobalizerBenchmarksProblemTest, solver_accepts_benchmark_interface)
{
  FakeContinuousProblem fake;
  Solver solver(&fake);
  ASSERT_EQ(solver.Solve(), 0);
  std::unique_ptr<SolutionResult> res(solver.GetSolutionResult());
  EXPECT_LT(res->GetBestValue(), 0.01);
  delete solver.GetProblem();  // адаптер создаётся Solver-ом через new
}

TEST_F(GlobalizerBenchmarksProblemTest, solver_handles_mixed_integer_benchmark)
{
  FakeMixedProblem fake;
  Solver solver(&fake);
  ASSERT_EQ(solver.Solve(), 0);
  std::unique_ptr<SolutionResult> res(solver.GetSolutionResult());
  EXPECT_NEAR(res->GetBestPoint()[1], 1.0, 1e-9) << "best discrete value is \"b\"";
  EXPECT_LT(res->GetBestValue(), 0.01);
  delete solver.GetProblem();
}

#endif // _GLOBALIZER_BENCHMARKS

// ================================================================================================
//  Загрузка задач из динамических библиотек
// ================================================================================================

class ProblemLoadingTest : public ::testing::Test
{
protected:
  std::string oldLibPath;
  void SetUp() override { oldLibPath = parameters.LibPath.ToString(); }
  void TearDown() override { parameters.LibPath = oldLibPath; }
};

TEST_F(ProblemLoadingTest, manager_reports_missing_library)
{
  ProblemManager manager;
  EXPECT_EQ(manager.GetProblem(), nullptr);
  ::testing::internal::CaptureStderr();
  EXPECT_EQ(manager.LoadProblemLibrary("no_such_problem_library_42"), (int)ProblemManager::ERROR_);
  ::testing::internal::GetCapturedStderr();
  EXPECT_EQ(manager.GetProblem(), nullptr);
}

TEST_F(ProblemLoadingTest, manager_reports_library_without_problem_symbols)
{
#ifdef WIN32
  const char* lib = "kernel32.dll";
#elif defined(__APPLE__)
  const char* lib = "libSystem.dylib";
#else
  const char* lib = "libm.so.6";
#endif
  ProblemManager manager;
  ::testing::internal::CaptureStderr();
  EXPECT_EQ(manager.LoadProblemLibrary(lib), (int)ProblemManager::ERROR_);
  ::testing::internal::GetCapturedStderr();
  EXPECT_EQ(manager.GetProblem(), nullptr);
}

TEST_F(ProblemLoadingTest, init_problem_fails_for_missing_library)
{
  parameters.LibPath = "no_such_problem_library_42";
  ProblemManager manager;
  IProblem* problem = nullptr;
  ::testing::internal::CaptureStderr();
  EXPECT_EQ(InitProblem(manager, problem, 0, nullptr, false), 1);
  ::testing::internal::GetCapturedStderr();
  EXPECT_EQ(problem, nullptr);
}

#ifdef _GLOBALIZER_BENCHMARKS
TEST_F(ProblemLoadingTest, init_benchmark_problem_fails_for_missing_library)
{
  parameters.LibPath = "no_such_problem_library_42";
  GlobalOptimizationProblemManager manager;
  IGlobalOptimizationProblem* problem = nullptr;
  ::testing::internal::CaptureStderr();
  ::testing::internal::CaptureStdout();
  EXPECT_EQ(InitProblemGlobalizerBenchmarks(manager, problem), 1);
  ::testing::internal::GetCapturedStdout();
  ::testing::internal::GetCapturedStderr();
  EXPECT_EQ(problem, nullptr);
}
#endif

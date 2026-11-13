#include "Calculation.h"
#include "Evolvent.h"
#include "MCOMethod.h"
#include "MCOProblemFromFunctions.h"
#include "MCOSolver.h"
#include "OMPCalculation.h"
#include "SearchInterval.h"
#include "test_reset.h"

#include <gtest/gtest.h>

#include <cmath>
#include <memory>
#include <stdexcept>
#include <vector>

typedef MCOProblemFromFunctions::Function Function;

namespace
{
  /// Задача (89) из диссертации Козинова: f1 = (y1 - 1) * y2^2 + 1, f2 = y2
  const std::vector<Function> criteria89 = {
    [](const double* y) { return (y[0] - 1) * y[1] * y[1] + 1; },
    [](const double* y) { return y[1]; } };

  /// Метод с двумя r, открытый для проверки формул
  class MCOMethodProbe : public MCOMethod
  {
  public:
    using MCOMethod::MCOMethod;
    using MCOMethod::CalculateCurrentPoint;
    using MCOMethod::CalculateGlobalR;
  };

  /// Несимметричная функция: в ней нет равных характеристик, порядок испытаний однозначен
  double Asymmetric(const double* y)
  {
    return (y[0] - 0.3) * (y[0] - 0.3) + (y[1] - 0.6) * (y[1] - 0.6) + 0.1 * std::sin(7 * y[0] + 3 * y[1]);
  }

  bool IsDominated(const MCOPoint& point, const std::vector<MCOPoint>& others)
  {
    for (const MCOPoint& other : others)
    {
      bool notWorse = true;
      bool better = false;
      for (size_t i = 0; i < point.criteria.size(); i++)
      {
        notWorse = notWorse && other.criteria[i] <= point.criteria[i];
        better = better || other.criteria[i] < point.criteria[i];
      }
      if (notWorse && better)
        return true;
    }
    return false;
  }
}

class MCOTest : public ::testing::Test
{
protected:
  void SetUp()
  {
    ResetCalculationState();
    ResetParametersToMethodDefaults(2);

    parameters.r = 2.0;
    parameters.Epsilon = 0.05;
    parameters.MaxNumOfPoints = 100000;
    parameters.IsPrintResultToConsole = false;
  }

  void TearDown()
  {
    parameters.TypeMethod = StandartMethod;
    ResetCalculationState();
  }

  /// Другие тесты оставляют в вычислителе свое состояние
  static void ResetCalculationState()
  {
    Calculation::isStartComputingAway = true;
    Calculation::countCalculation = 0;
    Calculation::firstCalculation = 0;
  }

  std::unique_ptr<IProblem> MakeProblem(const std::vector<Function>& criteria,
    const std::vector<Function>& constraints = {}, int dimension = 2)
  {
    std::unique_ptr<IProblem> problem(new MCOProblemFromFunctions(dimension, std::vector<double>(dimension, 0.0),
      std::vector<double>(dimension, 1.0), criteria, constraints));
    problem->Initialize();
    return problem;
  }

  static MCOParameters Lambdas(int count, double rLocal = 0.0)
  {
    MCOParameters mco;
    mco.numberOfLambdas = count;
    mco.rLocal = rLocal;
    return mco;
  }

  static void ExpectSameTrials(const std::vector<MCOPoint>& a, const std::vector<MCOPoint>& b)
  {
    ASSERT_EQ(a.size(), b.size());
    for (size_t i = 0; i < a.size(); i++)
      for (size_t j = 0; j < a[i].y.size(); j++)
        ASSERT_DOUBLE_EQ(a[i].y[j], b[i].y[j]) << "trial " << i;
  }
};

TEST_F(MCOTest, MethodTypeIsParsedByName)
{
  parameters.TypeMethod = std::string("MultiCriteriaMethod");
  EXPECT_EQ(MultiCriteriaMethod, (ETypeMethod)parameters.TypeMethod);
  EXPECT_EQ(std::string("MultiCriteriaMethod"), (std::string)parameters.TypeMethod);
}

TEST_F(MCOTest, LambdasGoFromSecondCriterionToFirst)
{
  auto problem = MakeProblem(criteria89);
  MCOTask task(problem.get(), Lambdas(3));

  ASSERT_EQ(task.GetNumberOfLambdas(), 3);
  EXPECT_EQ(task.GetLambda(), std::vector<double>({ 0.0, 1.0 }));
  task.NextLambda();
  EXPECT_EQ(task.GetLambda(), std::vector<double>({ 0.5, 0.5 }));
  task.NextLambda();
  EXPECT_EQ(task.GetLambda(), std::vector<double>({ 1.0, 0.0 }));
}

TEST_F(MCOTest, LambdasOfThreeCriteriaSumToOne)
{
  const Function f = [](const double* y) { return y[0]; };
  auto problem = MakeProblem({ f, f, f });
  MCOTask task(problem.get(), Lambdas(5));

  for (int i = 0; i < 5; i++)
  {
    double sum = 0.0;
    for (double value : task.GetLambda())
      sum += value;
    EXPECT_NEAR(sum, 1.0, 1e-12);
    if (i < 4)
      task.NextLambda();
  }
}

TEST_F(MCOTest, ExplicitLambdasMustMatchCriteria)
{
  auto problem = MakeProblem(criteria89);
  MCOParameters mco;
  mco.lambdas = { { 1.0, 0.0, 0.0 } };
  EXPECT_ANY_THROW(MCOTask(problem.get(), mco));
}

TEST_F(MCOTest, ConvolutionIsShiftedToMinimum)
{
  auto problem = MakeProblem(criteria89);
  MCOParameters mco;
  mco.lambdas = { { 0.25, 0.75 } };
  MCOTask task(problem.get(), mco);

  const double first[] = { 1.0, 2.0 };
  const double second[] = { 3.0, 0.5 };
  EXPECT_TRUE(task.UpdateMinMax(first));
  EXPECT_TRUE(task.UpdateMinMax(second));
  EXPECT_FALSE(task.UpdateMinMax(first));

  EXPECT_DOUBLE_EQ(task.getMin()[0], 1.0);
  EXPECT_DOUBLE_EQ(task.getMin()[1], 0.5);
  EXPECT_DOUBLE_EQ(task.getMax()[0], 3.0);
  EXPECT_DOUBLE_EQ(task.getMax()[1], 2.0);

  const double point[] = { 2.0, 1.5 };
  // max(0.25 * (2 - 1), 0.75 * (1.5 - 0.5))
  EXPECT_DOUBLE_EQ(task.Convolution(point), 0.75);
}

TEST_F(MCOTest, ConvolutionCanScaleCriteria)
{
  auto problem = MakeProblem(criteria89);
  MCOParameters mco;
  mco.lambdas = { { 0.5, 0.5 } };
  mco.isScaling = true;
  MCOTask task(problem.get(), mco);

  const double first[] = { 0.0, 0.0 };
  const double second[] = { 4.0, 10.0 };
  task.UpdateMinMax(first);
  task.UpdateMinMax(second);

  const double point[] = { 2.0, 1.0 };
  // max(0.5 * 2 / 4, 0.5 * 1 / 10)
  EXPECT_DOUBLE_EQ(task.Convolution(point), 0.25);
}

TEST_F(MCOTest, MethodSeesConstraintsAndOneConvolution)
{
  const Function constraint = [](const double* y) { return y[0] - 0.5; };
  auto problem = MakeProblem(criteria89, { constraint });
  MCOParameters mco;
  mco.lambdas = { { 1.0, 0.0 } };
  MCOTask task(problem.get(), mco);

  EXPECT_EQ(task.GetNumOfFunc(), 2);
  EXPECT_EQ(task.GetConvolutionIndex(), 1);

  const double y[] = { 0.25, 0.5 };
  EXPECT_DOUBLE_EQ(task.CalculateFuncs(y, 0), -0.25);
  // f1 = (0.25 - 1) * 0.25 + 1, а не значение ограничения
  EXPECT_DOUBLE_EQ(task.CalculateFuncs(y, 1), 0.8125);

  double criteria[2];
  EXPECT_TRUE(task.TakeCalculatedCriteria(y, criteria));
  EXPECT_DOUBLE_EQ(criteria[0], 0.8125);
  EXPECT_DOUBLE_EQ(criteria[1], 0.5);
  EXPECT_FALSE(task.TakeCalculatedCriteria(y, criteria));
}

TEST_F(MCOTest, TooManyFunctionsAreRejected)
{
  const Function f = [](const double* y) { return y[0]; };
  auto problem = MakeProblem(std::vector<Function>(MaxNumOfFunc, f));
  EXPECT_ANY_THROW(MCOTask(problem.get(), Lambdas(2)));
}

TEST_F(MCOTest, SolvesEveryLambdaAndCountsTrials)
{
  auto problem = MakeProblem(criteria89);
  MCOSolver solver(problem.get(), Lambdas(20));
  ASSERT_EQ(solver.Solve(), 0);

  ASSERT_EQ(solver.GetSubproblems().size(), 20u);
  int trials = 0;
  for (const MCOSubproblem& subproblem : solver.GetSubproblems())
  {
    trials += subproblem.newTrials;
    EXPECT_EQ(subproblem.solution.criteria.size(), 2u);
  }
  const std::vector<MCOPoint> allTrials = solver.GetTrials();
  EXPECT_EQ(trials, (int)allTrials.size());
  EXPECT_EQ(allTrials.back().lambdaIndex, 19);
}

TEST_F(MCOTest, ParetoSetIsNondominatedAndLiesOnTheFront)
{
  parameters.Epsilon = 0.02;
  auto problem = MakeProblem(criteria89);
  MCOSolver solver(problem.get(), Lambdas(30));
  ASSERT_EQ(solver.Solve(), 0);

  const std::vector<MCOPoint> pareto = solver.GetParetoSet();
  ASSERT_GT(pareto.size(), 20u);

  for (const MCOPoint& point : pareto)
  {
    EXPECT_FALSE(IsDominated(point, pareto));
    // Парето-фронт (89): f1 = 1 - f2^2 при y1 = 0
    const double frontF1 = 1.0 - point.criteria[1] * point.criteria[1];
    EXPECT_GE(point.criteria[0], frontF1 - 1e-12);
    EXPECT_LT(point.criteria[0], frontF1 + 0.05);
  }

  // Каждое испытание либо доминируется набором, либо совпадает с его точкой
  for (const MCOPoint& trial : solver.GetTrials())
  {
    bool isInSet = false;
    for (const MCOPoint& point : pareto)
      isInSet = isInSet || point.criteria == trial.criteria;
    EXPECT_TRUE(isInSet || IsDominated(trial, pareto));
  }
}

TEST_F(MCOTest, ReuseNeedsFewerTrialsThanSeparateSolves)
{
  const int tau = 15;
  auto problem = MakeProblem(criteria89);
  MCOSolver series(problem.get(), Lambdas(tau));
  ASSERT_EQ(series.Solve(), 0);
  const std::vector<MCOSubproblem> subproblems = series.GetSubproblems();
  const size_t seriesTrials = series.GetTrials().size();

  size_t separateTrials = 0;
  for (const MCOSubproblem& subproblem : subproblems)
  {
    MCOParameters mco;
    mco.lambdas = { subproblem.lambda };
    MCOSolver single(problem.get(), mco);
    ASSERT_EQ(single.Solve(), 0);
    separateTrials += single.GetTrials().size();
  }
  EXPECT_LT(seriesTrials, separateTrials);
}

TEST_F(MCOTest, OneCriterionRepeatsStandardMethod)
{
  auto problem = MakeProblem({ Asymmetric });

  parameters.TypeMethod = StandartMethod;
  Solver standard(problem.get());
  ASSERT_EQ(standard.Solve(), 0);
  std::vector<MCOPoint> expected;
  for (Trial* trial : standard.GetAllPoint())
    if (trial->K != 0)
      expected.push_back({ std::vector<double>(trial->y, trial->y + 2), {}, 0 });

  MCOSolver mco(problem.get(), Lambdas(1));
  ASSERT_EQ(mco.Solve(), 0);

  ASSERT_GT(expected.size(), 20u);
  ExpectSameTrials(expected, mco.GetTrials());
}

TEST_F(MCOTest, EqualRGivesTheSameSearchAsOneR)
{
  auto problem = MakeProblem(criteria89);
  MCOSolver oneR(problem.get(), Lambdas(10));
  ASSERT_EQ(oneR.Solve(), 0);

  MCOSolver twoR(problem.get(), Lambdas(10, parameters.r));
  ASSERT_EQ(twoR.Solve(), 0);

  ExpectSameTrials(oneR.GetTrials(), twoR.GetTrials());
}

TEST_F(MCOTest, DifferentRChangesTheSearch)
{
  auto problem = MakeProblem(criteria89);
  MCOSolver oneR(problem.get(), Lambdas(10));
  ASSERT_EQ(oneR.Solve(), 0);

  MCOSolver twoR(problem.get(), Lambdas(10, 1.5));
  ASSERT_EQ(twoR.Solve(), 0);

  const std::vector<MCOPoint> a = oneR.GetTrials();
  const std::vector<MCOPoint> b = twoR.GetTrials();
  bool isDifferent = a.size() != b.size();
  for (size_t i = 0; i < std::min(a.size(), b.size()) && !isDifferent; i++)
    isDifferent = a[i].y != b[i].y;
  EXPECT_TRUE(isDifferent);
}

TEST_F(MCOTest, WrongRLocalIsRejected)
{
  auto problem = MakeProblem(criteria89);
  MCOTask task(problem.get(), Lambdas(2, 0.5));
  SearchData data(2);
  Evolvent evolvent(2, 10);
  OMPCalculation calculation(task);
  EXPECT_ANY_THROW(MCOMethod(task, data, calculation, evolvent));
}

TEST_F(MCOTest, CharacteristicsOfTwoR)
{
  auto problem = MakeProblem(criteria89);
  MCOTask task(problem.get(), Lambdas(2, 1.5));
  SearchData data(2);
  Evolvent evolvent(2, 10);
  evolvent.SetBounds(task.GetA(), task.GetB());
  OMPCalculation calculation(task);
  MCOMethodProbe method(task, data, calculation, evolvent);

  Trial left, right;
  left.index = right.index = 0;
  left.SetX(0.2);
  right.SetX(0.6);
  SearchInterval interval;
  interval.LeftPoint = &left;
  interval.RightPoint = &right;
  interval.delta = 0.4;
  data.M[0] = 1.0;

  // rho = ((1 - 1/2) / (1 - 1/1.5))^2 = 2.25
  // Концы равны лучшему значению: R(r) = delta, побеждает rho * R(r_loc)
  data.Z[0] = 0.1;
  left.FuncValues[0] = right.FuncValues[0] = 0.1;
  EXPECT_NEAR(method.CalculateGlobalR(&interval), 2.25 * 0.4, 1e-12);

  // Концы выше лучшего: R(r) = delta - 4 / r, побеждает R(r_glob)
  data.Z[0] = 0.0;
  left.FuncValues[0] = right.FuncValues[0] = 1.0;
  EXPECT_NEAR(method.CalculateGlobalR(&interval), 0.4 - 4.0 / 2.0, 1e-12);
}

TEST_F(MCOTest, NewPointUsesTheWinningR)
{
  auto problem = MakeProblem(criteria89);
  MCOTask task(problem.get(), Lambdas(2, 1.5));
  SearchData data(2);
  Evolvent evolvent(2, 10);
  evolvent.SetBounds(task.GetA(), task.GetB());
  OMPCalculation calculation(task);
  MCOMethodProbe method(task, data, calculation, evolvent);

  Trial left, right, point;
  left.index = right.index = 0;
  left.SetX(0.2);
  right.SetX(0.6);
  SearchInterval interval;
  interval.LeftPoint = &left;
  interval.RightPoint = &right;
  interval.delta = 0.4;
  data.M[0] = 1.0;

  // Побеждает r_loc: x = 0.4 - (0.2 / 1)^2 / 2 / 1.5
  data.Z[0] = 0.1;
  left.FuncValues[0] = 0.1;
  right.FuncValues[0] = 0.3;
  method.CalculateCurrentPoint(point, &interval);
  EXPECT_NEAR(point.X().toDouble(), 0.4 - 0.04 / 2 / 1.5, 1e-12);

  // Побеждает r_glob: x = 0.4 - (0.2 / 1)^2 / 2 / 2
  data.Z[0] = -5.0;
  left.FuncValues[0] = 1.0;
  right.FuncValues[0] = 1.2;
  method.CalculateCurrentPoint(point, &interval);
  EXPECT_NEAR(point.X().toDouble(), 0.4 - 0.04 / 2 / 2.0, 1e-12);
}

TEST_F(MCOTest, ConstraintCutsPartOfTheFront)
{
  parameters.Epsilon = 0.02;
  parameters.r = 3.0;
  const Function constraint = [](const double* y) { return 0.3 - y[1]; };
  auto problem = MakeProblem(criteria89, { constraint });
  MCOSolver solver(problem.get(), Lambdas(30));
  ASSERT_EQ(solver.Solve(), 0);

  int violations = 0;
  for (const MCOPoint& trial : solver.GetTrials())
  {
    EXPECT_EQ(trial.criteria.empty(), trial.y[1] < 0.3);
    violations += trial.criteria.empty();
  }
  EXPECT_GT(violations, 0);

  // Без ограничения f2 дошел бы до нуля; с ним минимум равен 0.3
  double minF2 = 1.0;
  for (const MCOPoint& point : solver.GetParetoSet())
  {
    EXPECT_GE(point.y[1], 0.3);
    minF2 = std::min(minF2, point.criteria[1]);
  }
  EXPECT_NEAR(minF2, 0.3, 0.02);
}

TEST_F(MCOTest, FirstPointMayViolateConstraint)
{
  parameters.r = 3.0;
  // Центр области - первая точка метода - лежит вне допустимой области
  const Function constraint = [](const double* y)
  {
    return 0.04 - (y[0] - 0.5) * (y[0] - 0.5) - (y[1] - 0.5) * (y[1] - 0.5);
  };
  auto problem = MakeProblem(criteria89, { constraint });
  MCOSolver solver(problem.get(), Lambdas(10));
  ASSERT_EQ(solver.Solve(), 0);

  const std::vector<MCOPoint> trials = solver.GetTrials();
  ASSERT_FALSE(trials.empty());
  EXPECT_TRUE(trials[0].criteria.empty());

  for (const MCOSubproblem& subproblem : solver.GetSubproblems())
  {
    ASSERT_EQ(subproblem.solution.y.size(), 2u);
    const double* y = subproblem.solution.y.data();
    EXPECT_LE(constraint(y), 0.0);
    EXPECT_DOUBLE_EQ(subproblem.solution.criteria[0], criteria89[0](y));
    EXPECT_DOUBLE_EQ(subproblem.solution.criteria[1], criteria89[1](y));
  }
}

TEST_F(MCOTest, SolvesThreeCriteria)
{
  auto distanceTo = [](double a, double b) -> Function
  {
    return [a, b](const double* y) { return (y[0] - a) * (y[0] - a) + (y[1] - b) * (y[1] - b); };
  };
  auto problem = MakeProblem({ distanceTo(0.2, 0.2), distanceTo(0.8, 0.2), distanceTo(0.5, 0.8) });
  MCOSolver solver(problem.get(), Lambdas(10));
  ASSERT_EQ(solver.Solve(), 0);

  const std::vector<MCOPoint> pareto = solver.GetParetoSet();
  ASSERT_GT(pareto.size(), 5u);
  for (const MCOPoint& point : pareto)
  {
    EXPECT_FALSE(IsDominated(point, pareto));
    // Область Парето - треугольник между тремя центрами
    EXPECT_GE(point.y[1], 0.1);
    EXPECT_LE(point.y[1], 0.9);
  }
}

TEST_F(MCOTest, FailedCriterionMakesPointUncomputable)
{
  auto problem = MakeProblem(criteria89);
  const Function failing = [](const double* y) -> double
  {
    if (y[0] > 0.7)
      throw std::runtime_error("criterion failed");
    return y[1];
  };
  auto failingProblem = MakeProblem({ criteria89[0], failing });

  // Даже при нулевом весе отказавшего критерия свертка не должна стать обычным числом
  MCOParameters mco;
  mco.lambdas = { { 1.0, 0.0 } };
  MCOTask task(failingProblem.get(), mco);
  const double y[] = { 0.9, 0.5 };
  EXPECT_GE(task.CalculateFuncs(y, 0), MaxDouble);
  double criteria[2];
  EXPECT_FALSE(task.TakeCalculatedCriteria(y, criteria));

  MCOSolver solver(failingProblem.get(), Lambdas(5));
  ASSERT_EQ(solver.Solve(), 0);

  int failedTrials = 0;
  for (const MCOPoint& trial : solver.GetTrials())
  {
    EXPECT_EQ(trial.criteria.empty(), trial.y[0] > 0.7);
    failedTrials += trial.criteria.empty();
  }
  EXPECT_GT(failedTrials, 0);
  for (const MCOPoint& point : solver.GetParetoSet())
    EXPECT_LE(point.y[0], 0.7);
}

TEST_F(MCOTest, TrialLimitIsSharedBetweenLambdas)
{
  parameters.Epsilon = 1e-6;
  parameters.MaxNumOfPoints = 40;
  auto problem = MakeProblem(criteria89);
  MCOSolver solver(problem.get(), Lambdas(4));
  ASSERT_EQ(solver.Solve(), 0);

  ASSERT_EQ(solver.GetSubproblems().size(), 4u);
  for (const MCOSubproblem& subproblem : solver.GetSubproblems())
    EXPECT_EQ(subproblem.newTrials, 10);
}

TEST_F(MCOTest, SolvesWithScaling)
{
  auto problem = MakeProblem(criteria89);
  MCOParameters mco = Lambdas(10);
  mco.isScaling = true;
  MCOSolver solver(problem.get(), mco);
  ASSERT_EQ(solver.Solve(), 0);
  EXPECT_GT(solver.GetParetoSet().size(), 5u);
}

TEST_F(MCOTest, SolveCanBeRepeated)
{
  auto problem = MakeProblem(criteria89);
  MCOSolver solver(problem.get(), Lambdas(5));
  ASSERT_EQ(solver.Solve(), 0);
  const std::vector<MCOPoint> first = solver.GetTrials();

  ASSERT_EQ(solver.Solve(), 0);
  ExpectSameTrials(first, solver.GetTrials());
}

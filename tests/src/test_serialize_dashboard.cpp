/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//  File:      test_serialize_dashboard.cpp                                //
//                                                                         //
//  Purpose:   Модульные тесты SerializeToDashBoard: преобразование        //
//             испытаний/параметров/решения в JSON, сохранение и загрузка  //
//             состояния (новый и старый форматы).                          //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include <gtest/gtest.h>

#include <fstream>
#include <memory>

#include "Solver.h"
#include "SerializeToDashBoard.h"

#include "test_env.h"

using namespace test_env;
using json = nlohmann::json;

class SerializeToDashBoardTest : public ::testing::Test
{
protected:
  std::unique_ptr<TempWorkDir> workDir;
  std::unique_ptr<ProblemFromFunctionPointers> problem;
  std::unique_ptr<Solver> solver;
  std::unique_ptr<SolutionResult> result;
  SerializeToDashBoard serializer;

  void SetUp() override
  {
    workDir.reset(new TempWorkDir());
    ResetSolverParameters(2);
  }

  void TearDown() override
  {
    result.reset();
    solver.reset();
    problem.reset();
    ResetSolverParameters(2);
    workDir.reset();
  }

  void SolveProblem(ProblemFromFunctionPointers* p)
  {
    problem.reset(p);
    solver.reset(new Solver(problem.get()));
    ASSERT_EQ(solver->Solve(), 0);
    result.reset(solver->GetSolutionResult());
  }

  Trial* Best() { return solver->GetData()->GetBestTrial(); }
};

TEST_F(SerializeToDashBoardTest, trial_to_json)
{
  SolveProblem(MakeParaboloid(2));
  json j = serializer.TrialToJson(Best(), solver->GetTask());
  ASSERT_TRUE(j.contains("float_variables"));
  EXPECT_EQ(j["float_variables"].size(), 2u);
  EXPECT_DOUBLE_EQ(j["float_variables"][0].get<double>(), Best()->y[0]);
  EXPECT_TRUE(j["discrete_variables"].empty());
  ASSERT_EQ(j["function_values"].size(), 1u);
  EXPECT_DOUBLE_EQ(j["function_values"][0]["value"].get<double>(), Best()->FuncValues[0]);
  EXPECT_EQ(j["index"].get<int>(), Best()->index);
  EXPECT_DOUBLE_EQ(j["__z"].get<double>(), Best()->FuncValues[Best()->index]);
}

TEST_F(SerializeToDashBoardTest, trial_to_json_with_discrete_variables)
{
  SolveProblem(new ProblemFromFunctionPointers(2, 1,
    std::vector<double>(2, -1.0), std::vector<double>(2, 1.0), std::vector<int>{3},
    std::vector<std::function<double(const double*)>>(1, [](const double* y)
      { return (y[0] - 0.3) * (y[0] - 0.3) + y[1]; })));
  json j = serializer.TrialToJson(Best(), solver->GetTask());
  EXPECT_EQ(j["float_variables"].size(), 1u);
  ASSERT_EQ(j["discrete_variables"].size(), 1u);
  EXPECT_DOUBLE_EQ(j["discrete_variables"][0].get<double>(), -1.0);
}

TEST_F(SerializeToDashBoardTest, parameters_to_json)
{
  parameters.Epsilon = 0.02;
  parameters.NumPoints = 3;
  json j = serializer.ParametersToJson(parameters);
  EXPECT_DOUBLE_EQ(j["eps"].get<double>(), 0.02);
  EXPECT_DOUBLE_EQ(j["r"].get<double>(), (double)parameters.r);
  EXPECT_EQ(j["iters_limit"].get<int>(), (int)parameters.MaxNumOfPoints);
  EXPECT_EQ(j["number_of_parallel_points"].get<int>(), 3);
  EXPECT_TRUE(j["start_point"].empty());
}

TEST_F(SerializeToDashBoardTest, search_data_solution_and_problem_to_json)
{
  SolveProblem(MakeParaboloid(2));

  json jData = serializer.SearchDataToJson(solver->GetData());
  EXPECT_FALSE(jData.is_null());

  json jSolution = serializer.SolutionResultToJson(*result, solver->GetTask());
  EXPECT_FALSE(jSolution.is_null());

  json jProblem = serializer.ProblemToJson(*solver->GetTask());
  EXPECT_EQ(jProblem["version"].get<std::string>(), "2.0");

  double accuracy = serializer.CalculateAchievedAccuracy(solver->GetTask(), Best());
  EXPECT_GE(accuracy, 0.0);
  EXPECT_LT(accuracy, 0.1) << "best point is close to the known optimum";
}

TEST_F(SerializeToDashBoardTest, achieved_accuracy_without_known_optimum)
{
  SolveProblem(MakeParaboloid(2, /*withOptimum*/ false));
  double accuracy = serializer.CalculateAchievedAccuracy(solver->GetTask(), Best());
  EXPECT_FALSE(std::isnan(accuracy));
}

TEST_F(SerializeToDashBoardTest, save_and_load_full_state)
{
  SolveProblem(MakeParaboloid(2));
  std::vector<Trial*>& trials = solver->GetAllPoint();
  std::vector<Trial*> best{ Best() };

  ASSERT_TRUE(serializer.SaveFullState("state.json", solver->GetData(), *result,
    solver->GetTask(), parameters, trials, best));
  ASSERT_TRUE(workDir->Exists("state.json"));

  json root = json::parse(workDir->Read("state.json"));
  EXPECT_EQ(root["mode"].get<std::string>(), "full");
  EXPECT_EQ(root["SearchDataItem"].size(), trials.size());
  EXPECT_EQ(root["best_trials"].size(), 1u);

  std::vector<Trial*> loaded;
  Trial* loadedBest = nullptr;
  ASSERT_TRUE(serializer.LoadFromFile("state.json", loaded, loadedBest, solver->GetTask()));
  ASSERT_EQ(loaded.size(), trials.size());
  ASSERT_NE(loadedBest, nullptr);
  EXPECT_DOUBLE_EQ(loadedBest->y[0], Best()->y[0]);
  EXPECT_DOUBLE_EQ(loadedBest->y[1], Best()->y[1]);
  EXPECT_DOUBLE_EQ(loadedBest->FuncValues[0], Best()->FuncValues[0]);
  EXPECT_EQ(loadedBest->index, Best()->index);

  // Без задачи размерность берётся из самого файла.
  std::vector<Trial*> loaded2;
  Trial* loadedBest2 = nullptr;
  EXPECT_TRUE(serializer.LoadFromFile("state.json", loaded2, loadedBest2));
  EXPECT_EQ(loaded2.size(), trials.size());

  for (Trial* t : loaded) delete t;
  for (Trial* t : loaded2) delete t;
  delete loadedBest;
  delete loadedBest2;
}

TEST_F(SerializeToDashBoardTest, load_legacy_format)
{
  std::ofstream("legacy.json") << R"({
    "trials": [
      { "y": [0.1, 0.2], "FuncValues": [1.5], "x": 0.25, "index": 0,
        "discreteValuesIndex": 0, "K": 1, "TypeColor": 2, "iterationNumber": 3 }
    ],
    "best_trial": { "y": [0.1, 0.2], "FuncValues": [1.5], "x": 0.25, "index": 0 }
  })";

  std::vector<Trial*> loaded;
  Trial* best = nullptr;
  ASSERT_TRUE(serializer.LoadFromFile("legacy.json", loaded, best));
  ASSERT_EQ(loaded.size(), 1u);
  EXPECT_DOUBLE_EQ(loaded[0]->y[1], 0.2);
  EXPECT_DOUBLE_EQ(loaded[0]->FuncValues[0], 1.5);
  EXPECT_EQ(loaded[0]->K, 1);
  EXPECT_EQ(loaded[0]->TypeColor, 2);
  EXPECT_EQ(loaded[0]->iterationNumber, 3);
  ASSERT_NE(best, nullptr);
  EXPECT_DOUBLE_EQ(best->y[0], 0.1);

  for (Trial* t : loaded) delete t;
  delete best;
}

TEST_F(SerializeToDashBoardTest, load_failures)
{
  std::vector<Trial*> loaded;
  Trial* best = nullptr;
  EXPECT_FALSE(serializer.LoadFromFile("missing.json", loaded, best));

  std::ofstream("garbage.json") << "{ this is not json";
  EXPECT_FALSE(serializer.LoadFromFile("garbage.json", loaded, best));
  EXPECT_TRUE(loaded.empty());
  EXPECT_EQ(best, nullptr);
}

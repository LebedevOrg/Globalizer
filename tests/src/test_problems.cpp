#ifdef WIN32 //these tests assume windows enviroment only
#include "parameters.h"

#include <gtest/gtest.h>
#include <string>
#include <cstdlib>

#include "problem_manager.h"
#include "test_config.h"
#include "init_problem.h"

using namespace std;

// ------------------------------------------------------------------------------------------------
void SetProblem(string libName, string confName, char* dim, TParameters& parameters, TProblemManager& manager, IProblem*& problem)
{
  string lib = (string(TESTDATA_BIN_PATH) + "/" + libName + ".dll").c_str();
  int argc = 5;
  char* argv[7];

  argv[0] = new char(8);
  argv[1] = "-lib";
  argv[2] = new char[lib.length() + 1];
  for (unsigned int i = 0; i < lib.length(); i++)
    argv[2][i] = lib[i];
  argv[2][lib.length()] = 0;
  argv[3] = "-N";
  argv[4] = dim;

  if (confName != "")
  {
    argc = 7;
    string conf = string(TESTDATA_BIN_PATH) + "/" + confName + ".xml";
    argv[5] = "-libConf";
    argv[6] = new char[conf.length() + 1];
    for (unsigned int i = 0; i < conf.length(); i++)
      argv[6][i] = conf[i];
    argv[6][conf.length()] = 0;
  }

  ASSERT_NO_THROW(parameters.Init(argc, argv, false));

  ASSERT_EQ(parameters.IsStart(), true);

  ASSERT_EQ(InitProblem(manager, problem, argc, argv, false), 0);
}

// ------------------------------------------------------------------------------------------------
TEST(Problem_gkls, test_gkls)
{
  //gkls.dll
  IProblem* problem = 0;
  string libName = "gkls";
  string confName = "gkls_conf";
  int Dimension = 2;
  char dim[2] = {'0' + char(Dimension), 0};

  double* y = new double [Dimension];
  int size = 0;
  double* lower = new double[Dimension];
  double* upper = new double[Dimension];

  double l = -1;
  double u = 1;

  double val = 0.30747447713190862;

  for (int i = 0; i < Dimension; i++)
  {
    y[i] = 0;
    lower[i] = 0;
    upper[i] = 0;
  }

  TProblemManager manager;
  TParameters parameters;

  SetProblem(libName, confName, dim, parameters, manager, problem);

  EXPECT_DOUBLE_EQ(problem->CalculateFunctionals(y, 0), val);

  ASSERT_EQ(problem->GetDimension(), 2);

  ASSERT_EQ(problem->GetAllOptimumPoint(y, size), IProblem::UNDEFINED);

  ASSERT_NO_THROW(problem->GetBounds(lower, upper));

  for (int i = 0; i < Dimension; i++)
  {
    EXPECT_DOUBLE_EQ(lower[i], l);
    EXPECT_DOUBLE_EQ(upper[i], u);
  }

  ASSERT_EQ(problem->GetNumberOfConstraints(), 0);

  ASSERT_EQ(problem->GetNumberOfCriterions(), 1);

  ASSERT_EQ(problem->GetNumberOfFunctions(), 1);

  ASSERT_EQ(problem->GetOptimumPoint(y), IProblem::OK);

  ASSERT_EQ(problem->GetOptimumValue(val), IProblem::OK);

  EXPECT_DOUBLE_EQ(problem->CalculateFunctionals(y, 0), val);
}

// ------------------------------------------------------------------------------------------------
TEST(Problem_rastrigin, test_rastrigin)
{
  //rastrigin.dll
  IProblem* problem = 0;
  string libName = "rastrigin";
  string confName = "";
  int Dimension = 2;
  char dim[2] = { '0' + char(Dimension), 0 };

  double* y = new double[Dimension];
  int size = 0;
  double* lower = new double[Dimension];
  double* upper = new double[Dimension];

  double l = -2.2;
  double u = 1.8;

  double val = 3.8396601125010505;

  for (int i = 0; i < Dimension; i++)
  {
    y[i] = 0.1;
    lower[i] = 0;
    upper[i] = 0;
  }

  TProblemManager manager;
  TParameters parameters;

  SetProblem(libName, confName, dim, parameters, manager, problem);

  EXPECT_DOUBLE_EQ(problem->CalculateFunctionals(y, 0), val);

  ASSERT_EQ(problem->GetDimension(), 2);

  ASSERT_EQ(problem->GetAllOptimumPoint(y, size), IProblem::UNDEFINED);

  ASSERT_NO_THROW(problem->GetBounds(lower, upper));

  for (int i = 0; i < Dimension; i++)
  {
    EXPECT_DOUBLE_EQ(lower[i], l);
    EXPECT_DOUBLE_EQ(upper[i], u);
  }

  ASSERT_EQ(problem->GetNumberOfConstraints(), 0);

  ASSERT_EQ(problem->GetNumberOfCriterions(), 1);

  ASSERT_EQ(problem->GetNumberOfFunctions(), 1);

  ASSERT_EQ(problem->GetOptimumPoint(y), IProblem::OK);

  ASSERT_EQ(problem->GetOptimumValue(val), IProblem::OK);

  EXPECT_DOUBLE_EQ(problem->CalculateFunctionals(y, 0), val);
}

// ------------------------------------------------------------------------------------------------
TEST(Problem_gklsC, test_gklsC)
{
  //gklsC_conf.dll
  IProblem* problem = 0;
  string libName = "gklsC";
  string confName = "gklsC_conf";
  int Dimension = 2;
  char dim[2] = { '0' + char(Dimension), 0 };

  double* y = new double[Dimension];
  int size = 0;
  double* lower = new double[Dimension];
  double* upper = new double[Dimension];

  double l = -1;
  double u = 1;

  int NumberOfFunctions = 3;

  double* val = new double [NumberOfFunctions];

  val[0] = -3.2289740417665151;
  val[1] = -1;
  val[2] = -1;

  double* val2 = new double[NumberOfFunctions];
  for (int i = 0; i < NumberOfFunctions; i++)
  {
    val2[i] = 0;
  }

  for (int i = 0; i < Dimension; i++)
  {
    y[i] = 0.1;
    lower[i] = 0;
    upper[i] = 0;
  }

  TProblemManager* manager = new TProblemManager();
  TParameters* parameters = new TParameters();

  SetProblem(libName, confName, dim, *parameters, *manager, problem);

  EXPECT_DOUBLE_EQ(problem->CalculateFunctionals(y, 0), val[0]);

  ASSERT_EQ(problem->GetDimension(), 2);

  ASSERT_EQ(problem->GetAllOptimumPoint(y, size), IProblem::UNDEFINED);

  ASSERT_NO_THROW(problem->GetBounds(lower, upper));

  for (int i = 0; i < Dimension; i++)
  {
    EXPECT_DOUBLE_EQ(lower[i], l);
    EXPECT_DOUBLE_EQ(upper[i], u);
  }

  ASSERT_EQ(problem->GetNumberOfConstraints(), 2);

  ASSERT_EQ(problem->GetNumberOfCriterions(), 1);

  ASSERT_EQ(problem->GetNumberOfFunctions(), NumberOfFunctions);

  ASSERT_EQ(problem->GetOptimumPoint(y), IProblem::OK);

  ASSERT_EQ(problem->GetOptimumValue(val[0]), IProblem::OK);

  for (int i = 0; i < NumberOfFunctions; i++)
  {
    ASSERT_EQ(problem->GetOptimumValue(val2[i], i), IProblem::OK);
    ASSERT_EQ(val[i], val2[i]);
  }

  delete[] y;
  delete[] lower;
  delete[] upper;
  delete[] val;
  delete[] val2;

  ASSERT_NO_THROW(delete manager);
  ASSERT_NO_THROW(delete parameters);
}

// ------------------------------------------------------------------------------------------------
TEST(Problem_grishagin, test_grishagin)
{
  //grishagin.dll
  IProblem* problem = 0;
  string libName = "grishagin";
  string confName = "grishagin_conf";
  int Dimension = 2;
  char dim[2] = { '0' + char(Dimension), 0 };

  double* y = new double[Dimension];
  int size = 0;
  double* lower = new double[Dimension];
  double* upper = new double[Dimension];

  double l = 0;
  double u = 1;

  double val = -9.0461071393397603;

  for (int i = 0; i < Dimension; i++)
  {
    y[i] = 0.1;
    lower[i] = 0;
    upper[i] = 0;
  }

  TProblemManager manager;
  TParameters parameters;

  SetProblem(libName, confName, dim, parameters, manager, problem);

  EXPECT_DOUBLE_EQ(problem->CalculateFunctionals(y, 0), val);

  ASSERT_EQ(problem->GetDimension(), 2);

  ASSERT_EQ(problem->GetAllOptimumPoint(y, size), IProblem::UNDEFINED);

  ASSERT_NO_THROW(problem->GetBounds(lower, upper));

  for (int i = 0; i < Dimension; i++)
  {
    EXPECT_DOUBLE_EQ(lower[i], l);
    EXPECT_DOUBLE_EQ(upper[i], u);
  }

  ASSERT_EQ(problem->GetNumberOfConstraints(), 0);

  ASSERT_EQ(problem->GetNumberOfCriterions(), 1);

  ASSERT_EQ(problem->GetNumberOfFunctions(), 1);

  ASSERT_EQ(problem->GetOptimumPoint(y), IProblem::OK);

  ASSERT_EQ(problem->GetOptimumValue(val), IProblem::OK);

  EXPECT_DOUBLE_EQ(problem->CalculateFunctionals(y, 0), val);
}
#endif //WIN32
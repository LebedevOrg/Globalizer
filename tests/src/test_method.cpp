#include <gtest/gtest.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include "Method.h"
#include "Common.h"
#include "test_common.h"
#include "ProblemManager.h"
#include "test_config.h"
#include "MethodFactory.h"
using namespace std;
/**
  Вспомогательный класс, помогающий задать начальную конфигурацию объекта класса #Method,
  которая будет использоваться в тестах
 */
struct testParameters
{
  std::string libName;
  std::string actualDataFile;
  std::string expectedDataFile;
  int dim;
  testParameters(std::string _libName,
    std::string _actualDataFile,
    std::string _expectedDataFile,
    int _dim) :
    libName(_libName), actualDataFile(_actualDataFile), expectedDataFile(_expectedDataFile), dim(_dim) {}
};

class MethodTest : public ::testing::TestWithParam<testParameters>
{
protected:
  static const int MaxNumOfTrials = 10000;
  static const int CurL = 0;
  static const int L = 1;
  static const int m = 10;

  double eps;
  double r;
  double reserv;

  Task* pTask;
  SearchData* pData;
  Parameters* parameters;
  ProblemManager manager;

  void SetUp()
  {
    SetUp(std::string(LIB_RASTRIGIN), 4);
  }

  void TearDown()
  {
    delete pTask;
    delete pData;
    delete parameters;
  }

  void SetUp(string libName, int n)
  {
    eps = 0.01;
    r = 2.3;
    reserv = 0.001;

    IProblem* problem;
    std::string libPath = std::string(TESTDATA_BIN_PATH) + libName;
    if (ProblemManager::OK_ == manager.LoadProblemLibrary(libPath))
    {
      int argc = 1;
      char* argv[1];
      argv[0] = new char(8);
      parameters = new Parameters();
      parameters->Init(argc, argv);
      problem = manager.GetProblem();
      problem->SetDimension(n);
      problem->SetConfigPath(parameters->libConfigPath);
      problem->Initialize();
      pTask = new Task( problem, 0);

      parameters->Dimension = n;
      pData = new SearchData(MaxNumOfFunc, DefaultSearchDataSize);
    }
    else
    {
      pTask = NULL;
    }
  }

  bool DoIteration(Method* method)
  {
    bool IsStop;
    method->CalculateIterationPoints();
    IsStop = method->CheckStopCondition();
    method->CalculateFunctionals();
    method->EstimateOptimum();
    method->RenewSearchData();
    method->FinalizeIteration();
    return IsStop;
  }
};

//
///**
// * Проверка параметра Максимальное число испытаний #MaxNumOfTrials
// * MaxNumOfTrials >=1
// */
//TEST_F(MethodTest, throws_when_create_with_not_positive_MaxNumOfTrials)
//{
//  ASSERT_ANY_THROW(Method method(0, eps, r, reserv, m, L, CurL,
//    mpRotated, *parameters, pTask, pData));
//}
//
///**
// * Проверка параметра Точность решения задачи #Epsilon
// * 0 < Epsilon <= 0.01
// */
//TEST_F(MethodTest, throws_when_create_with_not_positive_epsilon)
//{
//  ASSERT_ANY_THROW(Method method(MaxNumOfTrials, 0, r, reserv, m, L, CurL,
//    mpRotated, *parameters, pTask, pData));
//}

//Нужно обсудить верхнюю границу
//TEST_F(MethodTest, throws_when_create_with_too_large_epsilon)
//{
//  ASSERT_ANY_THROW(Method method(MaxNumOfTrials, 0.011, r, reserv, m, L, CurL,
//                                  mpRotated, *parameters, pTask, pData));
//}

///**
// * Проверка параметра Надежность метода #r
// * r > 1
// */
//TEST_F(MethodTest, throws_when_create_with_too_low_r)
//{
//  ASSERT_ANY_THROW(Method method(MaxNumOfTrials, eps, 1, reserv, m, L, CurL,
//    mpRotated, *parameters, pTask, pData));
//}
//
///**
// * Проверка параметра параметр eps-резервирования #reserv
// * 0 <= reserv <= 0.5
// */
//TEST_F(MethodTest, throws_when_create_with_negative_reserv)
//{
//  ASSERT_ANY_THROW(Method method(MaxNumOfTrials, eps, r, -0.001, m, L, CurL,
//    mpRotated, *parameters, pTask, pData));
//}
//
//TEST_F(MethodTest, throws_when_create_with_too_large_reserv)
//{
//  ASSERT_ANY_THROW(Method method(MaxNumOfTrials, eps, r, 0.51, m, L, CurL,
//    mpRotated, *parameters, pTask, pData));
//}

/**
 * Проверка параметра Плотность построения развертки #m
 * 2 <= m <= MaxM
 */
//TEST_F(MethodTest, throws_when_create_with_too_low_m)
//{
//  ASSERT_ANY_THROW(Method method(MaxNumOfTrials, eps, r, reserv, 1, L, CurL,
//    mpRotated, *parameters, pTask, pData));
//}

//TEST_F(MethodTest, throws_when_create_with_too_large_m)
//{
//  ASSERT_ANY_THROW(Method method(MaxNumOfTrials, eps, r, reserv, MaxM + 1, L, CurL,
//    mpRotated, *parameters, pTask, pData));
//}

/**
 * Проверка параметра Число используемых разверток #L
 * Для сдвиговой разверки L <= #m, для вращаемой L <= N(N-1)+1
 */
//TEST_F(MethodTest, throws_when_create_with_not_positive_L)
//{
//  ASSERT_ANY_THROW(Method method(MaxNumOfTrials, eps, r, reserv, m, 0, CurL,
//    mpRotated, *parameters, pTask, pData));
//}

//TEST_F(MethodTest, throws_when_create_with_too_large_L_for_rotatedEvolvent)
//{
//  int N = pTask->GetN();
//  ASSERT_ANY_THROW(Method method(MaxNumOfTrials, eps, r, reserv, m, N * (N - 1) + 2, CurL,
//    mpRotated, *parameters, pTask, pData));
//}

//TEST_F(MethodTest, throws_when_create_with_too_large_L_for_ShiftedEvolvent)
//{
//  ASSERT_ANY_THROW(Method method(*parameters, *pTask, *pData));
//}

/**
 * Создание метода с корректными входными параметрами
 */
//TEST_F(MethodTest, can_create_with_correct_values)
//{
//  ASSERT_NO_THROW(Method method(*parameters, *pTask, *pData));
//}
//
///**
// * Проверка метода #FirstIteration
// */
//TEST_F(MethodTest, on_FirstIteration_can_reset_IterationCount)
//{
//  Method* method = new Method(*parameters, *pTask, *pData);
//  method->FirstIteration();
//  ASSERT_EQ(1, method->GetIterationCount());
//}
//
//TEST_F(MethodTest, on_FirstIteration_can_reset_BesTrial)
//{
//  Method* pMethod = new Method(*parameters, *pTask, *pData);
//  pMethod->FirstIteration();
//  ASSERT_EQ(-2, pMethod->GetOptimEstimation()[0].index);
//}
//
//TEST_F(MethodTest, on_FirstIteration_can_reset_NumberOfTrials)
//{
//  Method* pMethod = new Method(*parameters, *pTask, *pData);
//  pMethod->FirstIteration();
//  ASSERT_EQ(0, pMethod->GetNumberOfTrials());
//}
//
////TEST_F(MethodTest, on_FirstIteration_can_generate_new_points)
////{
////  Method* pMethod = new Method(*parameters, *pTask, *pData);
////  pMethod->FirstIteration();
////
////  int NumPoints = parameters->NumPoints;
////  double h = 1.0 / (NumPoints + 1);
////  for (int i = 0; i < NumPoints; i++)
////    ASSERT_EQ((i + 1) * h, pMethod->GetCurTrials()[i].x);
////}
//
///**
// * Проверка метода #FinalizeIteration
// */
//TEST_F(MethodTest, FinalizeIteration_can_increase_iterationCount_1)
//{
//  Method* pMethod = new Method(*parameters, *pTask, *pData);
//  pMethod->SetBounds();
//
//  pMethod->FirstIteration();
//  int count = pMethod->GetIterationCount();
//  pMethod->FinalizeIteration();
//
//  ASSERT_EQ(++count, pMethod->GetIterationCount());
//}
//
///**
// * Проверка метода #CheckStopCondition
// */
//TEST_F(MethodTest, CheckStopCondition_can_stop_method_when_too_many_inerations)
//{
//  int currentMaxNumOfTrials = 2;
//  Method* pMethod = new Method(currentMaxNumOfTrials, eps, r, reserv, m, L, CurL,
//    mpRotated, *parameters, pTask, pData);
//  pMethod->SetBounds();
//  bool IsStop = false;
//
//  pMethod->FirstIteration();
//  while (!IsStop)
//  {
//    IsStop = DoIteration(pMethod);
//  }
//
//  ASSERT_GE(pMethod->GetIterationCount(), currentMaxNumOfTrials);
//}
//
///**
// * Проверка решения задач с различными функциями
// */
//TEST_P(MethodTest, check_states_of_method_iterations)
//{
//  tesParameters params = GetParam();
//  std::string actualDataFile = std::string(TESTDATA_PATH) + std::string(params.actualDataFile);
//  std::string expectedDataFile = std::string(TESTDATA_PATH) + std::string(params.expectedDataFile);
//
//  FILE* currentf = fopen(actualDataFile.c_str(), "w");
//  fclose(currentf);
//
//  SetUp(std::string(params.libName), params.dim);
//  Method* pMethod = new Method(MaxNumOfTrials, eps, r, reserv, m, L, CurL,
//    mpBase, *parameters, pTask, pData);
//  pMethod->SetBounds();
//
//  pMethod->FirstIteration();
//  bool IsStop = false;
//  while (!IsStop)
//  {
//    if (pMethod->GetIterationCount() % 10 == 0)
//    {
//      pMethod->PrintStateToFile(actualDataFile);
//    }
//    IsStop = DoIteration(pMethod);
//  }
//  CheckMetodIteration(expectedDataFile, actualDataFile, pMethod->GetIterationCount() / 10);
//}

//INSTANTIATE_TEST_CASE_P(CheckMethod,
//  MethodTest,
//  ::testing::Values(
//    testParameters(LIB_RASTRIGIN, "/actualRastriginState.dat", "/expectedRastriginState.dat", 4),
//    testParameters(LIB_STRONGINC3, "/actualStronginc3State.dat", "/expectedStronginc3State.dat", 2)));
/*INSTANTIATE_TEST_CASE_P(CheckRastrigin1,
                        MethodTest,
                        ::testing::Values(
                        pair<string, string>("/actualRastriginState2.dat", "/expectedRastriginState.dat"),
                        pair<string, string>("/actualRastriginState3.dat", "/expectedRastriginState.dat")));
                        */

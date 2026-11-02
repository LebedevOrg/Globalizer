/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//  File:      test_misc.cpp                                               //
//                                                                         //
//  Purpose:   Модульные тесты вспомогательных классов и функций:          //
//             GetFileExtension, Exception, OutputMessage, SolutionResult, //
//             TaskFactory, HDTask, Performance, PriorityQueue, Trial,     //
//             SearchData, Evolvent (копирование), Parameters (печать).    //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include <gtest/gtest.h>

#include <cstdio>
#include <memory>

#include "Common.h"
#include "Exception.h"
#include "Messages.h"
#include "SolutionResult.h"
#include "TaskFactory.h"
#include "HDTask.h"
#include "Performance.h"
#include "Queue.h"
#include "Trial.h"
#include "TrialFactory.h"
#include "SearchData.h"
#include "Evolvent.h"

#include "test_env.h"

using namespace test_env;

// ================================================================================================
//  GetFileExtension
// ================================================================================================

TEST(GetFileExtensionTest, regular_extension)
{
  EXPECT_EQ(GetFileExtension("result.json"), "json");
  EXPECT_EQ(GetFileExtension("result.json", true), ".json");
  EXPECT_EQ(GetFileExtension("archive.tar.gz"), "gz");
  EXPECT_EQ(GetFileExtension("dir/sub/file.txt"), "txt");
  EXPECT_EQ(GetFileExtension("dir\\sub\\file.TXT", true), ".TXT");
}

TEST(GetFileExtensionTest, no_extension)
{
  EXPECT_EQ(GetFileExtension("README"), "");
  EXPECT_EQ(GetFileExtension("file."), "");
  EXPECT_EQ(GetFileExtension("dir.d/file"), "") << "dot belongs to a directory";
  EXPECT_EQ(GetFileExtension("dir.d\\file"), "");
  EXPECT_EQ(GetFileExtension(""), "");
}

// ================================================================================================
//  Exception и OutputMessage
// ================================================================================================

class OutputTest : public ::testing::Test
{
protected:
  std::unique_ptr<TempWorkDir> workDir;
  void SetUp() override { workDir.reset(new TempWorkDir()); }
  void TearDown() override { workDir.reset(); }
};

TEST_F(OutputTest, exception_stores_location_and_description)
{
  ::testing::internal::CaptureStdout();
  Exception e("file.cpp", 42, "Func", "Something failed");
  ::testing::internal::GetCapturedStdout();
  EXPECT_STREQ(e.GetFile(), "file.cpp");
  EXPECT_EQ(e.GetLine(), 42);
  EXPECT_STREQ(e.GetFunction(), "Func");
  EXPECT_STREQ(e.GetDescription(), "Something failed");

  Exception copy(e);
  EXPECT_STREQ(copy.GetFile(), "file.cpp");
  EXPECT_EQ(copy.GetLine(), 42);
  EXPECT_STREQ(copy.GetDescription(), "Something failed");
}

TEST_F(OutputTest, exception_macro_records_current_function)
{
  ::testing::internal::CaptureStdout();
  try
  {
    throw EXCEPTION("macro test");
  }
  catch (const Exception& e)
  {
    EXPECT_STREQ(e.GetDescription(), "macro test");
    EXPECT_GT(e.GetLine(), 0);
    EXPECT_NE(std::string(e.GetFile()).find("test_misc"), std::string::npos);
  }
  ::testing::internal::GetCapturedStdout();
}

TEST_F(OutputTest, exception_printing)
{
  ::testing::internal::CaptureStdout();
  Exception e("file.cpp", 7, "Func", "printing test");
  e.PrintToConsole();
  e.Print("exception_print.txt");
  e.PrintToFile("exception_to_file.txt");
  std::string out = ::testing::internal::GetCapturedStdout();
  EXPECT_NE(out.find("printing test"), std::string::npos);
  EXPECT_TRUE(workDir->Exists("exception_print.txt"));
  EXPECT_TRUE(workDir->Exists("exception_to_file.txt"));
}

TEST_F(OutputTest, important_and_file_messages)
{
  ::testing::internal::CaptureStdout();
  OutputMessage::PrintImportantMessage("important one", "src.cpp", 10, "important.txt");
  OutputMessage::PrintImportantMessage("important two", "", -1, "important.txt");
  OutputMessage::PrintMessageToFile("to file", "src.cpp", 11, "messages.txt");
  OutputMessage::PrintMessageToFile("to file 2", "", -1, "messages.txt");
  std::string out = ::testing::internal::GetCapturedStdout();
  EXPECT_NE(out.find("important one"), std::string::npos);
  EXPECT_EQ(out.find("to file"), std::string::npos) << "PrintMessageToFile must not print to console";
  EXPECT_TRUE(workDir->Exists("important.txt"));
  EXPECT_TRUE(workDir->Exists("messages.txt"));
}

TEST_F(OutputTest, file_streams_open_and_break)
{
  OutputMessage::OpenFileStream("stream.txt");
  OutputMessage::OpenFileStream("stream.txt");  // уже открыт — повторно не создаётся
  OutputMessage::BreakFileStream("stream.txt");
  OutputMessage::BreakFileStream("never_opened.txt");
  EXPECT_TRUE(workDir->Exists("stream.txt"));
}

TEST_F(OutputTest, error_name_falls_back_to_code)
{
  EXPECT_EQ(OutputMessage::ErrorName(123456), "123456");
}

TEST_F(OutputTest, print_error_with_ok_code_does_nothing)
{
  OutputMessage::PrintError(SYSTEM_OK, "file", 1, "expr");
  SUCCEED();
}

TEST_F(OutputTest, print_error_terminates_process)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  EXPECT_EXIT(OutputMessage::PrintError(3, "file.cpp", 5, "x != 0"),
    ::testing::ExitedWithCode(3), "");
}

// ================================================================================================
//  SolutionResult
// ================================================================================================

TEST(SolutionResultTest, empty_result)
{
  SolutionResult r;
  r.BestTrial = nullptr;
  r.IterationCount = 7;
  EXPECT_DOUBLE_EQ(r.GetBestValue(), 0.0);
  EXPECT_EQ(r.GetBestPoint(), nullptr);
  EXPECT_EQ(r.GetIterationsCount(), 7);
}

TEST(SolutionResultTest, result_with_trial)
{
  Trial t;
  t.y[0] = 0.5;
  t.FuncValues[0] = -1.0;
  t.FuncValues[1] = 2.0;
  t.index = 1;
  SolutionResult r;
  r.BestTrial = &t;
  EXPECT_DOUBLE_EQ(r.GetBestValue(), 2.0) << "value of the last calculated function";
  EXPECT_DOUBLE_EQ(r.GetBestPoint()[0], 0.5);
  r.BestTrial = nullptr;
}

// ================================================================================================
//  TaskFactory и HDTask
// ================================================================================================

class TaskFactoryTest : public ::testing::Test
{
protected:
  std::unique_ptr<ProblemFromFunctionPointers> problem;
  int oldStartPointSize = 0;
  std::vector<double> oldStartPoint;

  void SetUp() override
  {
    ResetSolverParameters(2);
    oldStartPointSize = parameters.StartPoint.GetSize();
    for (int i = 0; i < oldStartPointSize; i++)
      oldStartPoint.push_back(parameters.StartPoint[i]);
  }

  void TearDown() override
  {
    parameters.StartPoint.SetSize(oldStartPointSize);
    for (int i = 0; i < oldStartPointSize; i++)
      parameters.StartPoint[i] = oldStartPoint[i];
    ResetSolverParameters(2);
    problem.reset();
  }
};

TEST_F(TaskFactoryTest, creates_regular_task_by_default)
{
  problem.reset(MakeParaboloid(2));
  std::unique_ptr<Task> t(TaskFactory::CreateTask(problem.get(), 0));
  ASSERT_NE(t, nullptr);
  EXPECT_EQ(dynamic_cast<HDTask*>(t.get()), nullptr);
  EXPECT_EQ(t->GetN(), 2);

  std::unique_ptr<Task> empty(TaskFactory::CreateTask());
  ASSERT_NE(empty, nullptr);
  EXPECT_EQ(dynamic_cast<HDTask*>(empty.get()), nullptr);

  std::unique_ptr<Task> clone(TaskFactory::CreateTask(t.get()));
  ASSERT_NE(clone, nullptr);
  EXPECT_EQ(clone->GetN(), 2);
}

TEST_F(TaskFactoryTest, creates_hd_task_for_hd_search)
{
  parameters.TypeSolver = HDSearch;
  problem.reset(MakeParaboloid(2));
  std::unique_ptr<Task> t(TaskFactory::CreateTask(problem.get(), 0));
  EXPECT_NE(dynamic_cast<HDTask*>(t.get()), nullptr);
  std::unique_ptr<Task> empty(TaskFactory::CreateTask());
  EXPECT_NE(dynamic_cast<HDTask*>(empty.get()), nullptr);
}

/**
 HDTask решает подзадачу: координаты [start, start + Dimension) берутся из
 точки метода, остальные — из parameters.StartPoint. StartPoint должен быть
 задан на полную размерность задачи (так его готовил прежний HDSolver).
*/
TEST_F(TaskFactoryTest, hd_task_maps_subproblem_coordinates)
{
  problem.reset(MakeParaboloid(4));  // центр (0.3, -0.2, 0.3, -0.2)
  parameters.Dimension = 2;
  parameters.StartPoint.SetSize(4);
  for (int i = 0; i < 4; i++)
    parameters.StartPoint[i] = 0.0;

  HDTask task(problem.get(), 0);
  task.SetStartParameterNumber(2);
  EXPECT_DOUBLE_EQ(task.GetA()[0], -1.0);
  EXPECT_DOUBLE_EQ(task.GetB()[1], 1.0);

  double y[2] = { 0.3, -0.2 };
  // Точка задачи: (0, 0, 0.3, -0.2) -> 0.3^2 + 0.2^2.
  EXPECT_NEAR(task.CalculateFuncs(y, 0), 0.13, 1e-12);

  double full[4];
  task.TransformPoint(full, y);
  EXPECT_DOUBLE_EQ(full[0], 0.0);
  EXPECT_DOUBLE_EQ(full[2], 0.3);
  EXPECT_DOUBLE_EQ(full[3], -0.2);

  Trial point;
  task.CopyPoint(full, &point);
  EXPECT_DOUBLE_EQ(point.y[0], 0.3);
  EXPECT_DOUBLE_EQ(point.y[1], -0.2);

  EXPECT_THROW(task.CalculateFuncsInManyPoints(y, 0, 1, full), const char*);

  std::unique_ptr<Task> clone(task.Clone());
  HDTask* hdClone = dynamic_cast<HDTask*>(clone.get());
  ASSERT_NE(hdClone, nullptr);
  EXPECT_NEAR(hdClone->CalculateFuncs(y, 0), 0.13, 1e-12);
}

TEST_F(TaskFactoryTest, hd_task_mixed_integer_flags)
{
  problem.reset(new ProblemFromFunctionPointers(2, 1,
    std::vector<double>(2, -1.0), std::vector<double>(2, 1.0), std::vector<int>{3},
    std::vector<std::function<double(const double*)>>(1, [](const double* y) { return y[0] + y[1]; })));
  HDTask task(problem.get(), 0);
  EXPECT_EQ(task.GetNumberOfDiscreteVariable(), 1);
  EXPECT_EQ(task.GetNumberOfContinuousVariable(), 1);
  int startDiscrete = task.GetStartDiscreteVariable();
  task.SetStartIndex(3);
  EXPECT_EQ(task.GetStartDiscreteVariable(), startDiscrete + 3);

  task.UnsetMixedInteger();
  EXPECT_EQ(task.GetNumberOfDiscreteVariable(), 0);
  EXPECT_EQ(task.GetNumberOfContinuousVariable(), 2);
  task.SetMixedInteger();
  EXPECT_EQ(task.GetNumberOfDiscreteVariable(), 1);

  HDTask empty;
  std::unique_ptr<Task> emptyClone(empty.Clone());
  EXPECT_NE(dynamic_cast<HDTask*>(emptyClone.get()), nullptr);
}

// ================================================================================================
//  Performance (таймер)
// ================================================================================================

TEST(PerformanceTest, measures_non_negative_time)
{
  Performance timer;
  timer.Start();
  volatile double s = 0;
  for (int i = 0; i < 100000; i++)
    s += i * 0.5;
  double t1 = timer.GetTime();
  double t2 = timer.GetTime();
  EXPECT_GE(t1, 0.0);
  EXPECT_GE(t2, t1);
}

// ================================================================================================
//  PriorityQueue: операции, не покрытые test_queue.cpp
// ================================================================================================

TEST(PriorityQueueExtraTest, delete_by_value_and_element)
{
  // Значения очереди обязаны быть наследниками QueueBaseData:
  // куча сообщает им адрес их элемента (SetQueueElementa).
  PriorityQueue q(7);
  QueueBaseData a, b, c;
  q.Push(1.0, 1.0, &a);
  QueueElement* eb = q.Push(5.0, 5.0, &b);
  q.Push(3.0, 3.0, &c);
  ASSERT_EQ(q.GetSize(), 3);

  q.DeleteByValue(&a);
  EXPECT_EQ(q.GetSize(), 2);
  QueueBaseData unknown;
  q.DeleteByValue(&unknown);  // отсутствующее значение — без изменений
  EXPECT_EQ(q.GetSize(), 2);

  EXPECT_DOUBLE_EQ(q.FindMax().Key, 5.0);
  q.DeleteElement(eb);
  EXPECT_EQ(q.GetSize(), 1);

  double key = 0;
  void* value = nullptr;
  q.Pop(&key, &value);
  EXPECT_DOUBLE_EQ(key, 3.0);
  EXPECT_EQ(value, &c);
  EXPECT_THROW(q.Pop(&key, &value), Exception);
}

TEST(PriorityQueueExtraTest, trickle_up_after_key_change)
{
  PriorityQueue q(7);
  QueueBaseData a, b;
  q.Push(1.0, 1.0, &a);
  q.Push(2.0, 2.0, &b);
  // Push может переместить элементы кучи: актуальный адрес хранит само значение.
  QueueElement* ea = a.GetQueueElementa();
  ASSERT_NE(ea, nullptr);
  ea->Key = 10.0;
  q.TrickleUp(ea);
  EXPECT_EQ(q.FindMax().pValue, &a);
}

TEST(PriorityQueueExtraTest, resize_clears_queue)
{
  PriorityQueue q(3);
  QueueBaseData a;
  q.Push(1.0, 1.0, &a);
  q.Resize(15);
  EXPECT_EQ(q.GetSize(), 0);
  EXPECT_EQ(q.GetMaxSize(), 15);
  EXPECT_TRUE(q.IsEmpty());
}

// ================================================================================================
//  Trial: копирование
// ================================================================================================

TEST(TrialCopyTest, copy_constructor_and_assignment)
{
  Trial src;
  src.SetX(Extended(0.25));
  src.y[0] = 0.1;
  src.y[1] = 0.2;
  src.FuncValues[0] = 3.5;
  src.index = 0;
  src.K = 4;
  src.iterationNumber = 9;

  Trial copy(src);
  EXPECT_DOUBLE_EQ(copy.X().toDouble(), 0.25);
  EXPECT_DOUBLE_EQ(copy.y[1], 0.2);
  EXPECT_DOUBLE_EQ(copy.FuncValues[0], 3.5);
  EXPECT_EQ(copy.index, 0);
  EXPECT_EQ(copy.K, 4);

  EXPECT_EQ(copy.iterationNumber, 9);

  // operator= (в отличие от конструктора копирования) не переносит
  // iterationNumber и creationTime — поэтому здесь они не проверяются.
  Trial assigned;
  assigned = src;
  EXPECT_DOUBLE_EQ(assigned.X().toDouble(), 0.25);
  EXPECT_DOUBLE_EQ(assigned.y[0], 0.1);
  EXPECT_DOUBLE_EQ(assigned.FuncValues[0], 3.5);
  EXPECT_EQ(assigned.K, 4);
  assigned = assigned;  // самоприсваивание
  EXPECT_DOUBLE_EQ(assigned.y[0], 0.1);
}

// ================================================================================================
//  SearchData: конструктор с размером очереди и вспомогательные методы
// ================================================================================================

TEST(SearchDataExtraTest, constructor_with_queue_size_and_resize)
{
  SearchData data(1, DefaultSearchDataSize, 127);
  EXPECT_EQ(data.GetCount(), 0);
  data.ResizeQueue(255);
  EXPECT_EQ(data.GetCount(), 0);
}

// ================================================================================================
//  Evolvent: копирование
// ================================================================================================

TEST(EvolventCopyTest, copy_constructor_and_assignment_keep_mapping)
{
  ResetSolverParameters(2);
  Evolvent src(2, 10);
  Evolvent copy(src);
  Evolvent assigned(2, 6);
  assigned = src;

  double y1[2], y2[2], y3[2];
  Extended x(0.3);
  src.GetImage(x, y1);
  copy.GetImage(x, y2);
  assigned.GetImage(x, y3);
  EXPECT_DOUBLE_EQ(y1[0], y2[0]);
  EXPECT_DOUBLE_EQ(y1[1], y2[1]);
  EXPECT_DOUBLE_EQ(y1[0], y3[0]);
  EXPECT_DOUBLE_EQ(y1[1], y3[1]);
}

// ================================================================================================
//  Parameters: печать
// ================================================================================================

TEST(ParametersPrintTest, print_to_console_and_file)
{
  TempWorkDir dir;
  ::testing::internal::CaptureStdout();
  parameters.PrintParameters();
  std::string out = ::testing::internal::GetCapturedStdout();
  EXPECT_NE(out.find("Dimension"), std::string::npos);

  FILE* f = fopen("params.txt", "w");
  ASSERT_NE(f, nullptr);
  parameters.PrintParametersToFile(f);
  fclose(f);
  EXPECT_NE(dir.Read("params.txt").find("Dimension"), std::string::npos);
}

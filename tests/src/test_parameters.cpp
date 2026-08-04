/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//  File:      test_parameters.cpp                                         //
//                                                                         //
//  Purpose:   Модульные тесты для класса Parameters                       //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

/**
\file test_parameters.cpp
\brief Модульные тесты для класса Parameters (набор параметров системы оптимизации)

Замечания по окружению:
  * Init() вызывается с isMPIInit = false, чтобы не требовать MPI_Init/MPI_Comm_*.
  * Тестовый бинарник должен линковаться с теми же MPI/OpenMP-библиотеками,
    что и основной таргет параметров (см. Parameters.cpp: <mpi.h>, <omp.h>).
  * Некоторые тесты фиксируют ФАКТИЧЕСКОЕ поведение реализации (в т.ч. известные
    особенности парсинга), а не «идеальный» контракт — это отмечено в комментариях.
*/

#include "Parameters.h"

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <cstring>

using namespace std;

// ------------------------------------------------------------------------------------------------
// Вспомогательная обёртка: строит argv из вектора строк и корректно вызывает Init().
// Хранит копии строк, чтобы указатели в argv жили всё время использования.
// ------------------------------------------------------------------------------------------------
class ArgvBuilder
{
public:
  explicit ArgvBuilder(const std::vector<std::string>& args)
  {
    mStorage = args;
    mArgv.reserve(mStorage.size() + 1);
    for (auto& s : mStorage)
      mArgv.push_back(const_cast<char*>(s.c_str()));
    mArgv.push_back(nullptr); // терминальный nullptr на всякий случай
  }

  int    argc() { return static_cast<int>(mStorage.size()); }
  char** argv() { return mArgv.data(); }

private:
  std::vector<std::string> mStorage;
  std::vector<char*>       mArgv;
};

// ------------------------------------------------------------------------------------------------
// Fixture: свежий экземпляр Parameters на каждый тест.
// Используем ЛОКАЛЬНЫЙ объект (а не глобальный ::parameters), чтобы тесты были изолированы.
// ------------------------------------------------------------------------------------------------
class ParametersTest : public ::testing::Test
{
protected:
  Parameters* p = nullptr;

  void SetUp() override
  {
    p = new Parameters();
    // Инициализируем набор значениями по умолчанию (эмулируем то, что делает Init).
    // SetDefaultParameters защищён -> инициализируем через Init с минимальным argv.
    std::vector<std::string> args = { "test_app" };
    ArgvBuilder ab(args);
    p->Init(ab.argc(), ab.argv(), /*isMPIInit=*/false);
  }

  void TearDown() override
  {
    delete p;
    p = nullptr;
  }

  // Утилита: пере-инициализировать p заданными аргументами командной строки.
  void ReInit(const std::vector<std::string>& args)
  {
    delete p;
    p = new Parameters();
    ArgvBuilder ab(args);
    p->Init(ab.argc(), ab.argv(), /*isMPIInit=*/false);
  }
};

/* ======================================================================== *\
**  Конструирование                                                         **
\* ======================================================================== */

///  создание Parameters не бросает исключение
TEST(Parameters_Ctor, can_create_default)
{
  ASSERT_NO_THROW({ Parameters p; });
}

///  serializer создаётся в конструкторе (не nullptr)
TEST(Parameters_Ctor, serializer_is_allocated)
{
  Parameters p;
  ASSERT_NE(nullptr, p.serializer);
}

///  версия проинициализирована ожидаемой строкой
TEST(Parameters_Ctor, version_is_set)
{
  Parameters p;
  ASSERT_EQ("0.1.7", p.version);
}

///  Init не бросает исключение при пустой командной строке (без MPI)
TEST(Parameters_Init, init_no_throw_empty_args)
{
  Parameters p;
  std::vector<std::string> args = { "app" };
  ArgvBuilder ab(args);
  ASSERT_NO_THROW(p.Init(ab.argc(), ab.argv(), false));
}

/* ======================================================================== *\
**  Значения по умолчанию (SetDefaultParameters через Init)                 **
\* ======================================================================== */

///  скалярные числовые параметры имеют документированные значения по умолчанию
TEST_F(ParametersTest, defaults_numeric_scalars)
{
  ASSERT_DOUBLE_EQ(4.0, (double)p->r);
  ASSERT_DOUBLE_EQ(0.01, (double)p->Epsilon);
  ASSERT_DOUBLE_EQ(0.08, (double)p->alpha);
  ASSERT_DOUBLE_EQ(0.0, (double)p->rDynamic);
  ASSERT_DOUBLE_EQ(0.01, (double)p->rEps);

  ASSERT_EQ(1, (int)p->Dimension);
  ASSERT_EQ(1, (int)p->NumPoints);
  ASSERT_EQ(10, (int)p->m);
  ASSERT_EQ(1000000, (int)p->MaxNumOfPoints);
  ASSERT_EQ(1, (int)p->NumThread);
  ASSERT_EQ(32, (int)p->SizeInBlock);
  ASSERT_EQ(-1, (int)p->DeviceCount);
}

///  enum-параметры имеют документированные значения по умолчанию
TEST_F(ParametersTest, defaults_enum_params)
{
  ASSERT_EQ(StandartMethod, (ETypeMethod)p->TypeMethod);
  ASSERT_EQ(OMP, (ETypeCalculation)p->TypeCalculation);
  ASSERT_EQ(SynchronousProcess, (ETypeProcess)p->TypeProcess);
  ASSERT_EQ(mpBase, (EMapType)p->MapType);
  ASSERT_EQ(SingleSearch, (ETypeSolver)p->TypeSolver);
  ASSERT_EQ(Accuracy, (EStopCondition)p->StopCondition);
  ASSERT_EQ(None, (ELocalMethodScheme)p->LocalRefineSolution);
  ASSERT_EQ(HookeJeeves, (ETypeLocalMethod)p->TypeLocalMethod);
  ASSERT_EQ(WithoutLocalTuning, (ELocalTuningType)p->LocalTuningType);
  ASSERT_EQ(Off, (ESeparableMethodType)p->SepS);
  ASSERT_EQ(Evenly, (ETypeDistributionStartingPoints)p->TypeDistributionStartingPoints);
}

///  булевы/флаговые параметры имеют документированные значения по умолчанию
TEST_F(ParametersTest, defaults_bool_flags)
{
  ASSERT_EQ(false, (bool)p->HELP);
  ASSERT_EQ(false, (bool)p->IsPlot);
  ASSERT_EQ(false, (bool)p->IsPrintFile);
  ASSERT_EQ(true, (bool)p->IsPrintResultToConsole);
  ASSERT_EQ(false, (bool)p->IsSetDevice);
  ASSERT_EQ(false, (bool)p->IsUseStartPoint);
  ASSERT_EQ(false, (bool)p->RndS);
  ASSERT_EQ(false, (bool)p->AutomaticParametersSetting);
}

///  локальные параметры уточнения имеют значения по умолчанию
TEST_F(ParametersTest, defaults_local_refine)
{
  ASSERT_EQ(10000, (int)p->LocalIteration);
  ASSERT_DOUBLE_EQ(0.0001, (double)p->LocalVerificationEpsilon);
  ASSERT_EQ(1, (int)p->LocalVerificationNumPoint);
  ASSERT_EQ(0, (int)p->LocalMix);
  ASSERT_DOUBLE_EQ(15.0, (double)p->LocalAlpha);
  ASSERT_EQ(5, (int)p->MaxCountLocalPoint);
}

///  параметры логирования/визуализации имеют значения по умолчанию
TEST_F(ParametersTest, defaults_logging_and_plot)
{
  ASSERT_EQ(100000, (int)p->StepPrintMessages);
  ASSERT_EQ(1000000, (int)p->StepSavePoint);
  ASSERT_EQ(300, (int)p->PlotGridSize);
  ASSERT_EQ(25, (int)p->Levels);
  ASSERT_EQ(100, (int)p->ObjectiveGridSize);
  ASSERT_EQ(200, (int)p->ConstraintsGridSize);
  ASSERT_EQ(LevelLayers, (FigureTypes)p->FigureType);
  ASSERT_EQ(ObjectiveFunction, (CalcsTypes)p->CalcsType);
}

///  критерий остановки: значения по умолчанию
TEST_F(ParametersTest, defaults_stop_condition)
{
  ASSERT_EQ(100, (int)p->MaxIterationsWithoutImprovement);
  ASSERT_EQ(1000000, (int)p->IterationsCount);
}

/* ======================================================================== *\
**  Разбор командной строки (Init) — скалярные параметры                    **
\* ======================================================================== */

///  -N задаёт размерность
TEST_F(ParametersTest, parse_dimension)
{
  ReInit({ "app", "-N", "5" });
  ASSERT_EQ(5, (int)p->Dimension);
}

///  -r задаёт надёжность метода (double)
TEST_F(ParametersTest, parse_r)
{
  ReInit({ "app", "-r", "3.5" });
  ASSERT_DOUBLE_EQ(3.5, (double)p->r);
}

///  -E задаёт точность (double)
TEST_F(ParametersTest, parse_epsilon)
{
  ReInit({ "app", "-E", "0.001" });
  ASSERT_NEAR(0.001, (double)p->Epsilon, 1e-9);
}

///  -np задаёт число точек за итерацию
TEST_F(ParametersTest, parse_num_points)
{
  ReInit({ "app", "-np", "8" });
  ASSERT_EQ(8, (int)p->NumPoints);
}

///  -m задаёт плотность развёртки
TEST_F(ParametersTest, parse_m)
{
  ReInit({ "app", "-m", "12" });
  ASSERT_EQ(12, (int)p->m);
}

///  -MaxNP задаёт максимальное число испытаний
TEST_F(ParametersTest, parse_max_num_of_points)
{
  ReInit({ "app", "-MaxNP", "500000" });
  ASSERT_EQ(500000, (int)p->MaxNumOfPoints);
}

///  несколько параметров сразу
TEST_F(ParametersTest, parse_multiple_options)
{
  ReInit({ "app", "-N", "3", "-r", "5.0", "-np", "4", "-m", "8" });
  ASSERT_EQ(3, (int)p->Dimension);
  ASSERT_DOUBLE_EQ(5.0, (double)p->r);
  ASSERT_EQ(4, (int)p->NumPoints);
  ASSERT_EQ(8, (int)p->m);
}

/* ======================================================================== *\
**  Разбор командной строки — enum-параметры                                **
\* ======================================================================== */

///  -tm по имени задаёт тип метода
TEST_F(ParametersTest, parse_type_method_by_name)
{
  ReInit({ "app", "-tm", "IntegerMethod" });
  ASSERT_EQ(IntegerMethod, (ETypeMethod)p->TypeMethod);
}

///  -tm по числовому коду задаёт тип метода
TEST_F(ParametersTest, parse_type_method_by_code)
{
  ReInit({ "app", "-tm", "2" });
  ASSERT_EQ(RSAMethod, (ETypeMethod)p->TypeMethod);
}

///  -stopCond задаёт критерий остановки
TEST_F(ParametersTest, parse_stop_condition)
{
  ReInit({ "app", "-stopCond", "OptimumVicinity2" });
  ASSERT_EQ(OptimumVicinity2, (EStopCondition)p->StopCondition);
}

///  -ts задаёт тип решателя
TEST_F(ParametersTest, parse_type_solver)
{
  ReInit({ "app", "-ts", "HDSearch" });
  ASSERT_EQ(HDSearch, (ETypeSolver)p->TypeSolver);
}

/* ======================================================================== *\
**  Разбор командной строки — булевы и флаговые параметры                    **
\* ======================================================================== */

///  булев параметр -isPRC можно выставить в false
TEST_F(ParametersTest, parse_bool_false)
{
  // По умолчанию true -> выставим false
  ReInit({ "app", "-isPRC", "false" });
  ASSERT_EQ(false, (bool)p->IsPrintResultToConsole);
}

///  булев параметр -IsPF можно выставить в true
TEST_F(ParametersTest, parse_bool_true)
{
  ReInit({ "app", "-IsPF", "true" });
  ASSERT_EQ(true, (bool)p->IsPrintFile);
}

///  флаг -HELP включается фактом присутствия ключа
TEST_F(ParametersTest, parse_flag_help)
{
  ReInit({ "app", "-HELP" });
  ASSERT_EQ(true, (bool)p->HELP);
}

///  флаг -PLOT включается фактом присутствия ключа
TEST_F(ParametersTest, parse_flag_plot)
{
  ReInit({ "app", "-PLOT" });
  ASSERT_EQ(true, (bool)p->IsPlot);
}

/* ======================================================================== *\
**  Разбор командной строки — строковые и массивные параметры               **
\* ======================================================================== */

///  -Comment задаёт строковый параметр
TEST_F(ParametersTest, parse_string_comment)
{
  ReInit({ "app", "-Comment", "my_experiment" });
  ASSERT_EQ("my_experiment", p->Comment.ToString());
}

///  -lib задаёт путь к библиотеке задачи
TEST_F(ParametersTest, parse_string_libpath)
{
  ReInit({ "app", "-lib", "mylib.dll" });
  ASSERT_EQ("mylib.dll", p->LibPath.ToString());
  ASSERT_EQ(true, p->LibPath.GetIsChange());
}

///  -dt задаёт массив размерностей подзадач (разделитель "_")
TEST_F(ParametersTest, parse_ints_array_dim_in_task)
{
  ReInit({ "app", "-dt", "2_3_1_1" });
  ASSERT_EQ(2, p->DimInTask.GetData()[0]);
  ASSERT_EQ(3, p->DimInTask.GetData()[1]);
  ASSERT_EQ(1, p->DimInTask.GetData()[2]);
  ASSERT_EQ(1, p->DimInTask.GetData()[3]);
}

///  -fsm задаёт массив множителей знака функции (double, разделитель "_")
TEST_F(ParametersTest, parse_doubles_array_fsm)
{
  ReInit({ "app", "-fsm", "1.0_-1.0_1.0_1.0" });
  ASSERT_NEAR(1.0, p->FunctionSignMultiplier.GetData()[0], 1e-9);
  ASSERT_NEAR(-1.0, p->FunctionSignMultiplier.GetData()[1], 1e-9);
  ASSERT_NEAR(1.0, p->FunctionSignMultiplier.GetData()[2], 1e-9);
}

/* ======================================================================== *\
**  Устойчивость парсинга (негативные / граничные случаи)                    **
\* ======================================================================== */

///  неизвестный ключ не ломает Init (набор остаётся валидным)
TEST_F(ParametersTest, unknown_option_does_not_break_init)
{
  ASSERT_NO_THROW(ReInit({ "app", "-thisOptionDoesNotExist", "123" }));
  // Известные параметры при этом сохраняют значения по умолчанию
  ASSERT_EQ(1, (int)p->Dimension);
}

///  повторно указанный ключ: побеждает последнее значение
TEST_F(ParametersTest, duplicate_option_last_wins)
{
  ReInit({ "app", "-N", "2", "-N", "7" });
  ASSERT_EQ(7, (int)p->Dimension);
}

///  порядок ключей не влияет на результат
TEST_F(ParametersTest, option_order_independent)
{
  ReInit({ "app", "-r", "6.0", "-N", "4" });
  ASSERT_EQ(4, (int)p->Dimension);
  ASSERT_DOUBLE_EQ(6.0, (double)p->r);
}

/* ======================================================================== *\
**  CheckValueParameters — бизнес-логика валидации                          **
\* ======================================================================== */

///  NumPoints <= 0 нормализуется в 1 внутри CheckValueParameters
TEST_F(ParametersTest, check_value_num_points_nonpositive_becomes_one)
{
  // Выставляем некорректное значение и запускаем проверку.
  p->NumPoints = 0;
  p->CheckValueParameters();
  ASSERT_EQ(1, (int)p->NumPoints);

  p->NumPoints = -5;
  p->CheckValueParameters();
  ASSERT_EQ(1, (int)p->NumPoints);
}

///  для не-MPI типа расчёта требуется 1 процесс (mNeedMPIProcessorCount)
///  проверяем косвенно: CheckValueParameters не бросает и NumPoints корректен
TEST_F(ParametersTest, check_value_omp_keeps_num_points)
{
  p->TypeCalculation = OMP;
  p->NumPoints = 4;
  ASSERT_NO_THROW(p->CheckValueParameters());
  ASSERT_EQ(4, (int)p->NumPoints);
}

/* ======================================================================== *\
**  Геттеры процессов (без реального MPI)                                    **
\* ======================================================================== */

///  без MPI-инициализации GetProcRank == 0
TEST_F(ParametersTest, proc_rank_is_zero_without_mpi)
{
  ASSERT_EQ(0, p->GetProcRank());
}

///  без MPI-инициализации GetProcNum == 1
TEST_F(ParametersTest, proc_num_is_one_without_mpi)
{
  ASSERT_EQ(1, p->GetProcNum());
}

///  IsProblem() у Parameters возвращает false
TEST_F(ParametersTest, is_problem_returns_false)
{
  ASSERT_EQ(false, p->IsProblem());
}

/* ======================================================================== *\
**  Формирование имён файлов                                                 **
\* ======================================================================== */

///  GetPlotFileName возвращает явно заданное имя, если оно задано
TEST_F(ParametersTest, plot_file_name_explicit)
{
  ReInit({ "app", "-PlotFileName", "picture.png" });
  ASSERT_EQ("picture.png", p->GetPlotFileName());
}

///  GetPlotFileName при пустом имени формирует автогенерируемое имя (.png)
TEST_F(ParametersTest, plot_file_name_autogenerated_has_png)
{
  // Имя не задано -> должна вернуться строка, оканчивающаяся на ".png"
  std::string name = p->GetPlotFileName();
  ASSERT_FALSE(name.empty());
  ASSERT_GE(name.size(), 4u);
  ASSERT_EQ(".png", name.substr(name.size() - 4));
  ASSERT_EQ(0u, name.rfind("globalizer_", 0)); // начинается с "globalizer_"
}

///  GetJsonFileName возвращает явно заданное имя, если оно задано
TEST_F(ParametersTest, json_file_name_explicit)
{
  ReInit({ "app", "-fs", "data.json" });
  ASSERT_EQ("data.json", p->GetJsonFileName());
}

///  GetJsonFileName при пустом имени формирует автогенерируемое имя (.json)
TEST_F(ParametersTest, json_file_name_autogenerated_has_json)
{
  std::string name = p->GetJsonFileName();
  ASSERT_FALSE(name.empty());
  ASSERT_GE(name.size(), 5u);
  ASSERT_EQ(".json", name.substr(name.size() - 5));
  ASSERT_EQ(0u, name.rfind("globalizer_", 0));
}

/* ======================================================================== *\
**  Копирующий конструктор                                                   **
\* ======================================================================== */

///  копирующий конструктор переносит значения параметров
TEST_F(ParametersTest, copy_constructor_copies_values)
{
  ReInit({ "app", "-N", "6", "-r", "3.3", "-np", "5" });

  Parameters copy(*p);

  ASSERT_EQ(6, (int)copy.Dimension);
  ASSERT_DOUBLE_EQ(3.3, (double)copy.r);
  ASSERT_EQ(5, (int)copy.NumPoints);
}

///  копия независима от оригинала (изменение оригинала не меняет копию)
TEST_F(ParametersTest, copy_constructor_is_independent)
{
  ReInit({ "app", "-N", "6" });
  Parameters copy(*p);

  p->Dimension = 99;

  ASSERT_EQ(6, (int)copy.Dimension);   // копия не изменилась
  ASSERT_EQ(99, (int)p->Dimension);     // оригинал изменился
}

///  MyLevel и MyMap переносятся копирующим конструктором
TEST_F(ParametersTest, copy_constructor_copies_mylevel_mymap)
{
  p->MyLevel = 3;
  p->MyMap = 7;

  Parameters copy(*p);

  ASSERT_EQ(3, copy.MyLevel);
  ASSERT_EQ(7, copy.MyMap);
}

/* ======================================================================== *\
**  Прочее                                                                   **
\* ======================================================================== */

///  GetMaxNumOMP возвращает положительное число потоков
TEST_F(ParametersTest, get_max_num_omp_positive)
{
  ASSERT_GE(p->GetMaxNumOMP(), 1);
}

///  повторная инициализация теми же аргументами даёт те же значения (идемпотентность)
TEST_F(ParametersTest, reinit_is_deterministic)
{
  ReInit({ "app", "-N", "4", "-r", "2.5" });
  int    dim1 = (int)p->Dimension;
  double r1 = (double)p->r;

  ReInit({ "app", "-N", "4", "-r", "2.5" });
  ASSERT_EQ(dim1, (int)p->Dimension);
  ASSERT_DOUBLE_EQ(r1, (double)p->r);
}
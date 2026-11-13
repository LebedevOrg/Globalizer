/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//  File:      test_env.h                                                  //
//                                                                         //
//  Purpose:   Общие утилиты для модульных тестов верхнего уровня          //
//             (Solver, задачи, сериализация, локальные методы):            //
//             временный рабочий каталог, сброс глобальных parameters,     //
//             фабрика простых тестовых задач.                              //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include <gtest/gtest.h>

#include <cmath>
#include <filesystem>
#include <fstream>
#include <functional>
#include <string>
#include <vector>

#include "Parameters.h"
#include "Calculation.h"
#include "Extended.h"
#include "ProblemFromFunctionPointers.h"

#include "test_reset.h"

namespace test_env
{

/**
 * RAII: на время теста переключает текущий каталог во временную папку,
 * чтобы файлы, которые пишет решатель (optim.dat, логи, json), не засоряли
 * каталог с бинарниками. В деструкторе возвращает каталог и удаляет папку.
 */
class TempWorkDir
{
public:
  TempWorkDir()
  {
    namespace fs = std::filesystem;
    mOld = fs::current_path();
    RemoveStaleDirsOnce();
    static int counter = 0;
    const ::testing::TestInfo* info = ::testing::UnitTest::GetInstance()->current_test_info();
    std::string name = info ? std::string(info->test_suite_name()) + "_" + info->name() : "test";
    for (char& c : name)
      if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_') c = '_';
    mDir = fs::temp_directory_path() / ("globalizer_tests_" + name + "_" + std::to_string(++counter));
    std::error_code ec;
    fs::remove_all(mDir, ec);
    fs::create_directories(mDir);
    fs::current_path(mDir);
  }

  ~TempWorkDir()
  {
    namespace fs = std::filesystem;
    std::error_code ec;
    fs::current_path(mOld, ec);
    fs::remove_all(mDir, ec);
  }

  const std::filesystem::path& Path() const { return mDir; }

  bool Exists(const std::string& file) const
  {
    return std::filesystem::exists(mDir / file);
  }

  std::string Read(const std::string& file) const
  {
    std::ifstream in(mDir / file);
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  }

private:
  std::filesystem::path mOld;
  std::filesystem::path mDir;

  /// OutputMessage держит лог-файлы открытыми до конца процесса, поэтому
  /// часть папок не удаляется сразу. Убираем остатки прошлых запусков.
  static void RemoveStaleDirsOnce()
  {
    namespace fs = std::filesystem;
    static bool done = false;
    if (done) return;
    done = true;
    std::error_code ec;
    for (auto it = fs::directory_iterator(fs::temp_directory_path(), ec);
         !ec && it != fs::directory_iterator(); it.increment(ec))
    {
      const std::string name = it->path().filename().string();
      if (name.rfind("globalizer_tests_", 0) == 0)
      {
        std::error_code rmEc;
        fs::remove_all(it->path(), rmEc);
      }
    }
  }
};

/// Сбрасывает кэш вычислителя и статические поля Calculation.
inline void ResetCalculationState()
{
  ResetCalculationStatics();
  Calculation::countCalculation = 0;
  Calculation::isStartComputingAway = true;
}

/**
 * Приводит глобальные parameters к базовому состоянию для тестов решателя.
 * Вызывается и в SetUp, и в TearDown, чтобы тесты не влияли друг на друга
 * и на остальные наборы тестов.
 */
inline void ResetSolverParameters(int dimension)
{
  ResetParametersToMethodDefaults(dimension);
  ResetCalculationState();

  parameters.IsPrintResultToConsole = false;
  parameters.IsUseExtendedConsole = false;
  parameters.IsPrintFile = false;
  parameters.IsPlot = false;
  parameters.ResulLog = "000";
  parameters.IterationsCount = 10000;
  parameters.MaxNumOfPoints = 10000;
  parameters.AutomaticParametersSetting = false;
  parameters.TypeSolver = SingleSearch;
  parameters.TypeLocalMethod = HookeJeeves;
  parameters.LocalIteration = 1000;
  parameters.StepPrintMessages = 100000;
  parameters.StepSavePoint = 1000000;
  parameters.LtXi = 1e-6;
  parameters.r = 2.3;
}

/// Параболоид sum (x_i - c_i)^2 на [-1, 1]^dim, минимум 0 в точке c.
inline std::vector<double> ParaboloidCenter(int dim)
{
  std::vector<double> c(dim);
  for (int i = 0; i < dim; i++)
    c[i] = (i % 2 == 0) ? 0.3 : -0.2;
  return c;
}

inline double Paraboloid(const double* y, int dim)
{
  std::vector<double> c = ParaboloidCenter(dim);
  double s = 0.0;
  for (int i = 0; i < dim; i++)
    s += (y[i] - c[i]) * (y[i] - c[i]);
  return s;
}

/// Задача-параболоид (одна функция), опционально с известным оптимумом.
inline ProblemFromFunctionPointers* MakeParaboloid(int dim, bool withOptimum = true)
{
  return new ProblemFromFunctionPointers(
    dim, std::vector<double>(dim, -1.0), std::vector<double>(dim, 1.0),
    std::vector<std::function<double(const double*)>>(1, [dim](const double* y)
      { return Paraboloid(y, dim); }),
    withOptimum, 0.0, withOptimum ? ParaboloidCenter(dim) : std::vector<double>());
}

/**
 * Задача-параболоид с поддержкой групповых вычислений (для CPUMultiPoint):
 * values[i] = f(y + i*dim).
 */
inline ProblemFromFunctionPointers* MakeMultiPointParaboloid(int dim)
{
  return new ProblemFromFunctionPointers(
    dim, std::vector<double>(dim, -1.0), std::vector<double>(dim, 1.0),
    [dim](const double* y, int) { return Paraboloid(y, dim); },
    [dim](double* y, int, int& numPoints, double* values)
      {
        for (int i = 0; i < numPoints; i++)
          values[i] = Paraboloid(y + i * dim, dim);
      },
    1, true, 0.0, ParaboloidCenter(dim));
}

/**
 * Задача с ограничением: g(y) = 0.1 - y0 <= 0 (т.е. y0 >= 0.1),
 * критерий f(y) = y0^2 + y1^2. Условный минимум 0.01 в точке (0.1, 0).
 */
inline ProblemFromFunctionPointers* MakeConstrainedProblem()
{
  return new ProblemFromFunctionPointers(
    2, std::vector<double>(2, -1.0), std::vector<double>(2, 1.0),
    [](const double* y, int fNumber)
      {
        if (fNumber == 0)
          return 0.1 - y[0];
        return y[0] * y[0] + y[1] * y[1];
      },
    2, true, 0.01, std::vector<double>{0.1, 0.0});
}

} // namespace test_env

// ============================================================================
//  Globalizer_IntegrationTests
//  ---------------------------------------------------------------------------
//  Решает набор задач целиком (Solver) и проверяет результат.
//
//  Поддерживаются два формата описания случаев (JSON-массив):
//
//  1) Прежний формат tests.json: поля name/type/dimension/optimum/point/r...
//     (задачи Hill, Rastrigin*, StronginC3_Lambda, заданные в коде).
//
//  2) Формат, перенесённый из тестов iOpt (iopt_tests.json): есть поле
//     "problem". Параметры решателя задаются как аргументы командной строки
//     ("args"), ровно как у примеров (SampleMain и др.). Проверки — как в
//     iOpt (test_solving_test_problems.py и др.):
//       * каждая координата найденной точки отличается от известного
//         оптимума не более чем на eps*(b-a) ("point_eps");
//       * число испытаний совпадает с эталоном ("trials", "trials_delta");
//       * при необходимости — значение функции ("optimum", "value_tolerance").
//
//  Запуск:
//    Globalizer_IntegrationTests [file.json] [--group G] [--filter S]
//                                [--lib-dir DIR] [--list]
// ============================================================================

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "json.hpp"

// ПРИМЕЧАНИЕ: макрос _GLOBALIZER_BENCHMARKS (и _GLOBALIZER_GCGEN при сборке с
// GCGen) задаётся в CMake, поэтому здесь его повторно НЕ определяем.

#include "Globalizer.h"
#include "Solver.h"
#include "ProblemInterface.h"
#include "Parameters.h"
#include "Trial.h"
#ifdef _GLOBALIZER_BENCHMARKS
#include "IGlobalOptimizationProblem.h"
#include "GlobalOptimizationProblemManager.h"
#include "GlobalizerBenchmarksProblem.h"
#endif // _GLOBALIZER_BENCHMARKS
#ifdef _GLOBALIZER_GCGEN
#include "GlobalizerProblemGCGen.h"
#include "GKLS/GKLSProblem.hpp"
#include "Grishagin/grishagin_function.hpp"
#include "Hill/HillProblem.hpp"
#include "Shekel/ShekelProblem.hpp"
#endif // _GLOBALIZER_GCGEN

using json = nlohmann::json;

static const double kPi = 3.14159265358979323846;

// ==============================================================================
// Вспомогательные функции
// ==============================================================================

void PrintPoint(const double* p, int dim)
{
  std::cout << "[";
  for (int i = 0; i < dim; i++)
  {
    std::cout << std::fixed << std::setprecision(6) << p[i] << (i == dim - 1 ? "" : ", ");
  }
  std::cout << "]";
}

double CalculateDistance(const double* p1, const std::vector<double>& p2, int dim)
{
  double sum = 0.0;
  for (int i = 0; i < dim; i++)
  {
    double diff = p1[i] - p2[i];
    sum += diff * diff;
  }
  return std::sqrt(sum);
}

/// Текущий случай помечен "xfail": его отказы печатаются как [fail*], а итог
/// подводит ApplyXFail ([XFAIL]/[XPASS]) — чтобы поиск "[FAIL]" находил только
/// настоящие провалы.
static bool gCaseIsXFail = false;
static const char* FailLabel() { return gCaseIsXFail ? "fail*" : "FAIL"; }

/// Каталог с библиотеками задач (по умолчанию — текущий, т.е. _bin).
static std::string gLibDir = ".";

/// Начальные значения статических полей Extended (как при запуске примера).
static ExtendedTypeID gInitialExtendedType = etDouble;
static double gInitialExtendedPrecision = 0.01;

/// Исход одного случая. XFail/XPass — для случаев с пометкой "xfail"
/// (известный отказ): XFail не валит прогон, XPass сообщает, что пометку пора снять.
enum class Outcome { Pass, Fail, Skip, XFail, XPass };

// ==============================================================================
// Прежний формат (tests.json)
// ==============================================================================
IProblem* CreateProblem(const json& config, int dimension)
{
  std::string name = config["name"].get<std::string>();
  std::string type = config.value("type", "Benchmark");

  if (type == "Simple")
  {
    if (name == "Hill")
    {
      return new ProblemFromFunctionPointers(
        dimension, std::vector<double>(dimension, 0.0), std::vector<double>(dimension, 1.0),
        std::vector<std::function<double(const double*)>>(1, [dimension](const double* y) {
          double sum = 0.0;
          for (int j = 0; j < dimension; j++) sum += (y[j] - 0.5) * (y[j] - 0.5);
          return sum;
          }), true, 0.0, std::vector<double>(dimension, 0.5)
            );
    }
    else if (name.find("Rastrigin") != std::string::npos)
    {
      return new ProblemFromFunctionPointers(
        dimension, std::vector<double>(dimension, -2.2), std::vector<double>(dimension, 1.8),
        std::vector<std::function<double(const double*)>>(1, [dimension](const double* y) {
          double sum = 0.0;
          for (int j = 0; j < dimension; j++) sum += y[j] * y[j] - 10.0 * cos(2.0 * kPi * y[j]) + 10.0;
          return sum;
          }), true, 0.0, std::vector<double>(dimension, 0.0)
            );
    }
    else if (name == "StronginC3_Lambda")
    {
      return new ProblemFromFunctionPointers(
        dimension, { 0.0, -1.0 }, { 4.0, 3.0 },
        std::vector<std::function<double(const double*)>>({
          [](const double* y) { return 0.01 * ((y[0] - 2.2) * (y[0] - 2.2) + (y[1] - 1.2) * (y[1] - 1.2) - 2.25); },
          [](const double* y) { return 100.0 * (1.0 - ((y[0] - 2.0) / 1.2) * ((y[0] - 2.0) / 1.2) - (y[1] / 2.0) * (y[1] / 2.0)); },
          [](const double* y) { return 10.0 * (y[1] - 1.5 - 1.5 * sin(6.283 * (y[0] - 1.75))); },
          [](const double* y) {
            double t1 = pow(0.5 * y[0] - 0.5, 4.0);
            double t2 = pow(y[1] - 1.0, 4.0);
            return -((1.5 * y[0] * y[0] * exp(1.0 - y[0] * y[0] - 20.25 * (y[0] - y[1]) * (y[0] - y[1]))) + t1 * t2 * exp(2.0 - t1 - t2));
          }
          }), true, -1.489444, { 0.941176, 0.941176 }
          );
    }
  }
  return nullptr;
}

bool RunLegacyTest(const json& config)
{
  std::string name = config["name"].get<std::string>();
  int dimension = config["dimension"].get<int>();

  std::cout << "------------------------------------------------" << std::endl;
  std::cout << "[RUN] Testing: " << name << " (Dimension: " << dimension << ")" << std::endl;

  IProblem* manualProblem = CreateProblem(config, dimension);
  if (!manualProblem) { std::cout << "      Result: SKIP" << std::endl; return true; }

  char progName[] = "tester";
  char* dummyArgv[] = { progName, nullptr };

  try
  {
    parameters.Init(1, dummyArgv, false);
    parameters.Dimension = dimension;
    parameters.r = config["r"].get<double>();
    parameters.Epsilon = config.value("epsilon", 0.01);

    parameters.m = 10;

    parameters.IsPrintResultToConsole = false;
    parameters.DisablePrintParameters = true;

    std::string multipliers = "1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 "
      "1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0";
    parameters.SetVal("functionSignMultiplier", multipliers);
  }
  catch (...) { std::cout << "      Result: FAIL (Params)" << std::endl; return false; }

  manualProblem->Initialize();

  Solver* solver = new Solver(manualProblem);

  bool passed = true;
  try
  {
    if (solver->Solve() != SYSTEM_OK)
    {
      std::cout << "      Result: FAIL (Solver Error)" << std::endl;
      delete solver; return false;
    }
  }
  catch (...)
  {
    std::cout << "      Result: FAIL (Exception)" << std::endl;
    delete solver; return false;
  }

  SolutionResult* result = solver->GetSolutionResult();
  if (result && result->BestTrial)
  {
    int totalFuncs = manualProblem->GetNumberOfFunctions();
    double actualValue = result->BestTrial->FuncValues[totalFuncs - 1];
    double expectedValue = config["optimum"].get<double>();
    double tolerance = config["tolerance"].get<double>();

    std::cout << "      Value:    Expected " << std::fixed << std::setprecision(6) << expectedValue
      << ", Actual " << actualValue
      << " (Diff: " << std::scientific << std::abs(actualValue - expectedValue) << ")" << std::endl;

    if (std::abs(actualValue - expectedValue) > tolerance) passed = false;

    if (config.contains("point"))
    {
      std::vector<double> expectedPoint = config["point"].get<std::vector<double>>();
      double dist = CalculateDistance(result->BestTrial->y, expectedPoint, dimension);

      std::cout << "      Point:    Expected "; PrintPoint(expectedPoint.data(), dimension);
      std::cout << "\n                Actual   "; PrintPoint(result->BestTrial->y, dimension);
      std::cout << " (Dist: " << std::fixed << std::setprecision(6) << dist << ")" << std::endl;

      if (dist > 0.5) passed = false;
    }

    if (config.contains("expected_iterations"))
    {
      int expectedIters = config["expected_iterations"].get<int>();
      std::cout << "      Trials:   " << result->TrialCount << " calculations (iOpt: " << expectedIters << ")" << std::endl;
    }
    else
    {
      std::cout << "      Trials:   " << result->TrialCount << " calculations" << std::endl;
    }
  }
  else passed = false;

  std::cout << "      Status:   " << (passed ? "PASS" : "FAIL") << std::endl;

  delete solver;

  return passed;
}

// ==============================================================================
// Формат iOpt: задачи
// ==============================================================================

/// Задача вместе с объектами, которыми она владеет.
struct ProblemHolder
{
  IProblem* problem = nullptr;               ///< то, что передаётся в Solver
  std::function<void()> release;             ///< освобождение ресурсов
  std::string skipReason;                    ///< непусто -> случай пропускается
  std::string error;                         ///< непусто -> ошибка конфигурации (FAIL)

  ~ProblemHolder() { if (release) release(); }
};

/**
 Задачи из iOpt (problems/*.py), которых нет в Globalizer_Benchmarks.
 Формулы, границы и известные оптимумы перенесены без изменений.
*/
IProblem* CreateFunctionProblem(const json& p, std::string& error)
{
  const std::string name = p.at("name").get<std::string>();
  const int dim = p.value("dimension", 2);
  using Funcs = std::vector<std::function<double(const double*)>>;

  if (name == "XSquared")  // iOpt problems/xsquared.py
  {
    return new ProblemFromFunctionPointers(dim, std::vector<double>(dim, -1.0), std::vector<double>(dim, 1.0),
      Funcs(1, [dim](const double* x) { double s = 0; for (int i = 0; i < dim; i++) s += x[i] * x[i]; return s; }),
      true, 0.0, std::vector<double>(dim, 0.0));
  }
  if (name == "Rastrigin")  // iOpt problems/rastrigin.py
  {
    return new ProblemFromFunctionPointers(dim, std::vector<double>(dim, -2.2), std::vector<double>(dim, 1.8),
      Funcs(1, [dim](const double* x) {
        double s = 0; for (int i = 0; i < dim; i++) s += x[i] * x[i] - 10.0 * cos(2.0 * kPi * x[i]) + 10.0; return s; }),
      true, 0.0, std::vector<double>(dim, 0.0));
  }

  if (name == "RastriginInt")  // iOpt problems/rastriginInt.py
  {
    // Дискретные переменные iOpt принимают значения {"A", "B"}; в Globalizer
    // это 2 уровня дискретной переменной: -1 ("A") и +1 ("B").
    // Функция из Globalizer_Benchmarks (rastriginInt) устроена иначе
    // (другой множитель MultFunc), поэтому здесь перенесена версия iOpt.
    const int nd = p.at("discrete").get<int>();
    const int nf = dim - nd;
    auto toArray = [dim, nf](const double* x) {
      std::vector<double> arr(x, x + dim);
      for (int i = nf; i < dim; i++) arr[i] = (x[i] < 0) ? -2.2 : 1.8;  // A -> -2.2, B -> 1.8
      return arr;
    };
    auto multFunc = [dim, nf](const std::vector<double>& x) {
      double result = 0;
      for (int i = 0; i < dim; i++)
      {
        const double opt = (i < nf) ? 0.0 : 1.8;
        const double d = (1.8 - (-2.2)) / 2;
        const double a = (x[i] - opt) / d;
        result += a * a;
      }
      return -result;
    };
    double multKoef = 0;
    for (int c = 0; c < (1 << dim); c++)
    {
      std::vector<double> corner(dim);
      for (int j = 0; j < dim; j++) corner[j] = (((c >> j) & 1) == 0) ? -2.2 : 1.8;
      multKoef = std::max(multKoef, std::fabs(multFunc(corner)));
    }
    multKoef += 4;
    auto f = [nf, dim, toArray, multFunc, multKoef](const double* x) {
      double sum = 0;
      for (int i = 0; i < nf; i++) sum += x[i] * x[i] - 10 * cos(2 * kPi * x[i]) + 10;
      for (int i = nf; i < dim; i++) sum += (x[i] < 0) ? 2.2 : -1.8;
      return sum * (multFunc(toArray(x)) + multKoef);
    };
    std::vector<double> lower(dim, -2.2), upper(dim, 1.8), opt(dim, 0.0);
    for (int i = nf; i < dim; i++) { lower[i] = -1.0; upper[i] = 1.0; opt[i] = 1.0; }
    return new ProblemFromFunctionPointers(dim, nd, lower, upper, std::vector<int>(nd, 2),
      Funcs(1, f), true, f(opt.data()), opt);
  }

  // Общая часть критерия StronginC2/C3.
  auto stronginObjective = [](const double* x) {
    double t1 = pow(0.5 * x[0] - 0.5, 4.0);
    double t2 = pow(x[1] - 1.0, 4.0);
    double res = 1.5 * x[0] * x[0] * exp(1.0 - x[0] * x[0] - 20.25 * (x[0] - x[1]) * (x[0] - x[1]));
    return -(res + t1 * t2 * exp(2.0 - t1 - t2));
  };

  if (name == "StronginC2")  // iOpt problems/stronginc2.py
  {
    return new ProblemFromFunctionPointers(2, { 0.0, -1.0 }, { 4.0, 3.0 }, Funcs({
      [](const double* x) { return (x[0] - 2.2) * (x[0] - 2.2) + (x[1] - 1.2) * (x[1] - 1.2) - 1.25; },
      [](const double* x) { return 1.21 - (x[0] - 2.2) * (x[0] - 2.2) - (x[1] - 1.2) * (x[1] - 1.2); },
      stronginObjective }),
      true, stronginObjective(std::vector<double>{ 1.088, 1.088 }.data()), { 1.088, 1.088 });
  }
  if (name == "StronginC3")  // iOpt problems/stronginc3.py
  {
    return new ProblemFromFunctionPointers(2, { 0.0, -1.0 }, { 4.0, 3.0 }, Funcs({
      [](const double* x) { return 0.01 * ((x[0] - 2.2) * (x[0] - 2.2) + (x[1] - 1.2) * (x[1] - 1.2) - 2.25); },
      [](const double* x) { return 100.0 * (1.0 - ((x[0] - 2.0) / 1.2) * ((x[0] - 2.0) / 1.2) - (x[1] / 2.0) * (x[1] / 2.0)); },
      [](const double* x) { return 10.0 * (x[1] - 1.5 - 1.5 * sin(6.283 * (x[0] - 1.75))); },
      stronginObjective }),
      true, -1.489444, { 0.941176, 0.941176 });
  }
  if (name == "Romeijn3c")  // iOpt problems/romeijn3c.py
  {
    auto objective = [](const double* x) {
      static const double a[5] = { 0.5, 0.25, 1.0, 1.0 / 12.0, 2.0 };
      static const double c[5] = { 0.125, 0.25, 0.1, 0.2, 1.0 / 12.0 };
      static const double pt[5][2] = { { 0, 5 }, { 2, 5 }, { 3, 2 }, { 4, 4 }, { 5, 1 } };
      double result = 0;
      for (int i = 0; i < 5; i++)
      {
        double t = 0;
        for (int j = 0; j < 2; j++) t += (x[j] - pt[i][j]) * (x[j] - pt[i][j]);
        result += 1.0 / (a[i] * t + c[i]);
      }
      return result;
    };
    return new ProblemFromFunctionPointers(2, { -3.0, -4.0 }, { 10.0, 7.0 }, Funcs({
      [](const double* x) { return x[0] + x[1] - 5.0; },
      [](const double* x) { return x[0] - x[1] * x[1]; },
      [](const double* x) { return 5.0 * pow(x[0], 3) - 8.0 / 5.0 * x[1] * x[1]; },
      objective }),
      true, objective(std::vector<double>{ -3.0, -4.0 }.data()), { -3.0, -4.0 });
  }
  if (name == "Romeijn5c")  // iOpt problems/romeijn5c.py
  {
    auto objective = [](const double* x) {
      double t = x[1] - 1.275 * x[0] * x[0] + 5.0 * x[0] - 6.0;
      return -t * t - 10.0 * (1.0 - 1.0 / (8.0 * kPi)) * cos(kPi * x[0]) - 10.0;
    };
    return new ProblemFromFunctionPointers(2, { -1.5, 0.0 }, { 3.5, 15.0 }, Funcs({
      [](const double* x) { return -kPi * x[0] - x[1]; },
      [](const double* x) { return -(kPi * x[0]) * (kPi * x[0]) + 4.0 * x[1]; },
      objective }),
      true, objective(std::vector<double>{ 2.4656, 15.0 }.data()), { 2.4656, 15.0 });
  }

  error = "unknown function problem '" + name + "'";
  return nullptr;
}

/// Задачи-генераторы GCGen (тождественны генераторам iOpt: GKLS, Grishagin, Hill, Shekel).
bool CreateGCGenProblem(const json& p, int index, ProblemHolder& holder)
{
#ifdef _GLOBALIZER_GCGEN
  const std::string family = p.at("family").get<std::string>();
  IOptProblem* raw = nullptr;
  if (family == "GKLS")
  {
    GKLSClass cls = (p.value("class", std::string("simple")) == "hard") ? Hard : Simple;
    raw = new TGKLSProblem(index, p.value("dimension", 2), cls);
  }
  else if (family == "Grishagin")
    raw = new TGrishaginProblem(index);
  else if (family == "Hill")
    raw = new THillProblem(index);
  else if (family == "Shekel")
    raw = new TShekelProblem(index);
  else
  {
    holder.skipReason = "unknown GCGen family '" + family + "'";
    return false;
  }
  GlobalizerOptProblem* wrapped = new GlobalizerOptProblem(raw);
  wrapped->Initialize();
  holder.problem = wrapped;
  holder.release = [wrapped, raw]() { delete wrapped; delete raw; };
  return true;
#else
  (void)p; (void)index;
  holder.skipReason = "built without GCGen (GLOBALIZER_BUILD_GCGEN=OFF)";
  return false;
#endif
}

/// Задачи из динамических библиотек Globalizer_Benchmarks (как в SampleMain).
bool CreateBenchmarkProblem(const json& p, ProblemHolder& holder)
{
#ifdef _GLOBALIZER_BENCHMARKS
  const std::string lib = p.at("lib").get<std::string>();
#ifdef WIN32
  const std::vector<std::string> names = { "lib" + lib + ".dll", lib + ".dll", "lib" + lib + "_d.dll" };
#elif defined(__APPLE__)
  const std::vector<std::string> names = { "lib" + lib + ".dylib", "lib" + lib + ".so" };
#else
  const std::vector<std::string> names = { "lib" + lib + ".so" };
#endif
  std::string path;
  for (const auto& n : names)
  {
    std::ifstream f(gLibDir + "/" + n);
    if (f.good()) { path = gLibDir + "/" + n; break; }
  }
  if (path.empty())
  {
    holder.skipReason = "problem library '" + lib + "' not found in " + gLibDir;
    return false;
  }

  auto* manager = new GlobalOptimizationProblemManager();
  if (manager->LoadProblemLibrary(path) != GlobalOptimizationProblemManager::OK_)
  {
    delete manager;
    holder.skipReason = "cannot load " + path;
    return false;
  }
  IGlobalOptimizationProblem* bp = manager->GetProblem();
  // Порядок как требует интерфейс: SetDimension / дискретные — до Initialize.
  if (p.contains("dimension") &&
      bp->SetDimension(p["dimension"].get<int>()) != IGlobalOptimizationProblem::PROBLEM_OK)
  {
    holder.error = "problem '" + lib + "' rejected dimension " + std::to_string(p["dimension"].get<int>());
    holder.release = [manager]() { delete manager; };
    return false;
  }
  if (p.contains("discrete"))
    bp->SetNumberOfDiscreteVariable(p["discrete"].get<int>());
  bp->Initialize();
  parameters.Dimension = bp->GetDimension();

  auto* wrapped = new GlobalizerBenchmarksProblem(bp);
  holder.problem = wrapped;
  holder.release = [wrapped, manager]() { delete wrapped; delete manager; };
  return true;
#else
  (void)p;
  holder.skipReason = "built without benchmarks interface";
  return false;
#endif
}

// ==============================================================================
// Формат iOpt: решение и проверки
// ==============================================================================

/// Результат одного прогона решателя.
struct RunResult
{
  bool ok = false;
  std::string error;
  int dimension = 0;
  int trials = 0;
  int iterations = 0;
  double value = 0.0;
  bool feasible = false;   ///< лучшая точка удовлетворяет всем ограничениям
  std::vector<double> point;
};

/// Инициализирует parameters аргументами командной строки — как пример.
void InitParameters(const std::vector<std::string>& args)
{
  std::vector<std::string> storage;
  storage.push_back("Globalizer_IntegrationTests");
  storage.insert(storage.end(), args.begin(), args.end());
  std::vector<char*> argv;
  for (auto& s : storage) argv.push_back(&s[0]);
  argv.push_back(nullptr);

  parameters.Init((int)storage.size(), argv.data(), false);
  parameters.IsPrintResultToConsole = false;
  parameters.DisablePrintParameters = true;

  // Как при отдельном запуске примера: тип и точность Extended выбираются
  // заново (Solver::InitAutoPrecision срабатывает только при точности 0.01).
  Extended::SetTypeID(gInitialExtendedType);
  Extended::SetPrecision(gInitialExtendedPrecision);
}

RunResult SolveCase(const json& c, int index, std::vector<std::string> args, Outcome& outcome, std::string& skip)
{
  RunResult r;
  InitParameters(args);

  const json& p = c.at("problem");
  const std::string kind = p.at("kind").get<std::string>();
  ProblemHolder holder;
  std::string error;
  if (kind == "function")
  {
    holder.problem = CreateFunctionProblem(p, error);
    if (holder.problem)
    {
      holder.problem->Initialize();
      IProblem* raw = holder.problem;
      holder.release = [raw]() { delete raw; };
      parameters.Dimension = raw->GetDimension();
    }
  }
  else if (kind == "gcgen")
  {
    CreateGCGenProblem(p, index, holder);
    if (holder.problem) parameters.Dimension = holder.problem->GetDimension();
  }
  else if (kind == "benchmark")
    CreateBenchmarkProblem(p, holder);
  else
    error = "unknown problem kind '" + kind + "'";

  if (!holder.skipReason.empty()) { outcome = Outcome::Skip; skip = holder.skipReason; return r; }
  if (!holder.error.empty()) { r.error = holder.error; return r; }
  if (!holder.problem) { r.error = error; return r; }

  try
  {
    Solver solver(holder.problem);
    if (solver.Solve() != SYSTEM_OK) { r.error = "Solver::Solve() returned an error"; return r; }
    std::unique_ptr<SolutionResult> res(solver.GetSolutionResult());
    if (!res || !res->BestTrial) { r.error = "no solution"; return r; }
    r.dimension = holder.problem->GetDimension();
    r.trials = res->TrialCount;
    r.iterations = res->IterationCount;
    r.value = res->GetBestValue();
    r.feasible = res->BestTrial->index == holder.problem->GetNumberOfConstraints();
    r.point.assign(res->BestTrial->y, res->BestTrial->y + r.dimension);
    r.ok = true;
  }
  catch (const std::exception& e) { r.error = std::string("exception: ") + e.what(); }
  catch (...) { r.error = "unknown exception"; }
  return r;
}

/// Проверяет найденную точку по критерию iOpt: |x*_j - x_j| <= eps * (b_j - a_j).
bool CheckPoint(IProblem* problem, const RunResult& r, const json& expect, double eps,
  std::ostringstream& msg)
{
  const int n = r.dimension;
  std::vector<double> lower(n), upper(n), opt(n);
  problem->GetBounds(lower.data(), upper.data());
  if (expect.contains("point"))
    opt = expect["point"].get<std::vector<double>>();
  else if (problem->GetOptimumPoint(opt.data()) != IProblem::OK)
  {
    msg << " point: unknown optimum;";
    return false;
  }
  // Сравниваются только непрерывные координаты; дискретные — точно.
  bool ok = true;
  IIntegerProgrammingProblem* ip = dynamic_cast<IIntegerProgrammingProblem*>(problem);
  const int discrete = ip ? ip->GetNumberOfDiscreteVariable() : 0;
  const int continuous = n - discrete;
  for (int j = 0; j < (int)opt.size() && j < n; j++)
  {
    double d = std::fabs(opt[j] - r.point[j]);
    double lim = (j < continuous) ? eps * (upper[j] - lower[j]) : 1e-9;
    if (d > lim)
    {
      ok = false;
      msg << " x[" << j << "]=" << r.point[j] << " vs " << opt[j] << " (|d|=" << d << " > " << lim << ");";
    }
  }
  return ok;
}

/// Создаёт задачу ещё раз (только для проверки точки и границ).
std::unique_ptr<ProblemHolder> RecreateProblem(const json& c, int index)
{
  std::unique_ptr<ProblemHolder> h(new ProblemHolder());
  const json& p = c.at("problem");
  const std::string kind = p.at("kind").get<std::string>();
  std::string error;
  if (kind == "function")
  {
    h->problem = CreateFunctionProblem(p, error);
    if (h->problem) { h->problem->Initialize(); IProblem* raw = h->problem; h->release = [raw]() { delete raw; }; }
  }
  else if (kind == "gcgen")
    CreateGCGenProblem(p, index, *h);
  else if (kind == "benchmark")
    CreateBenchmarkProblem(p, *h);
  return h;
}

Outcome RunIOptCase(const json& c, int index, const std::string& title, const json& expectOverride)
{
  const json& expect = expectOverride.is_null() ? c.value("expect", json::object()) : expectOverride;
  std::vector<std::string> args = c.value("args", std::vector<std::string>());

  Outcome outcome = Outcome::Pass;
  std::string skip;
  RunResult r = SolveCase(c, index, args, outcome, skip);
  if (outcome == Outcome::Skip)
  {
    std::cout << "  [SKIP] " << title << "  " << skip << std::endl;
    return Outcome::Skip;
  }
  if (!r.ok)
  {
    std::cout << "  [" << FailLabel() << "] " << title << "  " << r.error << std::endl;
    return Outcome::Fail;
  }

  bool passed = true;
  std::ostringstream msg;
  msg << std::setprecision(6);

  if (expect.contains("point_eps"))
  {
    auto h = RecreateProblem(c, index);
    if (!h->problem || !CheckPoint(h->problem, r, expect, expect["point_eps"].get<double>(), msg))
      passed = false;
  }
  if (expect.contains("optimum"))
  {
    double opt = expect["optimum"].get<double>();
    double tol = expect.value("value_tolerance", 1e-2);
    if (std::fabs(r.value - opt) > tol)
    {
      passed = false;
      msg << " f=" << r.value << " vs " << opt << " (tol " << tol << ");";
    }
  }
  if (expect.contains("trials"))
  {
    int exp = expect["trials"].get<int>();
    int delta = expect.value("trials_delta", 0);
    if (std::abs(r.trials - exp) > delta)
    {
      passed = false;
      msg << " trials=" << r.trials << " vs " << exp << " (delta " << delta << ");";
    }
  }
  if (expect.value("feasible", false) && !r.feasible)
  {
    passed = false;
    msg << " best point violates constraints;";
  }
  if (expect.contains("max_trials") && r.trials > expect["max_trials"].get<int>())
  {
    passed = false;
    msg << " trials=" << r.trials << " > max " << expect["max_trials"].get<int>() << ";";
  }

  std::cout << "  [" << (passed ? "PASS" : FailLabel()) << "] " << title
    << "  f=" << std::setprecision(6) << r.value << " trials=" << r.trials;
  if (expect.contains("trials"))
    std::cout << " (ref " << expect["trials"].get<int>() << ")";
  // Эталон iOpt, если он отличается от эталона Globalizer (справочно).
  if (expect.contains("iopt_trials") &&
      (!expect.contains("trials") || expect["iopt_trials"] != expect["trials"]))
    std::cout << " (iOpt: " << expect["iopt_trials"].get<int>() << ")";
  std::cout << msg.str() << std::endl;
  return passed ? Outcome::Pass : Outcome::Fail;
}

/**
 Многошаговый случай: несколько запусков решателя подряд (общий рабочий
 каталог), затем сравнение результатов пар запусков "expect_equal".
 Аналог test_load_progress.py из iOpt: прогон 50 итераций с сохранением,
 загрузка и продолжение до 100 — результат должен совпасть с прогоном
 100 итераций без прерывания.
*/
Outcome RunMultiRunCase(const json& c, const std::string& title)
{
  const json& runs = c.at("runs");
  std::vector<RunResult> results;
  // Временные файлы удаляются при любом исходе случая.
  struct Cleanup
  {
    std::vector<std::string> files;
    ~Cleanup() { for (const auto& f : files) std::remove(f.c_str()); }
  } cleanup{ c.value("cleanup", std::vector<std::string>()) };
  for (size_t i = 0; i < runs.size(); i++)
  {
    Outcome outcome = Outcome::Pass;
    std::string skip;
    RunResult r = SolveCase(c, c["problem"].value("index", 1),
      runs[i].at("args").get<std::vector<std::string>>(), outcome, skip);
    if (outcome == Outcome::Skip)
    {
      std::cout << "  [SKIP] " << title << "  " << skip << std::endl;
      return Outcome::Skip;
    }
    if (!r.ok)
    {
      std::cout << "  [" << FailLabel() << "] " << title << "  run " << i << ": " << r.error << std::endl;
      return Outcome::Fail;
    }
    results.push_back(r);
  }

  bool passed = true;
  std::ostringstream msg;
  msg << std::setprecision(10);
  for (const auto& pair : c.value("expect_equal", json::array()))
  {
    const RunResult& a = results.at(pair.at(0).get<int>());
    const RunResult& b = results.at(pair.at(1).get<int>());
    bool same = std::fabs(a.value - b.value) <= 1e-12 * std::max(1.0, std::fabs(b.value));
    for (size_t j = 0; same && j < a.point.size(); j++)
      same = std::fabs(a.point[j] - b.point[j]) <= 1e-12;
    if (!same)
    {
      passed = false;
      msg << " run " << pair[0] << " f=" << a.value << " x=(";
      for (size_t j = 0; j < a.point.size(); j++) msg << (j ? "," : "") << a.point[j];
      msg << ") != run " << pair[1] << " f=" << b.value << " x=(";
      for (size_t j = 0; j < b.point.size(); j++) msg << (j ? "," : "") << b.point[j];
      msg << ");";
    }
  }
  std::cout << "  [" << (passed ? "PASS" : FailLabel()) << "] " << title << msg.str() << std::endl;
  return passed ? Outcome::Pass : Outcome::Fail;
}

/// Учитывает пометку "xfail" (известный, задокументированный отказ).
Outcome ApplyXFail(const json& c, Outcome o, const std::string& title)
{
  if (!c.contains("xfail") || (o != Outcome::Pass && o != Outcome::Fail))
    return o;
  const std::string reason = c["xfail"].get<std::string>();
  if (o == Outcome::Fail)
  {
    std::cout << "  [XFAIL] " << title << "  known issue: " << reason << std::endl;
    return Outcome::XFail;
  }
  std::cout << "  [XPASS] " << title << "  now passes - remove \"xfail\" from the case ("
    << reason << ")" << std::endl;
  return Outcome::XPass;
}

// ==============================================================================
// main
// ==============================================================================
int main(int argc, char* argv[])
{
  std::string jsonPath = "tests.json";
  std::string group, filter;
  bool listOnly = false;
  for (int i = 1; i < argc; i++)
  {
    std::string a = argv[i];
    if (a == "--group" && i + 1 < argc) group = argv[++i];
    else if (a == "--filter" && i + 1 < argc) filter = argv[++i];
    else if (a == "--lib-dir" && i + 1 < argc) gLibDir = argv[++i];
    else if (a == "--list") listOnly = true;
    else jsonPath = a;
  }

  std::ifstream f(jsonPath);
  if (!f.is_open()) { std::cerr << "Cannot open config file " << jsonPath << std::endl; return 2; }
  json testsData = json::parse(f);

  // Запоминаем исходное состояние Extended до первого решения.
  gInitialExtendedType = Extended::GetTypeID();
  gInitialExtendedPrecision = Extended::GetPrecision();

  int passed = 0, failed = 0, skipped = 0, xfailed = 0, xpassed = 0;
  auto count = [&](Outcome o) {
    switch (o)
    {
    case Outcome::Pass: passed++; break;
    case Outcome::Fail: failed++; break;
    case Outcome::Skip: skipped++; break;
    case Outcome::XFail: xfailed++; break;
    case Outcome::XPass: xpassed++; break;
    }
  };
  std::cout << "================================================" << std::endl;
  std::cout << "   GLOBALIZER INTEGRATION TESTS STARTING (" << jsonPath << ")" << std::endl;
  std::cout << "================================================" << std::endl;

  for (const auto& c : testsData)
  {
    const std::string name = c.value("name", std::string("?"));
    if (!group.empty() && c.value("group", std::string()) != group) continue;
    if (!filter.empty() && name.find(filter) == std::string::npos) continue;

    if (listOnly)
    {
      std::cout << "  " << c.value("group", std::string("-")) << "  " << name << std::endl;
      continue;
    }

    gCaseIsXFail = c.contains("xfail");
    if (!c.contains("problem"))
    {
      (RunLegacyTest(c) ? passed : failed)++;
      continue;
    }

    // Серия задач с одним набором параметров (например, 100 задач GKLS).
    if (c.contains("series"))
    {
      const json& s = c["series"];
      std::vector<int> indices = s.at("indices").get<std::vector<int>>();
      std::vector<int> trials = s.value("trials", std::vector<int>());
      for (size_t k = 0; k < indices.size(); k++)
      {
        json e = c.value("expect", json::object());
        if (k < trials.size()) e["trials"] = trials[k];
        const std::string title = name + "#" + std::to_string(indices[k]);
        count(ApplyXFail(c, RunIOptCase(c, indices[k], title, e), title));
      }
      continue;
    }

    if (c.contains("runs"))
    {
      count(ApplyXFail(c, RunMultiRunCase(c, name), name));
      continue;
    }

    count(ApplyXFail(c, RunIOptCase(c, c["problem"].value("index", 1), name, json()), name));
  }

  if (listOnly) return 0;
  std::cout << "================================================" << std::endl;
  std::cout << "Summary: " << passed << " passed, " << failed << " failed, " << skipped << " skipped";
  if (xfailed || xpassed)
    std::cout << ", " << xfailed << " xfailed (known issues), " << xpassed << " xpassed";
  std::cout << "." << std::endl;
  std::cout << "================================================" << std::endl;

  return failed == 0 ? 0 : 1;
}

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2026 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      PYGlobalizer.cpp                                            //
//                                                                         //
//  Purpose:   Python interface for Globalizer (module PYDGlobalizer)      //
//             - solve_legacy: параметры через запятую                     //
//             - solve: параметры через SolverParameters                   //
//                                                                         //
//  Author(s): Egorov K.                                                   //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <memory>
#include <stdexcept>
#ifdef USE_PYTHON
#ifndef WIN32
#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
#endif
#else
#ifdef _DEBUG
#undef _DEBUG
#include "python.h"
#define _DEBUG
#else
#include "python.h"
#endif
#endif
#endif
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "Globalizer.h"
#include "PYProblem.h"

#include "PYLogger.h"


namespace py = pybind11;

// ------------------------------------------------------------------------------------------------
static py::str to_py_str(const std::string& s)
{
  const Py_ssize_t n = static_cast<Py_ssize_t>(s.size());

  PyObject* obj = PyUnicode_DecodeUTF8(s.data(), n, "strict");
  if (obj)
    return py::reinterpret_steal<py::str>(obj);
  PyErr_Clear();

#ifdef _WIN32
  obj = PyUnicode_DecodeMBCS(s.data(), n, "replace");
#else
  obj = PyUnicode_DecodeUTF8(s.data(), n, "replace");
#endif
  if (!obj)
    throw py::error_already_set();
  return py::reinterpret_steal<py::str>(obj);
}

// ------------------------------------------------------------------------------------------------
struct SolverParameters
{
  int    max_iterations = 1000;
  double r = 4.0;
  int    num_threads = 1;
  double epsilon = 0.01;
  bool   local_refine = false;
  int    local_iterations = 100;   // число итераций локального метода
  int    verbose = 1;              // 0=silent 1=error 2=info 3=debug
  bool   print_result = true;      // печать итогов решателя в консоль (-isPRC)
  std::string log_prefix = "";     // префикс файлов журнала ("" - по умолчанию, в текущей папке)
  /// Критерий остановки: "accuracy" - по точности epsilon (по умолчанию),
  /// "iterations" - только по max_iterations (IterationOnly, как в IterationMain.cpp).
  std::string stop_condition = "accuracy";

  void validate() const {
    if (stop_condition != "accuracy" && stop_condition != "iterations")
      throw std::invalid_argument("stop_condition must be 'accuracy' or 'iterations', got '"
        + stop_condition + "'");
    if (max_iterations < 10 || max_iterations > 10000000)
      throw std::invalid_argument("max_iterations must be in [10, 10000000], got "
        + std::to_string(max_iterations));
    if (r <= 1.0 || r > 100.0)
      throw std::invalid_argument("r must be in (1.0, 100.0], got " + std::to_string(r));
    if (num_threads < 1 || num_threads > 256)
      throw std::invalid_argument("num_threads must be in [1, 256], got "
        + std::to_string(num_threads));
    if (epsilon <= 0.0 || epsilon >= 1.0)
      throw std::invalid_argument("epsilon must be in (0.0, 1.0), got " + std::to_string(epsilon));
    if (local_iterations < 0 || local_iterations > 100000)
      throw std::invalid_argument("local_iterations must be in [0, 100000], got "
        + std::to_string(local_iterations));
    if (verbose < 0 || verbose > 3)
      throw std::invalid_argument("verbose must be in [0, 3], got " + std::to_string(verbose));
  }

  std::string repr() const {
    return "SolverParameters(max_iterations=" + std::to_string(max_iterations) +
      ", r=" + std::to_string(r) +
      ", num_threads=" + std::to_string(num_threads) +
      ", epsilon=" + std::to_string(epsilon) +
      ", local_refine=" + (local_refine ? "True" : "False") +
      ", local_iterations=" + std::to_string(local_iterations) +
      ", verbose=" + std::to_string(verbose) +
      ", print_result=" + (print_result ? "True" : "False") +
      ", log_prefix='" + log_prefix + "'" +
      ", stop_condition='" + stop_condition + "')";
  }
};

// ------------------------------------------------------------------------------------------------
static SolutionResult* run_solver(IProblem* problem_ptr, std::string& solver_name)
{
  SolutionResult* result = nullptr;

  // Отпускаем GIL: решатель - чистый C++. GIL берётся заново
  // только при вызове целевой функции в PYProblem::CalculateFunctionals.


  py::gil_scoped_release release;

  solver_name = "Solver";
  PY_LOG_INFO("Using Solver (index method)");
  Solver solver(problem_ptr);
  if (solver.Solve() != SYSTEM_OK)
    throw std::runtime_error("Solver::Solve() failed");
  result = solver.GetSolutionResult();
  return result;
}

// ------------------------------------------------------------------------------------------------
static py::dict make_result_dict(SolutionResult* result,
  IProblem* problem_ptr,
  const std::string& solver_name)
{
  if (!result)
    throw std::runtime_error("Solver returned null result");

  py::dict d;
  d["success"] = true;
  d["best_value"] = result->GetBestValue();



  const double* bp = result->GetBestPoint();
  py::list point_list;
  for (int i = 0; i < problem_ptr->GetDimension(); ++i)
    point_list.append(bp[i]);
  d["best_point"] = point_list;
  d["iterations"] = result->GetIterationsCount();
  d["solver"] = solver_name;

  PY_LOG_INFO("Best value found: " << result->GetBestValue());
  return d;
}

// ------------------------------------------------------------------------------------------------
/// Номер текущего «сеанса» глобальных параметров Globalizer. Каждый solve()
/// и каждый IterativeSolver заново инициализирует глобальный объект
/// parameters, поэтому ранее созданный IterativeSolver после этого
/// продолжать нельзя - он проверяет, что его сеанс всё ещё текущий.
static long long g_session = 0;

// ------------------------------------------------------------------------------------------------
static void apply_parameters(const SolverParameters& params)
{
  parameters.MaxNumOfPoints = params.max_iterations;
  parameters.r = params.r;
  parameters.NumPoints = params.num_threads;
  parameters.NumThread = params.num_threads;
  parameters.Epsilon = params.epsilon;
  parameters.IsPrintResultToConsole = params.print_result;
  parameters.StopCondition = (params.stop_condition == "iterations") ? IterationOnly : Accuracy;

  if (params.local_refine) {
    parameters.LocalRefineSolution = (ELocalMethodScheme)1;
    parameters.TypeLocalMethod = (ETypeLocalMethod)0;
    parameters.LocalIteration = params.local_iterations;
    parameters.LocalVerificationEpsilon = params.epsilon * 0.1;
    PY_LOG_INFO("Local refinement ON: iters=" << params.local_iterations);
  }
  else {
    parameters.LocalRefineSolution = (ELocalMethodScheme)0;
    PY_LOG_INFO("Local refinement OFF");
  }
}

// ------------------------------------------------------------------------------------------------
static void ensure_pyproblem(py::object obj)
{
  py::gil_scoped_acquire gil;
  py::object cls = py::module_::import("PYProblem").attr("PYProblem");
  if (!py::isinstance(obj, cls))
    throw std::invalid_argument("Expected PYProblem instance, got: "
      + std::string(py::str(obj.get_type())));
}

// ------------------------------------------------------------------------------------------------
py::dict validate_problem(py::object problem)
{
  py::gil_scoped_acquire gil;
  py::dict info;

  try
  {
    PYProblem p(problem);
    p.Initialize();

    info["valid"] = true;
    info["dimension"] = p.GetDimension();
    info["n_functions"] = p.GetNumberOfFunctions();
    info["n_constraints"] = p.GetNumberOfConstraints();
    info["n_criteria"] = p.GetNumberOfCriterions();
    info["error"] = py::none();

    PY_LOG_INFO("Problem validation successful");
  }
  catch (const std::exception& e)
  {
    info["valid"] = false;
    info["error"] = to_py_str(e.what());
    PY_LOG_ERROR("Problem validation failed: " << e.what());
  }

  return info;
}

// ------------------------------------------------------------------------------------------------
py::dict solve(py::object problem, SolverParameters params = SolverParameters())
{
  try {
    params.validate();
    pyglob::SetLogLevel(params.verbose);

    PY_LOG_INFO("solve: max_iter=" << params.max_iterations
      << " r=" << params.r << " threads=" << params.num_threads);

    ensure_pyproblem(problem);
    ++g_session;
    GlobalizerInitialization(0, nullptr, false, false, params.log_prefix);
    apply_parameters(params);

    std::unique_ptr<IProblem> problem_ptr;
    problem_ptr = std::make_unique<PYProblem>(problem);
    if (problem_ptr->Initialize() != IProblem::OK)
      throw std::runtime_error("Problem initialization failed");
    parameters.Dimension = problem_ptr->GetDimension();


    std::string solver_name;
    SolutionResult* result = run_solver(problem_ptr.get(), solver_name);

    py::gil_scoped_acquire gil;
    return make_result_dict(result, problem_ptr.get(), solver_name);
  }
  catch (const py::error_already_set& e) {
    PY_LOG_ERROR("Python error in solve: " << e.what());
    py::gil_scoped_acquire gil;
    py::dict err;
    err["success"] = false;
    err["error"] = to_py_str(std::string("Python error: ") + e.what());
    return err;
  }
  catch (const std::exception& e) {
    PY_LOG_ERROR("Error in solve: " << e.what());
    py::gil_scoped_acquire gil;
    py::dict err;
    err["success"] = false;
    err["error"] = to_py_str(std::string("Error: ") + e.what());
    return err;
  }
}

// ------------------------------------------------------------------------------------------------
py::dict solve_legacy(py::object data,
  int maxParams = 50,
  double r = 5.0,
  bool localRefineSolution = false,
  int numThreads = 2)
{
  SolverParameters params;
  params.max_iterations = maxParams;
  params.r = r;
  params.local_refine = localRefineSolution;
  params.num_threads = numThreads;
  return solve(data, params);
}

// ------------------------------------------------------------------------------------------------
/// Пошаговое решение задачи - аналог examples/IterationMain.cpp:
///   Solver solver(problem); solver.Initialize(); solver.DoIteration(finished) ...
/// Между итерациями Python-код может менять данные, от которых зависят
/// функции задачи, читать текущий результат и решать, продолжать ли поиск.
class IterativeSolver
{
public:
  IterativeSolver(py::object problem, SolverParameters params)
    : mProblemObject(problem), mParams(params)
  {
    mParams.validate();
    pyglob::SetLogLevel(mParams.verbose);
    ensure_pyproblem(problem);

    mSession = ++g_session;
    GlobalizerInitialization(0, nullptr, false, false, mParams.log_prefix);
    apply_parameters(mParams);

    mProblem = std::make_unique<PYProblem>(problem);
    if (mProblem->Initialize() != IProblem::OK)
      throw std::runtime_error("Problem initialization failed");
    parameters.Dimension = mProblem->GetDimension();

    mSolver = std::make_unique<Solver>(mProblem.get());
    int rc;
    {
      py::gil_scoped_release release;   // Initialize уже вычисляет функции задачи
      rc = mSolver->Initialize();
    }
    if (rc != 0)
      throw std::runtime_error("Solver::Initialize() failed");
    PY_LOG_INFO("IterativeSolver: initialized, dim=" << mProblem->GetDimension());
  }

  /// Одна итерация метода. Возвращает True, если поиск завершён.
  bool do_iteration()
  {
    if (mFinished)
      return true;
    check_session();
    int rc;
    bool finished = false;
    {
      py::gil_scoped_release release;
      rc = mSolver->DoIteration(finished);
    }
    if (rc != 0)
      throw std::runtime_error("Solver::DoIteration() failed");
    mFinished = finished;
    if (mFinished)
      mFinalResult = make_result();     // итог доступен и после запуска другого решателя
    return mFinished;
  }

  /// До max_steps итераций (все оставшиеся при max_steps < 0) или до
  /// завершения поиска. Возвращает число выполненных итераций.
  int run(int max_steps)
  {
    int done = 0;
    while (!mFinished && (max_steps < 0 || done < max_steps))
    {
      do_iteration();
      ++done;
    }
    return done;
  }

  /// Текущий результат (лучшая найденная точка) в формате solve().
  /// Для завершённого поиска возвращается сохранённый итог.
  py::dict result()
  {
    if (mFinished && !mFinalResult.is_none())
      return mFinalResult.attr("copy")().cast<py::dict>();
    check_session();
    return make_result();
  }

  int iterations()
  {
    if (mFinished && !mFinalResult.is_none())
      return mFinalResult["iterations"].cast<int>();
    check_session();
    std::unique_ptr<SolutionResult> res(mSolver->GetSolutionResult());
    delete res->BestTrial;
    return res->IterationCount;
  }

  bool finished() const { return mFinished; }
  bool is_active() const { return mSession == g_session; }

  /// Печать итогов решателя (как Solver::PrintResultToConsole в IterationMain.cpp).
  void print_result()
  {
    check_session();
    mSolver->PrintResultToConsole();
  }

  /// Протокол итератора: for state in solver: ... - одна итерация на шаг.
  py::dict next()
  {
    if (mFinished)
      throw py::stop_iteration();
    do_iteration();
    return result();
  }

  std::string repr() const
  {
    return std::string("IterativeSolver(finished=") + (mFinished ? "True" : "False") +
      ", active=" + (is_active() ? "True" : "False") + ")";
  }

private:
  py::dict make_result()
  {
    std::unique_ptr<SolutionResult> res(mSolver->GetSolutionResult());
    py::dict d;
    const Trial* best = res->BestTrial;
    const int nf = mProblem->GetNumberOfFunctions();
    const bool has_point = best != nullptr && best->index >= 0 && best->index < nf;
    d["success"] = true;
    d["finished"] = mFinished;
    d["feasible"] = has_point && best->index == nf - 1;
    if (has_point)
    {
      d["best_value"] = best->FuncValues[best->index];
      py::list point;
      for (int i = 0; i < mProblem->GetDimension(); ++i)
        point.append(best->y[i]);
      d["best_point"] = point;
    }
    else
    {
      d["best_value"] = py::none();
      d["best_point"] = py::none();
    }
    d["iterations"] = res->IterationCount;
    d["trials"] = res->TrialCount;
    d["solver"] = "IterativeSolver";
    delete res->BestTrial;
    res->BestTrial = nullptr;
    return d;
  }

  void check_session() const
  {
    if (!is_active())
      throw std::runtime_error(
        "IterativeSolver is no longer valid: solve() or another IterativeSolver has "
        "re-initialized the global Globalizer parameters. Only one solver can be active.");
  }

  py::object mProblemObject;              // держит Python-объект задачи живым
  SolverParameters mParams;
  long long mSession = 0;
  bool mFinished = false;
  py::object mFinalResult = py::none();   // итог завершённого поиска
  std::unique_ptr<PYProblem> mProblem;    // объявлен до mSolver: удаляется после него
  std::unique_ptr<Solver> mSolver;
};


// ------------------------------------------------------------------------------------------------
PYBIND11_MODULE(PYDGlobalizer, m)
{
  m.doc() = R"pbdoc(
        PYDGlobalizer - Python interface for Globalizer global optimization system

        Supports two calling styles:
        1. Legacy: solve_legacy(problem, maxParams, r, localRefineSolution, numThreads)
        2. Modern: solve(problem, params=SolverParameters())
    )pbdoc";

  py::class_<SolverParameters>(m, "SolverParameters",
    R"pbdoc(
            Parameters for the Globalizer solver.

            Attributes
            ----------
            max_iterations : int
                Maximum number of function evaluations (default: 1000).
            r : float
                Reliability parameter, r > 1.0 (default: 4.0).
            num_threads : int
                Number of OpenMP threads (default: 1).
            epsilon : float
                Stop criterion (default: 0.01).
            local_refine : bool
                Run local refinement (default: False).
            local_iterations : int
                Iterations of the local method (default: 100).
            verbose : int
                Log level: 0=silent 1=error 2=info 3=debug (default: 1).
            print_result : bool
                Print the solver summary to the console (default: True).
            log_prefix : str
                Prefix (may include a directory) of the solver log files;
                empty - default name in the current directory (default: "").
            stop_condition : str
                "accuracy" - stop when epsilon is reached (default);
                "iterations" - stop only after max_iterations.
        )pbdoc")
    .def(py::init<>())
    .def_readwrite("max_iterations", &SolverParameters::max_iterations)
    .def_readwrite("r", &SolverParameters::r)
    .def_readwrite("num_threads", &SolverParameters::num_threads)
    .def_readwrite("epsilon", &SolverParameters::epsilon)
    .def_readwrite("local_refine", &SolverParameters::local_refine)
    .def_readwrite("local_iterations", &SolverParameters::local_iterations)
    .def_readwrite("verbose", &SolverParameters::verbose)
    .def_readwrite("print_result", &SolverParameters::print_result)
    .def_readwrite("log_prefix", &SolverParameters::log_prefix)
    .def_readwrite("stop_condition", &SolverParameters::stop_condition)
    .def("validate", &SolverParameters::validate, "Validate parameter values.")
    .def("__repr__", &SolverParameters::repr);

  m.def("solve_legacy", &solve_legacy,
    py::arg("problem"),
    py::arg("maxParams") = 50,
    py::arg("r") = 5.0,
    py::arg("localRefineSolution") = false,
    py::arg("numThreads") = 2,
    R"pbdoc(
              Legacy interface: solve with positional parameters.

              Returns
              -------
              dict
                  Same result dictionary as solve().
          )pbdoc");

  m.def("solve", py::overload_cast<py::object, SolverParameters>(&solve),
    py::arg("problem"),
    py::arg("params") = SolverParameters(),
    R"pbdoc(
              Modern interface: solve with SolverParameters structure.

              Returns
              -------
              dict
                  success (bool), best_value (float), best_point (list[float]),
                  iterations (int), solver (str), error (str, optional)
          )pbdoc");

  py::class_<IterativeSolver>(m, "IterativeSolver",
    R"pbdoc(
            Step-by-step solving (like examples/IterationMain.cpp).

            The constructor initializes the solver (the first trials are made).
            Between iterations Python code may change data used by the problem
            functions, inspect the current result and decide whether to go on.

            Only one solver can be active: solve() or a new IterativeSolver
            re-initializes the global Globalizer parameters, after which the
            old IterativeSolver raises RuntimeError.

            Example
            -------
            >>> solver = PYDGlobalizer.IterativeSolver(problem, params)
            >>> while not solver.do_iteration():
            ...     pass
            >>> solver.result()["best_value"]
            >>> for state in PYDGlobalizer.IterativeSolver(problem, params):
            ...     print(state["iterations"], state["best_value"])
        )pbdoc")
    .def(py::init<py::object, SolverParameters>(),
      py::arg("problem"), py::arg("params") = SolverParameters())
    .def("do_iteration", &IterativeSolver::do_iteration,
      "Perform one iteration. Returns True when the search is finished.")
    .def("run", &IterativeSolver::run, py::arg("max_steps") = -1,
      "Perform up to max_steps iterations (all remaining if max_steps < 0) or until "
      "the search is finished. Returns the number of performed iterations.")
    .def("result", &IterativeSolver::result,
      "Current result: success, finished, feasible, best_value, best_point, "
      "iterations, trials, solver.")
    .def("print_result", &IterativeSolver::print_result,
      "Print the solver summary to the console.")
    .def_property_readonly("finished", &IterativeSolver::finished)
    .def_property_readonly("iterations", &IterativeSolver::iterations)
    .def_property_readonly("is_active", &IterativeSolver::is_active,
      "False after solve() or another IterativeSolver has been started.")
    .def("__iter__", [](IterativeSolver& s) -> IterativeSolver& { return s; },
      py::return_value_policy::reference_internal)
    .def("__next__", &IterativeSolver::next)
    .def("__repr__", &IterativeSolver::repr);

  m.def("validate_problem", &validate_problem,
    py::arg("problem"),
    R"pbdoc(
              Validate problem without solving.

              Returns
              -------
              dict
                  valid, dimension, n_functions, n_constraints, n_criteria, error
          )pbdoc");

  m.def("set_log_level", [](int lvl) { pyglob::SetLogLevel(lvl); },
    py::arg("level"),
    "Set C++ log verbosity: 0=silent 1=error 2=info 3=debug.");
  m.def("get_log_level", []() { return pyglob::GetLogLevel(); });

  m.attr("__version__") = "2.1.1-pydglobalizer";
}

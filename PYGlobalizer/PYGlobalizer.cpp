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

  void validate() const {
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
      ", verbose=" + std::to_string(verbose) + ")";
  }
};

// ------------------------------------------------------------------------------------------------
static SolutionResult* run_solver(IProblem* problem_ptr, std::string& solver_name)
{
  SolutionResult* result = nullptr;

  // Отпускаем GIL: решатель - чистый C++. GIL берётся заново
  // только при вызове целевой функции в PYProblem::CalculateFunctionals.
  py::gil_scoped_release release;

  if (SelectSolver(problem_ptr)) {
    solver_name = "Solver";
    PY_LOG_INFO("Using Solver (index method)");
    Solver solver(problem_ptr);
    if (solver.Solve() != SYSTEM_OK)
      throw std::runtime_error("Solver::Solve() failed");
    result = solver.GetSolutionResult();
  }
  else {
    solver_name = "HDSolver";
    PY_LOG_INFO("Using HDSolver (high-dimensional method)");
    HDSolver solver(problem_ptr);
    if (solver.Solve() != SYSTEM_OK)
      throw std::runtime_error("HDSolver::Solve() failed");
    result = solver.GetSolutionResult();
  }
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
static void apply_parameters(const SolverParameters& params)
{
  parameters.MaxNumOfPoints = params.max_iterations;
  parameters.r = params.r;
  parameters.NumPoints = params.num_threads;
  parameters.NumThread = params.num_threads;
  parameters.Epsilon = params.epsilon;

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
    GlobalizerInitialization(0, nullptr, false, false);
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
        )pbdoc")
    .def(py::init<>())
    .def_readwrite("max_iterations", &SolverParameters::max_iterations)
    .def_readwrite("r", &SolverParameters::r)
    .def_readwrite("num_threads", &SolverParameters::num_threads)
    .def_readwrite("epsilon", &SolverParameters::epsilon)
    .def_readwrite("local_refine", &SolverParameters::local_refine)
    .def_readwrite("local_iterations", &SolverParameters::local_iterations)
    .def_readwrite("verbose", &SolverParameters::verbose)
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

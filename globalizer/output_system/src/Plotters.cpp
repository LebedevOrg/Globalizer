#include "Plotters.h"
#include "Trial.h"

#include <vector>
#include <string>
#include <iostream>

#include <filesystem>

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

#ifdef USE_PYTHON

// аналог np.arange
std::vector<double> arange(double start, double stop, double step) {
    std::vector<double> values;
    for (double value = start; value < stop; value += step) {
        values.push_back(value);
    }
    return values;
}

// аналог np.meshgrid
void meshgrid(
    const std::vector<double>& x,
    const std::vector<double>& y,
    std::vector<std::vector<double>>& xgrid,
    std::vector<std::vector<double>>& ygrid) {

    size_t num_rows = y.size();
    size_t num_cols = x.size();

    xgrid.resize(num_rows, std::vector<double>(num_cols));
    ygrid.resize(num_rows, std::vector<double>(num_cols));

    for (size_t i = 0; i < num_rows; ++i) {
        for (size_t j = 0; j < num_cols; ++j) {
            xgrid[i][j] = x[j];
            ygrid[i][j] = y[i];
        }
    }
}

// аналог вычисления матрицы значений по np.meshgrid
void calculate_z_matrix(
    IProblem* problem,
    double* optim_point,
    int indx1, int indx2,
    const std::vector<std::vector<double>>& xgrid,
    const std::vector<std::vector<double>>& ygrid,
    std::vector<std::vector<double>>& z_matrix, int func_i) {

    size_t rows = xgrid.size();
    size_t cols = xgrid[0].size();

    z_matrix.resize(rows, std::vector<double>(cols));

    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            optim_point[indx1] = xgrid[i][j];
            optim_point[indx2] = ygrid[i][j];

            try {
                z_matrix[i][j] = problem->CalculateFunctionals(optim_point, func_i);
            }
            catch (...) {
                z_matrix[i][j] = MaxDouble;
            }
        }
    }
}

#endif

#ifdef USE_PYTHON
void make_problem_info_file(IProblem* problem, SolutionResult* result, const char* problem_file_name = "_problem_info.txt",
  CalcsTypes calcs_type = ObjectiveFunction, CalcsTypes calcs_type_c = ObjectiveFunction, std::initializer_list<int> params = { 0, 1 },
  std::initializer_list<double[2]> bounds = {}, int objective_grid_size = 100, int constraints_grid_size = 200) {

    FILE* pf = fopen(problem_file_name, "w+");

    if (!pf) {
        throw std::logic_error("Error: cant open file");
    }

    fprintf(pf, "%d ", problem->GetDimension());

    double* left_bound = new double[problem->GetDimension()];
    double* right_bound = new double[problem->GetDimension()];
    problem->GetBounds(left_bound, right_bound);

    if (params.size() == bounds.size())
    {
        if (params.size() == 1) {
            int p = *(params.begin() + 0);
            left_bound[p] = *(bounds.begin() + 0)[0];
            right_bound[p] = *(bounds.begin() + 0)[1];
        }
        else if (params.size() == 2) {
            int p0 = *(params.begin() + 0);
            int p1 = *(params.begin() + 1);
            left_bound[p0] = *(*(bounds.begin() + 0) + 0);
            right_bound[p0] = *(*(bounds.begin() + 0) + 1);
            left_bound[p1] = *(*(bounds.begin() + 1) + 0);
            right_bound[p1] = *(*(bounds.begin() + 1) + 1);
        }
    }

    for (int i = 0; i < problem->GetDimension() - 1; i++) {
        fprintf(pf, "%lf_", left_bound[i]);
    }
    fprintf(pf, "%lf ", left_bound[problem->GetDimension() - 1]);
    for (int i = 0; i < problem->GetDimension() - 1; i++) {
        fprintf(pf, "%lf_", right_bound[i]);
    }
    fprintf(pf, "%lf\n", right_bound[problem->GetDimension() - 1]);

    fprintf(pf, "%d %d\n", calcs_type == ObjectiveFunction, problem->GetNumberOfConstraints() && calcs_type_c == ObjectiveFunction);

    if (calcs_type == ObjectiveFunction || problem->GetNumberOfConstraints() && calcs_type_c == ObjectiveFunction) {
        int points_count = (calcs_type == ObjectiveFunction) ? objective_grid_size : constraints_grid_size;

        double* dx = new double[params.size()];
        for (int i = 0; i < params.size(); i++) {
            dx[i] = fabs(right_bound[*(params.begin() + i)] - left_bound[*(params.begin() + i)]) / points_count;
        }

        std::vector<std::vector<double>> x(params.size());
        for (int i = 0; i < params.size(); i++) {
            x[i] = arange(left_bound[*(params.begin() + i)], right_bound[*(params.begin() + i)], dx[i]);
        }

        int NumOfFunctionalsForCalc = problem->GetNumberOfFunctions() - 1 + (calcs_type == ObjectiveFunction);

        if (params.size() == 2) {
            std::vector<std::vector<double>> xgrid;
            std::vector<std::vector<double>> ygrid;
            meshgrid(x[0], x[1], xgrid, ygrid);

            double* optim_point = new double[problem->GetDimension()];
            for (int j = 0; j < problem->GetDimension(); j++) {
                optim_point[j] = result->BestTrial->y[j];
            }

            std::vector <std::vector<std::vector<double>>> z(NumOfFunctionalsForCalc);
            for (int i = 0; i < NumOfFunctionalsForCalc; i++)
                calculate_z_matrix(problem, optim_point, *(params.begin() + 0), *(params.begin() + 1), xgrid, ygrid, z[i], i);

            size_t rows = xgrid.size();
            size_t cols = xgrid[0].size();

            for (size_t i = 0; i < rows; ++i) {
                for (size_t j = 0; j < cols; ++j) {
                    fprintf(pf, "%lf %lf ", xgrid[i][j], ygrid[i][j]);
                    for (int k = 0; k < NumOfFunctionalsForCalc; k++) {
                        fprintf(pf, "| %lf ", z[k][i][j]);
                    }
                    fprintf(pf, "\n");
                }
            }

            delete[] optim_point;
        }
        else if (params.size() == 1) {

            double* x_val = new double[problem->GetDimension()];
            for (int j = 0; j < problem->GetDimension(); j++) {
                x_val[j] = result->BestTrial->y[j];
            }

            for (int j = 0; j < objective_grid_size; j++) {
                std::vector<double> z(NumOfFunctionalsForCalc);
                x_val[*(params.begin())] = x[0][j];
                fprintf(pf, "%lf ", x_val[0]);

                for (int k = 0; k < NumOfFunctionalsForCalc; k++) {
                    try {
                        z[k] = problem->CalculateFunctionals(x_val, k);
                    }
                    catch (...) {
                        z[k] = MaxDouble;
                    }
                    fprintf(pf, "| %lf ", z[k]);
                }

                fprintf(pf, "\n");
            }
            delete[] x_val;
        }

        delete[] dx;
    }

    delete[] left_bound;
    delete[] right_bound;

    fclose(pf);
}
#endif

#ifdef USE_PYTHON

void Plotter::draw_plot(IProblem* problem, SolutionResult* result,
    std::initializer_list<int> params, std::initializer_list<double[2]> bounds,
    int continuous_params_num, 
    wchar_t* output_file_name, FigureTypes figure_type, CalcsTypes calcs_type,
    CalcsTypes calcs_type_c, int levels, int objective_grid_size, int constraints_grid_size,
    bool fill_feasible_region, bool hide_trials_points, bool hide_no_feasible_points,
    bool move_points_under_graph, bool show_figure)
{
    if (problem == nullptr)
    {
        throw std::invalid_argument("Error: problem is not init");
    }

    if (result == nullptr)
    {
        throw std::invalid_argument("Error: solution is not init");
    }

    if (params.size() == 0 && problem->GetDimension() == 1)
    {
        params = { 0 };
    }
    else if (params.size() > 1 && problem->GetDimension() == 1)
    {
        params = { *(params.begin()) };
    }
    else if (params.size() > 1 && continuous_params_num == 1)
    {
        params = { *(params.begin()) };
    }
    else if (params.size() == 0 && problem->GetDimension() >= 2)
    {
        params = { 0, 1 };
    }
    else if (params.size() > 2)
    {
        throw std::invalid_argument("Error: the number of parameters must be 1 or 2");
    }
    else
    {
        for (const int* param = params.begin(); param != params.end(); param++)
        {
            if (*param >= problem->GetDimension() || *param < 0)
            {
                throw std::invalid_argument("Error: invalid parameter value");
            }
        }
        if (params.size() == 2 && (*(params.begin() + 0) == *(params.begin() + 1)))
        {
            throw std::invalid_argument("Error: parameters cannot match");
        }
    }

    if (bounds.size() > 2)
    {
        throw std::invalid_argument("Error: the number of bounds must be 1 for 1-dim or 2 for 2-dim plot");
    }

    PyThreadState* main_ts = nullptr;
    bool isPythonInit = false;
    if (!Py_IsInitialized())
    {
        Py_Initialize();

        PyErr_Print();
        PyEval_InitThreads();

        PyErr_Print();
        main_ts = PyEval_SaveThread();  // Освобождаем GIL в каждом потоке
        isPythonInit = true;
    }

    PyGILState_STATE gstate = PyGILState_Ensure();

    std::filesystem::path build_path = std::filesystem::absolute("./");
    std::filesystem::path script_path = std::filesystem::absolute("../lib/plotters/start_cpp.py");

    std::string solver_output_file = parameters.IterPointsSavePath.ToString();
    std::string problem_info_file = "_problem_info.txt";

    std::wstring _params = L"[";
    _params += std::to_wstring(*(params.begin()));
    if (params.size() > 1)
    {
        _params += L", ";
        _params += std::to_wstring(*(params.begin() + 1));
    }
    _params += L"]";

    std::wstring _solver_output_file = std::wstring(solver_output_file.begin(), solver_output_file.end());
    std::wstring _problem_info_file = std::wstring(problem_info_file.begin(), problem_info_file.end());
    
    std::wstring _eps = std::to_wstring(parameters.Epsilon);
    std::wstring _levels = std::to_wstring(levels);
    std::wstring _objective_grid_size = std::to_wstring(objective_grid_size);
    std::wstring _constraints_grid_size = std::to_wstring(constraints_grid_size);

    make_problem_info_file(problem, result, problem_info_file.c_str(), calcs_type, calcs_type_c, params, bounds, objective_grid_size, constraints_grid_size);

    wchar_t* __build_path = wcsdup(build_path.wstring().c_str());
    wchar_t* __script_path = wcsdup(script_path.wstring().c_str());
    wchar_t* __params = wcsdup(_params.c_str());

    wchar_t* __solver_output_file = _solver_output_file.data();
    wchar_t* __problem_info_file = _problem_info_file.data();

    wchar_t* __eps = _eps.data();
    wchar_t* __levels = _levels.data();
    wchar_t* __objective_grid_size = _objective_grid_size.data();
    wchar_t* __constraints_grid_size = _constraints_grid_size.data();

    wchar_t __figure_type[64];
    wchar_t __calcs_type[64];
    wchar_t __calcs_type_c[64];

    wchar_t __fill_feasible_region[6];
    wchar_t __hide_trials_points[6];
    wchar_t __hide_no_feasible_points[6];
    wchar_t __move_points_under_graph[6];
    wchar_t __show_figure[6];

    wcscpy(__figure_type, figure_type == LevelLayers ? L"lines layers" : L"surface");
    wcscpy(__calcs_type, calcs_type == ObjectiveFunction ? L"objective function" :
        calcs_type == Approximation ? L"approximation" :
        calcs_type == Interpolation ? L"interpolation" :
        calcs_type == ByPoints ? L"by points" :
        L"only points");
    wcscpy(__calcs_type_c, calcs_type_c == ObjectiveFunction || calcs_type == ObjectiveFunction ? L"objective function" : L"interpolation");

    wcscpy(__fill_feasible_region, fill_feasible_region ? L"True" : L"False");
    wcscpy(__hide_trials_points, hide_trials_points ? L"True" : L"False");
    wcscpy(__hide_no_feasible_points, hide_no_feasible_points ? L"True" : L"False");
    wcscpy(__move_points_under_graph, move_points_under_graph ? L"True" : L"False");
    wcscpy(__show_figure, show_figure ? L"True" : L"False");

    wchar_t* args[] =
    {
            __script_path,
            __build_path,
            __solver_output_file,
            __problem_info_file,
            __eps,
            __levels,
            __objective_grid_size,
            __constraints_grid_size,
            __figure_type,
            __calcs_type,
            __calcs_type_c,
            __params,
            __move_points_under_graph,
            output_file_name,
            __show_figure,
            __hide_trials_points,
            __hide_no_feasible_points,
            __fill_feasible_region,
            NULL
    };

    PySys_SetArgvEx(sizeof(args) / sizeof(wchar_t*) - 1, args, 0);

    // ИСПРАВЛЕННАЯ ЧАСТЬ:
    std::cout << "The Python charting script has been launched...\n";

    // Способ 1: Чтение файла и выполнение через PyRun_SimpleString
    std::ifstream file_stream(script_path.string());
    if (!file_stream.is_open())
    {
        std::cerr << "Python script wasn't opened! File: " << script_path.string() << std::endl;

        // Освобождаем память
        free(__build_path);
        free(__script_path);
        free(__params);

        if (isPythonInit)
        {
            if (Py_IsInitialized())
            {
                PyEval_RestoreThread(main_ts);
                Py_Finalize();
            }
        }
        return;
    }

    std::cout << "The Python charting script has been opened...\n";

    // Читаем весь файл
    std::stringstream buffer;
    buffer << file_stream.rdbuf();
    std::string python_code = buffer.str();
    file_stream.close();

    // Выполняем скрипт
    int py_result = PyRun_SimpleString(python_code.c_str());

    if (py_result != 0)
    {
        std::cerr << "An error occurred while executing the script...\n";
        if (PyErr_Occurred())
        {
            PyErr_Print();

            // Дополнительная информация об ошибке
            PyObject* ptype, * pvalue, * ptraceback;
            PyErr_Fetch(&ptype, &pvalue, &ptraceback);

            if (pvalue != nullptr) {
                PyObject* pvalue_str = PyObject_Str(pvalue);
                if (pvalue_str != nullptr) {
                    const char* error_msg = PyUnicode_AsUTF8(pvalue_str);
                    if (error_msg != nullptr) {
                        std::cerr << "Python error: " << error_msg << std::endl;
                    }
                    Py_DECREF(pvalue_str);
                }
            }

            PyErr_Restore(ptype, pvalue, ptraceback);
        }
    }
    else
    {
        std::cout << "Python script executed successfully.\n";
    }
    PyGILState_Release(gstate);
    if (isPythonInit)
    {
        if (Py_IsInitialized())
        {
            PyEval_RestoreThread(main_ts);
            Py_Finalize();
        }
    }

    delete __build_path;
    delete __script_path;
    delete __params;

}
#endif
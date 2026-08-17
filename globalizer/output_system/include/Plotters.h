#pragma once


#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <algorithm>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <iostream>

#include "ProblemInterface.h"
#include "SolutionResult.h"


#ifdef _GLOBALIZER_BENCHMARKS
#include "IGlobalOptimizationProblem.h"
#include "GlobalOptimizationProblemManager.h"
#endif // _GLOBALIZER_BENCHMARKS

#ifndef WIN32
#include <unistd.h>
#endif

#include "Common.h"
/**
\file Plotters.h

\authors Усова М.А.
\date 2025
\copyright ННГУ им. Н.И. Лобачевского

\brief Объявление функций, организующих работу стороннего рисовальщика

*/

#ifdef USE_PYTHON
namespace Plotter
{
  /**
    Запуск отрисовки (требует обязательного совместного использования с ключом -sip)
    \param[in] problem - указатель на задачу,
    \param[in] result - указатель на полученное решение,
    \param[in] params - индексы вещественных параметров задачи, для которых следует произвести визуализацию
    (допускается 2 или 1 (TBD) параметра для визуализации),
    \param[in] bounds - границы построения графика по заданными параметрам
    (по умолчанию совпадают с границами области решения задачи),
    \param[in] output_file_name - имя файла для сохранения изображения с указанием формата,
    \param[in] figure_type - тип визуализации целевой функции
    (доступные режимы: LevelLayers, Surface),
    \param[in] calcs_type - тип вычислений значений для визуализации целевой функции
    (доступные режимы: ObjectiveFunction, Approximation, Interpolation, ByPoints, OnlyPoints),
    \param[in] calcs_type_с - тип вычислений значений для визуализации ограничений
    (в режиме calcs_type = ObjectiveFunction автоматически согласуется; иначе
    доступные режимы: ObjectiveFunction, Interpolation),
    \param[in] levels - количество линий уровня визуализации целевой функции,
    \param[in] objective_grid_size - размерность сетки для визуализации целевой функции,
    \param[in] constraints_grid_size - размерность сетки для визуализации ограничений
    (в режиме calcs_type = calcs_type_с = ObjectiveFunction игнорируется, используется значение objective_grid_size),
    \param[in] fill_feasible_region - флаг о необходимости выделить цветом допустимую область,
    \param[in] hide_trials_points - флаг о необходимости скрыть точки испытаний на графике
    (в формате булевого значения - true, false),
    \param[in] move_points_under_graph - флаг о необходимости сместить точки испытаний под график
    (в формате булевого значения - true, false),
    \param[in] show_figure - флаг о необходимости открыть полученный рисунок в интерактивном окне на экране
    (в формате булевого значения - true, false)
    */
    void draw_plot(IProblem* problem, SolutionResult* result,
        std::initializer_list<int> params = {}, std::initializer_list<double[2]> bounds = {},
        int continuous_params_num = 2, 
        wchar_t* output_file_name = L"globalizer_output.png", FigureTypes figure_type = LevelLayers,
        CalcsTypes calcs_type = ObjectiveFunction, CalcsTypes calcs_type_c = ObjectiveFunction,
        int levels = 25, int objective_grid_size = 100, int constraints_grid_size = 200,
        bool fill_feasible_region = false, bool hide_trials_points = false, bool hide_no_feasible_points = false,
        bool move_points_under_graph = false, bool show_figure = false);
};
#endif

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2025 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      NoConstraintsIterationMain.cpp                              //
//                                                                         //
//  Purpose:   Console version of Globalizer system                        //
//                                                                         //
//  Author(s): Varseeva E.                                                 //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


#include "Globalizer.h"
#include "IterationOutput.h"

// ------------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    GlobalizerInitialization(argc, argv);

    parameters.Dimension = 2; // Размерность задачи
    parameters.IsPlot = true; // Включаем рисование графика функции с точками испытаний (сохраняются в файл)
    IProblem* problem = new ProblemFromFunctionPointers(
        parameters.Dimension,
        std::vector<double>(parameters.Dimension, -2.2), // нижняя граница
        std::vector<double>(parameters.Dimension, 1.8), // верхняя граница
        std::vector<std::function<double(const double*)>>(1, [](const double* y) {
            const double pi = 3.14159265358979323846;
            double sum = 0.0;
            for (int i = 0; i < parameters.Dimension; ++i)
                sum += y[i] * y[i] - 10.0 * cos(2.0 * pi * y[i]) + 10.0;
            return sum;
            }),
        true, 0.0, std::vector<double>(parameters.Dimension, 0.0));
    problem->Initialize();

    // Решатель
    Solver solver(problem);
    // Инициализируем решатель
    solver.Initialize();

    // Пошагово решаем задачу
    bool finished = false;
    for (int iter = 0; !finished && iter < 2000; ++iter)
        solver.DoIteration(finished);

    PrintFinal(solver);

    if (parameters.IsMPIInit()) 
        MPI_Finalize();

    return 0;
}
// - end of file ----------------------------------------------------------------------------------
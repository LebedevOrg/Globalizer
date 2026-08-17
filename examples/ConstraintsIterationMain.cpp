/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2025 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      ConstraintsIterationMain.cpp                                //
//                                                                         //
//  Purpose:   Console version of Globalizer system                        //
//                                                                         //
//  Author(s): Varseeva E.                                                 //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


#include "Globalizer.h"
#include "IterationOutput.h"

// ------------------------------------------------------------------------------------------------
double StronginC3Functionals(const double* y, int fNumber)
{
    double res = 0.0;
    double x1 = y[0], x2 = y[1];
    switch (fNumber)
    {
    case 0: // ограничение 1
        res = 0.01 * ((x1 - 2.2) * (x1 - 2.2) + (x2 - 1.2) * (x2 - 1.2) - 2.25);
        break;
    case 1: // ограничение 2
        res = 100.0 * (1.0 - ((x1 - 2.0) / 1.2) * ((x1 - 2.0) / 1.2) -
            (x2 / 2.0) * (x2 / 2.0));
        break;
    case 2: // ограничение 3
        res = 10.0 * (x2 - 1.5 - 1.5 * sin(6.283 * (x1 - 1.75)));
        break;
    case 3: // критерий
    {
        double t1 = pow(0.5 * x1 - 0.5, 4.0);
        double t2 = pow(x2 - 1.0, 4.0);
        res = 1.5 * x1 * x1 * exp(1.0 - x1 * x1 - 20.25 * (x1 - x2) * (x1 - x2));
        res = res + t1 * t2 * exp(2.0 - t1 - t2);
        res = -res;
    }
    break;
    }

    return res;
}

// ------------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    GlobalizerInitialization(argc, argv);

    parameters.Dimension = 2; // Размерность задачи
    parameters.IsPlot = true; // Включаем рисование графика функции с точками испытаний (сохраняются в файл)

    IProblem* problem = new ProblemFromFunctionPointers(
        parameters.Dimension, 
        { 0.0,-1.0 }, // нижняя граница
        { 4.0,3.0 }, // верхняя граница
        StronginC3Functionals,
        4 // количество функций (3 ограничения + 1 критерий)
    );
    parameters.CalcsType = Interpolation;
    problem->Initialize();

    // Решатель
    Solver solver(problem);
    // Инициализируем решатель
    solver.Initialize();

    // Пошагово решаем задачу
    bool finished = false;
    for (int iter = 0; !finished && iter < 3000; ++iter)
        solver.DoIteration(finished);

    PrintFinal(solver); 

    if (parameters.IsMPIInit()) 
        MPI_Finalize();

    return 0;
}
// - end of file ----------------------------------------------------------------------------------
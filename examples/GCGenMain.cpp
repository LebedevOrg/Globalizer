/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2025 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      GCGenMain.cpp                                               //
//                                                                         //
//  Purpose:   Console version of Globalizer system                        //
//                                                                         //
//  Author(s): Lebedev I., Barkalov K.,                                    //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "Globalizer.h"
#include "GKLS/GKLSProblem.hpp"
#include "GlobalizerProblemGCGen.h"
#include "Grishagin/grishagin_function.hpp"

int main(int argc, char* argv[])
{
    GlobalizerInitialization(argc, argv, true, true);

    TGKLSProblem* gkls_problem = new TGKLSProblem(1, 2, Simple, TD);
    TGrishaginProblem* grishagin_problem = new TGrishaginProblem(5);

    // Создаем обертку для GKLS задачи под интерфейс IOptProblem
    GlobalizerOptProblem* wrapped_problem = new GlobalizerOptProblem(gkls_problem);

    wrapped_problem->Initialize();
    parameters.Dimension = 2;
    parameters.IsPlot = true; // Включаем рисование графика функции с точками испытаний (сохраняются в файл)
    parameters.ShowFigure = true;
    parameters.FigureType = 1; // 0 - LevelLayers - линии уровней (по умолчанию); 1 - Surface - поверхность
    parameters.CalcsType = 2;// 0 - ObjectiveFunction - строит линии уровня / поверхность по сетке 100 * 100,
    //1 - Approximation - строит аппроксимацию линий уровня / поверхности по имеющейся поисковой информации,
    //  2 - Interpolation - строит интерполяцию линий уровня / поверхности по имеющейся поисковой информации,
    //  3 - ByPoints - строит поверхность путем "натягивагия" ее на точки поисковой информации без сглаживания,
    //  4 - OnlyPoints - отображает только распределение точек поисковой информации в области поиска.

    Solver solver(wrapped_problem);
    if (solver.Solve() != SYSTEM_OK)
        throw EXCEPTION("Error: solver.Solve crash!!!");

    delete wrapped_problem;

    MPI_Finalize();
    return 0;
}
// - end of file ----------------------------------------------------------------------------------

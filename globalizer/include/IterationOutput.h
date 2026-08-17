/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2025 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      IterationOutput.h                                           //
// 
//  Author(s): Varseeva E.                                                 //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Solver.h"

inline void PrintFinal(Solver& solver)
{
    SolutionResult* res = solver.GetSolutionResult();
    if (!res || !res->BestTrial) { if (res) delete res; return; }

    Task* task = solver.GetTask();
    if (!task) { delete res; return; }

    const Trial* best = res->BestTrial;

    print << "\n";
    print << "Iteration = " << res->IterationCount << "\n";
    print << "min = " << best->FuncValues[best->index] << " \n";

    for (int i = 0; i < task->GetN(); i++)
        print << "x[" << i << "] = " << best->y[i] << " \n";

    if (task->GetNumOfFunc() > 1) {
        print << "\n";
        for (int i = 0; i < task->GetNumOfFunc(); i++)
            print << "M[" << i << "] = " << best->FuncValues[i] << " \n";
    }

    print << "NumberOfTrials = " << res->TrialCount << "\n";
    print << "\n\n";

    delete res;
}
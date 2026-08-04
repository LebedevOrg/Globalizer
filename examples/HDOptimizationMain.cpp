/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2025 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      HDOptimizationMain.cpp                                      //
//                                                                         //
//  Purpose:   Console version of Globalizer system                        //
//                                                                         //
//  Author(s): Lebedev I., Barkalov K.                                     //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


#include "Globalizer.h"

// ------------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
  GlobalizerInitialization(argc, argv);

#ifdef _GLOBALIZER_BENCHMARKS
  GlobalOptimizationProblemManager manager;
  IGlobalOptimizationProblem* problem = 0;
  if (InitProblemGlobalizerBenchmarks(manager, problem))
  {
    print << "Error during problem initialization\n";
    return 0;
  }

  // Решатель
  HDSolver solver(problem);

  // Решаем задачу
  if (solver.Solve() != SYSTEM_OK)
    throw EXCEPTION("Error: solver.Solve crash!!!");

#endif

  if (parameters.IsMPIInit())
    MPI_Finalize();

  return 0;
}
// - end of file ----------------------------------------------------------------------------------

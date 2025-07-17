/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2016 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      init_problem.h                                              //
//                                                                         //
//  Purpose:   Header file for program                                     //
//                                                                         //
//  Author(s): Sysoyev A., Lebedev I., Sovrasov V.                         //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __INIT_PROBLEM_H__
#define __INIT_PROBLEM_H__

#include "parameters.h"
#include "problem_manager.h"
#include "problem.h"

int InitProblem(TProblemManager& problemManager, IProblem*& problem,
                int argc, char* argv[], bool isMPIInit = false);

#endif //__INIT_PROBLEM_H__
// - end of file ----------------------------------------------------------------------------------

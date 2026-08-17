/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2025 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      Globalizer.h                                                //
//                                                                         //
//  Purpose:   Console version of Globalizer system                        //
//                                                                         //
//  Author(s): Lebedev I.                                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

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

#include "Solver.h"
#include "ProblemFromFunctionPointers.h"
#include "SolverRSA.hpp"
#include "HDSolver.h"

#ifdef _GLOBALIZER_BENCHMARKS
#include "IGlobalOptimizationProblem.h"
#include "GlobalOptimizationProblemManager.h"
#endif // _GLOBALIZER_BENCHMARKS

#ifndef WIN32
#include <unistd.h>
#endif

  /**
  Инициализация систем вывода
  \param[in] argc - Количество аргументов командной строки
  \param[in] argv - Аргументы командной строки
  \param[in] isMPIInit - Проинициализировать ли MPI,
  требуется для корректной печати в файл в многопроцессорном режиме
  \param[in] isPrintParameters - Печатать ли параметры оптимизации
  \param[in] mLogFileName - Имя (префикс) лог файла
  \param[in] processCount - Число процессов
  \param[in] processNumber - Номер текущего процесса
  \param[in] isPrintToFile - Печатать ли в файл все сообщения в debug, в release не печатается
  \param[in] errorsName - Имена (расшифровка) ошибок
  \param[in] errorsCode - Коды ошибок
  \param[in] errorsCount - Число ошибок
  последние три параметра инициализируются по умолчанию в методе SetDefaultErrors
  */
void GlobalizerInitialization(int argc=0, char* argv[]=nullptr, 
  bool isMPIInit = false, bool isPrintParameters = false,
  std::string mLogFileName = "", int processCount = -1, 
  int processNumber = -1, bool isPrintToFile = false, 
  std::string* errorsName = nullptr, int* errorsCode = nullptr, 
  int errorsCount = 0);

/**
Решение задачи оптимизации
\param[in] problem - задача оптимизации

*/
SolutionResult* GlobalizerSolveProblem(IProblem*& problem);

/** Создает параметры для тестовых задач //временное решение!!!
  \param[in] argc - Количество аргументов командной строки
  \param[in] argv - Аргументы командной строки
*/
void CreateCurentProblemsParameters(int argc, char* argv[]);

/**
Выбор решателя, true - Solver, false - HDSolver
\param[in] problem - задача оптимизации

*/
bool SelectSolver(IProblem* problem);
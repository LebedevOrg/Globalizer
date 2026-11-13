/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2026 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      HybridSimpleMain.cpp                                        //
//  Purpose:   Demonstration of Hybrid Local Calculation                   //
//                                                                         //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "Globalizer.h"
#include <iostream>
#include <functional>
#include <vector>
#include <cmath>

// ------------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
  GlobalizerInitialization(argc, argv);

  parameters.Dimension = 4; // Задача размерности 4
  
  parameters.TypeCalculation = HybridLocalCalc; 

  // Указываем, какие переменные будут оптимизироваться локально
  parameters.LocalVarIndices.SetSize(2);
  parameters.LocalVarIndices[0] = 2;
  parameters.LocalVarIndices[1] = 3;

  // Настройка параметров локального поиска
  parameters.HybridLocalStep = 0.1;       // Начальный шаг Хук-Дживса
  parameters.HybridLocalTolerance = 1e-4; // Точность локального метода
  parameters.HybridLocalMaxIter = 50;     // Максимум итераций локального метода

  parameters.MaxNumOfPoints = 1000; // Ограничим число испытаний для быстрого теста
  parameters.IsPlot = true;         // Включаем визуализацию (будет построена проекция на первые 2 оси)
  parameters.IsPrintResultToConsole = true;

  std::cout << "Initializing Hybrid Optimization:\n";
  std::cout << "  Global variables (AGP): x[0], x[1]\n";
  std::cout << "  Local variables: x[2], x[3]\n";
  std::cout << "  Problem: 4D Rastrigin function\n";

  IProblem* problem = new ProblemFromFunctionPointers(
    parameters.Dimension, // размерность задачи
    std::vector<double>(parameters.Dimension, -5.12), // нижняя граница
    std::vector<double>(parameters.Dimension,  5.12), // верхняя граница
    std::vector<std::function<double(const double*)>>(1, [](const double* y) {
      // Функция Растригина для 4 переменных
      const double pi = 3.14159265358979323846;
      double sum = 0.0;
      for (int j = 0; j < 4; j++) {
        sum += y[j] * y[j] - 10.0 * cos(2.0 * pi * y[j]) + 10.0;
      }
      return sum;
    }), 
    true, // определен ли оптимум
    0.0,  // значение глобального оптимума
    std::vector<double>(parameters.Dimension, 0.0) // координаты глобального минимума [0, 0, 0, 0]
  );

  problem->Initialize();

  Solver solver(problem);

  std::cout << "Starting solver...\n";
  if (solver.Solve() != SYSTEM_OK) {
    throw EXCEPTION("Error: solver.Solve crash!!!");
  }

  if (parameters.IsMPIInit()) {
    MPI_Finalize();
  }

  delete problem;
  return 0;
}
// - end of file ----------------------------------------------------------------------------------
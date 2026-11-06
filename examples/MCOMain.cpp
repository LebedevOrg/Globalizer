/**
\file MCOMain.cpp
\brief Решение многокритериальных задач (89)-(92) из диссертации Козинова и задачи BNH с ограничениями

Запуск: GlobalizerMCOMain <задача> <tau> <eps> <r> [r_loc] [файл.csv] [масштабирование]
  задача - 89, 90, 91, 92 или bnh; r_loc = 0 - метод с одним r;
  масштабирование 1 - делить критерии на разброс значений.
Пример: GlobalizerMCOMain 89 100 0.06 2 1.5 trials.csv
*/

#include "Globalizer.h"
#include "MCOProblemFromFunctions.h"
#include "MCOSolver.h"

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>

typedef MCOProblemFromFunctions::Function Function;

// ------------------------------------------------------------------------------------------------
static std::unique_ptr<IProblem> CreateProblem(const std::string& name)
{
  const double pi = 3.14159265358979323846;

  if (name == "89")
  {
    parameters.Dimension = 2;
    return std::make_unique<MCOProblemFromFunctions>(2, std::vector<double>{ 0.0, 0.0 }, std::vector<double>{ 1.0, 1.0 },
      std::vector<Function>{
        [](const double* y) { return (y[0] - 1) * y[1] * y[1] + 1; },
        [](const double* y) { return y[1]; } });
  }
  if (name == "90")
  {
    parameters.Dimension = 2;
    return std::make_unique<MCOProblemFromFunctions>(2, std::vector<double>{ 0.0, 0.0 }, std::vector<double>{ 2.0, 2.0 },
      std::vector<Function>{
        [](const double* y) { return y[0]; },
        [](const double* y) { return std::min(std::fabs(y[0] - 1), 1.5 - y[0]) + y[1] + 1; } });
  }
  if (name == "91")
  {
    parameters.Dimension = 3;
    return std::make_unique<MCOProblemFromFunctions>(3, std::vector<double>(3, -4.0), std::vector<double>(3, 4.0),
      std::vector<Function>{
        [](const double* y)
        {
          const double a = 1.0 / std::sqrt(3.0);
          double s = 0.0;
          for (int i = 0; i < 3; i++)
            s += (y[i] - a) * (y[i] - a);
          return 1.0 - std::exp(-s);
        },
        [](const double* y)
        {
          const double a = 1.0 / std::sqrt(3.0);
          double s = 0.0;
          for (int i = 0; i < 3; i++)
            s += (y[i] + a) * (y[i] + a);
          return 1.0 - std::exp(-s);
        } });
  }
  if (name == "92")
  {
    parameters.Dimension = 2;
    return std::make_unique<MCOProblemFromFunctions>(2, std::vector<double>{ 0.0, 0.0 }, std::vector<double>{ 1.0, 1.0 },
      std::vector<Function>{
        [](const double* y) { return y[0]; },
        [pi](const double* y)
        {
          const double g = 1 + 10 * y[1];
          return (1 - (y[0] / g) * (y[0] / g) - (y[0] / g) * std::sin(8 * y[0] * pi)) * g;
        } });
  }
  if (name == "bnh")
  {
    parameters.Dimension = 2;
    return std::make_unique<MCOProblemFromFunctions>(2, std::vector<double>{ 0.0, 0.0 }, std::vector<double>{ 5.0, 3.0 },
      std::vector<Function>{
        [](const double* y) { return 4 * y[0] * y[0] + 4 * y[1] * y[1]; },
        [](const double* y) { return (y[0] - 5) * (y[0] - 5) + (y[1] - 5) * (y[1] - 5); } },
      std::vector<Function>{
        [](const double* y) { return (y[0] - 5) * (y[0] - 5) + y[1] * y[1] - 25; },
        [](const double* y) { return 7.7 - (y[0] - 8) * (y[0] - 8) - (y[1] + 3) * (y[1] + 3); } });
  }
  throw EXCEPTION("Unknown task");
}

// ------------------------------------------------------------------------------------------------
static void SaveTrials(const std::string& fileName, const std::vector<MCOPoint>& trials)
{
  std::ofstream file(fileName);
  file << std::setprecision(17);
  for (const MCOPoint& trial : trials)
  {
    file << trial.lambdaIndex;
    for (double y : trial.y)
      file << ',' << y;
    for (double f : trial.criteria)
      file << ',' << f;
    file << '\n';
  }
}

// ------------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
  if (argc < 5)
  {
    std::cout << "Usage: GlobalizerMCOMain <89|90|91|92|bnh> <tau> <eps> <r> [r_loc] [trials.csv] [scaling]\n";
    return 1;
  }

  GlobalizerInitialization(1, argv);

  std::unique_ptr<IProblem> problem = CreateProblem(argv[1]);
  problem->Initialize();

  parameters.Epsilon = std::atof(argv[3]);
  parameters.r = std::atof(argv[4]);
  parameters.MaxNumOfPoints = 1000000;
  parameters.IsPrintResultToConsole = false;

  MCOParameters mco;
  mco.numberOfLambdas = std::atoi(argv[2]);
  mco.rLocal = (argc > 5) ? std::atof(argv[5]) : 0.0;
  mco.isScaling = (argc > 7) && std::atoi(argv[7]) != 0;

  MCOSolver solver(problem.get(), mco);
  if (solver.Solve() != SYSTEM_OK)
    throw EXCEPTION("Error: solver.Solve crash!!!");

  const std::vector<MCOPoint> trials = solver.GetTrials();
  std::cout << "Trials = " << trials.size() << "\nPareto points = " << solver.GetParetoSet().size() << "\n";

  if (argc > 6)
    SaveTrials(argv[6], trials);

  return 0;
}
// - end of file ----------------------------------------------------------------------------------

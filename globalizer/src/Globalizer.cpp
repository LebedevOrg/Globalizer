#include "Globalizer.h"

// ------------------------------------------------------------------------------------------------
void GlobalizerInitialization(int argc, char* argv[], bool isMPIInit,
  bool isPrintParameters, std::string mLogFileName, int processCount,
  int processNumber, bool isPrintToFile,
  std::string* errorsName, int* errorsCode, int errorsCount)
{
  if (isPrintParameters)
  {
    std::cout << "\n\n" << std::endl;
    for (int i = 1; i < argc; i++)
    {
      std::cout << argv[i] << " ";
    }
    std::cout << "\n\n" << std::endl;
  }

  if (isMPIInit)
    MPI_Init(&argc, &argv);

  // Инициализация параметров
  parameters.Init(argc, argv, isMPIInit);

  if (mLogFileName == "")
    mLogFileName = parameters.LogFileNamePrefix;

  if (processCount == -1)
    processCount = parameters.GetProcNum();

  if (processNumber == -1)
    processNumber = parameters.GetProcRank();

  // Инициализация системы вывода и печати ошибок
  OutputMessage::Init(isMPIInit, mLogFileName, processCount,
    processNumber, isPrintToFile, errorsName, errorsCode, errorsCount);

  if (!parameters.IsStart())
  {
    print << "Need command-line arguments!";
    _ERROR_(-1);
  }

  CreateCurentProblemsParameters(argc, argv);

  if (isPrintParameters)
  {
    if (parameters.GetProcRank() == 0 && !parameters.DisablePrintParameters)
    {
      parameters.PrintParameters();

      unsigned long size = 256;
      char* CompName = 0;
#ifdef WIN32
      LPWSTR buffer = new wchar_t[size];
      for (unsigned long i = 0; i < size; i++)
        buffer[i] = 0;
      GetComputerNameW(buffer, &size);
      CompName = new char[size + 1];
      for (unsigned long i = 0; i < size; i++)
        CompName[i] = (char)buffer[i];
      CompName[size] = 0;
      size++;
#else
      char* hostname = new char[size];
      for (unsigned long i = 0; i < size; i++)
        hostname[i] = 0;
      gethostname(hostname, 256);
      size = (unsigned long)strlen(hostname);
      CompName = new char[size + 1];
      for (unsigned long i = 0; i < size; i++)
        CompName[i] = (char)hostname[i];
      CompName[size] = 0;
      size++;
#endif

      print << CompName << "\tProcRank = " << parameters.GetProcRank() << "\tProcNum = " << parameters.GetProcNum() << "\n";
    }
  }
}


SolutionResult* GlobalizerSolveProblem(IProblem*& problem)
{
  ISolver* solver;

  parameters.AutomaticParametersSetting = true;

  if (parameters.MaxNumOfPoints > 100
    && parameters.NumThread.GetIsChange() == false && parameters.NumPoints.GetIsChange() == false
    && parameters.TypeCalculation == OMP)
  {
    parameters.NumThread = 1;
    parameters.NumPoints = parameters.NumThread;
  }

  int iterationByDimention = parameters.IterationsCount / parameters.Dimension;
  if (!IsBelowGraph(parameters.Dimension, parameters.IterationsCount))
  {
    if (parameters.IterationsCount.GetIsChange() || !(parameters.MaxNumOfPoints.GetIsChange()))
      parameters.MaxNumOfPoints = parameters.IterationsCount;

    solver = new Solver(problem);
  }
  else
  {
    int C = parameters.Dimension;
    int z = parameters.IterationsCount;
    int y = parameters.HDSolverIterationCount;
    int x = parameters.MaxNumOfPoints;
    FindXY(x, y, z, C);
    parameters.HDSolverIterationCount = y;
    parameters.MaxNumOfPoints = x + 3;

    if (!parameters.MaxIterationsWithoutImprovement.GetIsChange())
      parameters.MaxIterationsWithoutImprovement = parameters.IterationsCount / 10;

    // Решатель
    solver = new HDSolver(problem);
  }

  // Решаем задачу
  if (solver->Solve() != SYSTEM_OK)
    throw EXCEPTION("Error: solver.Solve crash!!!");

  return solver->GetSolutionResult();
}

// ------------------------------------------------------------------------------------------------
void CreateCurentProblemsParameters(int argc, char* argv[])
{

}

// ------------------------------------------------------------------------------------------------
bool SelectSolver(IProblem* problem)
{
  int dim = problem->GetDimension();
  if (dim == 1 ||
    dim == 2 && parameters.IterationsCount > 100 ||
    dim == 3 && parameters.IterationsCount > 300 ||
    dim == 4 && parameters.IterationsCount > 1000 ||
    dim == 5 && parameters.IterationsCount > 4000)
  {
    return true;
  }
  else
  {
    return false;
  }
}
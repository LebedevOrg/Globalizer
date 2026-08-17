#include "HDSolver.h"
#include "TaskFactory.h"

// ------------------------------------------------------------------------------------------------
void HDSolver::SetDimentions(std::vector<int> _dimentions)
{
  if (_dimentions.size() == 0)
  {
    dimensions.resize(originalDimension - originalDiscreteParamsNum);
    for (int i = 0; i < dimensions.size(); i++)
      dimensions[i] = 1;
    // Все дискретные параметры добавляются к последней задаче
    // (TODO: изменить алгоритм формирования групп параметров)
    dimensions[dimensions.size() - 1] += originalDiscreteParamsNum;
  }
  else
  {
    dimensions = _dimentions;
  }
}

// ------------------------------------------------------------------------------------------------
void HDSolver::CreateStartPoint()
{
  if (parameters.StartPoint.GetIsChange() == false)
  {
    double* A = new double[originalDimension - originalDiscreteParamsNum];
    double* B = new double[originalDimension - originalDiscreteParamsNum];
    problem->GetBounds(A, B);
    parameters.StartPoint.SetSize(originalDimension);

    int i = 0;
    for (; i < originalDimension - originalDiscreteParamsNum; i++)
    {
      parameters.StartPoint[i] = A[i] + (B[i] - A[i]) / 2.0;
    }

    for (; i < originalDimension; i++)
    {
        parameters.StartPoint[i] = 0;
    }

  }
  parameters.IsUseStartPoint = true;
}

// ------------------------------------------------------------------------------------------------
void HDSolver::Construct()
{
  solvers.resize(dimensions.size());
  tasks.resize(dimensions.size());
  for (int i = 0; i < dimensions.size(); i++)
  {
    solvers[i] = new Solver(problem);
    tasks[i] = nullptr;
  }
  solutionResult = nullptr;
  CreateStartPoint();
}



// ------------------------------------------------------------------------------------------------
HDSolver::HDSolver(IProblem* _problem, int discreteParamsNum, std::vector<int> _dimentions)
{
  problem = _problem;

  originalDimension = parameters.Dimension;
  originalDiscreteParamsNum = discreteParamsNum;

  // Настраиваем параметры
  AutoConfig();
  SetDimentions(_dimentions);
  Construct();

  finalSolver = nullptr;
  pData = nullptr;
  pTask = nullptr;
  parameters.startParameterNumber = 0;
}

// ------------------------------------------------------------------------------------------------
#ifdef _GLOBALIZER_BENCHMARKS
HDSolver::HDSolver(IGlobalOptimizationProblem* _problem, std::vector<int> _dimentions)
  : HDSolver::HDSolver(new GlobalizerBenchmarksProblem(_problem), _problem->GetNumberOfDiscreteVariable(), _dimentions)
{
}
#endif

// ------------------------------------------------------------------------------------------------
HDSolver::~HDSolver()
{
  if (solutionResult == nullptr)
    delete solutionResult;
  for (int i = 0; i < solvers.size(); i++)
  {
    if (solvers[i] != nullptr)
    {
      auto solver = solvers[i];
      delete solver;
    }
    if (tasks[i] != nullptr)
    {
      auto task = tasks[i];
      delete task;
    }
  }
}

// ------------------------------------------------------------------------------------------------
void HDSolver::AddPoint(Solver* solver, int curDimensions, std::vector<Trial*>& points, int startParameterNumber)
{
  std::vector<Trial*>& curPoints = solver->GetAllPoint();
  std::vector <double> curY(curDimensions);
  for (Trial* point : curPoints)
  {
    for (int k = 0; k < curDimensions; k++)
    {
      curY[k] = point->y[k];
    }
    for (int j = 0; j < originalDimension; j++)
    {
      point->y[j] = parameters.StartPoint[j];
    }
    for (int j = 0; j < curDimensions; j++)
    {
      point->y[j + startParameterNumber] = curY[j];
    }

    points.push_back(point->Clone());
  }
}

// ------------------------------------------------------------------------------------------------
void HDSolver::UpdateStartPoint(SolutionResult* solution, double& bestValue, int curDimensions,
  int startParameterNumber, std::vector<Trial*>& points, HDTask* curTask)
{
  if (solution->BestTrial->index == problem->GetNumberOfConstraints())
  {
#ifdef WIN32
    if (_finite(solution->BestTrial->GetValue()) != 0)
#else
    if (std::isfinite(solution->BestTrial->GetValue()) != 0)
#endif
    {
      if (bestValue > solution->BestTrial->GetValue())
      {
        bestValue = solution->BestTrial->GetValue();
        for (int j = 0; j < curDimensions; j++)
        {
          parameters.StartPoint[j + startParameterNumber] = solution->BestTrial->y[j];
        }

        print << "\t Iteration " << points.size() << "\t" << "bestValue =\t" << bestValue << "\n";
        countIterationsWithoutImprovement = points.size();

      }
    }
  }

  for (int j = 0; j < curDimensions; j++)
  {
    if (j + startParameterNumber < alternativeStartingPoint.size())
      alternativeStartingPoint[j + startParameterNumber] = solution->BestTrial->y[j];
  }
}

// ------------------------------------------------------------------------------------------------
void  HDSolver::CreateData()
{
  // В случае, если не совпадают (задача пришла снаружи — берём новые), иначе всё равно
  IProblem* _problem = problem;

  // Создание задачи (TODO: перенести в фабрику)
  if (pTask == 0)
  {
    pTask = TaskFactory::CreateTask(_problem, 0);
  }

  // Создаём данные для поисковой информации
  if (pData == 0)
  {
    pData = new SearchData(_problem->GetNumberOfFunctions());
    int qSize = GLOBALIZER_MAX((int)pow(2.0, (int)(log((double)parameters.MaxNumOfPoints)
      / log(2.0) - 2)) - 1, 1023);
    pData->ResizeQueue(qSize);
  }
  else
  {
    pData->Clear();
  }

  parameters.serializer->SetSearchData(pData);
  parameters.serializer->SetTask(pTask);
}

// ------------------------------------------------------------------------------------------------
void HDSolver::LoadPoint()
{
  std::string pointsPath = parameters.FirstPointFilePath;
  std::string pointsPathExtension = GetFileExtension(pointsPath);

  if (pointsPathExtension == "json")
  {
    CreateData();
    bool isOpen = parameters.serializer->LoadFromFile(pointsPath, fd);

    if (isOpen)
    {
      if (parameters.M_constant.GetSize() < fd.searchData.M.size())
        parameters.M_constant.SetSize(fd.searchData.M.size());

      for (int j = 0; j < fd.searchData.M.size(); j++)
      {
        parameters.M_constant[j] = fd.searchData.M[j];
      }
      parameters.StartPoint.SetSize(parameters.Dimension);
      parameters.startParameterNumber = (fd.methodParams.start_parameter_number + 1) % parameters.Dimension;

      Trial* best = fd.trials[0];

      for (auto trial : fd.trials)
      {
        if (trial->index > best->index || trial->index == best->index &&
          trial->FuncValues[best->index] < best->FuncValues[best->index])
        {
          best = trial;
        }
      }

      parameters.StartPoint.SetSize(parameters.Dimension);
      for (int i = 0; i < parameters.Dimension; i++)
      {
        parameters.StartPoint[i] = best->y[i];
      }
    }
  }
}

// ------------------------------------------------------------------------------------------------
int HDSolver::Solve()
{
  try
  {
    auto doLV = parameters.LocalRefineSolution;
    auto isPrint = parameters.IsPrintResultToConsole;

    parameters.M_constant.SetSize(problem->GetNumberOfFunctions());
    for (int j = 0; j < problem->GetNumberOfFunctions(); j++)
    {
      parameters.M_constant[j] = 1;
    }

    auto mnp = parameters.MaxNumOfPoints;
    int iterationCount = parameters.HDSolverIterationCount;

    alternativeStartingPoint.resize(originalDimension);
    for (int j = 0; j < originalDimension; j++)
    {
      alternativeStartingPoint[j] = parameters.StartPoint[j];
    }

    std::vector<Trial*> points;
    double bestValue = MaxDouble;

    LoadPoint();

    for (int iteration = 0; iteration < iterationCount; iteration++)
    {
      parameters.LocalRefineSolution = None;
      parameters.IsPrintResultToConsole = false;
      int startParameterNumber = parameters.startParameterNumber;

      for (int i = startParameterNumber; i < solvers.size(); i++)
      {
        parameters.Dimension = dimensions[i];

        if (originalDiscreteParamsNum != 0) {
          if (i < solvers.size() - 1) {
            if (parameters.TypeMethod == IntegerMethod)
            {
              parameters.TypeMethod = StandartMethod;
            }
          }
          else if (i == solvers.size() - 1) {
            if (parameters.TypeMethod != IntegerMethod)
            {
              parameters.TypeMethod = IntegerMethod;
            }
          }
        }

        Solver* solver = solvers[i];
        if (tasks[i] != nullptr)
          delete tasks[i];
        tasks[i] = dynamic_cast<HDTask*>(TaskFactory::CreateTask(problem, 0));

        tasks[i]->SetStartParameterNumber(startParameterNumber);

        if (originalDiscreteParamsNum != 0 && i == solvers.size() - 1) {
          tasks[i]->SetMixedInteger();
          tasks[i]->SetStartIndex(solvers.size() - 1);
        }
        else {
          tasks[i]->UnsetMixedInteger();
        }

        parameters.startParameterNumber = startParameterNumber;

        solver->Solve(tasks[i]);

        auto solution = solver->GetSolutionResult();

        AddPoint(solver, dimensions[i], points, startParameterNumber);

        UpdateStartPoint(solution, bestValue, dimensions[i], startParameterNumber, points, tasks[i]);

        startParameterNumber = startParameterNumber + parameters.Dimension;

        parameters.Dimension = originalDimension;

        Calculation::leafCalculation = 0;

        for (int j = 0; j < tasks[i]->GetNumOfFunc(); j++)
        {
          if (solver->GetData()->M[j] > parameters.M_constant[j])
          {
            parameters.M_constant[j] = solver->GetData()->M[j];
          }
        }

        if ((points.size() - countIterationsWithoutImprovement > parameters.MaxIterationsWithoutImprovement)
            && (parameters.StopCondition == MaxIterWithoutImprovement))
        {
          break;
        }

        // Если допустимая точка не найдена
        if (bestValue == MaxDouble)
        {
          for (int j = 0; j < originalDimension; j++)
          {
            parameters.StartPoint[j] = alternativeStartingPoint[j];
          }
        }
      }

      if (iteration == iterationCount - 1)
      {
        parameters.LocalRefineSolution = doLV;
        parameters.IsPrintResultToConsole = isPrint;
      }

      // Если нужно локальное улучшение
      if ((parameters.LocalRefineSolution != None) || (iteration == iterationCount - 1))
      {
        std::string FileSerializer = parameters.FileSerializer.ToString();
        parameters.FileSerializer = "";

        // Для срабатывания условия остановки (чтобы работало только локальное уточнение)
        parameters.MaxNumOfPoints = points.size() / solvers.size();

        if (finalSolver == nullptr)
          finalSolver = new Solver(problem);
        else
        {
          delete finalSolver;
          finalSolver = new Solver(problem);
        }

        finalSolver->SetPoint(points);

        finalSolver->Solve();

        if (solutionResult != nullptr)
          delete solutionResult;
        solutionResult = finalSolver->GetSolutionResult();

        AddPoint(finalSolver, originalDimension, points, 0);

        UpdateStartPoint(solutionResult, bestValue, originalDimension, startParameterNumber,
            points, dynamic_cast<HDTask*>(finalSolver->GetTask()));

        parameters.FileSerializer = FileSerializer;
      }

      // Если допустимая точка не найдена
      if (bestValue == MaxDouble)
      {
        for (int j = 0; j < originalDimension; j++)
        {
          parameters.StartPoint[j] = alternativeStartingPoint[j];
        }
      }

      parameters.startParameterNumber = 0;
    }
    parameters.MaxNumOfPoints = mnp;
  }
  catch (const Exception& e)
  {
    std::string excFileName = std::string("exception_") +
      toString(parameters.GetProcRank()) + ".txt";
    e.Print(excFileName.c_str());

    for (int i = 0; i < parameters.GetProcNum(); i++)
      if (i != parameters.GetProcRank())
        MPI_Abort(MPI_COMM_WORLD, i);
    return 1;
  }
  catch (...)
  {
    print << "\nUNKNOWN EXCEPTION !!!\n";
    std::string excFileName = std::string("exception_") +
      toString(parameters.GetProcRank()) + ".txt";
    Exception e("UNKNOWN FILE", -1, "UNKNOWN FUCNTION", "UNKNOWN EXCEPTION");
    e.Print(excFileName.c_str());

    for (int i = 0; i < parameters.GetProcNum(); i++)
      if (i != parameters.GetProcRank())
        MPI_Abort(MPI_COMM_WORLD, i);
    return 1;
  }
  if (parameters.GetProcRank() == 0)
  {
    if (parameters.GetProcNum() > 1) {
      int childNum = parameters.GetProcNum() - 1;
      int curr_child = 0;
      for (unsigned int i = 0; i < childNum; i++) {
        ///curr_child = parameters.parallel_tree.ProcChild[i];!!!!!
        int finish = 1;
        MPI_Send(&finish, 1, MPI_INT, curr_child, TagChildSolved, MPI_COMM_WORLD);
      }
    }
  }

  return 0;
}

// ------------------------------------------------------------------------------------------------
SolutionResult* HDSolver::GetSolutionResult()
{
  return solutionResult;
}

// ------------------------------------------------------------------------------------------------
void HDSolver::SetPoint(std::vector<Trial*>& points)
{
  finalSolver->SetPoint(points);
}

// ------------------------------------------------------------------------------------------------
std::vector<Trial*>& HDSolver::GetAllPoint()
{
  return finalSolver->GetAllPoint();
}

// ------------------------------------------------------------------------------------------------
void HDSolver::AutoConfig()
{
  int tasksNum = originalDimension - originalDiscreteParamsNum;
  parameters.TypeSolver = HDSearch;
  parameters.AutomaticParametersSetting = true;

  if (parameters.NumThread.GetIsChange() == false && parameters.NumPoints.GetIsChange() == false
    && parameters.TypeCalculation == OMP)
  {
    parameters.NumThread = 1;
    parameters.NumPoints = parameters.NumThread;
  }

  if (!parameters.MaxIterationsWithoutImprovement.GetIsChange())
    parameters.MaxIterationsWithoutImprovement = parameters.IterationsCount / 10;

  if (parameters.HDSolverIterationCount.GetIsChange() == false)
  {
    if (parameters.MaxNumOfPoints.GetIsChange() == true && parameters.IterationsCount.GetIsChange() == true)
    {
      parameters.HDSolverIterationCount = parameters.IterationsCount / parameters.MaxNumOfPoints / tasksNum;
    }
    // Если задано только число итераций на размерность
    else if (parameters.MaxNumOfPoints.GetIsChange() == true && parameters.IterationsCount.GetIsChange() == false)
    {
      parameters.IterationsCount = parameters.MaxNumOfPoints * tasksNum * parameters.HDSolverIterationCount;
    }
    // Если задано только общее число итераций
    else if (parameters.MaxNumOfPoints.GetIsChange() == false)
    {
      int trialsPerDimention = parameters.IterationsCount / tasksNum;
      if (trialsPerDimention < 100)
      {
        // Если небольшой ресурс и большая размерность - запускаем координатный решатель один раз 
        parameters.HDSolverIterationCount = 1;
        parameters.MaxNumOfPoints = parameters.IterationsCount / tasksNum + 3;
      }
      else
      {
        // Если большой ресурс и большая размерность - запускаем координатный решатель несколько раз
        parameters.MaxNumOfPoints = 100;
        parameters.HDSolverIterationCount = parameters.IterationsCount / parameters.MaxNumOfPoints / tasksNum;
      }
    }
  }
  // Если задачно число итераций HDSolver
  else
  {
    // В случае значений по умолчанию
    if (parameters.MaxNumOfPoints.GetIsChange() == false && parameters.IterationsCount.GetIsChange() == false)
    {
      parameters.MaxNumOfPoints = parameters.IterationsCount / parameters.HDSolverIterationCount / tasksNum;
    }
    else if (parameters.MaxNumOfPoints.GetIsChange() == true)
    {
      parameters.IterationsCount = parameters.MaxNumOfPoints * tasksNum * parameters.HDSolverIterationCount;
    }
  }
}

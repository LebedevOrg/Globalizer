#include "Globalizer.h"
#include "DynamicProblem.h"
#include <iostream>

enum ProblemName {
  DYNAMIC_PROBLEM,
  DYNAMIC_PROBLEM_LAMBDA_EXPRESSION
};

// ------------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
  GlobalizerInitialization(argc, argv);

  ProblemName problemName = DYNAMIC_PROBLEM;
  // Параметры задачи
  parameters.Dimension = 1;
  parameters.StopCondition = IterationOnly;
  parameters.TypeCalculation = CPUMultiPoint;
  parameters.IterationsCount = 10000;
  parameters.NumPoints = 16;
  parameters.r = 10;
  parameters.IsPrintResultToConsole = false;

  int iterationCount = 1;
  double initialData = 1.0;  // начальное значение data
  double range = pow(2.0, 64.0) / initialData;
  int N = 8;

  SolutionResult** results = new SolutionResult * [N * iterationCount];
  double left = 0;
  double right = 0;
  double h = range / (N * iterationCount);

  Performance Timer;
  Timer.Start();

  for (int iteration = 0; iteration < iterationCount; iteration++)
  {
    Solver** solvers = new Solver * [N];
    IGPUProblem** problems = new IGPUProblem * [N];


    left = h * N * iteration;
    right = h * N * (iteration + 1);

    //#pragma omp parallel for num_threads((int)N)
    for (int i = 0; i < N; i++)
    {
      double a = left + h * i;
      double b = left + h * (i + 1);


      if (problemName == DYNAMIC_PROBLEM)
      {
        // Создаём задачу с изменяемым параметром
        problems[i] = new DynamicProblem(
          parameters.Dimension,  // размерность
          a,                     // нижняя граница
          b,                     // верхняя граница
          initialData            // начальное значение data
        );
      }
      else if (problemName == DYNAMIC_PROBLEM_LAMBDA_EXPRESSION)
      {
        problems[i] = new ProblemFromFunctionPointers(parameters.Dimension, // размерность задачи
          { a }, // нижняя граница
          { b },// верхняя граница
          [&](const double* y, int fn) {
#define FunctionalParametersType double
#define IntParametersType unsigned long long int

            auto IntToFunctionalParametersType = [](IntParametersType t)
              {
                FunctionalParametersType res = 0;
                res = *((FunctionalParametersType*)((char*)(&t)));
                return res;
              };

            auto Functional = [](FunctionalParametersType x)
              {
                return sin(x) * cos(FunctionalParametersType(1.0) / x) + FunctionalParametersType(1.0);
              };

            IntParametersType t = (IntParametersType)(y[0]);
            // Для одномерной задачи берём первую координату
            double minVal = 0;
            double minX = 0;
            FunctionalParametersType x = IntToFunctionalParametersType(t);
            double result = Functional(x);
            minVal = result;
            minX = (double)t;


            return minVal;
          },
          [&](double* y, int fNumber, int& numPoints, double* values)
          {
#define FunctionalParametersType double
#define IntParametersType unsigned long long int

            auto IntToFunctionalParametersType = [](IntParametersType t)
              {
                FunctionalParametersType res = 0;
                res = *((FunctionalParametersType*)((char*)(&t)));
                return res;
              };

            auto Functional = [](FunctionalParametersType x)
              {
                return sin(x) * cos(FunctionalParametersType(1.0) / x) + FunctionalParametersType(1.0);
              };

            const int n = numPoints;

            for (size_t j = 0; j < n; ++j)
            {
              IntParametersType t = (IntParametersType)(y[j]);

              FunctionalParametersType x = IntToFunctionalParametersType(t);
              double result = Functional(x);
              values[j] = result;
            }
          }
        , 1);

      }

      // Инициализация задачи
      problems[i]->Initialize();

      // Решатель
      solvers[i] = new Solver(problems[i]);

      // Решаем задачу
      if (solvers[i]->Solve() != SYSTEM_OK)
        throw EXCEPTION("Error: solver.Solve crash!!!");

      results[iteration * N + i] = solvers[i]->GetSolutionResult();
    }

    for (int i = 0; i < N; i++)
    {


      double a = left + h * i;
      double b = left + h * (i + 1);
      std::cout << "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||\n";
      //std::cout << std::fixed << std::setprecision(0);
      std::cout << "a = " << a << " b = " << b << std::endl;
      solvers[i]->PrintResultToConsole();

      delete solvers[i];
      delete problems[i];
    }

    delete[] solvers;
    delete[] problems;
  }

  double duration = Timer.GetTime();

  Trial* optimum = results[0]->BestTrial;
  int optI = 0;
  int optIteration = 0;

  for (int iteration = 0; iteration < iterationCount; iteration++)
  {
    for (int i = 0; i < N; i++)
    {
      int index = iteration * N + i;
      if (optimum->GetValue() > results[index]->BestTrial->GetValue())
      {
        optimum = results[index]->BestTrial;
        optI = i;
        optIteration = iteration;

      }
    }
  }

  std::cout << "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||\n";
  std::cout << "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||\n";
  std::cout << "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||\n";

  left = h * N * optIteration;
  right = h * N * (optIteration + 1);
  double a = left + h * optI;
  double b = left + h * (optI + 1);

  DynamicProblem* problem = new DynamicProblem(
    parameters.Dimension,  // размерность
    a,                     // нижняя граница
    b,                     // верхняя граница
    initialData            // начальное значение data
  );
  problem->Initialize();

  double oldY = optimum->y[0];

  unsigned long long int t = 0;

  double optVal = problem->FinishCalculateFunctionals(optimum->y, t);

  std::cout << "optimum y = " << oldY << " t = " << t << " optimum value = " << optimum->GetValue() << std::endl;

  print << "\nAll Solve time = " << duration << "\n\n\n";

  return 0;
}

// - end of file ----------------------------------------------------------------------------------

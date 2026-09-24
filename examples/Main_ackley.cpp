/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//  File:      Main.cpp  (пример с новой тестовой функцией — Ackley)        //
//                                                                         //
//  Purpose:   Console version of Globalizer system                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "Globalizer.h"

// ------------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
  GlobalizerInitialization(argc, argv);
  IProblem* problem = nullptr;
#ifdef _GLOBALIZER_BENCHMARKS
  GlobalOptimizationProblemManager manager;
  IGlobalOptimizationProblem* globalizerBenchmarksProblem = 0;
  if (InitProblemGlobalizerBenchmarks(manager, globalizerBenchmarksProblem))
  {
    print << "Error during problem initialization\n";
    return 0;
  }
  problem = new GlobalizerBenchmarksProblem(globalizerBenchmarksProblem);
#else
  // ---------------------------------------------------------------------------------------------
  // ТЕСТОВАЯ ФУНКЦИЯ ЭКЛИ (Ackley) — классический многоэкстремальный бенчмарк.
  //   f(x) = -a*exp(-b*sqrt( (1/n) * sum x_i^2 ))
  //          - exp( (1/n) * sum cos(c*x_i) ) + a + e
  // при a = 20, b = 0.2, c = 2*pi.
  // Область поиска: x_i in [-32.768, 32.768]. Глобальный минимум: f(0,...,0) = 0.
  // Функция «зашита» в код так же, как Растригин в исходном examples/Main.cpp.
  // ---------------------------------------------------------------------------------------------
  parameters.Dimension = 5;
  problem = new ProblemFromFunctionPointers(parameters.Dimension, // размерность задачи
    std::vector<double>(parameters.Dimension, -32.768), // нижняя граница
    std::vector<double>(parameters.Dimension,  32.768), // верхняя граница
    std::vector<std::function<double(const double*)>>(1, [&](const double* y)
      {
        const int    dim = problem->GetDimension();
        const double a   = 20.0;
        const double b   = 0.2;
        const double c   = 2.0 * 3.14159265358979323846;

        double sumSq  = 0.0; // сумма квадратов координат
        double sumCos = 0.0; // сумма косинусов
        for (int j = 0; j < dim; j++)
        {
          sumSq  += y[j] * y[j];
          sumCos += cos(c * y[j]);
        }

        const double term1 = -a * exp(-b * sqrt(sumSq / dim));
        const double term2 = -exp(sumCos / dim);
        return term1 + term2 + a + exp(1.0);
      }), // критерий
    true, // определён ли оптимум
    0.0,  // значение глобального оптимума
    std::vector<double>(parameters.Dimension, 0.0) // координаты глобального минимума
  );
  problem->Initialize();
#endif

  ISolver* solver;

  if (SelectSolver(problem))
  {
    solver = new Solver(problem);
  }
  else
  {
    solver = new HDSolver(problem);
  }

  // Решаем задачу
  if (solver->Solve() != SYSTEM_OK)
    throw EXCEPTION("Error: solver.Solve crash!!!");

  if (parameters.IsMPIInit())
    MPI_Finalize();

  return 0;
}
// - end of file ----------------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2016 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      init_problem.cpp                                            //
//                                                                         //
//  Purpose:   Source file for program                                     //
//                                                                         //
//  Author(s): Sysoyev A., Lebedev I., Sovrasov V.                         //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "InitProblem.h"

int InitProblem(ProblemManager& problemManager, IProblem*& problem,
  int argc, char* argv[], bool isMPIInit)
{
  if (problemManager.LoadProblemLibrary(parameters.LibPath) != ProblemManager::OK_)
  {
    //сообщение об ошибке печатает manager
    return 1;
  }

  IProblem* baseProblem = problemManager.GetProblem();
  CombinableBaseParameters* newProblem = dynamic_cast<CombinableBaseParameters*>(baseProblem);
  problem = baseProblem;
  if (newProblem == 0)
  {
    problem->SetConfigPath(parameters.LibConfigPath);
    if (problem->Initialize() != ProblemManager::OK_)
    {
      std::cout << "Error during problem initialization\n";
      return 1;
    }

    if (parameters.Dimension.GetIsChange())
    {
      //вообще, вызов SetDimension лучше убрать и получать размерность из конфигурационного файла для всех задач, где она не фиксирована
      if (problem->SetDimension(parameters.Dimension) != ProblemManager::OK_)
      {
        std::cout << "Unsupported problem dimension!\n";
        return 1;
      }
    }
    //размерность задачи из конфигурационного файла имеет приоритет над значением из командной строки
    parameters.Dimension = problem->GetDimension();
  }
  else
  {
    problem->SetConfigPath(parameters.LibConfigPath);
    newProblem->SetInitParam(argc, argv, isMPIInit);
    if (problem->Initialize() != ProblemManager::OK_)
    {
      std::cout << "Error during problem initialization\n";
      return 1;
    }

    parameters.CombineOptions(newProblem->GetOptions(), newProblem->GetOptionsCount());
    newProblem->CombineOptions(parameters.GetOptions(), parameters.GetOptionsCount());

    newProblem->InitDataByParameters();
  }
  return 0;
}


#ifdef _GLOBALIZER_BENCHMARKS
void ReadParameters(IGlobalOptimizationProblem*& problem)
{
  std::vector<std::string> names; 
  std::vector<std::string> values;
  problem->GetParameters(names, values);

  for (int i = 0; i < names.size(); i++)
  {
    values[i] = parameters.GetStringVal(names[i]);
    if (values[i] != "")
      problem->SetParameter(names[i], values[i]);
  }
}

// ------------------------------------------------------------------------------------------------
int InitProblemGlobalizerBenchmarks(GlobalOptimizationProblemManager& problemManager, IGlobalOptimizationProblem*& problem)
{
  std::string LibPath = parameters.LibPath;
  if (problemManager.LoadProblemLibrary(LibPath) != GlobalOptimizationProblemManager::OK_)
  {
    //сообщение об ошибке печатает manager
    return 1;
  }

  IGlobalOptimizationProblem* baseProblem = problemManager.GetProblem();

  ReadParameters(baseProblem);

  baseProblem->Initialize();

  if (parameters.Dimension.GetIsChange())
  {
    auto res = baseProblem->SetDimension(parameters.Dimension);
    if (res != IGlobalOptimizationProblem::PROBLEM_OK)
    {
      print << " WARNING!!! Dimentions not change! \n";
      parameters.Dimension = baseProblem->GetDimension();
    }
  }
  else
    parameters.Dimension = baseProblem->GetDimension();


  std::vector<double> y(baseProblem->GetDimension() - baseProblem->GetNumberOfDiscreteVariable());
  std::vector<std::string> u(baseProblem->GetNumberOfDiscreteVariable());
  std::vector<double> values(baseProblem->GetNumberOfFunctions());

  int err = baseProblem->GetStartTrial(y, u, values);
  if ((err == IGlobalOptimizationProblem::PROBLEM_OK) && parameters.IsUseStartPoint)
  {
    parameters.StartPoint.SetSize(baseProblem->GetDimension() - baseProblem->GetNumberOfDiscreteVariable());
    for (int i = 0; i < baseProblem->GetDimension() - baseProblem->GetNumberOfDiscreteVariable(); i++)
    {
      parameters.StartPoint[i] = y[i];
    }
  }

  problem = baseProblem;
  return 0;
}
#endif // _GLOBALIZER_BENCHMARKS

// - end of file ----------------------------------------------------------------------------------

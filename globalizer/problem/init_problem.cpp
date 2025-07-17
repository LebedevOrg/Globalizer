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

#include "init_problem.h"

int InitProblem(TProblemManager& problemManager, IProblem*& problem,
  int argc, char* argv[], bool isMPIInit)
{
  if (problemManager.LoadProblemLibrary(parameters.libPath) != TProblemManager::OK_)
  {
    //сообщение об ошибке печатает manager
    return 1;
  }

  IProblem* baseProblem = problemManager.GetProblem();
  TCombinableBaseParameters* newProblem = dynamic_cast<TCombinableBaseParameters*>(baseProblem);
  problem = baseProblem;
  if (newProblem == 0)
  {
    problem->SetConfigPath(parameters.libConfigPath);
    if (problem->Initialize() != TProblemManager::OK_)
    {
      printf("Error during problem initialization\n");
      return 1;
    }

    if (parameters.Dimension.GetIsChange())
    {
      //вообще, вызов SetDimension лучше убрать и получать размерность из конфигурационного файла для всех задач, где она не фиксирована
      if (problem->SetDimension(parameters.Dimension) != TProblemManager::OK_)
      {
        printf("Unsupported problem dimension!\n");
        return 1;
      }
    }
    //размерность задачи из конфигурационного файла имеет приоритет над значением из командной строки
    parameters.Dimension = problem->GetDimension();
  }
  else
  {
    problem->SetConfigPath(parameters.libConfigPath);
    newProblem->SetInitParam(argc, argv, isMPIInit);
    if (problem->Initialize() != TProblemManager::OK_)
    {
      printf("Error during problem initialization\n");
      return 1;
    }

    parameters.CombineOptions(newProblem->GetOptions(), newProblem->GetOptionsCount());
    newProblem->CombineOptions(parameters.GetOptions(), parameters.GetOptionsCount());

    newProblem->InitDataByParameters();
  }
  return 0;
}
// - end of file ----------------------------------------------------------------------------------

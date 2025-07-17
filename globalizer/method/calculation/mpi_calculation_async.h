/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2021 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      mpi_calculation.h                                           //
//                                                                         //
//  Purpose:   Header file for Async MPI calculation class                 //
//                                                                         //
//  Author(s): Lebedev I.                                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __MPI_CALCULATION_ASYNC_H__
#define __MPI_CALCULATION_ASYNC_H__

#include "calculation.h"

class TMPICalculationAsync : public TCalculation
{
protected:
  bool isFirst = true;

  std::vector<TTrial*> vecTrials;

  /// MPI-номер потомка, закончившего решение выделенной задачи
  int ChildNumRecv; 

  /// "Внутренний" номер потомка среди всех потомков данного процесса
  int ChildNum;     

  void FirstStartCalculate(TInformationForCalculation& inputSet, TResultForCalculation& outputSet);
  void StartCalculate(TInformationForCalculation& inputSet, TResultForCalculation& outputSet);
  void RecieveCalculatedFunctional();

public:
  static void AsyncFinilize();

  TMPICalculationAsync(TTask& _pTask) : TCalculation(_pTask)
  {
  }

  // Вычисляет функции (ограничения и критерий) и индекс невыполненного ограничения по координатам
  virtual void Calculate(TInformationForCalculation& inputSet, TResultForCalculation& outputSet);
};

#endif
// - end of file ----------------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2021 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      mpi_calculation.h                                           //
//                                                                         //
//  Purpose:   Header file for MPI calculation class                       //
//                                                                         //
//  Author(s): Lebedev I.                                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __MPI_CALCULATION_H__
#define __MPI_CALCULATION_H__

#include "calculation.h"

class TMPICalculation : public TCalculation
{
protected:

  bool isFirst = true;

  //Производим вычисления
  void StartCalculate(TInformationForCalculation& inputSet, TResultForCalculation& outputSet);

  //Производим вычисления на концах изначального отрезка при нужных параметрах запуска
  //Метод не будет работать при n < 3
  void StartCalculateInBorder(TInformationForCalculation& inputSet, TResultForCalculation& outputSet);

public:
  TMPICalculation(TTask& _pTask) : TCalculation(_pTask)
  {
  }

  // Вычисляет функции (ограничения и критерий) и индекс невыполненного ограничения по координатам
  virtual void Calculate(TInformationForCalculation& inputSet, TResultForCalculation& outputSet);
};

#endif
// - end of file ----------------------------------------------------------------------------------

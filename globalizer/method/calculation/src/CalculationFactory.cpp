/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      CalculationFactory.cpp                                      //
//                                                                         //
//  Purpose:   Source file for method factory class                        //
//                                                                         //
//  Author(s): Lebedev I.                                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "CalculationFactory.h"

#include "OMPCalculation.h"
#include "MPICalculationAsync.h"
#include "MPICalculation.h"
#include "CudaCalculation.h"
#include "CPUMultiPointCalculation.h"
#include "HybridLocalCalculation.h"


// ------------------------------------------------------------------------------------------------
Calculation* CalculationFactory::CreateCalculation(Task& _pTask, IEvolvent* evolvent)
{
  Calculation* calculation = 0;

  if (!_pTask.IsLeaf())
  {
    if ((parameters.TypeCalculation == OMP))
    {
      calculation = new OMPCalculation(_pTask);
    }
    else if ((parameters.TypeCalculation == CPUMultiPoint))
    {
      calculation = new CPUMultiPointCalculation(_pTask);
    }

    else if (parameters.TypeCalculation == CUDA)
    {
      calculation = new CUDACalculation(_pTask);
    }
    else if (parameters.TypeCalculation == MPI_calc)
    {
      calculation = new MPICalculation(_pTask);
    }
    else if (parameters.TypeCalculation == AsyncMPI)
    {
      calculation = new MPICalculationAsync(_pTask);
    }
    else if (parameters.TypeCalculation == HybridLocalCalc)
    {
        calculation = new HybridLocalCalculation(_pTask);
    }
    else
    {

      if (parameters.CalculationsArray.GetSize() < _pTask.GetProcLevel())
        calculation = new CUDACalculation(_pTask);
      else
      {
        if (parameters.CalculationsArray[_pTask.GetProcLevel()] == OMP)
          calculation = new OMPCalculation(_pTask);
        else
          calculation = new CUDACalculation(_pTask);
      }
    }
  }
  return calculation;
}

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


// ------------------------------------------------------------------------------------------------
Calculation* CalculationFactory::CreateCalculation2(Task& _pTask, IEvolvent* evolvent)
{
    Calculation* calculation = 0;

    if (_pTask.IsLeaf())
    {
        if ((parameters.TypeCalculation == OMP) || (parameters.TypeCalculation == MPI_calc && parameters.GetProcNum() == 1))
        {
            if (Calculation::leafCalculation == 0)
            {
                calculation = new OMPCalculation(_pTask);
                Calculation::leafCalculation = calculation;
            }
            else
            {
                calculation = Calculation::leafCalculation;
            }
        }
        else if (parameters.TypeCalculation == CUDA)
        {
            if (Calculation::leafCalculation == 0)
            {
                calculation = new CUDACalculation(_pTask);
                Calculation::leafCalculation = calculation;
            }
            else
            {
                calculation = Calculation::leafCalculation;
            }
        }

        else if (parameters.TypeCalculation == MPI_calc)
        {
            if (Calculation::leafCalculation == 0)
            {
                calculation = new MPICalculation(_pTask);
                Calculation::leafCalculation = calculation;
            }
            else
            {
                calculation = Calculation::leafCalculation;
            }
        }
        else if (parameters.TypeCalculation == AsyncMPI)
        {
            if (Calculation::leafCalculation == 0)
            {
                calculation = new MPICalculationAsync(_pTask);
                Calculation::leafCalculation = calculation;
            }
            else
            {
                calculation = Calculation::leafCalculation;
            }
        }
        else
        {
            if (Calculation::leafCalculation == 0)
            {
                if (parameters.CalculationsArray.GetSize() < _pTask.GetProcLevel())
                    calculation = new CUDACalculation(_pTask);
                else
                {
                    if (parameters.CalculationsArray[_pTask.GetProcLevel()] == OMP)
                        calculation = new OMPCalculation(_pTask);
                    else
                        calculation = new CUDACalculation(_pTask);

                    Calculation::leafCalculation = calculation;
                }
            }
            else
            {
                calculation = Calculation::leafCalculation;
            }
        }

        if (_pTask.GetProcLevel() == 0)
        {

        }
    }


    return calculation;
}


// ------------------------------------------------------------------------------------------------
Calculation* CalculationFactory::CreateCalculation(Task& _pTask, IEvolvent* evolvent)
{
    Calculation* calculation = 0;

    if (!_pTask.IsLeaf())
    {
        if ((parameters.TypeCalculation == OMP))
        {
            if (Calculation::leafCalculation == 0)
            {
                calculation = new OMPCalculation(_pTask);
                Calculation::leafCalculation = calculation;
            }
            else
            {
                calculation = Calculation::leafCalculation;
            }
        }

        if (parameters.TypeCalculation == CUDA)
        {
            if (Calculation::leafCalculation == 0)
            {
                calculation = new CUDACalculation(_pTask);
                Calculation::leafCalculation = calculation;
            }
            else
            {
                calculation = Calculation::leafCalculation;
            }
        }
        else if (parameters.TypeCalculation == MPI_calc)
        {
            if (Calculation::leafCalculation == 0)
            {
                calculation = new MPICalculation(_pTask);
                Calculation::leafCalculation = calculation;
            }
            else
            {
                calculation = Calculation::leafCalculation;
            }
        }
        else if (parameters.TypeCalculation == AsyncMPI)
        {
            if (Calculation::leafCalculation == 0)
            {
                calculation = new MPICalculationAsync(_pTask);
                Calculation::leafCalculation = calculation;
            }
            else
            {
                calculation = Calculation::leafCalculation;
            }
        }
        else
        {
            if (Calculation::leafCalculation == 0)
            {
                if (parameters.CalculationsArray.GetSize() < _pTask.GetProcLevel())
                    calculation = new CUDACalculation(_pTask);
                else
                {
                    if (parameters.CalculationsArray[_pTask.GetProcLevel()] == OMP)
                        calculation = new OMPCalculation(_pTask);
                    else
                        calculation = new CUDACalculation(_pTask);

                    Calculation::leafCalculation = calculation;
                }
            }
            else
            {
                calculation = Calculation::leafCalculation;
            }
        }
    }
    return calculation;
}

Calculation* CalculationFactory::CreateNewCalculation(Task& _pTask, IEvolvent* evolvent)
{
    Calculation* calculation = 0;

    if (_pTask.IsLeaf())
    {
        if ((parameters.TypeCalculation == OMP))
        {
            calculation = new OMPCalculation(_pTask);
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

                Calculation::leafCalculation = calculation;
            }

        }


    }
    return calculation;
}
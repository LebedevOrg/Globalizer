/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2021 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      MPICalculation.cpp                                          //
//                                                                         //
//  Purpose:   Source file for MPI calculation class                       //
//                                                                         //
//  Author(s): Lebedev I.                                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "MPICalculation.h"

#include <stdlib.h>
#include <string.h>
#include <cstring>

#include "TaskFactory.h"
#include "TrialFactory.h"
#include "OMPCalculation.h"

// ------------------------------------------------------------------------------------------------

void MPICalculation::StartCalculate(InformationForCalculation& inputSet,
    TResultForCalculation& outputSet)
{
    for (unsigned int i = 0; i < parameters.GetProcNum() - 1; i++)
    {
        int isFinish = 0;
        // Отправляем флаг, что есть работа
        MPI_Send(&isFinish, 1, MPI_INT, i + 1, TagChildSolved, MPI_COMM_WORLD);

        // Отправляем блок точек на каждый процесс
        for (unsigned int j = 0; j < parameters.MpiBlockSize; j++) {
            Trial* trail = inputSet.trials[i * (parameters.MpiBlockSize) + j];
            trail->index = -1;
            // Отправляем координаты y
            MPI_Send(trail->y, parameters.Dimension, MPI_DOUBLE, i + 1, TagChildSolved, MPI_COMM_WORLD);
        }
    }

    MPI_Status status;
    // Сбор результатов от рабочих процессов
    for (unsigned int i = 0; i < parameters.GetProcNum() - 1; i++)
    {
        // Принимаем все отправленные точки обратно
        for (unsigned int j = 0; j < parameters.MpiBlockSize; j++) {
            Trial* trail = inputSet.trials[i * (parameters.MpiBlockSize) + j];
            trail->index = -1;

            // Принимаем вычисленное значение функции
            MPI_Recv(trail->FuncValues, MaxNumOfFunc, MPI_DOUBLE, i + 1, TagChildSolved, MPI_COMM_WORLD, &status);

            // Определяем индекс
            int fNumber = 0;
            while ((trail->index == -1) && (fNumber < pTask->GetNumOfFunc()))
            {
                if ((fNumber == (pTask->GetNumOfFunc() - 1)) || (trail->FuncValues[fNumber] > 0))
                {
                    trail->index = fNumber;
                }
                fNumber++;
            }
        }
    }

    // Подсчет количества вычислений
    for (unsigned int i = 0; i < inputSet.trials.size(); i++)
    {
        for (int j = 0; j <= outputSet.trials[i]->index; j++)
            outputSet.countCalcTrials[j]++;
    }
}

// ------------------------------------------------------------------------------------------------

void MPICalculation::StartCalculateInBorder(InformationForCalculation& inputSet,
    TResultForCalculation& outputSet)
{
    Calculation* calculation;
    calculation = new OMPCalculation(*pTask);
    calculation->Calculate(inputSet, outputSet);

    for (unsigned int i = 0; i < inputSet.trials.size(); i++)
    {
        for (int j = 0; j <= outputSet.trials[i]->index; j++)
            outputSet.countCalcTrials[j]++;
    }
}


// ------------------------------------------------------------------------------------------------

void MPICalculation::Calculate(InformationForCalculation& inputSet,
    TResultForCalculation& outputSet)
{

    if (inputSet.trials.size() > 0)
    {
        outputSet.trials.clear();
        outputSet.trials.resize(inputSet.trials.size());
        for (unsigned i = 0; i < outputSet.trials.size(); i++)
            outputSet.trials[i] = inputSet.trials[i];


        outputSet.countCalcTrials.clear();
        outputSet.countCalcTrials.resize(pTask->GetNumOfFunc());
        for (int i = 0; i < pTask->GetNumOfFunc(); i++)
            outputSet.countCalcTrials[i] = 0;

    }

    if (isStartComputingAway)
    {
        if ((isFirst) && ((parameters.IsCalculationInBorderPoint == true) || (parameters.LocalTuningType != 0)))
        {
            isFirst = false;
            StartCalculateInBorder(inputSet, outputSet);
        }
        else
            StartCalculate(inputSet, outputSet);
    }
    else
    {
        if (countCalculation > 0)
        {
            countCalculation--;

            for (unsigned i = 0; i < outputSet.trials.size(); i++)
            {
                inputCalculation.trials.push_back(inputSet.trials[i]);
            }

            if (countCalculation > 0)
                firstCalculation->ContinueComputing();

            for (unsigned int i = 0; i < inputSet.trials.size(); i++)
            {
                for (int j = 0; j <= outputSet.trials[i]->index; j++)
                    outputSet.countCalcTrials[j]++;
            }
        }

        if (countCalculation == 0)
        {
            countCalculation--;
            isStartComputingAway = true;


            for (unsigned i = 0; i < resultCalculation.trials.size(); i++)
                resultCalculation.trials[i] = inputCalculation.trials[i];

            StartCalculate(inputCalculation, resultCalculation);
        }
    }

}
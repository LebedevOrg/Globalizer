/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2026 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      HybridLocalCalculation.cpp                                  //
//                                                                         //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "HybridLocalCalculation.h"
#include "TrialFactory.h"
#include <cmath>
#include <algorithm>

// ------------------------------------------------------------------------------------------------
HybridLocalCalculation::HybridLocalCalculation(Task& originalTask)
    : Calculation(originalTask),
    mOriginalTask(&originalTask),
    mLocalTask(nullptr)
{
    std::vector<int> localIndices;
    if (parameters.LocalVarIndices.GetSize() > 0 && parameters.LocalVarIndices[0] != 0) {
        for (int i = 0; i < parameters.LocalVarIndices.GetSize(); ++i) {
            localIndices.push_back(parameters.LocalVarIndices[i]);
        }
    }
    else {
        int dim = originalTask.GetN();
        if (dim >= 2) {
            localIndices.push_back(dim - 2);
            localIndices.push_back(dim - 1);
        }
    }

    mLocalTask = new HybridLocalTask(&originalTask, localIndices);
}

// ------------------------------------------------------------------------------------------------
HybridLocalCalculation::~HybridLocalCalculation()
{
    delete mLocalTask;
}

// ------------------------------------------------------------------------------------------------
void HybridLocalCalculation::Calculate(InformationForCalculation& input, TResultForCalculation& output)
{
    int numFuncs = mOriginalTask->GetNumOfFunc();

    output.trials.resize(input.trials.size());
    output.countCalcTrials.resize(numFuncs, 0);

    for (size_t j = 0; j < input.trials.size(); ++j)
    {
        Trial* agpTrial = input.trials[j];
        if (!agpTrial) continue;

        int fNum = (agpTrial->index >= 0) ? agpTrial->index : (numFuncs - 1);

        mLocalTask->SetFixedPoint(agpTrial->y);

        Trial startPoint;
        startPoint.index = fNum;
        for (int i = 0; i < mOriginalTask->GetN(); ++i) {
            startPoint.y[i] = agpTrial->y[i];
        }
        for (int i = 0; i < numFuncs; ++i) {
            startPoint.FuncValues[i] = MaxDouble;
        }

        // Запускаем локальный метод
        LocalMethod* currentLocalMethod = new LocalMethod(mLocalTask, startPoint, 0);
        currentLocalMethod->SetMaxTrials(100);
        currentLocalMethod->SetInitialStep(0.1);

        Trial bestTrial = currentLocalMethod->StartOptimization();
        int trialsMade = currentLocalMethod->GetTrialsCounter();
        delete currentLocalMethod;

        // Проверяем, не вернул ли локальный метод мусор или HUGE_VAL
        bool localSearchFailed = (bestTrial.FuncValues[fNum] >= MaxDouble / 2.0);

        if (localSearchFailed) {
            // Локальный поиск не смог улучшить точку (или сломался). 
            // Вычисляем исходную точку штатным способом
            agpTrial->FuncValues[fNum] = mOriginalTask->CalculateFuncs(agpTrial->y, fNum);
            trialsMade = 1; // Засчитываем как одно вычисление
        }
        else {
            // Локальный поиск успешен, используем его результат
            for (int i = 0; i < mOriginalTask->GetN(); ++i) {
                agpTrial->y[i] = bestTrial.y[i];
            }
            agpTrial->FuncValues[fNum] = bestTrial.FuncValues[fNum];
        }

        agpTrial->index = fNum;

        output.trials[j] = TrialFactory::CreateTrial(agpTrial->y);
        output.trials[j]->index = agpTrial->index;

        for (int k = 0; k < numFuncs; ++k) {
            output.trials[j]->FuncValues[k] = agpTrial->FuncValues[k];
        }

        output.countCalcTrials[fNum] += trialsMade;
    }
}
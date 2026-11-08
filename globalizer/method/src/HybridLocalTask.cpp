/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2026 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      HybridLocalCalculation.h                                    //
//                                                                         //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "HybridLocalTask.h"
#include <cmath>

// ------------------------------------------------------------------------------------------------
HybridLocalTask::HybridLocalTask(Task* originalTask, const std::vector<int>& localIndices)
    : mOriginalTask(originalTask)
{
    int dim = mOriginalTask->GetN();
    mFixedValues.resize(dim);

    std::vector<bool> isLocal(dim, false);
    for (int idx : localIndices) {
        if (idx >= 0 && idx < dim) isLocal[idx] = true;
    }
    for (int i = 0; i < dim; ++i) {
        if (!isLocal[i]) mGlobalIndices.push_back(i);
    }
}

// ------------------------------------------------------------------------------------------------
HybridLocalTask::~HybridLocalTask() {}

// ------------------------------------------------------------------------------------------------
void HybridLocalTask::SetFixedPoint(const double* fullDimensionPoint)
{
    for (int idx : mGlobalIndices) {
        mFixedValues[idx] = fullDimensionPoint[idx];
    }
}

int HybridLocalTask::GetN() { return mOriginalTask->GetN(); }
const double* HybridLocalTask::GetA() { return mOriginalTask->GetA(); }
const double* HybridLocalTask::GetB() { return mOriginalTask->GetB(); }
int HybridLocalTask::GetNumOfFunc() { return mOriginalTask->GetNumOfFunc(); }
int HybridLocalTask::GetNumberOfDiscreteVariable() { return mOriginalTask->GetNumberOfDiscreteVariable(); }
bool HybridLocalTask::IsLeaf() { return mOriginalTask->IsLeaf(); }

// ------------------------------------------------------------------------------------------------
double HybridLocalTask::CalculateFuncs(const double* x, int fNumber)
{
    // Если алгоритм пытается изменить глобальную переменную, мы его штрафуем
    for (int idx : mGlobalIndices) {
        if (std::abs(x[idx] - mFixedValues[idx]) > 1e-9) {
            return HUGE_VAL;
        }
    }
    return mOriginalTask->CalculateFuncs(x, fNumber);
}
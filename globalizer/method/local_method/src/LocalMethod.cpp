/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      local_method.cpp                                            //
//                                                                         //
//  Purpose:   Source file for local method class                          //
//                                                                         //
//  Author(s): Sovrasov V.                                                 //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "LocalMethod.h"
#include <cmath>
#include <cstring>
#include <algorithm>
#include "TrialFactory.h"

LocalMethod::LocalMethod(Task* _pTask, Trial _StartPoint, int logPoints) :
  mCurrentPoint(NULL), mCurrentResearchDirection(NULL), mPreviousResearchDirection(NULL)
{
  mEps = parameters.Epsilon / 100;
  if (mEps > 0.0001)
    mEps = 0.0001;
  mBestPoint = _StartPoint;
  mStep = parameters.Epsilon * 2;
  mStepMultiplier = 2;
  mTrialsCounter = 0;
  mIsLogPoints = logPoints;

  mPTask = _pTask;

  mDimension = mPTask->GetN() - mPTask->GetNumberOfDiscreteVariable();
  mConstraintsNumber = mPTask->GetNumOfFunc() - 1;

  mStartPoint = new OBJECTIV_TYPE[mDimension];
  std::memcpy(mStartPoint,
    _StartPoint.y,
    mDimension * sizeof(OBJECTIV_TYPE));

  mFunctionsArgument = new OBJECTIV_TYPE[mPTask->GetN()];
  std::memcpy(mFunctionsArgument,
    _StartPoint.y,
    mPTask->GetN() * sizeof(OBJECTIV_TYPE));
  mMaxTrial = MAX_LOCAL_TRIALS_NUMBER;
}

// ------------------------------------------------------------------------------------------------
LocalMethod::~LocalMethod()
{
  if (mStartPoint)
    delete[] mStartPoint;
  if (mFunctionsArgument)
    delete[] mFunctionsArgument;
}

// ------------------------------------------------------------------------------------------------
void LocalMethod::DoStep()
{
  //ј здесь сдвигаем на другое
  for (int i = 0; i < mDimension; i++)
    mCurrentPoint[i] = (1 + mStepMultiplier)*mCurrentResearchDirection[i] -
    mStepMultiplier*mPreviousResearchDirection[i];
}

// ------------------------------------------------------------------------------------------------
Trial LocalMethod::StartOptimization()
{
  int k = 0, i = 0;
  bool needRestart = true;
  double currentFValue = 0.0;
  mTrialsCounter = 0;

  mCurrentPoint = new OBJECTIV_TYPE[mDimension];
  mCurrentResearchDirection = new OBJECTIV_TYPE[mDimension];
  mPreviousResearchDirection = new OBJECTIV_TYPE[mDimension];

  while (mTrialsCounter < mMaxTrial) {
    i++;
    if (needRestart) {
      k = 0;
      std::memcpy(mCurrentPoint, mStartPoint, sizeof(OBJECTIV_TYPE)*mDimension);
      std::memcpy(mCurrentResearchDirection, mStartPoint, sizeof(OBJECTIV_TYPE)*mDimension);
      currentFValue = EvaluateObjectiveFunctiuon(mCurrentPoint);
      needRestart = false;
    }

    std::swap(mPreviousResearchDirection, mCurrentResearchDirection);
    std::memcpy(mCurrentResearchDirection, mCurrentPoint, sizeof(OBJECTIV_TYPE)*mDimension);
    double nextFValue = MakeResearch(mCurrentResearchDirection);

    if (currentFValue > nextFValue) {
      DoStep();

      if (mIsLogPoints == 2)
      {
        Trial currentTrial;
        currentTrial.index = mBestPoint.index;
        if (currentTrial.index >= 0 && currentTrial.index < MaxNumOfFunc)
          currentTrial.FuncValues[currentTrial.index] = nextFValue;

        std::memcpy(currentTrial.y, mCurrentPoint, sizeof(OBJECTIV_TYPE)*mDimension);
        mSearchSequence.push_back(currentTrial);
      }
      k++;
      currentFValue = nextFValue;
    }
    else if (mStep > mEps) {
      if (k != 0)
        std::memcpy(mStartPoint, mPreviousResearchDirection, sizeof(OBJECTIV_TYPE)*mDimension);
      else
        mStep /= mStepMultiplier;
      needRestart = true;
    }
    else
      break;
  }

  if (currentFValue < mBestPoint.FuncValues[mConstraintsNumber])
  {
    std::memcpy(mBestPoint.y,
      mPreviousResearchDirection, sizeof(OBJECTIV_TYPE)*mDimension);
    mBestPoint.FuncValues[mConstraintsNumber] = currentFValue;
    mSearchSequence.push_back(mBestPoint);
  }

  delete[] mCurrentPoint;
  delete[] mPreviousResearchDirection;
  delete[] mCurrentResearchDirection;

  return mBestPoint;
}

// ------------------------------------------------------------------------------------------------
double LocalMethod::EvaluateObjectiveFunctiuon(const OBJECTIV_TYPE* x)
{
  if (mTrialsCounter >= mMaxTrial)
    return HUGE_VAL;

  for (int i = 0; i < mDimension; i++)
    if (x[i] < mPTask->GetA()[i] ||
      x[i] > mPTask->GetB()[i])
      return HUGE_VAL;

  std::memcpy(mFunctionsArgument, x, mDimension * sizeof(OBJECTIV_TYPE));
  for (int i = 0; i <= mConstraintsNumber; i++)
  {
    double value = mPTask->CalculateFuncs(mFunctionsArgument, i);
    if (i < mConstraintsNumber && value > 0)
    {
      mTrialsCounter++;
      return HUGE_VAL;
    }
    else if (i == mConstraintsNumber)
    {
      mTrialsCounter++;

      if (mIsLogPoints == 1)
      {
        Trial* currentTrial = TrialFactory::CreateTrial(x);
        currentTrial->FuncValues[0] = value;
        mSearchSequence.push_back(*currentTrial);
        
      }

      return value;
    }
  }

  return HUGE_VAL;
}

// ------------------------------------------------------------------------------------------------
void LocalMethod::SetEps(double eps)
{
  mEps = eps;
}

// ------------------------------------------------------------------------------------------------
void LocalMethod::SetInitialStep(double value)
{
  mStep = value;
}

// ------------------------------------------------------------------------------------------------
void LocalMethod::SetStepMultiplier(double value)
{
  mStepMultiplier = value;
}

// ------------------------------------------------------------------------------------------------
void LocalMethod::SetMaxTrials(int count)
{
  mMaxTrial = std::min(count, MAX_LOCAL_TRIALS_NUMBER);
}

// ------------------------------------------------------------------------------------------------
int LocalMethod::GetTrialsCounter() const
{
  return mTrialsCounter;
}

// ------------------------------------------------------------------------------------------------
std::vector<Trial> LocalMethod::GetSearchSequence() const
{
  return mSearchSequence;
}

// ------------------------------------------------------------------------------------------------
double LocalMethod::MakeResearch(OBJECTIV_TYPE* StartPoint)
{
  double bestValue = EvaluateObjectiveFunctiuon(StartPoint);


  for (int i = 0; i < mDimension; i++)
  {
    //—двигаем стартовое значение на одно значение
    StartPoint[i] += mStep;
    double rightFvalue = EvaluateObjectiveFunctiuon(StartPoint);


    if (rightFvalue > bestValue)
    {
      StartPoint[i] -= 2 * mStep;
      double leftFValue = EvaluateObjectiveFunctiuon(StartPoint);

      if (leftFValue > bestValue)
        StartPoint[i] += mStep;
      else
        bestValue = leftFValue;
    }
    else
      bestValue = rightFvalue;
  }

  return bestValue;
}
// - end of file ----------------------------------------------------------------------------------

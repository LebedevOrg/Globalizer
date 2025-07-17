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

#include "local_method.h"
#include <cmath>
#include <cstring>
#include <algorithm>
#include "TrialFactory.h"

TLocalMethod::TLocalMethod(TTask* _pTask, TTrial _startPoint, int logPoints) :
  mCurrentPoint(NULL), mCurrentResearchDirection(NULL), mPreviousResearchDirection(NULL)
{
  mEps = parameters.Epsilon / 100;
  if (mEps > 0.0001)
    mEps = 0.0001;
  mBestPoint = _startPoint;
  mStep = parameters.Epsilon * 2;
  mStepMultiplier = 2;
  mTrialsCounter = 0;
  mIsLogPoints = logPoints;

  mPTask = _pTask;

  mDimension = mPTask->GetN() - mPTask->GetFixedN() - mPTask->GetNumberOfDiscreteVariable();
  mConstraintsNumber = mPTask->GetNumOfFunc() - 1;

  mStartPoint = new OBJECTIV_TYPE[mDimension];
  std::memcpy(mStartPoint,
    _startPoint.y + mPTask->GetFixedN(),
    mDimension * sizeof(OBJECTIV_TYPE));

  mFunctionsArgument = new OBJECTIV_TYPE[mPTask->GetN()];
  std::memcpy(mFunctionsArgument,
    _startPoint.y,
    mPTask->GetN() * sizeof(OBJECTIV_TYPE));
  mMaxTrial = MAX_LOCAL_TRIALS_NUMBER;
}

// ------------------------------------------------------------------------------------------------
TLocalMethod::~TLocalMethod()
{
  if (mStartPoint)
    delete[] mStartPoint;
  if (mFunctionsArgument)
    delete[] mFunctionsArgument;
}

// ------------------------------------------------------------------------------------------------
void TLocalMethod::DoStep()
{
  //ј здесь сдвигаем на другое
  for (int i = 0; i < mDimension; i++)
    mCurrentPoint[i] = (1 + mStepMultiplier)*mCurrentResearchDirection[i] -
    mStepMultiplier*mPreviousResearchDirection[i];
}

// ------------------------------------------------------------------------------------------------
TTrial TLocalMethod::StartOptimization()
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
        TTrial currentTrial;
        currentTrial.index = mBestPoint.index;
        currentTrial.FuncValues[currentTrial.index] = nextFValue;
        std::memcpy(currentTrial.y, mFunctionsArgument, sizeof(OBJECTIV_TYPE)*mPTask->GetFixedN());
        std::memcpy(currentTrial.y + mPTask->GetFixedN(), mCurrentPoint, sizeof(OBJECTIV_TYPE)*mDimension);
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
    std::memcpy(mBestPoint.y + mPTask->GetFixedN(),
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
double TLocalMethod::EvaluateObjectiveFunctiuon(const OBJECTIV_TYPE* x)
{
  if (mTrialsCounter >= mMaxTrial)
    return HUGE_VAL;

  for (int i = 0; i < mDimension; i++)
    if (x[i] < mPTask->GetA()[mPTask->GetFixedN() + i] ||
      x[i] > mPTask->GetB()[mPTask->GetFixedN() + i])
      return HUGE_VAL;

  std::memcpy(mFunctionsArgument + mPTask->GetFixedN(), x, mDimension * sizeof(OBJECTIV_TYPE));
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
        TTrial* currentTrial = TTrialFactory::CreateTrial(x);
        currentTrial->FuncValues[0] = value;
        mSearchSequence.push_back(*currentTrial);
        
      }

      return value;
    }
  }

  return HUGE_VAL;
}

// ------------------------------------------------------------------------------------------------
void TLocalMethod::SetEps(double eps)
{
  mEps = eps;
}

// ------------------------------------------------------------------------------------------------
void TLocalMethod::SetInitialStep(double value)
{
  mStep = value;
}

// ------------------------------------------------------------------------------------------------
void TLocalMethod::SetStepMultiplier(double value)
{
  mStepMultiplier = value;
}

// ------------------------------------------------------------------------------------------------
void TLocalMethod::SetMaxTrials(int count)
{
  mMaxTrial = std::min(count, MAX_LOCAL_TRIALS_NUMBER);
}

// ------------------------------------------------------------------------------------------------
int TLocalMethod::GetTrialsCounter() const
{
  return mTrialsCounter;
}

// ------------------------------------------------------------------------------------------------
std::vector<TTrial> TLocalMethod::GetSearchSequence() const
{
  return mSearchSequence;
}

// ------------------------------------------------------------------------------------------------
double TLocalMethod::MakeResearch(OBJECTIV_TYPE* startPoint)
{
  double bestValue = EvaluateObjectiveFunctiuon(startPoint);


  for (int i = 0; i < mDimension; i++)
  {
    //—двигаем стартовое значение на одно значение
    startPoint[i] += mStep;
    double rightFvalue = EvaluateObjectiveFunctiuon(startPoint);


    if (rightFvalue > bestValue)
    {
      startPoint[i] -= 2 * mStep;
      double leftFValue = EvaluateObjectiveFunctiuon(startPoint);

      if (leftFValue > bestValue)
        startPoint[i] += mStep;
      else
        bestValue = leftFValue;
    }
    else
      bestValue = rightFvalue;
  }

  return bestValue;
}
// - end of file ----------------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      local_method.h                                              //
//                                                                         //
//  Purpose:   Header file for local method class                          //
//                                                                         //
//  Author(s): Sovrasov V.                                                 //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __LOCALMETHOD_H__
#define __LOCALMETHOD_H__

#include "Parameters.h"
#include "Task.h"
#include "Trial.h"
#include "Common.h"
#include <vector>

#define MAX_LOCAL_TRIALS_NUMBER 100000

class LocalMethod
{
protected:

  int mDimension;
  int mConstraintsNumber;
  int mTrialsCounter;
  int mMaxTrial;

  Trial mBestPoint;
  std::vector<Trial> mSearchSequence;
  Task* mPTask;

  int mIsLogPoints;

  double mEps;
  double mStep;
  double mStepMultiplier;

  OBJECTIV_TYPE *mFunctionsArgument;
  OBJECTIV_TYPE *mStartPoint;
  OBJECTIV_TYPE* mCurrentPoint;
  OBJECTIV_TYPE* mCurrentResearchDirection;
  OBJECTIV_TYPE* mPreviousResearchDirection;

  virtual double MakeResearch(OBJECTIV_TYPE*);
  virtual void DoStep();
  virtual double EvaluateObjectiveFunctiuon(const OBJECTIV_TYPE*);

public:

  LocalMethod(Task* _pTask, Trial _StartPoint, int logPoints = 0);
  LocalMethod(LocalMethod&) { throw EXCEPTION("Copy constructor is not implemented"); }
  virtual ~LocalMethod();

  virtual void SetEps(double);
  virtual void SetInitialStep(double);
  virtual void SetStepMultiplier(double);
  virtual void SetMaxTrials(int);

  virtual int GetTrialsCounter() const;
  virtual std::vector<Trial> GetSearchSequence() const;

  virtual Trial StartOptimization();
};

#endif //__LOCALMETHOD_H__
// - end of file ----------------------------------------------------------------------------------

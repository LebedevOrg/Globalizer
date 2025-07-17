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

#include "parameters.h"
#include "task.h"
#include "Trial.h"
#include "common.h"
#include <vector>

#define MAX_LOCAL_TRIALS_NUMBER 100000

class TLocalMethod
{
protected:

  int mDimension;
  int mConstraintsNumber;
  int mTrialsCounter;
  int mMaxTrial;

  TTrial mBestPoint;
  std::vector<TTrial> mSearchSequence;
  TTask* mPTask;

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

  TLocalMethod(TTask* _pTask, TTrial _startPoint, int logPoints = 0);
  TLocalMethod(TLocalMethod&) { throw EXCEPTION("Copy constructor is not implemented"); }
  virtual ~TLocalMethod();

  virtual void SetEps(double);
  virtual void SetInitialStep(double);
  virtual void SetStepMultiplier(double);
  virtual void SetMaxTrials(int);

  virtual int GetTrialsCounter() const;
  virtual std::vector<TTrial> GetSearchSequence() const;

  virtual TTrial StartOptimization();
};

#endif //__LOCALMETHOD_H__
// - end of file ----------------------------------------------------------------------------------

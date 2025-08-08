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

#include "ParallelHookeJeevesMethod.h"
#include <cmath>
#include <cstring>
#include <algorithm>
#include <queue>
#include <vector>

#include <unordered_map>

#include "TrialFactory.h"

ParallelHookeJeevesMethod::ParallelHookeJeevesMethod(Task* _pTask, Trial _startPoint, Calculation& _calculation, int logPoints) :
  LocalMethod(_pTask, _startPoint, logPoints), calculation(_calculation)
{

  inputSet.Resize(parameters.NumPoints);
  outputSet.Resize(parameters.NumPoints);
}

// ------------------------------------------------------------------------------------------------
double ParallelHookeJeevesMethod::CheckCoordinate(const OBJECTIV_TYPE* x)
{
  if (mTrialsCounter >= mMaxTrial)
    return HUGE_VAL;

  for (int i = 0; i < mDimension; i++)
    if (x[i] < mPTask->GetA()[i] ||
      x[i] > mPTask->GetB()[i])
      return HUGE_VAL;

  return 0;
}

bool IncrementMask(std::string& mask, int dim, int startIndex)
{
  int maskIndex = startIndex; //�������� ������ �����
  while (1) //�������� �����
  {

    if (maskIndex == dim)
    {
      return true;
      break;
    }

    if (mask[maskIndex] == 0)
    {
      mask[maskIndex] = 1;
      break;
    }
    else if (mask[maskIndex] == 1)
    {
      mask[maskIndex] = -1;
      break;
    }
    else if (mask[maskIndex] == -1)
    {
      mask[maskIndex] = 0;
      maskIndex++;
    }
  }
  maskIndex = startIndex; //�������� ������ �����

  return false;
}


// ------------------------------------------------------------------------------------------------
double ParallelHookeJeevesMethod::MakeResearch(OBJECTIV_TYPE* startPoint)
{

  std::unordered_map<std::string, Trial*> data;//�����
  std::string mask(MaxDim, 0); // �������� �����, -1 - �����, +1 - � �����, 0 - �������� �� �����.
  std::string bestMask(MaxDim, 0); // �������� �����, -1 - �����, +1 - � �����, 0 - �������� �� �����.
  double bestValue = 0;
  Trial* currentTrial = 0;
  double Fvalue = 0;

  if (parameters.NumPoints > 1)
  {
    bestValue = EvaluateObjectiveFunctiuon(startPoint);;

    Fvalue = CheckCoordinate(startPoint);//��������� �����, ����� �� � ������� �����������   
    if (Fvalue == 0)
    {
      currentTrial = TrialFactory::CreateTrial(startPoint);
      currentTrial->FuncValues[0] = bestValue;
    }
  }
  else
  {
    Fvalue = CheckCoordinate(startPoint);//��������� �����, ����� �� � ������� �����������  
    std::memcpy(mFunctionsArgument, startPoint, mDimension * sizeof(OBJECTIV_TYPE));

    currentTrial = TrialFactory::CreateTrial(mFunctionsArgument);
    if (Fvalue == 0)
    {
      inputSet[0] = currentTrial;
    }

    calculation.Calculate(inputSet, outputSet);

    bestValue = currentTrial->FuncValues[0];

    if (Fvalue != 0)
    {
      bestValue = Fvalue;
      currentTrial = 0;
    }
  }

  auto ins = data.insert(std::pair< std::string, Trial*>(mask, currentTrial));

  // ������� ������ � ����� ���������� � ����� �����
  int currIndex = 0;
  double currH = 0;
  char currMI = 0;

  //  ������ � ����� ���������� � ����� ����� �� ������ ����� ����� ��� �����������, ���� ������������
  int startIndex = -1;
  double startH = 0;
  double startMI = 0;

  std::string startMask(MaxDim, 0);

  int maxIndex = 0; //������ �����
  int startMaxIndex = 0;


  double currCoordinate[MaxDim];//���������� � ������� ����������� �����
  double bestCoordinate[MaxDim];//���������� � ������� ����������� �����

  for (int i = 0; i < mDimension; i++)
  {
    mask[i] = 0;
    bestMask[i] = 0;
    bestCoordinate[i] = startPoint[i];
    currCoordinate[i] = bestCoordinate[i];
  }

  bool isStop = false;
  int k = 0;//���������� ������������ ����� �� �����������
  int startK = 0;//� ���� �����������

  int j = 0;
  while (!isStop)
  {
    mask[currIndex] = currMI;
    for (int i = 0; i < mDimension; i++) //��������� ����������
    {
      currCoordinate[i] = startPoint[i] + mStep * double(mask[i]);
    }

    auto tmp = data.find(mask);


    if (tmp == data.end())//������ ��� �� ����.
    {
      currentTrial = 0;

      Fvalue = CheckCoordinate(currCoordinate);//��������� �����, ����� �� � ������� �����������      
      if (Fvalue == 0)
      {
        std::memcpy(mFunctionsArgument, currCoordinate, mDimension * sizeof(OBJECTIV_TYPE));
        currentTrial = TrialFactory::CreateTrial(mFunctionsArgument);

        inputSet[j] = currentTrial;

        j++;
        if (startIndex == -1)//������ ��� ����� ������� �� ����������� �����
        {
          startIndex = currIndex;
          startH = currH;
          startMI = currMI;
          startK = k;
          startMask = bestMask;
          startMaxIndex = maxIndex;
        }
      }
      auto ins = data.insert(std::pair< std::string, Trial*>(mask, currentTrial));


    }
    else//��� ���������
    {
      if (startIndex == -1)
      {
        if (tmp->second != 0)
        {
          if (tmp->second->GetValue() < bestValue)//��������� ��������
          {
            bestValue = tmp->second->GetValue();
            bestMask[currIndex] = mask[currIndex];
            bestCoordinate[currIndex] = currCoordinate[currIndex];
            currIndex++;
            maxIndex = currIndex;

            startH = 0;
            startMI = 0;
            startK = 0;

            currH = 0;
            currMI = 0;

            k = 0;

            continue;


          }
        }
      }
    }

    k++;
    if (k == 3) //���������� ��� ����� �� �����������
    {
      currIndex++; //��������� � ���������
      currH = 0;
      currMI = 0;
      k = 0;

      mask = bestMask;


      if (startIndex == -1)//���� ��� ����� ���������, �� ��������� �� ����� ����������� � �������� ��� ��� ����������
      {
        maxIndex = currIndex;
      }
    }
    else
    {
      if (currMI == 0)
      {
        currH = +mStep;
        currMI = 1;
      }
      else if (currMI == 1)
      {
        currH = -mStep;
        currMI = -1;
      }
    }

    if (j == (parameters.NumPoints))//���� ��������� NumPoints �����, �� ��������� ��� �����������
    {
      calculation.Calculate(inputSet, outputSet);

      for (int i = 0; i < outputSet.trials.size(); i++)
      {
        if (mIsLogPoints == 1)
        {
          mSearchSequence.push_back(*(outputSet.trials[i]));
        }
        mTrialsCounter++;
      }
      j = 0;
      currIndex = startIndex;//������������ � ������ ������� ����������
      currH = startH;
      currMI = startMI;
      k = startK;

      startIndex = -1;
      startH = 0;
      startMI = 0;

      bestMask = startMask;
      mask = bestMask;
      maxIndex = startMaxIndex;
    }

    if (maxIndex == mDimension)//����� �� �����
    {
      isStop = true;
      break;
    }

    if (currIndex == mDimension)//������� ����� ������������, ����� ������� � ���������� ������
    {
      isStop = IncrementMask(bestMask, mDimension, maxIndex);//�������� ��������� �����
      mask = bestMask;
      currIndex = maxIndex;
      currH = int(bestMask[maxIndex]) * mStep;
      currMI = bestMask[maxIndex];

      if (mask[currIndex] == 0)
        k = 0;
      if (mask[currIndex] == 1)
        k = 1;
      if (mask[currIndex] == -1)
        k = 2;
    }
  }

  if (j != 0)//����������� �����
  {
    calculation.Calculate(inputSet, outputSet);

    for (int i = 0; i < j; i++)
    {
      if (mIsLogPoints == 1)
      {
        mSearchSequence.push_back(*(inputSet.trials[i]));
      }
      mTrialsCounter++;

      if (bestValue > inputSet[i]->GetValue())
      {
        bestValue = inputSet[i]->GetValue();
        for (int q = 0; q < mDimension; q++)
        {
          bestCoordinate[q] = inputSet[i]->y[q];
        }
      }
    }
  }

  for (int i = 0; i < mDimension; i++)
  {
    startPoint[i] = bestCoordinate[i];
  }

  if (mIsLogPoints != 1)
  {
    for (auto iter = data.begin(); iter != data.end(); iter++)
      if (iter->second != 0)
      {
        delete iter->second;
        iter->second = 0;
      }
  }
  return bestValue;
}

// ------------------------------------------------------------------------------------------------
double ParallelHookeJeevesMethod::EvaluateObjectiveFunctiuon(const OBJECTIV_TYPE* x)
{
  if (mTrialsCounter >= mMaxTrial)
    return HUGE_VAL;

  for (int i = 0; i < mDimension; i++)
    if (x[i] < mPTask->GetA()[i] ||
      x[i] > mPTask->GetB()[i])
      return HUGE_VAL;

  std::memcpy(mFunctionsArgument + parameters.Dimension, x, mDimension * sizeof(OBJECTIV_TYPE));
  double value = HUGE_VAL;
  Trial* currentTrial = TrialFactory::CreateTrial(mFunctionsArgument);

  inputSet[0] = currentTrial;

  calculation.Calculate(inputSet, outputSet);
  mTrialsCounter++;
  double bestValue = currentTrial->GetValue();

  if (mIsLogPoints == 1)
  {
    mSearchSequence.push_back(*currentTrial);
  }
  value = bestValue;
  
  if (mIsLogPoints != 1)
  {
    delete currentTrial;
  }
  return value;
}



// - end of file ----------------------------------------------------------------------------------

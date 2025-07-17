#ifndef __TEST_COMMON_H__
#define __TEST_COMMON_H__
#include <gtest/gtest.h>

#define _N 4
  void ReadIterationFromFile(FILE* pf, int &IterationCount,
                             double &AchievedAccuracy, bool &recalc,
                             double &BestTrial_FuncValue,
                             double &BestTrial_x, double* BestTrial_y,
                             double &pCurTrials_FuncValue,
                             double &pCurTrials_x, double* pCurTrials_y)
  {
    int recalc_int;
    char tmp[30];
    fscanf(pf, "%s%s%d", tmp, tmp, &IterationCount);
    fscanf(pf, "%s%s%lf", tmp, tmp, &AchievedAccuracy);
    fscanf(pf, "%s%s%i", tmp, tmp, &recalc_int); //bool type does not exists in C language, so C-style I/O functions like scanf have no template %b
    fscanf(pf, "%s%s%lf", tmp, tmp, &BestTrial_FuncValue);
    fscanf(pf, "%s%s%lf", tmp, tmp, &BestTrial_x);
    recalc_int == 0 ? recalc = false : recalc = true;
    for (int i = 0; i <_N; i++)
    {
        fscanf(pf, "%s%s%lf", tmp, tmp, &BestTrial_y[i]);
    }

    fscanf(pf, "%s%s%lf", tmp, tmp, &pCurTrials_FuncValue);
    fscanf(pf, "%s%s%lf", tmp, tmp, &pCurTrials_x);
    for (int i = 0; i < _N; i++)
    {
      fscanf(pf, "%s%s%lf", tmp, tmp, &pCurTrials_y[i]);
    }
  }

void CheckMetodIteration(const std::string& stateFile, const std::string& currentFile, int countOfIterations)
{
  FILE* rightf;
  FILE* currentf;

  int IterationCount;
  double AchievedAccuracy;
  bool recalc;
  double BestTrial_FuncValue;
  double BestTrial_x;
  double BestTrial_y[MaxDim];
  double pCurTrials_FuncValue;
  double pCurTrials_x;
  double pCurTrials_y[MaxDim];

  int currentIterationCount;
  double currentAchievedAccuracy;
  bool currentrecalc;
  double currentBestTrial_FuncValue;
  double currentBestTrial_x;
  double currentBestTrial_y[MaxDim];
  double currentpCurTrials_FuncValue;
  double currentpCurTrials_x;
  double currentpCurTrials_y[MaxDim];

  rightf = fopen(stateFile.c_str(),"r");
  currentf = fopen(currentFile.c_str(),"r");

  if(rightf == 0 || currentf == 0)
  {
    ASSERT_TRUE(false);
  }
  else
  {
    while(countOfIterations > 0)
    {
      ReadIterationFromFile(rightf, IterationCount, AchievedAccuracy, recalc,
          BestTrial_FuncValue, BestTrial_x, BestTrial_y, pCurTrials_FuncValue,
          pCurTrials_x, pCurTrials_y);

      ReadIterationFromFile(currentf, currentIterationCount, currentAchievedAccuracy,
          currentrecalc, currentBestTrial_FuncValue, currentBestTrial_x, currentBestTrial_y,
          currentpCurTrials_FuncValue, currentpCurTrials_x, currentpCurTrials_y);

      ASSERT_EQ(IterationCount, currentIterationCount);
      ASSERT_DOUBLE_EQ(AchievedAccuracy, currentAchievedAccuracy);
      ASSERT_EQ(recalc, currentrecalc);
      ASSERT_DOUBLE_EQ(BestTrial_FuncValue, currentBestTrial_FuncValue);
      ASSERT_DOUBLE_EQ(BestTrial_x, currentBestTrial_x);
      ASSERT_DOUBLE_EQ(pCurTrials_FuncValue, currentpCurTrials_FuncValue);
      ASSERT_DOUBLE_EQ(pCurTrials_x, currentpCurTrials_x);
      for (int i = 0; i < _N; i++)
      {
        ASSERT_DOUBLE_EQ(BestTrial_y[i], currentBestTrial_y[i]);
        ASSERT_DOUBLE_EQ(pCurTrials_y[i], currentpCurTrials_y[i]);
      }

      countOfIterations--;
    }
  }
  fclose(rightf);
  fclose(currentf);
  }
#endif
#ifndef __SOLUTION_RESULT_H__
#define __SOLUTION_RESULT_H__

#include "common.h"
#include "SearchData.h"
/**
���������� ������ �������
*/
struct TSolutionResult
{
  /// ������ ��������, ���������� ��� ������ ������� ������
  TTrial* BestTrial;
  /// ����� ����������� ��������
  int IterationCount;
  /// ���������� ���������
  int TrialCount;
};

#endif

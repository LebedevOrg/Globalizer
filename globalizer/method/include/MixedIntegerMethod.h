/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      MixedIntegerMethod.h                                        //
//                                                                         //
//  Purpose:   Header file for method class                                //
//                                                                         //
//  Author(s):  Lebedev.i                                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


/**
\file MixedIntegerMethod.h

\authors  ������� �.
\date 2025-2026
\copyright ���� ��. �.�. ������������

\brief ���������� ������ #MixedIntegerMethod

\details ���������� ������ #MixedIntegerMethod � ������������� ����� ������
*/


#ifndef __MIXED_INTEGER_METHOD_H__
#define __MIXED_INTEGER_METHOD_H__

#include "Method.h"


/**
������� �����, ����������� �������� ����������� ������.

� ������ #Method ����������� �������� �������, ������������ ������ ��������� ����������� ������.
*/
class MixedIntegerMethod : public Method
{
protected:



  // ----------------------------------------------------------------------------
  // ���������� ������ ������
  // ----------------------------------------------------------------------------



  ///// ������� ������ ��� �����������, ����������� � CalculateFunctionals()
  InformationForCalculation inputSet;
  ///// �������� ������ �����������, �������������� � CalculateFunctionals()
  TResultForCalculation outputSet;
  /// ���������� � ������ ������� ��������
  SearchIteration iteration;
  /// ���� �������� ����� � ����������� ����������� ��������
  bool isFoundOptimalPoint;


  /// ����������� ��������
  double            AchievedAccuracy;
  /** ����������� ��������� ���������

  �������� �������� ��������� alfa �� 1 (����������) �� 20 (���������) �����
  ������������� �������� alfa = 15.
  */
  double alfa;

  /// ����� ����������� �������� ������ �������
  std::vector<int> functionCalculationCount;

  /// ����� �� ������ ��������
  bool isFindInterval;

  std::vector< double > globalM;

  ///����� ����� ��������������� � �������� ������������� ����������� ���������� ��������
  bool isSetInLocalMinimumInterval;

  /// ���������� ����� ����������� ��������� �������
  int localPointCount;
  /// ����� �������� �������� ������
  int numberLocalMethodtStart;
  /// ����� ���������������
  bool isStop;

  /**
  ���������� ���������� ��������
  ������������ ����� �������� ���� ���������� ����������.
  ����� ����� ����������.
  */
  int mDiscreteValuesCount;
  /// �������� ���������� ����������
  std::vector< std::vector< double > > mDiscreteValues;
  /// ������ ������� ����������� ���������
  int startDiscreteVariable;


  /// ��������� ��������� ��������
  std::vector<Trial*> localMinimumPoints;

  //=====================================================================================================================================================
  //��� ������� ���������� ��������� ����� ���������
  double* Xmax;
  double* mu;
  SearchInterval* intervalXMax;
  bool isSearchXMax;
  //=====================================================================================================================================================

  /// ������ ��� ���������� ����� ��� ����������� ������ � ���������
  std::vector<Trial*> printPoints;


  /// ���������� ��������� ����� ��������� ��� ��������\������������ ���������
  virtual void CalculateCurrentPoint(Trial& pCurTrialsj, SearchInterval* BestIntervalsj);


  /// ������ �������� ����������� ���������
  virtual void SetDiscreteValue(int u, std::vector< std::vector <double> > dvs);

  /// �������� ��������� ����������, ����� ��� ����������� ������
  virtual SearchData* GetSearchData(Trial* trial);


public:

  MixedIntegerMethod(Task& _pTask, SearchData& _pData,
    Calculation& _Calculation, Evolvent& _Evolvent);
  virtual ~MixedIntegerMethod();

  /** ������� ��������� ������ �������� ������
  */
  virtual void FirstIteration();

  /** ���������� ����� ��������� ��������

����������� ����� ��������� �������� ������������ � ������ #iteration.pCurTrials
*/
  virtual void CalculateIterationPoints();



};

#endif
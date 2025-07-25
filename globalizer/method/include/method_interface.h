/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      method.h                                                    //
//                                                                         //
//  Purpose:   Header file for method class                                //
//                                                                         //
//  Author(s): Barkalov K., Sysoyev A.                                     //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


/**
\file method.h

\authors �������� �., ������ �.
\date 2015-2016
\copyright ���� ��. �.�. ������������

\brief ���������� ������ #TMethod

\details ���������� ������ #TMethod � ������������� ����� ������
*/


#ifndef __METHOD_INTERFACE_H__
#define __METHOD_INTERFACE_H__

#include "common.h"
#include "task.h"
#include "Trial.h"
#include "evolvent.h"
#include "parameters.h"

// ------------------------------------------------------------------------------------------------

/**
������� �����, ����������� �������� ����������� ������.

� ������ #TMethod ����������� �������� �������, ������������ ������ ��������� ����������� ������.
*/
class IMethod
{
public:
  /** ������� ��������� ������ �������� ������
  */
  virtual void FirstIteration() = 0;

  /** ���������� ����� ��������� ��������

  ����������� ����� ��������� �������� ������������ � ������ #pCurTrials
  */
  virtual void CalculateIterationPoints() = 0;

  /** ���������� ������� ������

  ���������� ��������� � ������ �� ������� #pCurTrials, ���������� ����������� ���������
  ������������ � ��� �� ������
  */
  virtual void CalculateFunctionals() = 0;

  /** ���������� ��������� ����������
  */
  virtual void RenewSearchData() = 0;

  /** �������� ���������� �������� ��������� ������

  ����� ���������� ������ � ��������� �������:
  - ����� ��������� ��������� ����������� ���������� ��������
  - ���� �������� ���� ������ � �������� �������� \f$ x_t - x_{t-1} < \epsilon \f$
  - ���� �������� ����� ����� � �������� �������� \f$ \| y^k - y^\ast \| < \epsilon \f$

  \return ������, ���� �������� ��������� ��������; ���� - � ��������� ������.
  */
  virtual bool CheckStopCondition() = 0;

  /** ������� ������� �������� ��������

  \return ������, ���� ������� ���������; ���� - � ��������� ������
  */
  virtual bool EstimateOptimum() = 0;

  /** ������� ���������� � ����� ���������� ��������
  */
  virtual void FinalizeIteration() = 0;

  /** �������� ����� ���������

  \return ����� ���������
  */
  virtual int GetIterationCount() = 0;

  /** �������� ������� ������ ��������

  \return ���������, ��������������� �������� ��������
  */
  virtual TTrial* GetOptimEstimation() = 0;

  /**���� ����������

  ������� ���������� ����� ����� ���������, ����������� ��� ������� ������� ������ � ���� ���������
  ��������
  \return ����� ����� ���������
  */
  virtual int GetNumberOfTrials() = 0;

  ///���������� ����� ���������� ������ �������
  virtual std::vector<int> GetFunctionCalculationCount() = 0;

  /// ���������� ����������� ��������
  virtual double GetAchievedAccuracy() = 0;


  /**��������� ��������� � ��������� ����������, ��� ���� �������� ��������� ø������ �
  ������ ��������

  \param[in] points ����� ���������, ������� ����� ���������
  */
  virtual void InsertPoints(const std::vector<TTrial*>& points) = 0;

  /// ������ ����� � ����, ��� ���������
  virtual void PrintPoints(const std::string & fileName) = 0;

  /// ������ ���������� ������ ����-������
  virtual void LocalSearch() = 0;

  /// ����� ����������� ����� � ����������� ������
  virtual void  SavePoints() = 0;

  /// ���������� ����� ����� ���������� �� ���������� ������
  virtual int GetLocalPointCount() = 0;
  /// ���������� ����� �������� �������� ������
  virtual int GetNumberLocalMethodtStart() = 0;
  /// �������� ���������� � ��������
  virtual void PrintSection() = 0;
};

#endif
// - end of file ----------------------------------------------------------------------------------

#pragma once

#include "Task.h"

class SeparableOptimizationTask : public Task
{
protected:

  /// ������ ����� ����������
  int startParameterNumber;


public:
  SeparableOptimizationTask(IProblem* _problem, int _ProcLevel);
  SeparableOptimizationTask();

  /// ������� ����� ������
  virtual Task* Clone();

  /// ���������� ����� ������� ������� ������
  virtual const double* GetA() const;
  /// ���������� ������ ������� ������� ������
  virtual const double* GetB() const;

  /**
 ���������� ������� ��������� ���������� ����� ����������� ��������
 ����� ������ ������� ����� ������� resetOptimumPoint()
 */
  virtual const double* GetOptimumPoint() const;
  /// ��������� �������� ������� � ������� fNumber � ����� y
  virtual double CalculateFuncs(const double* y, int fNumber);
  /**
  ��������� numPoints �������� ������� � ������� fNumber, � ����������� y � ������ values
  �������� ������ ���� ������ �������� ����������� IGPUProblem
  */
  virtual void CalculateFuncsInManyPoints(double* y, int fNumber, int numPoints, double* values);

  /// ������ ������ ����� ����������
  void SetStartParameterNumber(int _startParameterNumber);

  /**
  * \brief �������� ���������� ����� �� �������, �������� ��������� ��������
  * \param[in] y ��������� ����������.
  * \param[out] point ����� ����������.
  * \return true, ���� �������� ���������, ����� false.
  */
  virtual void CopyPoint(double* y, Trial* point);

};
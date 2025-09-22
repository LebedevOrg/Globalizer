#pragma once

#include "Solver.h"
#include "SeparableOptimizationTask.h"


/**
 ������� ����� ��� ������������� �����������
**/
class SeparableOptimizationSolver : public ISolver
{
protected:

  /// �������� ��� ����������� �� ������� ����������
  std::vector< Solver*> solvers;
  /// ���������� ����� ���������, �� ��������� �� 1
  std::vector<int> dimensions;
  /// ������� ������ �����������
  SolutionResult* solutionResult;
  /// ����������� �������� ������
  int originalDimension;
  /// ������ ��� ����������� �� ������� ����������
  std::vector <SeparableOptimizationTask*> tasks;

  /// ������ �����������
  IProblem* problem;

  /// ������� �����������
  void SeparableOptimizationSolver::SetDimentions(std::vector<int> _dimentions);

  /// ������� ��������� ����� ������� �����
  void CreateStartPoint();

  /// ��������� �������� ���� ������
  void Construct();

public:
  SeparableOptimizationSolver(IProblem* problem, std::vector<int> _dimentions = {});

#ifdef _GLOBALIZER_BENCHMARKS

  SeparableOptimizationSolver(IGlobalOptimizationProblem* problem, std::vector<int> _dimentions = {});

#endif

  virtual ~SeparableOptimizationSolver();

  /// ������� ������ �� ���������
  virtual int Solve();



  SolutionResult* GetSolutionResult();
};
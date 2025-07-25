#ifndef __SOLVER_H__
#define __SOLVER_H__

#define _CRT_SECURE_NO_WARNINGS

#include <mpi.h>
#include <exception>

#include "common.h"
#include "GlobProcess.h"
#include "exception.h"
#include "init_problem.h"
#include "output_system.h"
#include "messages.h"
#include "parallel_tree.h"
#include "solution_result.h"
#include "SolverInterface.h"


/**
* ������� ������ ��� ������� ����� ���������� �����������
**/
// ------------------------------------------------------------------------------------------------



// ------------------------------------------------------------------------------------------------
/**
�������� ������
*/
class TSolver : public ISolver
{
protected:
  ///������� �������� ������
  TProcess* mProcess;
  /// ������
  IProblem* mProblem;

  /// ����� �������� ������
  TTask* pTask;
  /// ���� ������(��������� ����������)
  TSearchData* pData;

  /// ������ �����������������
  //TParallelTree* parTree;
  /// ��������� ������ �������
  TSolutionResult* result;

  /// �������� ������
  virtual void ClearData();
  /** ������������� ����� � ����������� ���������

  ������� ������������� �������� ������������ ��� ������ � ����������� �� ����������� ��������
  ������ � ��������� ���������
  */
  virtual void InitAutoPrecision();
  /// �������� �������� � ����� ����������
  virtual int CreateProcess();
  /// ��������� ��� ��������� ������� ������������� �������� ������
  int CheckParameters();
  /// �������� ������������ ���������� � ������������ ������� ������������ ����� �� mpi
  void BlockSchemeCalculation();
  /// �������� ������������ ���������� ��� ������������ ��� (����������������� �� ������) �� MPI
  void MpiCalculation();
  /// �������� ������������ ��� ������������ �����
  void AsyncCalculation();
  /// �������� ������������ ��������������� ��������� ���������� �� ��������� � ����������, ����� ���� ���������� ������ ���������� ������� ������������ �����
  void SeparationVariablesSolver();
public:
  TSolver(IProblem* problem);

  /// ������� ������ �� ���������
  virtual int Solve();
  /// ������� ��������� � ���������� �����������
  virtual int Solve(TTask* task);
  virtual ~TSolver();

  void SetProblem(IProblem* problem);
  IProblem* GetProblem();
  TSolutionResult* GetSolutionResult();
};


class TSeparableSolver : public TSolver
{
public:
  TSeparableSolver(IProblem* problem) : TSolver(problem)
  {  }

  //TTask * CreateTask(int taskNumber)
  //{
  //  TTask * pTask = new TTask(params->Dimension, params->DimInTask[ProcRank], problem);

  //  return pTask;
  //}
  virtual int Solve();
  virtual int CreateProcess(TProcess** process, IProblem* problem);
};
//
//class TSequentialSolver : public TSolver
//{
//public:
//  TSequentialSolver(IProblem* problem, TParameters& params) : TSolver(problem, params)
//  {
//  }
//};


#endif //solver.h
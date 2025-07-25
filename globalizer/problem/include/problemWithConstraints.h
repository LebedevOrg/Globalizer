/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2016 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      problemWithConstraints.h                                    //
//                                                                         //
//  Purpose:   Header file for ExaMin problem interface                    //
//                                                                         //
//                                                                         //
//  Author(s): Lebedev I.                                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

/**
\file problemWithConstraints.h

\authors ������� �.
\date 2016
\copyright ���� ��. �.�. ������������

\brief ���������� �������� ������ ��� ����� � �������������

\details
*/

#ifndef __PROBLEM_WITH_CONSTRAINTS_H__
#define __PROBLEM_WITH_CONSTRAINTS_H__

#include <omp.h>
#include "problem.h"
#include "ConstrainedProblem.hpp"
#include "ConstrainedProblemGenerator.hpp"


template <class Owner, class FType>
class TProblemWithConstraints :
  public TBaseProblem<Owner>, public TConstrainedProblemGenerator<FType>, public TConstrainedProblem<FType>
{
#undef OWNER_NAME
#define OWNER_NAME TProblemWithConstraints
protected:
  /// �������� ������������ ����� ��������� ������ ����������
  virtual int CheckValue(int index = -1);

  /// ������� �������� �� ��������� ������� ����������
  virtual void SetBaseDefaultParameters();


  /** ����� ���������� ����� ����� ������� � ������ (��� ����� ����� ����������� + ����� ���������)
  \return ����� �������
  */
  virtual int GetRealNumberOfFunctions() const;

  /** ����� ���������� ����� ����������� � ������
  \return ����� �����������
  */
  virtual int GetRealNumberOfConstraints() const;

  template <class ClassType, class Type>
  void ProblemParameterCheckSize(ClassType& par, Type defVal, Type left, Type right)
  {
    if (par.GetSize() < GetRealNumberOfFunctions())
    {
      if (par.GetSize() == 1)
      {
        Type d = par[0];
        if ((d > right) || (d < left))
          d = defVal;
        par.SetSize(GetRealNumberOfFunctions());
        for (int i = 0; i < GetRealNumberOfFunctions(); i++)
          par[i] = d;
      }
      else
      {
        int oldSize = par.GetSize();
        Type d = par[0];
        if ((d > right) || (d < left))
          d = defVal;
        par.SetSize(GetRealNumberOfFunctions());
        for (int i = oldSize; i < GetRealNumberOfFunctions(); i++)
          (par.GetData())[i] = d;
      }
    }
  }
public:

  TProblemWithConstraints();

  /// ���������� �����������
  TInt<Owner> constraint_count;
  /// ���� ������� ������, ���� ������, �� Q �����������
  TDoubles<Owner> Deltas;
  /// ����� �����������, ��� �� RHS
  TDoubles<Owner> Q;
  /// �������������� ��� ��� �����������, ���� ���� � ������ ����������� ���������� ������� �� �������, �� �� ������������
  TFlag<Owner> IsZoom;
  /// �������� ��� ��� ���������� ������� ����������� � ���������� ����������� �������� ������� �������
  TFlag<Owner> IsShift;
  /// �������� ��� ��� ���������� ������� ������� ������� �� ������� ���������� �������
  TFlag<Owner> IsBoundaryShift;
  /// �������� ������ ��������� ����� �� ������� ���������� ������� (������� 2^{-1})
  TInt<Owner> BoundarySearchPrecision;
  /// �������� �� ������� ������� ����� ���������� ����������� �� �����������
  TFlag<Owner> IsImprovementOfTheObjective;
  /// ����������� ���������
  TDoubles<Owner> ImprovementCoefficients;

  /// �������������� ������� � ������� index
  //virtual void InitFunc(FType* func, int index) = 0;

  /// ������������� ����������
  virtual void Init(int argc, char* argv[], bool isMPIInit = false);

  virtual double CalculateFunctionals(const double* y, int fNumber);


  /** ����� ���������� ����� ����� ������� � ������ (��� ����� ����� ����������� + ����� ���������)
  \return ����� �������
  */
  virtual int GetNumberOfFunctions() const;
  /** ����� ���������� ����� ����������� � ������
  \return ����� �����������
  */
  virtual int GetNumberOfConstraints() const;

  /// ���������� ����� �����������
  virtual int GetConstraintsNumber() const
  {
    return GetNumberOfConstraints();
  }

  /// ����������, ����� �� �������������� ������
  bool GetIsZoom() const
  {
    return this->mIsZoom;
  }
  /// ������, ����� �� �������������� ������
  void SetIsZoom(bool isZoom)
  {
    this->mIsZoom = isZoom;
  }
  /// ����������, ����� �� �������� ������� �����������
  bool GetIsShift() const
  {
    return this->mIsShift;
  }
  /// ������, ����� �� �������� ������� �����������
  void SetIsShift(bool isShift)
  {
    this->mIsShift = isShift;
  }
  /// ����������, ����� �� �������� ������� ������� ������� �� �������
  bool GetIsBoundaryShift() const
  {
    return this->mIsBoundaryShift;
  }
  /// ������, ����� �� �������� ������� ������� ������� �� �������
  void SetIsBoundaryShift(bool isBoundaryShift)
  {
    this->mIsBoundaryShift = isBoundaryShift;
  }
  /// ���������� �������� ������ ��������� ����� �� ������� ������� (������� 0.5)
  int GetBoundarySearchPrecision() const
  {
    return this->mBoundarySearchPrecision;
  }
  /// ������ �������� ������ ��������� ����� �� ������� ������� (������� 0.5)
  void SetBoundarySearchPrecision(int boundarySearchPrecision)
  {
    this->mBoundarySearchPrecision = boundarySearchPrecision;
  }
  /// ����������, ����� �� �������������� ������� �������
  bool GetIsImprovementOfTheObjective() const
  {
    return this->mIsImprovementOfTheObjective;
  }
  /// ������, ����� �� �������������� ������� �������
  void SetIsImprovementOfTheObjective(bool isImprovementOfTheObjective)
  {
    this->mIsImprovementOfTheObjective = isImprovementOfTheObjective;
  }

  /** ����� ���������� ���������� ����� ����������� �������� ������� �������
  \param[out] y �����, � ������� ����������� ����������� ��������
  \return ��� ������ (#OK ��� #UNDEFINED)
  */
  virtual int GetOptimumPoint(double* y) const
  {
    return TBaseProblem<Owner>::UNDEFINED;
  }

  /// ���������� ����� ����������� �������� ��� ������� fNumber
  virtual int GetConstraintOptimumPoint(double* point, int fNumber)
  {
    return TBaseProblem<Owner>::UNDEFINED;
  }
};

// ------------------------------------------------------------------------------------------------
/// �������� ������������ ����� ��������� ������ ����������
template <class Owner, class FType>
int TProblemWithConstraints<Owner, FType>::CheckValue(int index)
{
  TBaseProblem<Owner>::CheckValue(index);

  if ((index < this->mOptionsCount) && (index >= 0))
  {
    if (this->mOptions[index] == &Deltas)
    {
      if (Deltas.GetSize() == 1)
        this->mIsTotalDelta = true;
      else
        this->mIsTotalDelta = false;
    }
  }

  if (this->mIsInit)
  {
    if ((constraint_count > 50) || (constraint_count < 0))
      constraint_count = 0;

    if (Deltas.GetSize() < constraint_count)
    {
      if (Deltas.GetSize() == 1)
      {
        double d = Deltas[0];
        if ((d > 1) || (d < 0))
          d = 0.5;
        Deltas.SetSize(constraint_count);
        for (int i = 0; i < constraint_count; i++)
          Deltas[i] = d;
      }
      else
      {
        int oldSize = Deltas.GetSize();
        double d = Deltas[0];
        if ((d > 1) || (d < 0))
          d = 0.5;
        Deltas.SetSize(constraint_count);
        for (int i = oldSize; i < constraint_count; i++)
          Deltas[i] = d;
      }
    }

    if (Q.GetSize() < GetRealNumberOfFunctions())
    {
      if (Q.GetSize() == 1)
      {
        double d = Q[0];
        if ((d > 1) || (d < 0))
          d = 0.5;
        Q.SetSize(GetRealNumberOfFunctions());
        for (int i = 0; i < GetRealNumberOfConstraints(); i++)
          Q[i] = d;
        for (int i = GetRealNumberOfConstraints(); i < GetRealNumberOfFunctions(); i++)
          Q[i] = 0;
      }
      else
      {
        int oldSize = Q.GetSize();
        double d = Q[0];
        if ((d > 1) || (d < 0))
          d = 0.5;
        Q.SetSize(GetRealNumberOfFunctions());
        for (int i = oldSize; i < GetRealNumberOfConstraints(); i++)
          Q[i] = d;
        for (int i = std::max(GetRealNumberOfConstraints(), oldSize); i < GetRealNumberOfFunctions(); i++)
          Q[i] = 0;
      }
    }


    if (ImprovementCoefficients.GetSize() < GetRealNumberOfFunctions())
    {
      if (ImprovementCoefficients.GetSize() == 1)
      {
        double d = ImprovementCoefficients[0];
        ImprovementCoefficients.SetSize(GetRealNumberOfFunctions());
        for (int i = 0; i < GetRealNumberOfConstraints(); i++)
          ImprovementCoefficients[i] = d;
        for (int i = GetRealNumberOfConstraints(); i < GetRealNumberOfFunctions(); i++)
          ImprovementCoefficients[i] = 0;
      }
      else
      {
        int oldSize = ImprovementCoefficients.GetSize();
        double d = ImprovementCoefficients[0];
        ImprovementCoefficients.SetSize(GetRealNumberOfFunctions());
        for (int i = oldSize; i < GetRealNumberOfConstraints(); i++)
          ImprovementCoefficients[i] = d;
        for (int i = std::max(GetRealNumberOfConstraints(), oldSize); i < GetRealNumberOfFunctions(); i++)
          ImprovementCoefficients[i] = 0;
      }
    }

  }
  return 0;
}

// ------------------------------------------------------------------------------------------------
/// ������� �������� �� ��������� ������� ����������
template <class Owner, class FType>
void TProblemWithConstraints<Owner, FType>::SetBaseDefaultParameters()
{
  TBaseProblem<Owner>::SetBaseDefaultParameters();
  int sizeVal = 1;

  Owner* aqqq = (Owner*)this;


  constraint_count.InitializationParameterProperty(
    aqqq, &TProblemWithConstraints::CheckValue, this->mOptionsCount, this->Separator, sizeVal,
    "constraint_count", "Constraint count", "-CC", "0");
  this->AddOption((TBaseProperty<Owner>*)(&constraint_count));

  Deltas.InitializationParameterProperty(
    aqqq, &TProblemWithConstraints::CheckValue, this->mOptionsCount, this->Separator, sizeVal,
    "delta", "The share of the search area", "-delta", "0.5");
  this->AddOption((TBaseProperty<Owner>*)(&Deltas));
  this->mIsTotalDelta = true;

  Q.InitializationParameterProperty(
    aqqq, &TProblemWithConstraints::CheckValue, this->mOptionsCount, this->Separator, sizeVal,
    "Q", "Shift constraints", "-Q", "0");
  this->AddOption((TBaseProperty<Owner>*)(&Q));

  this->mQ = (double**)Q.GetValue();

  IsZoom.InitializationParameterProperty(
    aqqq, &TProblemWithConstraints::GetIsZoom, &TProblemWithConstraints::SetIsZoom, &TProblemWithConstraints::CheckValue,
    this->mOptionsCount, this->Separator, sizeVal,
    "IsZoom", "Is Zoom", "-IZ", "false");
  this->AddOption((TBaseProperty<Owner>*)(&IsZoom));

  IsShift.InitializationParameterProperty(
    aqqq, &TProblemWithConstraints::GetIsShift, &TProblemWithConstraints::SetIsShift, &TProblemWithConstraints::CheckValue,
    this->mOptionsCount, this->Separator, sizeVal,
    "IsShift", "Is Shift", "-ISH", "false");
  this->AddOption((TBaseProperty<Owner>*)(&IsShift));

  IsBoundaryShift.InitializationParameterProperty(
    aqqq, &TProblemWithConstraints::GetIsBoundaryShift, &TProblemWithConstraints::SetIsBoundaryShift, &TProblemWithConstraints::CheckValue,
    this->mOptionsCount, this->Separator, sizeVal,
    "IsBoundaryShift", "Is Boundary Shift", "-IBSH", "false");
  this->AddOption((TBaseProperty<Owner>*)(&IsBoundaryShift));

  BoundarySearchPrecision.InitializationParameterProperty(
    aqqq, &TProblemWithConstraints::GetBoundarySearchPrecision, &TProblemWithConstraints::SetBoundarySearchPrecision, &TProblemWithConstraints::CheckValue,
    this->mOptionsCount, this->Separator, sizeVal,
    "BoundarySearchPrecision", "Boundary Search Precision", "-bsp", "20");
  this->AddOption((TBaseProperty<Owner>*)(&BoundarySearchPrecision));

  IsImprovementOfTheObjective.InitializationParameterProperty(aqqq,
    &TProblemWithConstraints::GetIsImprovementOfTheObjective, &TProblemWithConstraints::SetIsImprovementOfTheObjective, &TProblemWithConstraints::CheckValue,
    this->mOptionsCount, this->Separator, sizeVal,
    "IsImprovementOfTheObjective", "Is improvement of the objective", "-IIO", "false");
  this->AddOption((TBaseProperty<Owner>*)(&IsImprovementOfTheObjective));

  ImprovementCoefficients.InitializationParameterProperty(
    aqqq, &TProblemWithConstraints::CheckValue, this->mOptionsCount, this->Separator, sizeVal,
    "ImprovementCoefficients", "Improvement of the objective coefficients", "-IC", "100_100");
  this->AddOption((TBaseProperty<Owner>*)(&ImprovementCoefficients));

  this->mImprovementCoefficients = (double**)ImprovementCoefficients.GetValue();
}

// ------------------------------------------------------------------------------------------------
/** ����� ���������� ����� ����� ������� � ������ (��� ����� ����� ����������� + ����� ���������)
\return ����� �������
*/
template <class Owner, class FType>
int TProblemWithConstraints<Owner, FType>::GetRealNumberOfFunctions() const
{
  return this->mNumberOfCriterions + constraint_count.GetAvailableData();
}

// ------------------------------------------------------------------------------------------------
/** ����� ���������� ����� ����������� � ������
\return ����� �����������
*/
template <class Owner, class FType>
int TProblemWithConstraints<Owner, FType>::GetRealNumberOfConstraints() const
{
  return constraint_count.GetAvailableData();
}

// ------------------------------------------------------------------------------------------------
template <class Owner, class FType>
TProblemWithConstraints<Owner, FType>::TProblemWithConstraints() :
  TBaseProblem<Owner>(), TConstrainedProblemGenerator<FType>(), TConstrainedProblem<FType>()
{
  this->mPFunction = 0;
}

// ------------------------------------------------------------------------------------------------
/// ������������� ����������
template <class Owner, class FType>
void TProblemWithConstraints<Owner, FType>::Init(int argc, char* argv[], bool isMPIInit)
{
  TBaseProblem<Owner>::Init(argc, argv, false);
  this->mPFunction = new FType*[GetRealNumberOfFunctions()];
  this->mZoomRatios = new double[GetRealNumberOfFunctions()];
  this->mShift = new double*[GetRealNumberOfFunctions()];
  this->mBoundaryShift = new double[this->Dimension];
  this->mvZoomRatios = &this->mZoomRatios;
  this->mvShift = &this->mShift;
  this->mvBoundaryShift = this->mBoundaryShift;

  for (int i = 0; i < GetRealNumberOfFunctions(); i++)
  {
    this->mZoomRatios[i] = 1.0;

    this->mShift[i] = new double[this->Dimension];
    for (int j = 0; j < this->Dimension; j++)
    {
      this->mShift[i][j] = 0.0;
    }
  }
  for (int i = 0; i < this->Dimension; i++)
  {
    this->mBoundaryShift[i] = 0.0;
  }
  for (int i = 0; i < GetRealNumberOfFunctions(); i++)
  {
    this->mPFunction[i] = new FType();
    this->mPConstraints.push_back(this->mPFunction[i]);
    this->InitFunc(this->mPFunction[i], i);
  }
  this->mPObjective = this->mPFunction[GetRealNumberOfFunctions() - 1];

  if (IsZoom)
  {
    this->SetZoom();
  }

  if (IsShift)
  {
    this->SetShift();
  }

  this->mmIsImprovementOfTheObjective = IsImprovementOfTheObjective;
  this->mmQ = new double*[1];
  (*this->mmQ) = new double[GetRealNumberOfConstraints() + 1];

  this->improvementCoefficients = new double*[1];
  *this->improvementCoefficients = new double[GetRealNumberOfConstraints() + 1];
  for (int i = 0; i < GetRealNumberOfConstraints(); i++)
    (*this->improvementCoefficients)[i] = ImprovementCoefficients[i];
  (*this->improvementCoefficients)[GetRealNumberOfConstraints()] = 0;

  this->currentFunctionNumber = 0;
  if (Deltas.GetIsReadValue())
  {
    /*for (int j = 0; j < Dimension; j++)
      objectiveMin[j] = 0;*/
      //GetOptimumPoint(objectiveMin);
    if (this->mIsTotalDelta)
    {
      double q = this->CalculateRHS(Deltas[0]);
      for (int i = 0; i < GetRealNumberOfConstraints(); i++)
      {
        Q[i] = q;
        (*this->mmQ)[i] = Q[i];
      }
    }
    else
    {
      for (int i = 0; i < GetRealNumberOfConstraints(); i++)
      {
        this->currentFunctionNumber = i;
        this->currentFunction = this->mPFunction[i];
        Q[i] = this->CalculateRHS(Deltas[i]);
        (*this->mmQ)[i] = Q[i];
      }
    }
  }

  if (IsBoundaryShift)
  {
    this->SetBoundaryShift();
  }

}

// ------------------------------------------------------------------------------------------------
template <class Owner, class FType>
double TProblemWithConstraints<Owner, FType>::CalculateFunctionals(const double* y, int fNumber)
{

  double koef = 1.0;
  double sum = 0.0;
  //for (int j = 0; j < this->Dimension; j++)
  //{
  //  for (int k = 0; k < 1000; k++)
  //    koef = (koef + exp(1.0) * y[j] / 4.0) * (y[j] / 4.0);
  //}
  sum += koef * this->CalculateFunction(y, fNumber) / koef;
  return sum;
}

// ------------------------------------------------------------------------------------------------
/** ����� ���������� ����� ����� ������� � ������ (��� ����� ����� ����������� + ����� ���������)
\return ����� �������
*/
template <class Owner, class FType>
int TProblemWithConstraints<Owner, FType>::GetNumberOfFunctions() const
{
  return this->mNumberOfCriterions + constraint_count.GetAvailableData();
}

// ------------------------------------------------------------------------------------------------
/** ����� ���������� ����� ����������� � ������
\return ����� �����������
*/
template <class Owner, class FType>
int TProblemWithConstraints<Owner, FType>::GetNumberOfConstraints() const
{
  return constraint_count.GetAvailableData();
}

#endif /// __PROBLEM_WITH_CONSTRAINTS_H__
// - end of file ----------------------------------------------------------------------------------

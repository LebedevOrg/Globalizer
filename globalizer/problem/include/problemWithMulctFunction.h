/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2016 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      problemWithMulctFunction.h                                    //
//                                                                         //
//  Purpose:   Header file for ExaMin problem interface                    //
//                                                                         //
//                                                                         //
//  Author(s): Lebedev I.                                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

/**
\file problemWithMulctFunction.h

\authors Лебедев В.
\date 2016
\copyright ННГУ им. Н.И. Лобачевского

\brief Объявление базового класса для задач с ограничениями

\details
*/

#ifndef __PROBLEM_WITH_MULCT_FUNCTION_H__
#define __PROBLEM_WITH_MULCT_FUNCTION_H__

#include <omp.h>
#include "problemWithConstraints.h"

template <class Owner, class Func>
class TProblemWithMulctFunction: public TProblemWithConstraints<Owner, Func>
{
  #undef OWNER_NAME
#define OWNER_NAME TProblemWithMulctFunction
protected:

  virtual int CheckValue(int index = -1)
  {
    TProblemWithConstraints::CheckValue(index);

    if (mIsInit)
    {
      if (Mulcts.GetSize() < constraint_count)
      {
        if (Mulcts.GetSize() == 1)
        {
          double d = Mulcts[0];
          Mulcts.SetSize(constraint_count);
          for (int i = 0; i < constraint_count; i++)
            Mulcts[i] = d;
        }
        else
        {
          int oldSize = Mulcts.GetSize();
          double d = Mulcts[0];
          Mulcts.SetSize(constraint_count);
          for (int i = oldSize; i < constraint_count; i++)
            Mulcts[i] = d;
        }
      }
    }

    return 0;
  }

  /// Задание значений по умолчанию базовых параметров
  virtual void SetBaseDefaultParameters()
  {
    TProblemWithConstraints::SetBaseDefaultParameters();
    int sizeVal = 1;

    Owner* aqqq = (Owner*)this;

    Mulcts.InitializationParameterProperty(aqqq, &TProblemWithMulctFunction::CheckValue,
      mOptionsCount, Separator, sizeVal, "mulct", "Mulcts", "-Mulcts", "10000");
    AddOption((TBaseProperty<Owner>*)(&Mulcts));
  }

public:

  TProblemWithMulctFunction() : TProblemWithConstraints()
  {
  }

  /// Коэфициент штрафа
  TDoubles<Owner> Mulcts;

  virtual double CalculateFunctionals(const double* y, int fNumber)
  {
    double x[MAX_TRIAL_DIMENSION];
    //for (int i = 0; i < Dimension; i++)
    //{
    //  x[i] = (y[i] - mShift[GetRealNumberOfConstraints()][i]) / mZoomRatios[GetRealNumberOfConstraints()];
    //}
    double result = TProblemWithConstraints::CalculateFunctionals(y, GetRealNumberOfConstraints()); //mPFunction[GetRealNumberOfConstraints()]->Calculate(y);
    for (int i = 0; i < GetRealNumberOfConstraints(); i++)
    {
      for (int j = 0; j < Dimension; j++)
      {
        x[j] = (y[j] - mShift[i][j]) / mZoomRatios[i];
      }
      double v = GLOBALIZER_MAX(mPFunction[i]->Calculate(x) - Q[i], 0.0);
      result += Mulcts[i] * (v * v);
    }
    return result;
  }

    /** Метод возвращает число общее функций в задаче (оно равно число ограничений + число критериев)
  \return Число функций
  */
  virtual int GetNumberOfFunctions() const
  {
    return 1;
  }

  /** Метод возвращает число ограничений в задаче
  \return Число ограничений
  */
  virtual int GetNumberOfConstraints() const
  {
    return 0;
  }

};


#endif /// __PROBLEM_WITH_MULCT_FUNCTION_H__
// - end of file ----------------------------------------------------------------------------------

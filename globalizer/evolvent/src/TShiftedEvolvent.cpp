/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      shiftedevolvent.cpp                                         //
//                                                                         //
//  Purpose:   Source file for shifted evolvent classes                    //
//                                                                         //
//  Author(s): Barkalov K., Sysoyev A.                                     //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#pragma warning(disable:4996) 

/**
\file shiftedevolvent.cpp

\authors Баркалов К., Сысоев А.
\date 2015-2016
\copyright ННГУ им. Н.И. Лобачевского

\brief Реализация класса #ShiftedEvolvent

\details Реализация методов класса #ShiftedEvolvent
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>

#include "TShiftedEvolvent.h"
#include "Exception.h"

// ------------------------------------------------------------------------------------------------
ShiftedEvolvent::ShiftedEvolvent(int _N, int _m, int _L) :
    Evolvent(_N, _m)
{
    if ((_L < 0) || (_L >= m))
    {
        throw EXCEPTION("L is out of range");
    }
    L = _L;
    //Инициализация массива степеней двойки
    PowOf2[0] = 1;
    for (int i = 1; i <= L * N; i++)
        PowOf2[i] = PowOf2[i - 1] * 2;

}

// ------------------------------------------------------------------------------------------------
ShiftedEvolvent::~ShiftedEvolvent()
{
}

// ------------------------------------------------------------------------------------------------
void ShiftedEvolvent::transform_P_to_Pl(int EvolventNum)
{
    //  if (N == 1) return;
      // transformation from hypercube P to hypercube P[l]
    double temp;
    if (EvolventNum == 0)
    {
        temp = 0.0;
    }
    else
    {
        temp = 1.0 / PowOf2[EvolventNum]; // temp = 1 / 2^l (l = 1,...,L)
    }
    for (int i = 0; i < N; i++)
    {
        y[i] = y[i] * 2 + 0.5 - temp;
    }
}

// ------------------------------------------------------------------------------------------------
void ShiftedEvolvent::transform_Pl_to_P(int EvolventNum)
{
    //  if (N == 1) return;
      // transformation from hypercube P to hypercube P[l]
    double temp;
    if (EvolventNum == 0)
    {
        temp = 0;
    }
    else
    {
        temp = 1.0 / PowOf2[EvolventNum]; // temp = 1 / 2^l (l = 1,...,L)
    }
    for (int i = 0; i < N; i++)
    {
        y[i] = (y[i] - 0.5 + temp) / 2;
    }
}

// ------------------------------------------------------------------------------------------------
double ShiftedEvolvent::ZeroConstraint()
{
    double CurZ = -MaxDouble;
    for (int i = 0; i < N; i++)
    {
        if (fabs(y[i]) - 0.5 > CurZ)
        {
            CurZ = fabs(y[i]) - 0.5;
        }
    }
    return CurZ;
}

// - end of file ----------------------------------------------------------------------------------

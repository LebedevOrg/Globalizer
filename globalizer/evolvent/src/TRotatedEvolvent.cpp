/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      rotatedevolvent.cpp                                         //
//                                                                         //
//  Purpose:   Source file for rotated evolvent classes                    //
//                                                                         //
//  Author(s): Barkalov K., Sysoyev A.                                     //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#pragma warning(disable:4996) 

/**
\file rotatedevolvent.cpp

\authors Баркалов К., Сысоев А.
\date 2015-2016
\copyright ННГУ им. Н.И. Лобачевского

\brief Реализация класса #RotatedEvolvent

\details Реализация методов класса #RotatedEvolvent
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>

#include "TRotatedEvolvent.h"
#include "Exception.h"

RotatedEvolvent::RotatedEvolvent(int _N, int _m, int _L) :
    Evolvent(_N, _m)
{
    L = _L;
    // !!!!!!!!!!!!!
    if (N == 1)
        return;
    // !!!!!!!!!!!!!
    PlaneCount = N * (N - 1) / 2;
    if ((L < 1) || (L > 2 * PlaneCount + 1))
    {
        throw EXCEPTION("L is out of range");
    }
    GetAllPlanes();
    PowOfHalf[0] = 1;
    for (int i = 1; i < m + 2; i++)
        PowOfHalf[i] = PowOfHalf[i - 1] / 2;
}

// ------------------------------------------------------------------------------------------------
RotatedEvolvent::~RotatedEvolvent()
{
}

// ------------------------------------------------------------------------------------------------
void RotatedEvolvent::GetAllPlanes()
{
    const int k = 2; // Подмножества из двух элементов
    int plane[k];    // Два номера под элементы

    for (int i = 0; i < k; i++)
        plane[i] = i;

    if (N <= k)
    {
        for (int i = 0; i < k; i++)
        {
            Planes[0][i] = plane[i];
        }
        return;
    }
    int p = k - 1;
    int counter = 0; //счетчик числа перестановок
    while (p >= 0)
    {
        for (int i = 0; i < k; i++)
        {
            Planes[counter][i] = plane[i];
        }
        counter++;

        if (plane[k - 1] == N - 1)
        {
            p--;
        }
        else
        {
            p = k - 1;
        }

        if (p >= 0)
        {
            for (int i = k - 1; i >= p; i--)
            {
                plane[i] = plane[p] + i - p + 1;
            }
        }
    }
}

// - end of file ----------------------------------------------------------------------------------

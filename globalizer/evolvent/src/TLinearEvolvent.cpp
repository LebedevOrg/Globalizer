/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      linarevolvent.cpp                                           //
//                                                                         //
//  Purpose:   Source file for linar evolvent classes                      //
//                                                                         //
//  Author(s): Barkalov K., Sysoyev A.                                     //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#pragma warning(disable:4996) 

/**
\file linarevolvent.cpp

\authors Баркалов К., Сысоев А.
\date 2015-2016
\copyright ННГУ им. Н.И. Лобачевского

\brief Реализация класса #LinearEvolvent

\details Реализация методов класса #LinearEvolvent
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>

#include "TLinearEvolvent.h"
#include "Exception.h"

namespace
{
    // ------------------------------------------------------------------------------------------------
    void numbr(Extended* iss, const int n1, const Extended& nexp,
        int& l, int* iu, int* iv,
        const Extended& extOne,
        const Extended& extZero,
        const Extended& extHalf)
    {
        /* calculate s(u)=is,l(u)=l,v(u)=iv by u=iu */

        Extended iff, is;
        int n, k1, k2, l1;

        n = n1 + 1;
        iff = nexp;
        is = extZero;
        k1 = -1;
        for (int i = 0; i < n; i++)
        {
            iff = iff * extHalf;
            k2 = -k1 * iu[i];
            iv[i] = iu[i];
            k1 = k2;
            if (k2 < 0) l1 = i;
            else { is += iff; l = i; }
        }
        if (is == extZero) l = n1;
        else
        {
            iv[n1] = -iv[n1];
            if (is == (nexp - extOne)) l = n1;
            else if (l1 == n1) iv[l] = -iv[l];
            else l = l1;
        }
        *iss = is;
    }

    // ------------------------------------------------------------------------------------------------
    void xyd(Extended* xx, int m, double y[], int n,
        const Extended& nexp,
        const Extended& extOne,
        const Extended& extZero,
        const Extended& extHalf)
    {
        /* calculate preimage x  for nearest level  m center to y */
        /* (x - left boundary point of level m interval)          */
    //    int n1, l, iq, iu[MaxDim], iv[MaxDim];
        int n1, l, iu[MaxDim]{}, iv[MaxDim]{};

        Extended r1, x;
        double r;
        int iw[MaxDim + 1]{};
        int it;
        Extended is;

        n1 = n - 1;
        for (int i = 0; i < n; i++)
        {
            iw[i] = 1;
        }
        r = 0.5;
        r1 = extOne;
        x = extZero;
        it = 0;
        for (int j = 0; j < m; j++)
        {
            r *= 0.5;
            for (int i = 0; i < n; i++)
            {
                iu[i] = (y[i] < 0) ? -1 : 1;
                y[i] -= r * iu[i];
                iu[i] *= iw[i];
            }
            std::swap(iu[0], iu[it]);
            numbr(&is, n1, nexp, l, iu, iv, extOne, extZero, extHalf);
            std::swap(iv[0], iv[it]);
            for (int i = 0; i < n; i++)
                iw[i] = -iw[i] * iv[i];
            if (l == 0) l = it;
            else if (l == it) l = 0;
            it = l;
            r1 = r1 / nexp;
            x += r1 * is;
        }
        *xx = x;
    }

    // ------------------------------------------------------------------------------------------------
    void invmad(int m, Extended xp[], int kp,
        int* kxx, double p[], int n, int incr,
        const Extended& nexp,
        const Extended& mne,
        const Extended& extOne,
        const Extended& extZero,
        const Extended& extHalf)
    {
        /* calculate kx preimage p node */
        /*   node type mapping m level  */

        Extended dr, dd, del, d1, x;
        double r, d, u[MaxDim], y[MaxDim];
        int i, k, kx;

        kx = 0;
        kp--;
        for (int i = 0; i < n; i++)
        {
            u[i] = -1.0;
        }
        dr = nexp;
        for (r = 0.5, i = 0; i < m; i++)
        {
            r *= 0.5;
        }
        dr = mne / nexp;

        dr = dr - fmod(dr.toDouble(), extOne.toDouble());
        //dr = (dr>0) ? floor(dr) : ceil(dr);

        del = extOne / (mne - dr);
        d1 = del * (incr + 0.5);
        for (kx = -1; kx < kp;)
        {
            for (i = 0; i < n; i++)
            {       /* label 2 */
                d = p[i];
                y[i] = d - r * u[i];
            }
            for (i = 0; (i < n) && (fabs(y[i]) < 0.5); i++);
            if (i >= n)
            {
                xyd(&x, m, y, n, nexp, extOne, extZero, extHalf);
                dr = x * mne;
                dd = dr - fmod(dr.toDouble(), extOne.toDouble());
                //dd = (dr>0) ? floor(dr) : ceil(dr);
                dr = dd / nexp;
                dd = dd - dr + fmod(dr.toDouble(), extOne.toDouble());
                //dd = dd - ((dr>0) ? floor(dr) : ceil(dr));
                x = dd * del;
                if (kx > kp) break;
                k = kx++;                     /* label 9 */
                if (kx == 0) xp[0] = x;
                else
                {
                    while (k >= 0)
                    {
                        dr = fabs(x - xp[k]);     /* label 11 */
                        if (dr <= d1) {
                            for (kx--; k < kx; k++, xp[k] = xp[k + 1]);
                            goto m6;
                        }
                        else
                            if (x <= xp[k])
                            {
                                xp[k + 1] = xp[k]; k--;
                            }
                            else break;
                    }
                    xp[k + 1] = x;
                }
            }
        m6: for (i = n - 1; (i >= 0) && (u[i] = (u[i] <= 0.0) ? 1 : -1) < 0; i--);
            if (i < 0) break;
        }
        *kxx = ++kx;

    }

    // ------------------------------------------------------------------------------------------------
    void node(Extended is, int n1, Extended nexp, int& l, int& iq, int iu[], int iv[],
        const Extended& extOne,
        const Extended& extZero,
        const Extended& extHalf)
    {
        /* calculate iu=u[s], iv=v[s], l=l[s] by is=s */

        Extended iff;

        int n = n1 + 1;
        if (is == extZero)
        {
            l = n1;
            for (int i = 0; i < n; i++)
            {
                iu[i] = -1; iv[i] = -1;
            }
        }
        else if (is == (nexp - extOne))
        {
            l = n1;
            iu[0] = 1;
            iv[0] = 1;
            for (int i = 1; i < n; i++) {
                iu[i] = -1; iv[i] = -1;
            }
            iv[n1] = 1;
        }
        else
        {
            iff = nexp;
            int k1 = -1, k2;
            for (int i = 0; i < n; i++)
            {
                iff = iff / 2;
                if (is >= iff) {
                    if ((is == iff) && (is != extOne)) { l = i; iq = -1; }
                    is = is - iff;
                    k2 = 1;
                }
                else
                {
                    k2 = -1;
                    if ((is == (iff - extOne)) && (is != extZero)) { l = i; iq = 1; }
                }
                int j = -k1 * k2;
                iv[i] = j;
                iu[i] = j;
                k1 = k2;
            }
            iv[l] = iv[l] * iq;
            iv[n1] = -iv[n1];
        }
    }

    // ------------------------------------------------------------------------------------------------
    void mapd(Extended x, int m, double* y, int n, int key,
        const Extended& nexp,
        const Extended& mne,
        const Extended& extOne,
        const Extended& extZero,
        const Extended& extHalf)
    {
        /* mapping y(x) : 1 - center, 2 - line, 3 - node */
        // use key = 1

        int n1, l, iq, iu[MaxDim], iv[MaxDim];
        Extended d, is;
        double p, r;
        int iw[MaxDim];
        int it, k;

        p = 0.0;
        n1 = n - 1;
        d = x;
        r = 0.5;
        it = 0;
        for (int i = 0; i < n; i++)
        {
            iw[i] = 1; y[i] = 0.0;
        }

        if (key == 2)
        {
            d = d * (extOne - extOne / mne); k = 0;
        }
        else if (key > 2)
        {
            Extended dr = mne / nexp;
            dr = dr - fmod(dr.toDouble(), extOne.toDouble());
            //dr=(dr>0)?floor(dr):ceil(dr);
            Extended dd = mne - dr;
            dr = d * dd;
            dd = dr - fmod(dr.toDouble(), extOne.toDouble());
            //dd=(dr>0)?floor(dr):ceil(dr);
            dr = dd + (dd - extOne) / (nexp - extOne);
            dd = dr - fmod(dr.toDouble(), extOne.toDouble());
            //dd=(dr>0)?floor(dr):ceil(dr);
            d = dd * (extOne / (mne - extOne));
        }

        for (int j = 0; j < m; j++)
        {
            iq = 0;
            if (x == extOne)
            {
                is = nexp - extOne; d = extZero;
            }
            else
            {
                d = d * nexp;
                is = floor(d);
                //is = (int)d.toDouble(); //опасное преобразование при n > 32
                d = d - is;
            }
            node(is, n1, nexp, l, iq, iu, iv, extOne, extZero, extHalf);
            std::swap(iu[0], iu[it]);
            std::swap(iv[0], iv[it]);
            if (l == 0)
                l = it;
            else if (l == it) l = 0;
            if ((iq > 0) || ((iq == 0) && (is == 0))) k = l;
            else if (iq < 0) k = (it == n1) ? 0 : n1;
            r = r * 0.5;
            it = l;
            for (int i = 0; i < n; i++)
            {
                iu[i] = iu[i] * iw[i];
                iw[i] = -iv[i] * iw[i];
                p = r * iu[i];
                p = p + y[i];
                y[i] = p;
            }
        }
        if (key == 2)
        {
            int i;
            if (is == (nexp - extOne)) i = -1;
            else i = 1;
            p = 2 * i * iu[k] * r * d.toDouble();
            p = y[k] - p;
            y[k] = p;
        }
        else if (key == 3)
        {
            for (int i = 0; i < n; i++)
            {
                p = r * iu[i];
                p = p + y[i];
                y[i] = p;
            }
        }
    }
}

// ------------------------------------------------------------------------------------------------
LinearEvolvent::LinearEvolvent(int _N, int _m) :
    Evolvent(_N, _m)
{
    nexpExtended = extOne;
    for (int i = 0; i < _N; nexpExtended *= 2, i++);
    mneExtended = extOne;
    for (int i = 0; i < _m; mneExtended *= nexpExtended, i++);
}

// ------------------------------------------------------------------------------------------------
LinearEvolvent::~LinearEvolvent()
{
}

// ------------------------------------------------------------------------------------------------
void LinearEvolvent::GetImage(const Extended& x, double* _y, int EvolventNum)
{
    if ((x < 0) || (x > 1))
    {
        throw EXCEPTION("x is out of range");
    }
    // x ---> y
    if (N == 1)
    {
        y[0] = x.toDouble() - 0.5;
        return;
    }
    mapd(x, m, y, N, 2, nexpExtended, mneExtended, extOne, extNull, extHalf);
    transform_P_to_D();
    memcpy(_y, y, N * sizeof(double));
}

// ------------------------------------------------------------------------------------------------
void LinearEvolvent::GetInverseImage(double* _y, Extended& x)
{
    // y ---> x
    memcpy(y, _y, N * sizeof(double));
    transform_D_to_P();
    if (N == 1)
    {
        x = y[0] + 0.5;
        return;
    }
    xyd(&x, m, y, N, nexpExtended, extOne, extNull, extHalf);
}

// - end of file ----------------------------------------------------------------------------------

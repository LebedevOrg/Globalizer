/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      smoothevolvent.cpp                                          //
//                                                                         //
//  Purpose:   Source file for smooth evolvent classes                     //
//                                                                         //
//  Author(s): Barkalov K., Sysoyev A.                                     //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#pragma warning(disable:4996) 

/**
\file smoothevolvent.cpp

\authors Баркалов К., Сысоев А.
\date 2015-2016
\copyright ННГУ им. Н.И. Лобачевского

\brief Реализация класса #SmoothEvolvent

\details Реализация методов класса #SmoothEvolvent
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>

#include "TSmoothEvolvent.h"
#include "Exception.h"

namespace
{
    // ------------------------------------------------------------------------------------------------
    int _Pow_int(int x, int n)
    {
        int val = 1;
        for (int i = 0; i < n; i++)
            val *= x;
        return val;
    }

    // ------------------------------------------------------------------------------------------------
    double Hermit(double y0, double d0, double y1, double d1, double h, double x)
    {
        return y0 + (x + h) * (d0 + (x + h) * (d0 - (y0 - y1) / (-2 * h) +
            (x - h) * (d0 - 2 * (y0 - y1) / (-2 * h) + d1) / (-2 * h)) / (-2 * h));
    }

    // ------------------------------------------------------------------------------------------------
    double HermitDer(double y0, double d0, double y1, double d1, double h, double x)
    {
        return pow(h, -3) * (d0 * h * (-0.25 * h * h - 0.5 * h * x + 0.75 * x * x)
            + d1 * h * (-0.25 * h * h + 0.5 * h * x + 0.75 * x * x)
            - 0.75 * h * h * y0 + 0.75 * h * h * y1
            + 0.75 * x * x * y0
            - 0.75 * x * x * y1);
    }

    // ------------------------------------------------------------------------------------------------
    int node_smooth(int is, int n, int& iq, int nexp, int* iu, int* iv)
    {
        /* calculate iu=u[s], iv=v[s], l=l[s] by is=s */
        iq = 0;
        int k1, k2, iff;
        static int l = 0;
        if (is == 0)
        {
            l = n - 1;
            for (int i = 0; i < n; i++)
            {
                iu[i] = -1;
                iv[i] = -1;
            }
        }
        else
            if (is == (nexp - 1))
            {
                l = n - 1;
                iu[0] = 1;
                iv[0] = 1;
                for (int i = 1; i < n; i++)
                {
                    iu[i] = -1;
                    iv[i] = -1;
                }
                iv[n - 1] = 1;
            }
            else
            {
                iff = nexp;
                k1 = -1;
                for (int i = 0; i < n; i++)
                {
                    iff /= 2;
                    if (is >= iff)
                    {
                        if ((is == iff) && (is != 1)) { l = i; iq = -1; }
                        is = is - iff;
                        k2 = 1;
                    }
                    else
                    {
                        k2 = -1;
                        if ((is == (iff - 1)) && (is != 0)) { l = i; iq = 1; }
                    }
                    iu[i] = iv[i] = -k1 * k2;
                    k1 = k2;
                }
                iv[l] *= iq;
                iv[n - 1] = -iv[n - 1];
            }
        return l;
    }

    // ------------------------------------------------------------------------------------------------
    void SmoothEvolventDer(double x, int n, int m, std::vector<double>& y, std::vector<double>& y_, bool c)
    {
        if (y.size() != n)
            y.assign(n, .0);
        if (y_.size() != n)
            y_.assign(n, .0);
        int l = 0, iq = 0;
        std::vector<int> iu(n, 0);
        std::vector<int> iv(n, 0);
        int nexp = _Pow_int(2, n); // nexp=2**n */
        double mnexp = _Pow_int(nexp, m); // mnexp=2**(nm)
        double d = 1.0 / mnexp;
        std::vector<int> iw(n, 0);
        double xd = x;
        int it = 0;
        double dr = nexp;
        for (int i = 0; i < n; i++)
        {
            iw[i] = 1;
            y[i] = 0;
        }
        int k = 0;
        double r = 0.5;
        int ic;
        for (int j = 0; j < m; j++)
        {
            if (x == 1.0 - d)
            {
                ic = nexp - 1;
                xd = 0.0;
            }
            else
            {
                xd = xd * nexp;
                ic = (int)xd;
                xd = xd - ic;
            }
            iq = 0;
            l = node_smooth(ic, n, iq, nexp, iu.data(), iv.data());
            int swp = iu[it];
            iu[it] = iu[0];
            iu[0] = swp;
            swp = iv[it];
            iv[it] = iv[0];
            iv[0] = swp;
            if (l == 0)
                l = it;
            else if (l == it)
                l = 0;
            if ((iq > 0) || ((iq == 0) && (ic == 0)))
                k = l;
            else if (iq < 0)
            {
                if (it == n - 1)
                    k = 0;
                else
                    k = n - 1;
            }
            r *= 0.5;
            it = l;
            for (int i = 0; i < n; i++)
            {
                iu[i] *= iw[i];
                iw[i] *= -iv[i];
                y[i] += r * iu[i];
            }
        }
        if (c)
        {
            if (ic == (nexp - 1))
            {
                y[k] += 2 * iu[k] * r * xd;
                y_[k] = ((iu[k] > 0) ? 1 : (iu[k] < 0) ? -1 : 0) * pow(2, m * (n - 1));
            }
            else
            {
                y[k] -= 2 * iu[k] * r * xd;
                y_[k] = -((iu[k] > 0) ? 1 : (iu[k] < 0) ? -1 : 0) * pow(2, m * (n - 1));
            }
            if (x == 1.0 - d)
            {
                std::vector<double> y0(n, 0);
                y_[k] = 0;
                ::SmoothEvolventDer(x - d / 2, n, m, y0, y_, true);
            }
        }
    }
}

SmoothEvolvent::SmoothEvolvent(int _N, int _m, double _h) :
    Evolvent(_N, _m)
{
    if (_N > 2)
    {
        std::cout << "Warning: smooth evolvent is very slow when problem dimension > 2\n";
    }
    h = _h;
    if (h < 0 || h > 1)
    {
        throw EXCEPTION("h is out of range");
    }
    continuously = h != 1. ? true : false;
    smoothPointCount = 0;
    tmp_y.resize(N);
    tmp_y_.resize(N);
}

// ------------------------------------------------------------------------------------------------
void SmoothEvolvent::GetInverseImage(double* _y, Extended& x)
{
    throw EXCEPTION("This method is not implemented for the smooth evolvent");
}

// ------------------------------------------------------------------------------------------------
void SmoothEvolvent::GetYOnXSmooth(double x, std::vector<double>& y, std::vector<double>& y_)
{
    y.assign(N, .0);
    y_.assign(N, .0);

    if (N == 1)
    {
        y[0] = x - 0.5;
        return;
    }

    int l = 0, iq = 0;
    std::vector<int> iu(N, 0);
    std::vector<int> iv(N, 0);
    int nexp = _Pow_int(2, N); // nexp=2**n
    double mnexp = pow(nexp, m); // mnexp=2**(nm)
    double d = 1.0 / mnexp;
    double dh = d * h;
    double xc = 0;
    while (xc < x)
        xc += d;
    if (((h > 0) && (h <= .5) && (x > dh) && (x < 1 - d - dh) && ((xc - x < dh) || (xc - x > d - dh))) && continuously)
    {
        smoothPointCount++;

        std::vector<double> y0(N, 0);
        std::vector<double> y1(N, 0);
        std::vector<double> y0_(N, 0);
        std::vector<double> y1_(N, 0);
        double xh = 0;
        if (xc - x < dh)
        {
            xh += x - xc;
            SmoothEvolventDer(xc - dh, N, m, y0, y0_, true);
            SmoothEvolventDer(xc + dh, N, m, y1, y1_, true);
        }
        else
        {
            xh += x - xc + d;
            SmoothEvolventDer(xc - d - dh, N, m, y0, y0_, true);
            SmoothEvolventDer(xc - d + dh, N, m, y1, y1_, true);
        }
        int i0 = -1, i1 = -1;
        for (int i = 0; i < N; i++)
        {
            if (y1[i] != y0[i])
            {
                if (i0 == -1)
                    i0 = i;
                else
                    i1 = i;
            }
            else
            {
                y[i] = y0[i];
                y_[i] = 0;
            }
        }
        if (i0 != -1 && i1 != -1)
        {
            y[i0] = Hermit(y0[i0], y0_[i0], y1[i0], y1_[i0], dh, xh);
            y_[i0] = HermitDer(y0[i0], y0_[i0], y1[i0], y1_[i0], dh, xh);
            y[i1] = Hermit(y0[i1], y0_[i1], y1[i1], y1_[i1], dh, xh);
            y_[i1] = HermitDer(y0[i1], y0_[i1], y1[i1], y1_[i1], dh, xh);
            return;
        }
    }
    SmoothEvolventDer(x, N, m, y, y_, continuously);
}

// ------------------------------------------------------------------------------------------------
SmoothEvolvent::~SmoothEvolvent()
{
}

// - end of file ----------------------------------------------------------------------------------

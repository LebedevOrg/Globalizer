/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      rotatedevolvent.h                                           //
//                                                                         //
//  Purpose:   Header file for rotatedevolvent classes                     //
//                                                                         //
//  Author(s): Barkalov K., Sysoyev A.                                     //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


/**
\file rotatedevolvent.h

\authors Баркалов К., Сысоев А.
\date 2015-2016
\copyright ННГУ им. Н.И. Лобачевского

\brief Объявление класса #RotatedEvolvent

\details Объявление класса #RotatedEvolvent и сопутствующих типов данных
*/

#ifndef __ROTATEDEVOLVENT_H__
#define __ROTATEDEVOLVENT_H__

#include "Evolvent.h"
#include "Common.h"
#include "Extended.h"
#include <vector>

// ------------------------------------------------------------------------------------------------

/**
\brief Класс, реализующий отображение между гиперкубом и гиперинтервалом

Класс #RotatedEvolvent предоставляет средства для преобразования координат между
гиперкубом [-1/2, 1/2]^N и гиперинтервалом D.
*/
class RotatedEvolvent : public Evolvent
{
protected:

	/// Количество развёрток, по умолчанию 1, означает, что используется одна развёртка
	int L;
	
	/// текущее количество уровней
	int PlaneCount; // current number of planes
	
	/// массив осей
	int Planes[MaxDim * (MaxDim - 1) / 2][2];
	
	/// Статический массив хранящий степени 1/2
	double PowOfHalf[MaxM + 2];

	void GetAllPlanes();
public:

	/**
	\brief Конструктор класса #RotatedEvolvent
	*/
	RotatedEvolvent(int _N = 2, int _m = 10, int _L = 0);
	
	/// Деструктор класса #RotatedEvolvent
	virtual ~RotatedEvolvent();
	
	/**
	\brief Преобразование x в y (x -> y)
	*/
	void GetImage(const Extended& x, double* _y, int EvolventNum = 0) override {
		if (L == 1 || EvolventNum == 0)
		{
			Evolvent::GetImage(x, _y);
			return;
		}

		int PlaneIndex = EvolventNum - 1; // теперь PlaneIndex - номер перестановки
		PlaneIndex = PlaneIndex % PlaneCount;

		GetYOnX(x);

		// shift to center for convenient rotation
		//for (int i = 0; i < N; i++)
		//  y[i] += PowOfHalf[m + 1];

		// rotate
		double tmpCoord = y[Planes[PlaneIndex][1]];
		y[Planes[PlaneIndex][1]] = y[Planes[PlaneIndex][0]];
		y[Planes[PlaneIndex][0]] = -tmpCoord;

		//Меняем знак преобразования, если число разверток больше числа плоскостей
		if (EvolventNum > PlaneCount)
		{
			y[Planes[PlaneIndex][0]] = -y[Planes[PlaneIndex][0]];
			y[Planes[PlaneIndex][1]] = -y[Planes[PlaneIndex][1]];
		}

		// shift back to corner
		//for (int i = 0; i < N; i++)
		//  y[i] -= PowOfHalf[m + 1];

		transform_P_to_D();
		memcpy(_y, y, N * sizeof(double));
	}
	
	/**
	\brief Преобразование y в x (y -> x)
	*/
	void GetPreimages(double* _y, Extended* x) override {
		memcpy(y, _y, N * sizeof(double));
		transform_D_to_P();
		// прообраз для первой развертки
		x[0] = GetXOnY();

		if (L == 1)
			return;

		for (int i = 1; i < L; i++)
		{
			memcpy(y, _y, N * sizeof(double));
			transform_D_to_P();
			// обратное преобразование координат
			int PlaneIndex = (i - 1) % PlaneCount;

			double tmpCoord = y[Planes[PlaneIndex][1]];
			y[Planes[PlaneIndex][1]] = -y[Planes[PlaneIndex][0]];
			y[Planes[PlaneIndex][0]] = tmpCoord;

			if (i > PlaneCount)//Меняем знак преобразования, если число разверток больше числа плоскостей
			{
				y[Planes[PlaneIndex][0]] = -y[Planes[PlaneIndex][0]];
				y[Planes[PlaneIndex][1]] = -y[Planes[PlaneIndex][1]];
			}

			// прообраз для i - 1 развертки
			x[i] = GetXOnY();
		}
	}
};

#endif
// - end of file ----------------------------------------------------------------------------------
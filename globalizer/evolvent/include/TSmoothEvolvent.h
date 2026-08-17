/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      smoothevolvent.h                                            //
//                                                                         //
//  Purpose:   Header file for smooth evolvent classes                     //
//                                                                         //
//  Author(s): Barkalov K., Sysoyev A.                                     //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


/**
\file smoothevolvent.h

\authors Баркалов К., Сысоев А.
\date 2015-2016
\copyright ННГУ им. Н.И. Лобачевского

\brief Объявление класса #SmoothEvolvent

\details Объявление класса #SmoothEvolvent и сопутствующих типов данных
*/

#ifndef __SMOOTHEVOLVENT_H__
#define __SMOOTHEVOLVENT_H__

#include "Evolvent.h"
#include "Common.h"
#include "Extended.h"
#include "Exception.h"
#include <vector>

// ------------------------------------------------------------------------------------------------

/**
\brief Класс, реализующий отображение между гиперкубом и гиперинтервалом

Класс #SmoothEvolvent предоставляет средства для преобразования координат между
гиперкубом [-1/2, 1/2]^N и гиперинтервалом D.
*/
class SmoothEvolvent : public Evolvent
{
protected:

	/// Гладкость
	double h;
	int smoothPointCount;
	bool continuously;
	std::vector<double> tmp_y, tmp_y_;


	void GetYOnXSmooth(double x, std::vector<double>& y, std::vector<double>& y_);
public:

	/**
	\brief Конструктор класса #SmoothEvolvent
	*/
	SmoothEvolvent(int _N = 2, int _m = 10, double _h = 0.25);

	/// Деструктор класса #SmoothEvolvent
	virtual ~SmoothEvolvent();

	/**
	\brief Преобразование x в y (x -> y)
	*/
	void GetImage(const Extended& x, double* _y, int EvolventNum = 0) override {
		if ((x < 0) || (x > 1))
		{
			throw EXCEPTION("x is out of range");
		}
		// x ---> y
		GetYOnXSmooth(x.toDouble(), tmp_y, tmp_y_);
		std::copy(tmp_y.begin(), tmp_y.end(), y);
		transform_P_to_D();

		memcpy(_y, y, N * sizeof(double));
	}

	/**
	\brief Преобразование y в x (y -> x)
	*/
	void GetInverseImage(double* _y, Extended& x);
};

#endif
// - end of file ----------------------------------------------------------------------------------
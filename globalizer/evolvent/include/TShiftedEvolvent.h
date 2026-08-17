/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      shiftedevolvent.h                                           //
//                                                                         //
//  Purpose:   Header file for shifted evolvent classes                    //
//                                                                         //
//  Author(s): Barkalov K., Sysoyev A.                                     //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


/**
\file shiftedevolvent.h

\authors Баркалов К., Сысоев А.
\date 2015-2016
\copyright ННГУ им. Н.И. Лобачевского

\brief Объявление класса #ShiftedEvolvent

\details Объявление класса #ShiftedEvolvent и сопутствующих типов данных
*/

#ifndef __SHIFTEDEVOLVENT_H__
#define __SHIFTEDEEVOLVENT_H__

#include "Evolvent.h"
#include "Common.h"
#include "Extended.h"
#include <vector>

// ------------------------------------------------------------------------------------------------

/**
\brief Класс, реализующий отображение между гиперкубом и гиперинтервалом

Класс #ShiftedEvolvent предоставляет средства для преобразования координат между
гиперкубом [-1/2, 1/2]^N и гиперинтервалом D.
*/
class ShiftedEvolvent : public Evolvent
{
protected:

	/// Количество развёрток - 1, по умолчанию 0, означает, что используется одна развёртка
	int    L;
	
	/// Статический массив хранящий степени 2
	double PowOf2[MaxDim * MaxL + 1];
	
	void transform_P_to_Pl(int EvolventNum);

	void transform_Pl_to_P(int EvolventNum);

	double ZeroConstraint();
public:

	/**
	\brief Конструктор класса #ShiftedEvolvent
	*/
	ShiftedEvolvent(int _N = 2, int _m = 10, int _L = 0);

	/// Деструктор класса #ShiftedEvolvent
	virtual ~ShiftedEvolvent();

	/**
	\brief Преобразование x в y (x -> y)
	*/
	void GetImage(const Extended& x, double* _y, int EvolventNum = 0) override {
		/*
			double* tmp = NULL;
			int i;
			//непонятно, зачем в GlobalExpert данная точка была вынесена в отдельную ветку

			if (x == extNull)
			{
			  for (i = 0; i < N; i++)
			  {
				_y[i] = 0.0;
			  }
			}
			else
			  */
		GetYOnX(x);

		transform_P_to_Pl(EvolventNum);
		transform_P_to_D();

		memcpy(_y, y, N * sizeof(double));
	}

	/**
	\brief Преобразование y в x (y -> x)
	*/
	void GetPreimages(double* _y, Extended* x) override {
		for (int i = 0; i <= L; i++)
		{
			memcpy(y, _y, N * sizeof(double));
			transform_D_to_P();
			transform_Pl_to_P(i);
			x[i] = GetXOnY();
		}
	}

	/// Вычисляет функцию существования точки в развертки EvolventNum для y, <0 - существует
	double ZeroConstraintCalc(const double* _y, int EvolventNum = 0) override {
		// копируем y
		memcpy(y, _y, N * sizeof(double));
		// центрируем и нормируем область
		transform_D_to_P();
		// сдвигаем область в соответствии с разверткой
		transform_Pl_to_P(EvolventNum);
		// вычисляем функционал
		return ZeroConstraint();
	}
};
#endif
// - end of file ----------------------------------------------------------------------------------
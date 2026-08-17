/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      linarevolvent.h                                             //
//                                                                         //
//  Purpose:   Header file for linar evolvent classes                      //
//                                                                         //
//  Author(s): Barkalov K., Sysoyev A.                                     //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


/**
\file linarevolvent.h

\authors Баркалов К., Сысоев А.
\date 2015-2016
\copyright ННГУ им. Н.И. Лобачевского

\brief Объявление класса #LinearEvolvent

\details Объявление класса #LinearEvolvent и сопутствующих типов данных
*/

#ifndef __LINAREVOLVENT_H__
#define __LINAREVOLVENT_H__

#include "Evolvent.h"
#include "Common.h"
#include "Extended.h"
#include <vector>

// ------------------------------------------------------------------------------------------------

/**
\brief Класс, реализующий отображение между гиперкубом и гиперинтервалом

Класс #LinearEvolvent предоставляет средства для преобразования координат между
гиперкубом [-1/2, 1/2]^N и гиперинтервалом D.
*/

class LinearEvolvent : public Evolvent
{
protected:
	Extended mneExtended;
public:

	/**
	\brief Конструктор класса #LinearEvolvent
	*/
	LinearEvolvent(int _N = 2, int _m = 10);

	/// Деструктор класса #LinearEvolvent
	virtual ~LinearEvolvent();

	/**
	\brief Преобразование x в y (x -> y)
	*/
	void GetImage(const Extended& x, double* _y, int EvolventNum = 0);

	/**
	\brief Преобразование y в x (y -> x)
	*/
	void GetInverseImage(double* _y, Extended& x);
};

#endif
// - end of file ----------------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      noninjectiveevolvent.h                                      //
//                                                                         //
//  Purpose:   Header file for noninjective evolvent classes               //
//                                                                         //
//  Author(s): Barkalov K., Sysoyev A.                                     //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


/**
\file noninjectiveevolvent.h

\authors Баркалов К., Сысоев А.
\date 2015-2016
\copyright ННГУ им. Н.И. Лобачевского

\brief Объявление класса #NoninjectiveEvolvent

\details Объявление класса #NoninjectiveEvolvent и сопутствующих типов данных
*/

#ifndef __NONINJECTIVEEVOLVENT_H__
#define __NONINJECTIVEEVOLVENT_H__

#include "Evolvent.h"
#include "Common.h"
#include "Extended.h"
#include <vector>

// ------------------------------------------------------------------------------------------------

/**
\brief Класс, реализующий отображение между гиперкубом и гиперинтервалом

Класс #NoninjectiveEvolvent предоставляет средства для преобразования координат между
гиперкубом [-1/2, 1/2]^N и гиперинтервалом D.
*/
class NoninjectiveEvolvent : public Evolvent
{
protected:

	/// максимальное количество прообразов
	int max_preimages;

	Extended mneExtended;
public:

	/**
	\brief Конструктор класса #NoninjectiveEvolvent
	*/
	NoninjectiveEvolvent(int _N = 2, int _m = 10, int _max_preimages = 64);
	
	/// Деструктор класса #NoninjectiveEvolvent
	virtual ~NoninjectiveEvolvent();
	
	/**
	\brief Преобразование x в y (x -> y)
	*/
	void GetImage(const Extended& x, double* _y, int EvolventNum = 0);
	
	/**
	\brief Неинъективное преобразование y в x (y -> x)
	*/
	int GetNoninjectivePreimages(double* _y, Extended* x);
	
	/// Получить максимальное количество прообразов
	int GetMaxPreimagesNumber() const { return max_preimages; };
};

#endif
// - end of file ----------------------------------------------------------------------------------
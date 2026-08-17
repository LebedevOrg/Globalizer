/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      evolvent_factory.cpp                                        //
//                                                                         //
//  Purpose:   Source file for evolvent factory class                      //
//                                                                         //
//  Author(s): Zaitsev A.                                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "EvolventFactory.h"
#include "Evolvent.h"
#include "TLinearEvolvent.h"
#include "TNoninjectiveEvolvent.h"
#include "TRotatedEvolvent.h"
#include "TShiftedEvolvent.h"
#include "TSmoothEvolvent.h"



// ------------------------------------------------------------------------------------------------
IEvolvent* EvolventFactory::CreateEvolvent(int _N, int _m)
{
	IEvolvent* pEvolvent = nullptr;
	if (parameters.MapType == mpBase)
		pEvolvent = new Evolvent(_N, _m);
	if (parameters.MapType == mpLinar)
		pEvolvent = new LinearEvolvent(_N, _m);
	if (parameters.MapType == mpNoninjective)
		pEvolvent = new NoninjectiveEvolvent(_N, _m,1<<_N);
	if (parameters.MapType == mpRotated)
		pEvolvent = new RotatedEvolvent(_N, _m);
	if (parameters.MapType == mpShifted)
		pEvolvent = new ShiftedEvolvent(_N, _m);
	if (parameters.MapType == mpSmooth)
		pEvolvent = new SmoothEvolvent(_N, _m);


	return pEvolvent;
}
/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      Calculatin_factory.h                                            //
//                                                                         //
//  Purpose:   Header file for method factory class                        //
//                                                                         //
//  Author(s): Lebedev I.                                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __CALCULATION_FACTORY_H__
#define __CALCULATION_FACTORY_H__

#include "calculation.h"
#include "evolvent.h"


/// Класс необходим для правильного создания вычислителей
class TCalculationFactory
{
public:
  ///Стандартный метод создания вычислителя, используется в процессе, при повторном создании возвращает уже созданный
  static TCalculation* CreateCalculation(TTask& _pTask, TEvolvent* evolvent = 0);
  static TCalculation* CreateCalculation2(TTask& _pTask, TEvolvent* evolvent = 0);
  
  ///Всегда создает новый вычислитель, НЕ ИСПОЛЬЗОВАТЬ В МНОГОШАГОВОЙ СХЕМЕ!
  static TCalculation* CreateNewCalculation(TTask& _pTask, TEvolvent* evolvent = 0);
  
};

#endif
// - end of file ----------------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      CalculationFactory.h                                        //
//                                                                         //
//  Purpose:   Header file for method factory class                        //
//                                                                         //
//  Author(s): Lebedev I.                                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

/**
 * \file CalculationFactory.h
 *
 * \authors Лебедев И.
 * \date 2015
 * \copyright ННГУ им. Н.И. Лобачевского
 *
 * \brief Объявление фабрики для создания объектов-вычислителей
 */

#ifndef __CALCULATION_FACTORY_H__
#define __CALCULATION_FACTORY_H__

#include "Calculation.h"
#include "EvolventInterface.h"

 /**
  * \brief Класс-фабрика для создания объектов-вычислителей.
  *
  * Отвечает за создание конкретного экземпляра вычислителя
  * в зависимости от глобальных параметров и типа задачи.
  */
class CalculationFactory
{
public:
    /**
     * \brief Создает или возвращает существующий вычислитель.
     *
     * Использует singleton-логику для переиспользования объектов
     * #Calculation::firstCalculation и #Calculation::leafCalculation.
     * \param _pTask Задача, для которой создается вычислитель.
     * \param evolvent Указатель на эвольвенту (опционально).
     * \return Указатель на объект #Calculation.
     */
    static Calculation* CreateCalculation(Task& _pTask, IEvolvent* evolvent = 0);
    /**
     * \brief Создает или возвращает существующий вычислитель (альтернативная логика).
     *
     * \param _pTask Задача, для которой создается вычислитель.
     * \param evolvent Указатель на эвольвенту (опционально).
     * \return Указатель на объект #Calculation.
     */
    static Calculation* CreateCalculation2(Task& _pTask, IEvolvent* evolvent = 0);

    /**
     * \brief Всегда создает новый экземпляр вычислителя.
     *
     * ВНИМАНИЕ: Не использовать в многошаговой схеме.
     * \param _pTask Задача, для которой создается вычислитель.
     * \param evolvent Указатель на эвольвенту (опционально).
     * \return Указатель на новый объект #Calculation.
     */
    static Calculation* CreateNewCalculation(Task& _pTask, IEvolvent* evolvent = 0);

};

#endif
// - end of file ----------------------------------------------------------------------------------
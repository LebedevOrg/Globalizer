/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2026 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      HybridLocalCalculation.h                                    //
//                                                                         //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Calculation.h"
#include "Task.h"
#include "LocalMethod.h" 
#include "HybridLocalTask.h"
#include <vector>

/**
 * \brief Класс-вычислитель для гибридной схемы оптимизации.
 *
 * Наследует Calculation и отвечает за пакетную обработку точек.
 * Для каждой входящей точки создает экземпляр задачи-обертки и локального метода,
 * выполняет локальный поиск, затем возвращает улучшенные координаты и значения функций.
 */
class HybridLocalCalculation : public Calculation
{
private:
    Task* mOriginalTask;
    HybridLocalTask* mLocalTask;
    LocalMethod* mLocalMethod; 

public:
    /**
    * \brief Конструктор гибридного вычислителя.
    * \param originalTask Ссылка на исходную задачу оптимизации.
    */
    HybridLocalCalculation(Task& originalTask);
    virtual ~HybridLocalCalculation();

    /**
     * \brief Основной метод вычисления функционалов для пакета точек.
     * \param input Набор точек от глобального метода.
     * \param output Набор точек с вычисленными значениями (создается внутри метода).
     */
    virtual void Calculate(InformationForCalculation& input, TResultForCalculation& output) override;
};
/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      Calculation.h                                               //
//                                                                         //
//  Purpose:   Header file for calculation base class                      //
//                                                                         //
//  Author(s): Lebedev I.                                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


/**
 * \file Calculation.h
 *
 * \authors Лебедев И.
 * \date 2015
 * \copyright ННГУ им. Н.И. Лобачевского
 *
 * \brief Объявление абстрактного базового класса #Calculation
 *
 * \details Определяет общий интерфейс для всех классов-вычислителей,
 * использующих различные технологии (OpenMP, CUDA, MPI).
 */

#ifndef __CALCULATION_H__
#define __CALCULATION_H__

#include "Common.h"
#include "Parameters.h"
#include "Task.h"
#include "InformationForCalculation.h"
#include "Trial.h"
#include "SearchData.h"

 /**
  * \brief Абстрактный базовый класс для проведения испытаний.
  *
  * Определяет интерфейс "Стратегии" для выполнения вычислений.
  * Конкретные реализации (#OMPCalculation, #CUDACalculation, etc.)
  * предоставляют различные способы вычислений.
  */
class Calculation
{
    friend class CalculationTest;
protected:

    /// Указатель на решаемую задачу
    Task* pTask;

    /// Указатель на данные о процессе поиска
    SearchData* pData;

public:

  // Глобальные статические поля для управления сложными режимами вычислений

    /// Количество вычислений, которые нужно накопить перед запуском
    static int countCalculation;
    /// Флаг, указывающий, нужно ли запускать вычисления немедленно
    static bool isStartComputingAway;
    /// Глобальный буфер для накопления входных данных
    static InformationForCalculation inputCalculation;
    /// Глобальный буфер для получения результатов
    static TResultForCalculation resultCalculation;

    /// Указатель на единственный вычислитель для корневого процесса
    static Calculation* firstCalculation;
    /// Указатель на единственный вычислитель для листовых процессов
    static Calculation* leafCalculation;

    /**
     * \brief Конструктор.
     * \param _pTask Ссылка на объект задачи.
     */
    Calculation(Task& _pTask);

    /**
     * \brief Деструктор.
     */
    virtual ~Calculation() {}

    /**
     * \brief Устанавливает количество вычислений для накопления.
     * \param c Количество вычислений.
     */
    void SetCountCalculation(int c);

    /**
     * \brief Метод для продолжения асинхронных вычислений (если применимо).
     */
    virtual void ContinueComputing()
    {};

    /**
     * \brief Основной метод для выполнения вычислений (чисто виртуальный).
     * \param[in] inputSet Входные данные (точки для вычислений).
     * \param[out] outputSet Выходные данные (результаты вычислений).
     */
    virtual void Calculate(InformationForCalculation& inputSet,
        TResultForCalculation& outputSet) = 0;

    /**
     * \brief Устанавливает новый объект задачи для вычислителя.
     * \param _pTask Указатель на новый объект задачи.
     */
    void SetTask(Task* _pTask);

    /**
     * \brief Устанавливает данные поиска для вычислителя.
     * \param _pData Указатель на данные поиска.
     */
    void SetSearchData(SearchData* _pData);

    /**
     * \brief Сбрасывает внутреннее состояние вычислителя.
     */
    virtual void Reset();
};

#endif
// - end of file ----------------------------------------------------------------------------------
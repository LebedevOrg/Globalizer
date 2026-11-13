/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2026 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      HybridLocalTask.h                                           //
//                                                                         //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Task.h"
#include <vector>

/**
 * \brief Класс-обертка задачи для гибридного вычисления (АГП + локальный метод).
 *
 */
class HybridLocalTask : public Task
{
private:
    Task* mOriginalTask; ///< Указатель на исходную N-мерную задачу
    std::vector<int> mGlobalIndices; ///< Индексы переменных, запрещенных для локального поиска
    std::vector<double> mFixedValues; ///< Текущие значения глобальных переменных

public:
    /**
     * \brief Конструктор класса HybridLocalTask.
     * \param originalTask Указатель на исходную задачу оптимизации.
     * \param localIndices Индексы переменных, доступных для локальной оптимизации (0-based).
    */
    HybridLocalTask(Task* originalTask, const std::vector<int>& localIndices);
    virtual ~HybridLocalTask();

    /**
     * \brief Фиксирует значения глобальных переменных перед запуском локального поиска.
     * \param fullDimensionPoint Указатель на точку полной размерности N.
    */
    void SetFixedPoint(const double* fullDimensionPoint);

    int GetN() const override;
    const double* GetA() const override; ///< Возвращает указатель на массив нижних границ
    const double* GetB() const override; ///< Возвращает указатель на массив верхних границ
    double CalculateFuncs(const double* x_local, int fNumber) override; ///< Вычисляет функционалы с проверкой переменных
    int GetNumOfFunc() const override; ///< Возвращает количество функций задачи
    int GetNumberOfDiscreteVariable() override; ///< Возвращает количество дискретных переменных
    bool IsLeaf() override; ///< Возвращает флаг, является ли задача листовой
};
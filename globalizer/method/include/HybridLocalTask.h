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

    virtual int GetN();
    virtual const double* GetA(); ///< Возвращает указатель на массив нижних границ
    virtual const double* GetB(); ///< Возвращает указатель на массив верхних границ
    virtual double CalculateFuncs(const double* x, int fNumber); ///< Вычисляет функционалы с проверкой переменных
    virtual int GetNumOfFunc(); ///< Возвращает количество функций задачи
    virtual int GetNumberOfDiscreteVariable(); ///< Возвращает количество дискретных переменных
    virtual bool IsLeaf(); ///< Возвращает флаг, является ли задача листовой
};
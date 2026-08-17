/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      CudaCalculation.h                                           //
//                                                                         //
//  Purpose:   Header file for CUDA calculation class                      //
//                                                                         //
//  Author(s): Lebedev I.                                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

/**
 * \file CudaCalculation.h
 *
 * \authors Лебедев И.
 * \date 2015
 * \copyright ННГУ им. Н.И. Лобачевского
 *
 * \brief Объявление класса #CUDACalculation для вычислений с помощью CUDA
 */

#ifndef __CUDA_CALCULATION_H__
#define __CUDA_CALCULATION_H__

#include "Calculation.h"

 /**
  * \brief Реализация вычислителя с использованием технологии CUDA.
  *
  * Позволяет выполнять массовые параллельные вычисления на GPU от NVIDIA.
  */
class CUDACalculation : public Calculation
{
protected:
	// распределения вычислений по нескольким GPU

	/// Количество доступных GPU
	int DeviceCount;
	/// Размер данных для каждого GPU
	int* dataSize;
	/// Начальный индекс данных для каждого GPU
	int* dataStart;
	/// Индексы используемых GPU
	int* devicesIndex;

	/// Буфер для координат точек, передаваемых в CUDA-ядро
	double* coordinates;
	/// Размер буфера coordinates
	int coordinatesSize;
	/// Буфер для значений функций, получаемых из CUDA-ядра
	double* FuncValues;
	/// Размер буфера FuncValues
	int FuncValuesSize;

	/// Флаг, показывающий, была ли проведена инициализация
	bool mIsInitialized;

	/**
	 * \brief Внутренний метод, запускающий процесс вычислений.
	 * \param[in] inputSet Входные данные.
	 * \param[out] outputSet Выходные данные.
	 */
	void StartCalculate(InformationForCalculation& inputSet, TResultForCalculation& outputSet);

public:
	/**
	 * \brief Конструктор.
	 * \param _pTask Ссылка на объект задачи.
	 */
	CUDACalculation(Task& _pTask) : Calculation(_pTask)
	{
		mIsInitialized = false;
		coordinates = 0;
		FuncValues = 0;
		coordinatesSize = 0;
		FuncValuesSize = 0;
	}

	/**
	 * \brief Выполняет вычисления для набора испытаний с использованием CUDA.
	 * \param[in] inputSet Входные данные.
	 * \param[out] outputSet Выходные данные.
	 */
	virtual void Calculate(InformationForCalculation& inputSet, TResultForCalculation& outputSet);
};

#endif
// - end of file ----------------------------------------------------------------------------------
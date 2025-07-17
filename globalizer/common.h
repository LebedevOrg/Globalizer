/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      common.h                                                    //
//                                                                         //
//  Purpose:   Common Header file                                          //
//                                                                         //
//  Author(s): Sysoyev A., Barkalov K.                                     //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __COMMON_H__
#define __COMMON_H__

#include "defines.h"

/* ============================================================================================= *\
**  Constants                                                                                    **
\* ============================================================================================= */

//const int MaxPathLength = 512
const int MaxNumOfTaskLevels = 5;
const int MaxNumOfFunc = 20;
const int MaxDim = MAX_TRIAL_DIMENSION;
const int MaxNumOfGlobMinima = MAX_NUM_MIN;
const int MaxM = 20;
const int MaxL = 10;

const double MaxDouble = 1.7e308;
const double MinDouble = -1.7e308;
const double AccuracyDouble = 1.0e-8;

const int DefaultQueueSize = 32767;//1023;//65535;//32767;//16383;//16777215;//8388607;//1048575;//8388607;//524287; //262143; // должно быть равно 2^k - 1
const int DefaultSearchDataSize = 100000;

const int TagChildStartSolve = 101;
const int TagChildSolved = 102;
const int ChildStopMsg = -101;

/* ============================================================================================= *\
**  Types                                                                                        **
\* ============================================================================================= */
class TProcess;
#ifdef WIN32
typedef void(__cdecl* tIterationHandler)(TProcess* pProcess);
typedef OBJECTIV_TYPE(__cdecl* tFunction)(const OBJECTIV_TYPE*);
#else
typedef void(*tIterationHandler)(TProcess* pProcess);
typedef OBJECTIV_TYPE(*tFunction)(const OBJECTIV_TYPE*);
#endif


/**
Тип итерации метода

При работе смешанного алгоритм в зависимости от параметра смешивания итерации могут выполняться
либо с использованием глобальных характеристик интервала, либо - локальных.
*/
enum IterationType
{
  ///Итерация выполняется по правилам глобального поиска
  Global,
  ///Итерация выполняется по правилам локального поиска
  Local
};


enum EParameterType
{
  Pbool,
  Pint,
  Pdouble,
  Pstring,
  PETypeMethod,
  PETypeCalculation,
  PELocalMethodScheme,
  PESeparableMethodType,
  PEStopCondition,
  PETypeProcess,
  PETypeSolver,
  PEMapType,
  Pints,
  Pdoubles,
  Pflag
};

enum ETypeMethod
{
  StandartMethod,
  HybridMethod,
  ManyNumPointMethod,
  UniformSearchMethod,
  GlobalSearchMethod,
  MCO_Method,
  MultievolventsMethod,
  ParallelMultievolventsMethod,
  IntegerMethod,
  AdaptivMethod
};

enum ESeparableMethodType
{
  Off,
  GridSearch,
  GlobalMethod
};

enum ELocalMethodScheme
{
  /// Без локального метода
  None,
  /// Локальный метод в конце
  FinalStart,
  /// Локальный метод запускается при обновление лучшей точки
  UpdatedMinimum
};

enum EStopCondition
{
  Accuracy,
  OptimumVicinity,
  OptimumVicinity2,
  OptimumValue,
  AccuracyWithCheck,
  InLocalArea
};

enum ETypeCalculation
{
  /// Параллельный АГП - OpenMP вычислитель
  OMP,
  /// Параллельный АГП - CUDA вычислитель
  CUDA,
  /// Параллельный АГП - вычисления на Intel XeonPHI
  PHI,
  /// Последовательная многошаговая схема
  BlockScheme,
  /// Адаптивная схема, ТОЛЬКО ПРИ ETypeMethod = AdaptivMethod
  Adaptiv,
  /// Множественные развертки
  Multievolvents,
  /// Параллельная блочная многошаговая схема, MPI для нижнего уровня
  ParallelBlockScheme,
  /// Параллельный АГП - MPI Вычислитель
  MPICalculation,
  /// Параллельный АГП - MPI Вычислитель, ассинхронный
  AsyncMPI,
  /// Схема с использованием Аппроксимации
  ApproximationScheme,
  /// MPI Вычислитель - Intel oneAPI вычислитель
  OneApi
};

enum ETypeProcess
{
  /// Обычный процесс
  SynchronousProcess,
  /// Не поддерживается
  SynchronousProcessNew,
  /// Не поддерживается
  AsynchronousProcess
};

/// <summary>
/// Тип решателя, определяет алгоритм\способ решения задачи
/// </summary>
enum ETypeSolver
{
  /// Класический АГП
  SingleSearch,
  /// Последовательный спуск (не поддерживается в этой версии)
  SequentialSearch,
  /// Сепарабельная оптимизация (не поддерживается в этой версии)
  SeparableSearch,
  /// Автоматическое разделение переменных и запуск блочной многошаговой схемы
  SeparationVariables
};

///Тип развертки
enum EMapType
{
  ///Вращаемая развертка
  mpRotated,
  ///Сдвиговая развертка
  mpShifted,
  ///Базовый, одиночный вариант
  mpBase
};

///Тип распределения начальных точек
enum ETypeDistributionStartingPoints
{
  /// Равномерно
  Evenly,
  /// Адаптивно
  Adaptively
};

enum ETypeLocalMethod
{
  /// Метод Хука — Дживса
  HookeJeeves,
  /// Метод наименьших квадратов
  LeastSquareMethod,
  //Метод BFGS с ограниченным использованием памяти в многомерном кубе
  L_BFGS_B,
  /// Параллельный Метод Хука — Дживса
  ParallelHookeJeeves,
  //Параллельный L-BFGS-B
  Parallel_L_BFGS_B
};

enum ETypeStartLocalMethod
{
  /// 0 - найдены любые countPointInLocalMinimum точек образующих параболоид
  AnyPoints,
  /// 1 найдено countPointInLocalMinimum / 2 точек слева и справа
  EqualNumberOfPoints
};

enum ETypeAddLocalPoint
{
  /// 0 - как обычные точки (для запуска из подозрительного на локальный минимум испытания)
  RegularPoints,
  ///1 - точки локального метода не учитываются в критерии остановки по точности (для запуска из подозрительного на локальный минимум испытания)
  NotTakenIntoAccountInStoppingCriterion,
  /// 2 - Добавлять только одну точку (для полноценного запуска локального метода)
  IntegratedOnePoint,
  /// 3 - Добавлять все точки (для полноценного запуска локального метода)
  IntegratedAllPoint,
  /// 4 - Добавлять только точки из лучшей траектории (для полноценного запуска локального метода)
  IntegratedBestPath
};

///Способ выбора интервала хранящего локальный минимум
enum ETypeLocalMinInterval
{
  /// 0 - по точкам
  NPoints,
  ///1 - по дереву решения
  DecisionTrees,
  ///2 - Нахождение всех минимумов 
  AllMinimum
};

/// Тип локального уточнения: 0 - без него; 1 - минимаксное; 2 - адаптивное; 3 - адаптивно-минимаксное
enum ELocalTuningType
{
  ///0 - без него
  WithoutLocalTuning,
  ///1 - минимаксное
  MiniMax,
  ///2 - адаптивное
  Adaptive,
  ///3 - адаптивно-минимаксное
  AdaptiveMiniMax
};

#endif
// - end of file ----------------------------------------------------------------------------------

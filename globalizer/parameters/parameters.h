/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      parameters.h                                                //
//                                                                         //
//  Purpose:   Header file for parameters class                            //
//                                                                         //
//  Author(s): Lebedev I.                                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

/**
\file parameters.h

\authors Лебедев И.
\date 2015-2016
\copyright ННГУ им. Н.И. Лобачевского

\brief Объявление общих параметров

\details Объявление общих параметров, реализованны в виде класса #TParameters
*/


#ifndef __PARAMETERS_H__
#define __PARAMETERS_H__

#include "messages.h"
#include "common.h"

#include "baseParameters.h"

#include <string>
#include <stdio.h>

#include "parallel_tree.h"

//#define _NOT_PRINT_





class TParameters : public TBaseParameters<TParameters>
{
#undef OWNER_NAME
#define OWNER_NAME TParameters

protected:
  /// Определить параметры MPI
  void DetermineProc();
  /// Расставить номера на устройсва
  void SetDeviceIndex();

  /**
  Задание значений по умолчанию для всех параметров
  Пример:
  InitOption(имя параметра, значение по умолчанию, "короткая команда", "справка по параметру", кол-во элементов);
  *кол-во элементов для не массивов всегда равно 1.
  InitOption(Separator,_, "-Separator", "eparator", 1);
  */
  virtual void SetDefaultParameters();

  /// Индекс процесса
  int mProcRank;
  /// Кол-во процессов
  int mProcNum;
  /// Необходимое число процессов
  int mNeedMPIProcessorCount;

public:
  TParallelTree parallel_tree;
  /// Количество разверток обрабатывающие процессом владельцем
  int* MapCount;
  /// Уровень текущего процесса
  int MyLevel;
  /// Номер
  int MyMap;
  /// последний сохраненный номер итерации
  int iterationNumber;

  //Параметры командной строки
  TInt<TParameters> NumPoints; // число точек, порождаемых методом на 1 итерации
  TInt<TParameters> StepPrintMessages; //
  /// Через какое количество итераций сохранять точки
  TInt<TParameters> StepSavePoint; 
  TETypeMethod<TParameters> TypeMethod;
  TETypeCalculation<TParameters> TypeCalculation;
  TETypeProcess<TParameters> TypeProcess; //
  TInt<TParameters> NumThread;
  TInt<TParameters> SizeInBlock; //размер CUDA блока
  /// Печатать ли отчетет в файл
  TBool<TParameters> IsPrintFile;
  /// Файл для печати результата, если "000" то не печатаем
  TString<TParameters> ResulLog; 
  TInt<TParameters> Dimension; //размерность исходной задачи
  TDouble<TParameters> r; // надежность метода (> 1)
  TDoubles<TParameters> rs;
  ///Добавка при динамичеки изменяемом r, r = r + rDynamic / (Iteration ^ (1/N))
  TDouble<TParameters> rDynamic;
  TDouble<TParameters> rEps; //параметр eps-резервирования
  TDoubles<TParameters> Eps; //точность решения задачи на каждом уровне; //   размер - NumOfProcLevels
  TDouble<TParameters> Epsilon; //единая точность
  TString<TParameters> Comment; //Коментарий к эксперименту

  TDoubles<TParameters> M_constant;
  TInt<TParameters> m; // плотность построения развертки (точность 1/2^m по к-те)
  TInt<TParameters> deviceCount; //кол-во используемых ускорителей
  TEMapType<TParameters> MapType; // тип развертки (сдвиговая, вращаемая)
  TInt<TParameters> NumOfTaskLevels; // число уровней в дереве задач ; // максимальное число уровней в дереве - 5 совпадает с NumOfProcLevels
  TInts<TParameters> DimInTaskLevel; // число размерностей на каждом уровне дерева задач ; // размер - NumOfTaskLevels
  TInts<TParameters> ChildInProcLevel; // число потомков у процессов на уровнях с 0 до NumOfTaskLevels - 2; // размер - NumOfProcLevels - 1//уровень NumOfTaskLevels - 1 - процессы-листья
  TInt<TParameters> FullOrShort; // Полное вычисление в параллельной блочной многошаговой схеме или только подсчет значения функции
  TInt<TParameters> DebugAsyncCalculation; // Флаг для проверки работы асинхронной схемы, если не 0, то вычисления проводятся в строго заданном порядке

  /** число разверток (L + 1) на каждом уровне дерева процессов
  размер - NumOfProcLevels
  последний уровень по разверткам не параллелится
  * = L общее число разверток на уровне дерева
  */
  TInts<TParameters> MapInLevel;
  /**число процессов на каждом уровне дерева процессов, использующих множественную развертку
  размер - NumOfProcLevels
  последний уровень по разверткам не параллелится
  * число процессов обрабатывающие разные развертки на уровне (узле дерева распараллеливания)
  *если один корень то MapInLevel[0]*ProcNum=MapInLevel[0]
  *определяет число соседей
  */
  TInts<TParameters> MapProcInLevel;

  /// Печатать ли информацию о сечении в многошаговой схеме
  TBool<TParameters> IsPrintSectionPoint;

  TInts<TParameters> MaxNumOfPoints; // максимальное число итераций для процессов на каждом уровне //  размер - NumOfProcLevels{100, 100, 100, 100};// // параметры метода
  TFlag<TParameters> HELP;
  TFlag<TParameters> IsPlot;
  TInt<TParameters> PlotGridSize;
  /// Число испытаний за итерацию будет вычисляться на каждой итерации в методе CalculateNumPoint()
  TFlag<TParameters> IsCalculateNumPoint;
  TBool<TParameters> IsSetDevice; //Назначать каждому процессу свое устройство (ускоритель)
  ///Индекс используемого устройства (ускорителей), если -1 используется первые deviceCount устройств
  TInt<TParameters> deviceIndex;

  TInt<TParameters> ProcRank;

  TELocalMethodScheme<TParameters> localVerificationType; //cпособ использования локального метода(только для синхронного типа процесса)

  /// Количество итераций локального метода
  TInt<TParameters> localVerificationIteration;
  /// Точность локального метода
  TDouble<TParameters> localVerificationEpsilon; 
  /// Количество точек точек параллельно вычисляемых локальным методом
  TInt<TParameters> localVerificationNumPoint;

  TInt<TParameters> localMix; //параметр смешивания в локально-глобальном алгоритме
  TDouble<TParameters> localAlpha; //степень локальной адаптации в локально-глобальном алгоритме
  TInts<TParameters> calculationsArray; //Распределение типов вычислений по
  TESeparableMethodType<TParameters> sepS;  //флаг сепарабельного поиска на первой итерации
  TBool<TParameters> rndS;  //флаг случайного поиска на первой итерации
  TString<TParameters> libPath;  //путь к библиотеке с задачей
  TString<TParameters> libConfigPath; //путь к библиотеке с задачей
  /// тип критерия остановки
  TEStopCondition<TParameters> stopCondition; 
  /// Критерий применим только к верхнему уровню или к любому (для адаптивной схемы)
  TBool<TParameters> isStopByAnyLevel;
  TString<TParameters> iterPointsSavePath; //путь, по которому будут сохранены многомерные точки, поставленные методом корневого процесса
  TFlag<TParameters> printAdvancedInfo; //флаг, включающий печать дополнительной статистики: оценки констант Гёльдера и значения функций в точке оптимума
  TFlag<TParameters> disablePrintParameters; //флаг, выключающий печать параметров при запуске системы
  TString<TParameters> logFileNamePrefix; //префикс в имени лог-файла

  TETypeSolver<TParameters> TypeSolver;
  TInts<TParameters> DimInTask; // размерности каждой из подзадач в режиме сепарабильного или сикуенсального поиска

  /// Размер блока, отправляемого в MPI другим процессам
  TInt<TParameters> mpiBlockSize;
  /// Использовать ли специальное вычисление R как характеристики задачи
  TBool<TParameters> isUseTaskR;
  /// Использовать глобальный пересчет характеристик при изменение M или Z
  TBool<TParameters> isUseFullRecount;
  /// Использовать ли специальное вычисление R как характеристики интервалов
  TBool<TParameters> isUseIntervalR;
  /// Использовать ли глобальное Z
  TBool<TParameters> isUseGlobalZ;
  /// Не использовать Z
  TBool<TParameters> isNotUseZ;

  /// Использовать ли локальный метод при нахождение локального минимума
  TBool<TParameters> isUseLocalUpdate;
  /// Кол-во точек которые определяют локальный минимум
  TInt<TParameters> countPointInLocalMinimum;
  /// Тип локального метода (0 - Хука-Дживас, 1 - квадратичная апросксимация)
  TETypeLocalMethod<TParameters> TypeLocalMethod;
  /// Параметр включения локального уточнения (длина локальной подобласти это 1 / Localr от максимальной длины)
  TDouble<TParameters> Localr;
  /// Тип критерия старта локального метода (0 - найдены любые countPointInLocalMinimum точек образующих параболоид, 1 найдено countPointInLocalMinimum / 2 точек слева и справа)
  TETypeStartLocalMethod<TParameters> TypeStartLocalMethod;
  /// Тип добавления точек локального уточнения (0 - как обычные точки, 1 - точки локального метода не учитываются в критерии остановки по точности)
  TETypeAddLocalPoint<TParameters> TypeAddLocalPoint;
  /// Максимальное Кол-во точек устанавлиемых локальным методом
  TInt<TParameters> maxCountLocalPoint;
  /// Тип взятия точек: 0 - берем сразу np точек, 1 - берем с учетом окрестности локальных минимумов.
  TInt<TParameters> PointTakingType;
  /// Тип изменения статуса интервала: 0 - не изменять у потомков, 1 - изменять.
  TInt<TParameters> StatusIntervalChangeType;
  /// Вычислять ли значения функции в крайних точках интервала
  TBool<TParameters> isCalculationInBorderPoint;
  /// Тип локального уточнения: 0 - без него; 1 - минимаксное; 2 - адаптивное; 3 - адаптивно-минимаксное
  TELocalTuningType<TParameters> LocalTuningType;
  /// Параметр кси, используемый в локальном уточении
  TDouble<TParameters> ltXi;
  /// Способ выбора интервала хранящего локальный минимум NPoints - по 5 точкам DecisionTrees по дереву решения
  TETypeLocalMinInterval<TParameters> TypeLocalMinIntervale;
  /// Максимальная глубина обучения в дереве решения
  TInt<TParameters> DecisionTreesMaxDepth;
  /// Точность построения дерева решения
  TDouble<TParameters> DecisionTreesRegressionAccuracy;

  /// Число точек до запуска апроксимации
  TInt<TParameters> NumPointsForApproximation;

  //MCO
  ///параметры свертки
  TDoubles<TParameters> Lamda;
  TInt<TParameters> rLamda; //параметры свертки
  TInt<TParameters> numberOfLamda; //количество коэффициентов свертки
  TBool<TParameters> isCriteriaScaling; // нужно ли масштабирование значений критериев при свертке
  TInt<TParameters> itrEps; //число итераций до попадания в eps-окрестность
  /// Количество критериев в общей постановке задачи MCO
  TInt<TParameters> MCO_N_Criteria;
  /// Количество ограничений в общей постановке задачи MCO
  TInt<TParameters> MCO_N_Constraint;
  /// Выборка критериев в общей постановке задачи MCO
  TInts<TParameters> MCO_Criteria_perm;
  /// Выборка ограничений в общей постановке задачи MCO
  TInts<TParameters> MCO_Constraint_perm;
  /// Допуски для ограничений в общей постановке задачи MCO
  TDoubles<TParameters> MCO_q;

  /// Загружать начальныеточки из файла или распределять их равномерно
  TBool<TParameters> isLoadFirstPointFromFile;
  /// Путь откуда будут считаны начальные точки испытания
  TString<TParameters> FirstPointFilePath;
  /// Тип распределения начальных точек
  TETypeDistributionStartingPoints<TParameters> TypeDistributionStartingPoints;

  /// Множитель перед функцие определяющий минимизируем или максимизируем функцию
  TDoubles<TParameters> functionSignMultiplier;

  /// Проверка правильности при изменение параметров
  virtual int CheckValueParameters(int index = 0);
  /// Возвращает номер текущего процесса
  int GetProcRank() const;
  /// Возвращает общее число процессов
  int GetProcNum();
  /// Возвращает имя файла для сохранения картинки построенных линий уровней
  std::string GetPlotFileName();
  /// Печать текущих значений параметров
  void PrintParameters();

  ///Печать текущих значений параметров в файл
  void PrintParametersToFile(FILE* pf);

  /// Инициализация параметров
  virtual void Init(int argc, char* argv[], bool isMPIInit = false);
  TParameters();
  TParameters(TParameters& _parameters);
  virtual ~TParameters();

  /// Является ли класс задачей
  virtual bool IsProblem();

};

extern TParameters parameters;

#endif
// - end of file ----------------------------------------------------------------------------------

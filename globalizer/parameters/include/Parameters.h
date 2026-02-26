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

\details Объявление общих параметров, реализованны в виде класса #Parameters
*/


#ifndef __PARAMETERS_H__
#define __PARAMETERS_H__

#include "Messages.h"
#include "Common.h"

#include "BaseParameters.h"

#include <string>
#include <stdio.h>

class SearchDataSerializer;


/// Параметры системы оптимизации
class Parameters : public BaseParameters<Parameters>
{
#undef OWNER_NAME
#define OWNER_NAME Parameters

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
  /// Уровень текущего процесса
  int MyLevel;
  /// Номер
  int MyMap;
  /// последний сохраненный номер итерации
  int iterationNumber;
  
  /// Суммарное время решения
  double timeSolve;

  //Параметры командной строки

  /// число точек, порождаемых методом на 1 итерации
  TInt<Parameters> NumPoints;
  /// Шаг печать информации в консоль
  TInt<Parameters> StepPrintMessages;
  /// Через какое количество итераций сохранять точки
  TInt<Parameters> StepSavePoint;
  /// Тип метода АГП
  TETypeMethod<Parameters> TypeMethod;
  /// Организация проведения испытаний
  TETypeCalculation<Parameters> TypeCalculation;
  /// Тип процесса
  TETypeProcess<Parameters> TypeProcess;
  /// Число параллельных потоков\процессов задействованых в проведении испытаний
  TInt<Parameters> NumThread;
  ///размер CUDA блока
  TInt<Parameters> SizeInBlock;
  /// Печатать ли отчетет в файл
  TBool<Parameters> IsPrintFile;
  /// Файл для печати результата, если "000" то не печатаем
  TString<Parameters> ResulLog;
  ///размерность исходной задачи
  TInt<Parameters> Dimension;
  /// надежность метода (> 1)
  TDouble<Parameters> r;
  ///Добавка при динамичеки изменяемом r, r = r + rDynamic / (Iteration ^ (1/N))
  TDouble<Parameters> rDynamic;
  ///параметр eps-резервирования
  TDouble<Parameters> rEps;
  ///единая точность
  TDouble<Parameters> Epsilon;
  ///Коментарий к эксперименту
  TString<Parameters> Comment;

  TDoubles<Parameters> M_constant;
  /// плотность построения развертки (точность 1/2^m по к-те)
  TInt<Parameters> m;
  ///кол-во используемых ускорителей
  TInt<Parameters> deviceCount;
  /// тип развертки (сдвиговая, вращаемая)
  TEMapType<Parameters> MapType;
  /// Флаг для проверки работы асинхронной схемы, если не 0, то вычисления проводятся в строго заданном порядке
  TInt<Parameters> DebugAsyncCalculation;

  /// Печатать ли информацию о сечении в многошаговой схеме
  TBool<Parameters> IsPrintSectionPoint;

  /// максимальное число итераций для процессов на каждом уровне //  размер - NumOfProcLevels{100, 100, 100, 100};// // параметры метода
  TInt<Parameters> MaxNumOfPoints;
  /// Распечатать справку
  TFlag<Parameters> HELP;
  /// Имя файла для сохранения изображения
  TString<Parameters> PlotFileName;
  /// Нарисовать график функции
  TFlag<Parameters> IsPlot;
  /// Плотность линий уровней
  TInt<Parameters> PlotGridSize;
  /** тип визуализации целевой функции
  доступные режимы : 
  0 - LevelLayers - линии уровней
  1 - Surface - поверхность
  */
  TInt<Parameters> FigureType;
  /** тип вычислений значений для визуализации целевой функции   
  доступные режимы: 
  0 - ObjectiveFunction - строит линии уровня / поверхность по сетке 100 * 100,
  1 - Approximation - строит аппроксимацию линий уровня / поверхности по имеющейся поисковой информации,
  2 - Interpolation - строит интерполяцию линий уровня / поверхности по имеющейся поисковой информации,
  3 - ByPoints - строит поверхность путем "натягивагия" ее на точки поисковой информации без сглаживания,
  4 - OnlyPoints - отображает только распределение точек поисковой информации в области поиска.
  */
  TInt<Parameters> CalcsType;
  /// флаг о необходимости открыть полученный рисунок в интерактивном окне на экране
  TBool<Parameters> ShowFigure;
  /// Число испытаний за итерацию будет вычисляться на каждой итерации в методе CalculateNumPoint()
  TFlag<Parameters> IsCalculateNumPoint;
  ///Назначать каждому процессу свое устройство (ускоритель)
  TBool<Parameters> IsSetDevice;
  ///Индекс используемого устройства (ускорителей), если -1 используется первые deviceCount устройств
  TInt<Parameters> deviceIndex;

  TInt<Parameters> ProcRank;
  ///cпособ использования локального метода(только для синхронного типа процесса)
  TELocalMethodScheme<Parameters> localRefineSolution;

  /// Количество итераций локального метода
  TInt<Parameters> localIteration;
  /// Точность локального метода
  TDouble<Parameters> localVerificationEpsilon;
  /// Количество точек точек параллельно вычисляемых локальным методом
  TInt<Parameters> localVerificationNumPoint;

  /// Количество итераций решателя задач большой размерности
  TInt<Parameters> HDSolverIterationCount;

  ///параметр смешивания в локально-глобальном алгоритме
  TInt<Parameters> localMix;
  ///степень локальной адаптации в локально-глобальном алгоритме
  TDouble<Parameters> localAlpha;
  ///Распределение типов вычислений по
  TInts<Parameters> calculationsArray;
  ///флаг сепарабельного поиска на первой итерации
  TESeparableMethodType<Parameters> sepS;
  ///флаг случайного поиска на первой итерации
  TBool<Parameters> rndS;
  ///путь к библиотеке с задачей
  TString<Parameters> libPath;
  ///путь конфигарационному файлу задачи
  TString<Parameters> libConfigPath;
  /// тип критерия остановки
  TEStopCondition<Parameters> stopCondition;
  /// Критерий применим только к верхнему уровню или к любому (для адаптивной схемы)
  TBool<Parameters> isStopByAnyLevel;
  /// Печатать ли результаты работы алгоритма в консоль
  TBool<Parameters> isPrintResultToConsole;
  ///путь, по которому будут сохранены многомерные точки, поставленные методом корневого процесса
  TString<Parameters> iterPointsSavePath;
  ///флаг, включающий печать дополнительной статистики: оценки констант Гёльдера и значения функций в точке оптимума
  TFlag<Parameters> printAdvancedInfo;
  ///флаг, выключающий печать параметров при запуске системы
  TFlag<Parameters> disablePrintParameters;
  ///префикс в имени лог-файла
  TString<Parameters> logFileNamePrefix;

  TETypeSolver<Parameters> TypeSolver;
  /// размерности каждой из подзадач в режиме сепарабильного или сикуенсального поиска
  TInts<Parameters> DimInTask;

  /// Размер блока, отправляемого в MPI другим процессам
  TInt<Parameters> mpiBlockSize;
  /// Использовать ли специальное вычисление R как характеристики задачи
  TBool<Parameters> isUseTaskR;
  /// Использовать глобальный пересчет характеристик при изменение M или Z
  TBool<Parameters> isUseFullRecount;
  /// Использовать ли специальное вычисление R как характеристики интервалов
  TBool<Parameters> isUseIntervalR;
  /// Использовать ли глобальное Z
  TBool<Parameters> isUseGlobalZ;
  /// Не использовать Z
  TBool<Parameters> isNotUseZ;

  /// Тип локального метода (0 - Хука-Дживас)
  TETypeLocalMethod<Parameters> TypeLocalMethod;

  /// Тип добавления точек локального уточнения (0 - как обычные точки, 1 - точки локального метода не учитываются в критерии остановки по точности)
  TETypeAddLocalPoint<Parameters> TypeAddLocalPoint;
  /// Максимальное Кол-во точек устанавлиемых локальным методом
  TInt<Parameters> maxCountLocalPoint;
  /// Вычислять ли значения функции в крайних точках интервала
  TBool<Parameters> isCalculationInBorderPoint;
  /// Тип локального уточнения: 0 - без него; 1 - минимаксное; 2 - адаптивное; 3 - адаптивно-минимаксное
  TELocalTuningType<Parameters> LocalTuningType;
  /// Параметр кси, используемый в локальном уточении
  TDouble<Parameters> ltXi;

  /// Загружать начальныеточки из файла или распределять их равномерно
  TBool<Parameters> isLoadFirstPointFromFile;
  /// Путь откуда будут считаны начальные точки испытания
  TString<Parameters> FirstPointFilePath;
  /// Тип распределения начальных точек
  TETypeDistributionStartingPoints<Parameters> TypeDistributionStartingPoints;

  /// Множитель перед функцие определяющий минимизируем или максимизируем функцию
  TDoubles<Parameters> functionSignMultiplier;

  /// Начальная точка для решения задачи оптимизации
  TDoubles<Parameters> startPoint;

  /// Значения функций в начальная точка для решения задачи оптимизации
  TDoubles<Parameters> startPointValues;

  /// Использовать  стартовую точку из задачи 
  TBool<Parameters> IsUseStartPoint;

  /// Использовать  расширенный консольный интерфейс 
  TBool<Parameters> IsUseExtendedConsole;

  /// Включить автоматическую настройку параметров алгоритма оптимизации, если выключено - используются значения по умолчанию
  TBool<Parameters> automaticParametersSetting;

  /// Путь для сохранения и загрузки
  TString<Parameters> fileSerializer;

  /// Сохранение в файл
  SearchDataSerializer* serializer;

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
  Parameters();
  Parameters(Parameters& _parameters);
  virtual ~Parameters();

  /// Является ли класс задачей
  virtual bool IsProblem();

  /// Возвращает число потоков рекомендуемое OMP
  virtual int GetMaxNumOMP();

};

extern Parameters parameters;

#endif
// - end of file ----------------------------------------------------------------------------------

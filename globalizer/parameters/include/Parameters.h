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

  /// Текущая версия
  std::string version = "0.1.7";

  /// Номер параметра с которого начинается оптимизация
  int startParameterNumber = 0;

  //Параметры командной строки


  // -----------------------------------------------------------------------
  // Параметры метода оптимизации
  // -----------------------------------------------------------------------

  /// надежность метода (> 1)
  TDouble<Parameters> r;
  /// единая точность
  TDouble<Parameters> Epsilon;
  /// качество исследования невычислимых областей задачи (0 < alpha <= 1)
  TDouble<Parameters> alpha;
  /// Добавка при динамически изменяемом r, r = r + rDynamic / (Iteration ^ (1/N))
  TDouble<Parameters> rDynamic;
  /// параметр eps-резервирования
  TDouble<Parameters> rEps;
  /// максимальное число испытаний
  TInt<Parameters> MaxNumOfPoints;
  /// число точек, порождаемых методом на 1 итерации
  TInt<Parameters> NumPoints;
  /// Тип метода АГП
  TETypeMethod<Parameters> TypeMethod;
  /// плотность построения развертки (точность 1/2^m по к-те)
  TInt<Parameters> m;
  /// тип развертки (сдвиговая, вращаемая)
  TEMapType<Parameters> MapType;
  /// Начальные оценки константы M для каждой функции
  TDoubles<Parameters> M_constant;
  /// Тип решателя
  TETypeSolver<Parameters> TypeSolver;


  // -----------------------------------------------------------------------
  // Параметры размерности и задачи
  // -----------------------------------------------------------------------

  /// размерность исходной задачи
  TInt<Parameters> Dimension;
  /// размерности каждой из подзадач в режиме сепарабельного или секвенциального поиска
  TInts<Parameters> DimInTask;
  /// Множитель перед функцией, определяющий минимизируем или максимизируем функцию
  TDoubles<Parameters> FunctionSignMultiplier;


  // -----------------------------------------------------------------------
  // Параметры параллельных вычислений
  // -----------------------------------------------------------------------

  /// Организация проведения испытаний
  TETypeCalculation<Parameters> TypeCalculation;
  /// Тип процесса
  TETypeProcess<Parameters> TypeProcess;
  /// Число параллельных потоков/процессов, задействованных в проведении испытаний
  TInt<Parameters> NumThread;
  /// размер CUDA блока
  TInt<Parameters> SizeInBlock;
  /// количество используемых ускорителей
  TInt<Parameters> DeviceCount;
  /// Назначать каждому процессу своё устройство (ускоритель)
  TBool<Parameters> IsSetDevice;
  /// Индекс используемого устройства (ускорителя), если -1 — используются первые DeviceCount устройств
  TInt<Parameters> DeviceIndex;
  /// Размер блока, отправляемого в MPI другим процессам
  TInt<Parameters> MpiBlockSize;
  /// Распределение типов вычислений по процессам
  TInts<Parameters> CalculationsArray;
  /// Флаг для проверки работы асинхронной схемы
  TInt<Parameters> DebugAsyncCalculation;
  /// Номер процесса
  TInt<Parameters> ProcRank;
  /// Число испытаний за итерацию будет вычисляться на каждой итерации в методе CalculateNumPoint()
  TFlag<Parameters> IsCalculateNumPoint;


  // -----------------------------------------------------------------------
  // Параметры локального уточнения
  // -----------------------------------------------------------------------

  /// Способ использования локального метода (только для синхронного типа процесса)
  TELocalMethodScheme<Parameters> LocalRefineSolution;
  /// Тип локального метода (0 - Хука-Дживас)
  TETypeLocalMethod<Parameters> TypeLocalMethod;
  /// Количество итераций локального метода
  TInt<Parameters> LocalIteration;
  /// Точность локального метода
  TDouble<Parameters> LocalVerificationEpsilon;
  /// Количество точек, параллельно вычисляемых локальным методом
  TInt<Parameters> LocalVerificationNumPoint;
  /// параметр смешивания в локально-глобальном алгоритме
  TInt<Parameters> LocalMix;
  /// степень локальной адаптации в локально-глобальном алгоритме
  TDouble<Parameters> LocalAlpha;
  /// Тип добавления точек локального уточнения
  TETypeAddLocalPoint<Parameters> TypeAddLocalPoint;
  /// Максимальное количество точек, устанавливаемых локальным методом
  TInt<Parameters> MaxCountLocalPoint;
  /// Тип локального уточнения: 0 - без него; 1 - минимаксное; 2 - адаптивное; 3 - адаптивно-минимаксное
  TELocalTuningType<Parameters> LocalTuningType;
  /// Параметр кси, используемый в локальном уточнении
  TDouble<Parameters> LtXi;
  /// Вычислять ли значения функции в крайних точках интервала
  TBool<Parameters> IsCalculationInBorderPoint;
  /// Количество итераций решателя задач большой размерности
  TInt<Parameters> HDSolverIterationCount;


  // -----------------------------------------------------------------------
  // Параметры критерия остановки
  // -----------------------------------------------------------------------

  /// Тип критерия остановки
  TEStopCondition<Parameters> StopCondition;
  /// Максимальное количество итераций без улучшения
  TInt<Parameters> MaxIterationsWithoutImprovement;
  /// Максимальное количество итераций алгоритма оптимизации, используется в автоматическом режиме
  TInt<Parameters> IterationsCount;


  // -----------------------------------------------------------------------
  // Параметры начальной точки и начального поиска
  // -----------------------------------------------------------------------

  /// Начальная точка для решения задачи оптимизации
  TDoubles<Parameters> StartPoint;
  /// Значения функций в начальной точке для решения задачи оптимизации
  TDoubles<Parameters> StartPointValues;
  /// Использовать стартовую точку из задачи
  TBool<Parameters> IsUseStartPoint;
  /// Загружать начальные точки из файла или распределять их равномерно
  TBool<Parameters> IsLoadFirstPointFromFile;
  /// Путь откуда будут считаны начальные точки испытания
  TString<Parameters> FirstPointFilePath;
  /// Тип распределения начальных точек
  TETypeDistributionStartingPoints<Parameters> TypeDistributionStartingPoints;
  /// флаг сепарабельного поиска на первой итерации
  TESeparableMethodType<Parameters> SepS;
  /// флаг случайного поиска на первой итерации
  TBool<Parameters> RndS;


  // -----------------------------------------------------------------------
  // Параметры задачи (библиотека)
  // -----------------------------------------------------------------------

  /// путь к библиотеке с задачей
  TString<Parameters> LibPath;
  /// путь к конфигурационному файлу задачи
  TString<Parameters> LibConfigPath;


  // -----------------------------------------------------------------------
  // Параметры сериализации и сохранения данных
  // -----------------------------------------------------------------------

  /// Имя файла для сохранения и загрузки в формате json
  TString<Parameters> FileSerializer;
  /// Включить сохранение в json для dashboard
  TBool<Parameters> IsSerializeToDashBoard;
  /// путь, по которому будут сохранены многомерные точки, поставленные методом корневого процесса
  TString<Parameters> IterPointsSavePath;
  /// Через какое количество итераций сохранять точки
  TInt<Parameters> StepSavePoint;
  /// Сохранение в файл
  SearchDataSerializer* serializer;


  // -----------------------------------------------------------------------
  // Параметры вывода и логирования
  // -----------------------------------------------------------------------

  /// Распечатать справку
  TFlag<Parameters> HELP;
  /// Печатать ли отчёт в файл
  TBool<Parameters> IsPrintFile;
  /// Файл для печати результата, если "000" — не печатаем
  TString<Parameters> ResulLog;
  /// Комментарий к эксперименту
  TString<Parameters> Comment;
  /// Шаг печати информации в консоль
  TInt<Parameters> StepPrintMessages;
  /// Печатать ли результаты работы алгоритма в консоль
  TBool<Parameters> IsPrintResultToConsole;
  /// флаг, включающий печать дополнительной статистики
  TFlag<Parameters> PrintAdvancedInfo;
  /// флаг, выключающий печать параметров при запуске системы
  TFlag<Parameters> DisablePrintParameters;
  /// префикс в имени лог-файла
  TString<Parameters> LogFileNamePrefix;
  /// Печатать ли информацию о сечении в многошаговой схеме
  TBool<Parameters> IsPrintSectionPoint;


  // -----------------------------------------------------------------------
  // Параметры визуализации
  // -----------------------------------------------------------------------

  /// Нарисовать график функции
  TFlag<Parameters> IsPlot;
  /// Имя файла для сохранения изображения
  TString<Parameters> PlotFileName;
  /// Плотность линий уровней
  TInt<Parameters> PlotGridSize;
  /** тип визуализации целевой функции
  доступные режимы:
  0 - LevelLayers - линии уровней
  1 - Surface - поверхность
  */
  TEFigureTypes<Parameters> FigureType;
  /** тип вычислений значений для визуализации целевой функции
  доступные режимы:
  0 - ObjectiveFunction
  1 - Approximation
  2 - Interpolation
  3 - ByPoints
  4 - OnlyPoints
  */
  TECalcsTypes<Parameters> CalcsType;
  /** тип вычислений значений для визуализации ограничений */
  TECalcsTypes<Parameters> CalcsTypeC;
  /// Количество линий уровня визуализации целевой функции
  TInt<Parameters> Levels;
  /// Размерность сетки для визуализации целевой функции
  TInt<Parameters> ObjectiveGridSize;
  /// Размерность сетки для визуализации ограничений
  TInt<Parameters> ConstraintsGridSize;
  /// флаг о необходимости выделить цветом допустимую область
  TBool<Parameters> FillFeasibleRegion;
  /// флаг о необходимости скрыть точки испытаний, не принадлежащие допустимой области
  TBool<Parameters> HideNoFeasiblePoints;
  /// флаг о необходимости скрыть точки испытаний при построении графика
  TBool<Parameters> HideTrialsPoints;
  /// флаг о необходимости сместить точки испытаний под график
  TBool<Parameters> MoveTrialPointsUnderGraph;
  /// флаг о необходимости открыть полученный рисунок в интерактивном окне на экране
  TBool<Parameters> ShowFigure;


  // -----------------------------------------------------------------------
  // Прочие параметры
  // -----------------------------------------------------------------------

  /// Использовать расширенный консольный интерфейс
  TBool<Parameters> IsUseExtendedConsole;
  /// Включить автоматическую настройку параметров алгоритма оптимизации
  TBool<Parameters> AutomaticParametersSetting;




  /// Проверка правильности при изменение параметров
  virtual int CheckValueParameters(int index = 0);
  /// Возвращает номер текущего процесса
  int GetProcRank() const;
  /// Возвращает общее число процессов
  int GetProcNum();
  /// Возвращает имя файла для сохранения картинки построенных линий уровней
  std::string GetPlotFileName();

  /// Возвращает имя файла json для построения DashBoard
  std::string GetJsonFileName();
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

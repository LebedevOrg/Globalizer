/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      method.h                                                    //
//                                                                         //
//  Purpose:   Header file for method class                                //
//                                                                         //
//  Author(s): Barkalov K., Sysoyev A.                                     //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


/**
\file method.h

\authors Баркалов К., Сысоев А.
\date 2015-2016
\copyright ННГУ им. Н.И. Лобачевского

\brief Объявление класса #Method

\details Объявление класса #Method и сопутствующих типов данных
*/


#ifndef __METHOD_H__
#define __METHOD_H__

#include "MethodInterface.h"
#include "Calculation.h"
#include "InformationForCalculation.h"
#include "SearchIteration.h"
#include "SearchInterval.h"

#include "SearchDataSerializer.h"

// ------------------------------------------------------------------------------------------------


/**
Базовый класс, реализующий алгоритм глобального поиска.

В классе #Method реализованы основные функции, определяющие работу алгоритма глобального поиска.
*/
class Method : public IMethod
{
protected:
  // ----------------------------------------------------------------------------
  // Копия параметров для конкретного уровня дерева
  // ----------------------------------------------------------------------------
  /// Максимальное число испытаний
  int MaxNumOfTrials;
  /// число итераций до включения смешанного алгоритма
  int StartLocalIteration;


  /// Обновлено глобальное М
  bool isGlobalMUpdate;
  /// Обновлена лучшая точка в текущей задаче
  bool isLocalZUpdate;

  // ----------------------------------------------------------------------------
  // Ссылки на объекты используемых методом
  // ----------------------------------------------------------------------------
  /// Указатель на решаемую задачу
  Task& pTask;
  /// Указатель на матрицу состояния поиска
  SearchData* pData;

  /// Вычислитель
  Calculation& calculation;
  /** Указатель на развертку

  В зависимости от вида отображения это может быть:
  - единственная развертка
  - множественная сдвиговая развертка
  - множественная вращаемая развертка
  */
  Evolvent& evolvent;

  // ----------------------------------------------------------------------------
  // Внутренние данные метода
  // ----------------------------------------------------------------------------

  /// Входные данные для вычислителя, формирубтся в CalculateFunctionals()
  InformationForCalculation inputSet;
  /// Выходные данные вычислителя, обрабатывается в CalculateFunctionals()
  TResultForCalculation outputSet;
  /// информация о данных текущей итерации
  SearchIteration iteration;
  /// Была получена точка в окрестности глобального оптимума
  bool isFoundOptimalPoint;

  /// достигнутая точность
  double            AchievedAccuracy;
  /** Коэффициент локальной адаптации

  Диапазон значений параметра alfa от 1 (глобальный) до 20 (локальный) поиск
  Рекомендуемое значение alfa = 15.
  */
  double alfa;

  /// Число вычисленных значений каждой функции
  std::vector<int> functionCalculationCount;

  /// нужно ли искать интервал
  bool isFindInterval;

  ///Новая точка устанавливается в интервал принадлежащий окрестности локального минимума
  bool isSetInLocalMinimumInterval;

  /// количество точек вычисленных локальным методом
  int localPointCount;
  /// число запусков локально метода
  int numberLocalMethodtStart;
  /// Нужно останавливаться
  bool isStop;

  /// найденные локальные минимумы
  std::vector<Trial*> localMinimumPoints;

  //=====================================================================================================================================================
  //Для методов локального уточнения нужны миксимумы

  /// Максимальные длины интервалов для разных индексов правой точки
  double* Xmax;
  /// Значения оценки константы Липшица для разных индексов правой точки
  double* mu;
  /// Инициализирован ли Xmax
  bool isSearchXMax;
  //=====================================================================================================================================================

  /// Массив для сохранения точек для последующей печати и рисования
  std::vector<Trial*> printPoints;

  /// Количество сохраненных точек
  int lastSavedTrialsCount = 0;

  /// Метод сохраняющий точки в статический массив
  virtual void  SavePoints();

  /** Вычисление "глобальной" характеристики

  \param[in] p указатель на интервал, характеристику которого надо вычислить
  \return "Глобальная" характеристика интервала
  */
  virtual double CalculateGlobalR(SearchInterval* p);
  /** Вычисление "локальной" характеристики

  Данная функция должна вызываться только для интервала, у которого вычислена глобальная
  характеристика, т.е. после вызова функции #CalculateGlobalR
  \param[in] p указатель на интервал, характеристику которого надо вычислить
  \return "Локальная" характеристика интервала
  */
  virtual double CalculateLocalR(SearchInterval* p);
  /** Вычисление оценки константы Липшица

  Обновленная оценка константы Липшица записывается в базе алгоритма
  \param[in] p указатель на интервал
  */
  virtual void CalculateM(SearchInterval* p);
  /** Определение типа текущей итерации: локальная или глобальная

  \param[in] iterationNumber номер итерации
  \param[in] localMixParameter параметр смешивания локального и глобального алгоритмов.
  Возможны три варианта:
  - localMixParameter == 0 - работает только глобальный алгоритм
  - localMixParameter > 0 - выполняется localMixParameter глобальных итераций, затем - одна локальная
  - localMixParameter < 0 - выполняется localMixParameter локальных итераций, затем - одна глобальная
  \return тип итерации
  */
  virtual IterationType GetIterationType(int iterationNumber, int localMixParameter);
  /** Определение, является интервал граничным или нет
  0 - Не граничный; 1 - Левая граница; 2 - Правая граница
  */
  virtual int IsBoundary(SearchInterval* p);

  /** Обновление константы Липшица для функции с заданным индексом

  Если константа обновлена, поднимает флаг #recalc. Данная функция используется в
  функции #CalculateM
  \param[in] newValue новое значение константы Липшица
  \param[in] index индекс функции
  \param[in] boundaryStatus является ли интервал граничным
  \param[in] p рассматриваемый интервал
  */
  virtual void UpdateM(double newValue, int index, int boundaryStatus, SearchInterval* p);

  /** Обновление текущей оценки оптимума

  Если переданная точка лучше текущей оценки оптимума, то эта оценка обновляется и поднимается
  флаг #recalc.
  \param[in] trial точка, которую необходимо сравнить с текущим оптимумом
  \return true, если оптимум обновлён, иначе false
  */
  virtual bool UpdateOptimumEstimation(Trial& trial);

  /// Вычисление координат точек испытания для основной\единственной развертки
  virtual void CalculateCurrentPoint(Trial& pCurTrialsj, SearchInterval* BestIntervalsj);

  /// Вычисляем координаты точек которые будем использовать на текущей итерации
  virtual void CalculateCurrentPoints(std::vector<SearchInterval*>& BestIntervals);

  /// Пренадлежит ли newInterval отрезку в котором находится basicInterval
  virtual bool IsIntervalInSegment(SearchInterval* basicInterval, SearchInterval* newInterval);


  /**
  Изменение при динамичеки изменяемом r, r = r + rDynamic / (Iteration ^ (1/N))
  */
  virtual double Update_r(int iter = -1, int procLevel = -1);

  /// x--> y; Вычисляет координаты y в гиперкубе по x из отрезка
  virtual void CalculateImage(Trial& pCurTrialsj);



  /// Добавление основных (из основной\единственной развертки) точек испытания в базу
  virtual SearchInterval* AddCurrentPoint(Trial& pCurTrialsj, SearchInterval* BestIntervalsj);

  /// Перерасчет характеристик и перестройка очереди
  virtual void Recalc();


  /// Получаем поисковую информацию, важно для адаптивного метода
  virtual SearchData* GetSearchData(Trial* trial);


  /**Изменить количество текущих точек испытаний, переписывает #iteration.pCurTrials и
#iteration.BestIntervals
*/
  virtual void SetNumPoints(int newNP);


public:

  Method(Task& _pTask, SearchData& _pData,
    Calculation& _Calculation, Evolvent& _Evolvent);
  virtual ~Method();

  /** Функция выполняет первую итерацию метода
  */
  virtual void FirstIteration();

  /** Вычисления точек очередной итерации

  Вычисленные точки очередной итерации записываются в массив #iteration.pCurTrials
  */
  virtual void CalculateIterationPoints();

  /** Вычисление функций задачи

  Проводятся испытания в точках из массива #iteration.pCurTrials, результаты проведенных испытаний
  записываются в тот же массив
  */
  virtual void CalculateFunctionals();


  /** Обновление поисковой информации
  */
  virtual void RenewSearchData();

  /** Проверка выполнения критерия остановки метода

  Метод прекращает работу в следующих случаях:
  - число испытаний превысило максимально допустимое значение
  - если решается одна задача и выполнен критерий \f$ x_t - x_{t-1} < \epsilon \f$
  - если решается серия задач и выполнен критерий \f$ \| y^k - y^\ast \| < \epsilon \f$

  \return истина, если критерий остановки выполнен; ложь - в противном случае.
  */
  virtual bool CheckStopCondition();

  /** Оценить текущее значение оптимума

  \return истина, если оптимум изменился; ложь - в противном случае
  */
  virtual bool EstimateOptimum();

  /** Функция вызывается в конце проведения итерации
  */
  virtual void FinalizeIteration();

  /** Получить число испытаний

  \return число испытаний
  */
  virtual int GetIterationCount();


  /** Получить текущую оценку оптимума

  \return испытание, соответствующее текущему оптимуму
  */
  virtual Trial* GetOptimEstimation();

  /**Сбор статистики

  Функция возвращает общее число испытаний, выполненных при решении текущей задачи и всех вложенных
  подзадач
  \return общее число испытаний
  */
  virtual int GetNumberOfTrials();

  /// сохраняем точки с уровня
  virtual void PrintLevelPoints(const std::string& fileName);

  /// Сохраняем все точки, со всех уровней, в файл
  virtual void PrintPoints(const std::string& fileName);

  /// Метод Хука-Дживса
  void HookeJeevesMethod(Trial& point, std::vector<Trial*>& localPoints);

  ///Возвращает Число вычислений каждой функции
  virtual std::vector<int> GetFunctionCalculationCount();

  /// Возвращает достигнутую точность
  virtual double GetAchievedAccuracy();

  /** Обновление поисковой информации
  */
  virtual void ResetSearchData() {};


  /**Добавляет испытания в поисковую информацию, при этом обновляя константу Гёльдера и
  оценку оптимума

  \param[in] points точки испытаний, которые будут добавлены
  */
  void InsertPoints(const std::vector<Trial*>& points);
  /**Добавляет испытания полученные локальным методом в поисковую информацию, при этом обновляя константу Гёльдера и
оценку оптимума

\param[in] points точки испытаний, которые будут добавлены
*/
  virtual void InsertLocalPoints(const std::vector<Trial*>& points, Task* task = 0);

  /// Запускает локальный метод
  virtual void LocalSearch();

  /// Возвращает число точек полученное от локальныго метода
  virtual int GetLocalPointCount();

  /// Возвращает число запусков локально метода
  virtual int GetNumberLocalMethodtStart();

  /// Печатает информацию о сечениях
  virtual void PrintSection();

  void SaveCurrentProgress()
  {
    if (parameters.fileSerializer.ToString().empty()) return;

    // Получаем новые точки и интервалы с последнего сохранения
    std::vector<Trial*> newTrials;
    std::vector<SearchInterval*> newIntervals;

    std::vector<Trial*>& allTrials = pData->GetTrials();
    for (int i = lastSavedTrialsCount; i < (int)allTrials.size(); ++i)
    {
      newTrials.push_back(allTrials[i]);
    }

    // Собираем новые интервалы
    int currentIntervalsCount = pData->GetCount();
    int intervalCounter = 0;
    for (SearcDataIterator it = pData->GetBeginIterator(); it; ++it)
    {
      if (intervalCounter >= lastSavedTrialsCount - 1)
      {
        newIntervals.push_back(*it);
      }
      intervalCounter++;
    }

    parameters.serializer->SaveProgress(parameters.fileSerializer.ToString(), newTrials, newIntervals, pData->GetBestTrial());

    lastSavedTrialsCount = allTrials.size();
  }
};

#endif
// - end of file ----------------------------------------------------------------------------------

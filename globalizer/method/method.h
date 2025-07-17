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

\brief Объявление класса #TMethod

\details Объявление класса #TMethod и сопутствующих типов данных
*/


#ifndef __METHOD_H__
#define __METHOD_H__

#include "method_interface.h"
#include "calculation.h"
#include "InformationForCalculation.h"
#include "SearchIteration.h"
#include "SearchInterval.h"

#include "ApproxTask.h"

#ifdef USE_OpenCV
#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/ml/ml.hpp"

//using namespace cv;
//using namespace ml;
//
#endif

// ------------------------------------------------------------------------------------------------


/**
Базовый класс, реализующий алгоритм глобального поиска.

В классе #TMethod реализованы основные функции, определяющие работу алгоритма глобального поиска.
*/
class TMethod : public IMethod
{
protected:
  // ----------------------------------------------------------------------------
  // Копия параметров для конкретного уровня дерева
  // ----------------------------------------------------------------------------
  /// Максимальное число испытаний
  int               MaxNumOfTrials;
  /// число итераций до включения смешанного алгоритма
  int               StartLocalIteration;
  /// точность решения задачи
  double            Epsilon;
  /// Надежность метода
  /// Параметр надежности должне быть строго больше 1
  double            r;
  /** Плотность построения развертки

   Развертка, построенная с плотностью m, обеспечивает точность поиска \f$ 1/2^m \f$ по координате
  */
  int               m;

  /// число точек испытаний используемое на каждой итерации
  int               NumPoints;
  /// Обновлено глобальное М
  bool isGlobalMUpdate;
  /// Обновлена лучшая точка в текущей задаче
  bool isLocalZUpdate;

  // ----------------------------------------------------------------------------
  // Ссылки на объекты используемых методом
  // ----------------------------------------------------------------------------
  /// Указатель на решаемую задачу
  TTask&            pTask;
  /// Указатель на матрицу состояния поиска
  TSearchData*      pData;

  /// Вычислитель
  TCalculation& Calculation;
  /** Указатель на развертку

  В зависимости от вида отображения это может быть:
  - единственная развертка
  - множественная сдвиговая развертка
  - множественная вращаемая развертка
  */
  TEvolvent& Evolvent;

  int numberOfRepetitions;

  // ----------------------------------------------------------------------------
  // Внутренние данные метода
  // ----------------------------------------------------------------------------



  ///// Входные данные для вычислителя, формирубтся в CalculateFunctionals()
  TInformationForCalculation inputSet;
  ///// Выходные данные вычислителя, обрабатывается в CalculateFunctionals()
  TResultForCalculation outputSet;
  /// информация о данных текущей итерации
  TSearchIteration iteration;
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

  std::vector< double > globalM;

  ///Новая точка устанавливается в интервал принадлежащий окрестности локального минимума
  bool isSetInLocalMinimumInterval;

  /// Массив для сохранения точек для последующей печати и рисования
  static std::vector< TTrial* > printPoints;
  /// количество точек вычисленных локальным методом
  static int localPointCount;
  /// число запусков локально метода
  static int numberLocalMethodtStart;
  /// Нужно останавливаться
  bool isStop;

  /// Количество вызовов рисовалки
  static int printCount;

  /**
  Количество дискретных значений
  Произведение числа значений всех дискретных переменных.
  Равно числу интервалов.
  */
  int mDiscreteValuesCount;
  /// Значения дискретных параметров
  std::vector< std::vector< double > > mDiscreteValues;
  /// Индекс первого дискретного параметра
  int startDiscreteVariable;
  /// Размерность используемая при вычислении длинны интервала
  int rootDim;

  /// найденные локальные минимумы
  std::vector<TTrial*> localMinimumPoints;

  //=====================================================================================================================================================
  //Для методов локального уточнения нужны миксимумы
  double* Xmax;
  double* mu;
  TSearchInterval* intervalXMax;
  bool isSearchXMax;
  //=====================================================================================================================================================

#ifdef USE_OpenCV
  //  Для многомерных деревьев решений=================================================================================================
  cv::Mat X;
  cv::Mat ff;
  // Равномерная сетка, использкющая при выборе интервала, хранящего локальный минимум с использованием деревьев решений
  cv::Mat uniformPartition;
  cv::Ptr< cv::ml::DTrees > Mytree;
  //cv::Mat results;
  std::vector<TTrial*> pointsForTree;
  std::vector<TTrial*> pointsForLocalMethod;
  bool isFirst = true;
  int indexForTrainingMax = 0;
  std::vector<TTrial*> localMins;
  int countOfRepetitions = 0;

  //  =================================================================================================================================
#endif

  /// Метод сохраняющий точки в статический массив
  virtual void  SavePoints();
  /// Рисуем точки в подзадачах
  virtual void  PlotPoint();
  /// Рисуем точки образованные разверткой
  virtual void  PlotPoint2();

  /** Вычисление "глобальной" характеристики

  \param[in] p указатель на интервал, характеристику которого надо вычислить
  \return "Глобальная" характеристика интервала
  */
  virtual double CalculateGlobalR(TSearchInterval* p);
  /** Вычисление "локальной" характеристики

  Данная функция должна вызываться только для интервала, у которого вычислена глобальная
  характеристика, т.е. после вызова функции #CalculateGlobalR
  \param[in] p указатель на интервал, характеристику которого надо вычислить
  \return "Локальная" характеристика интервала
  */
  virtual double CalculateLocalR(TSearchInterval* p);
  /** Вычисление оценки константы Липшица

  Обновленная оценка константы Липшица записывается в базе алгоритма
  \param[in] p указатель на интервал
  */
  virtual void CalculateM(TSearchInterval* p);
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
  virtual int IsBoundary(TSearchInterval* p);

  /** Обновление константы Липшица для функции с заданным индексом

  Если константа обновлена, поднимает флаг #recalc. Данная функция используется в
  функции #CalculateM
  \param[in] newValue новое значение константы Липшица
  \param[in] index индекс функции
  \param[in] boundaryStatus является ли интервал граничным
  \param[in] p рассматриваемый интервал
  */
  virtual void UpdateM(double newValue, int index, int boundaryStatus, TSearchInterval* p);

  /** Обновление текущей оценки оптимума

  Если переданная точка лучше текущей оценки оптимума, то эта оценка обновляется и поднимается
  флаг #recalc.
  \param[in] trial точка, которую необходимо сравнить с текущим оптимумом
  \return true, если оптимум обновлён, иначе false
  */
  virtual bool UpdateOptimumEstimation(TTrial& trial);

  /// Проверяет попала ли точка в окрестность глобального манимума
  virtual bool CheckLocalityOptimalPoint(TTrial* trial);

  /// Вычисление координат точек испытания для основной\единственной развертки
  virtual void CalculateCurrentPoint(TTrial& pCurTrialsj, TSearchInterval* BestIntervalsj);

  /// Вычисляем координаты точек которые будем использовать на текущей итерации
  virtual void CalculateCurrentPoints(std::vector<TSearchInterval*>& BestIntervals);

  /// Пренадлежит ли newInterval отрезку в котором находится basicInterval
  /// Уже не нужен???
  virtual bool IsIntervalInSegment(TSearchInterval* basicInterval, TSearchInterval* newInterval);

  /**Изменить количество текущих точек испытаний, переписывает #iteration.pCurTrials и
  #iteration.BestIntervals
  */
  virtual void SetNumPoints(int newNP);

  /**
  Изменение при динамичеки изменяемом r, r = r + rDynamic / (Iteration ^ (1/N))
  */
  virtual double Update_r(int iter = -1, int procLevel = -1);

  /// x--> y; Вычисляет координаты y в гиперкубе по x из отрезка
  virtual void CalculateImage(TTrial& pCurTrialsj);



  /// Добавление основных (из основной\единственной развертки) точек испытания в базу
  virtual TSearchInterval* AddCurrentPoint(TTrial& pCurTrialsj, TSearchInterval* BestIntervalsj);

  /// Перерасчет характеристик и перестройка очереди
  virtual void Recalc();

  ///Определяем нужно ли запускать локальный метод по 5 точкам
  virtual bool UpdateStatusNPoint(TTrial* trial, TTrial*& inflection, 
    std::vector<TTrial*>& leftLocalMinPoint,  std::vector<TTrial*>& RightLocalMinPoint,
    int& countPointLeft, int& countPointRight);


  ///Определяем нужно ли запускать локальный метод по дереву решения
  virtual bool UpdateStatusDecisionTrees(TTrial* trial, TTrial*& inflection,
    std::vector<TTrial*>& leftLocalMinPoint, std::vector<TTrial*>& RightLocalMinPoint,
    int& countPointLeft, int& countPointRight);

#ifdef USE_OpenCV
  double FindDistance(cv::Mat point, int index, TTrial*& inflection);

  void recursiveFilling(int dim, const double* lb, const double* ub, int numPointPerDim, cv::Mat *uniformPartition, double* &value, int  &index, int reset);

  void fillDataForDecisionTree(TTrial* point);

  void PrepareDataForDecisionTree(int N, TSearchData* data);

  void CreateTree();

  void FillTheSegment(int numPointPerDim);

  int FindIndexOfNearestPoint(int numPointPerDim, TTrial*& inflection);

  std::vector<int> FindNeighbours(int numPointPerDim, int nearestPointIndex, int* masOfIndexes);

  bool FindAndCheckPointWithNeighbours(int numPointPerDim, int nearestPointIndex, cv::Mat results);

  virtual void UpdateStatusDecisionTreesMultiDims(TTrial* trial, TTrial*& inflection);
#endif

  ///Определяем нужно ли запускать локальный метод по дереву решения
  virtual bool UpdateStatusAllMinimum(TTrial* trial, TTrial*& inflection,
    std::vector<TTrial*>& leftLocalMinPoint, std::vector<TTrial*>& RightLocalMinPoint,
    int& countPointLeft, int& countPointRight);


  /// Задать значения дискретного параметра
  virtual void SetDiscreteValue(int u, std::vector< std::vector <double> > dvs);

  /// Получаем поисковую информацию, важно для адаптивного метода
  virtual TSearchData* GetSearchData(TTrial* trial);

public:

  TMethod(TTask& _pTask, TSearchData& _pData,
    TCalculation& _Calculation, TEvolvent& _Evolvent);
  virtual ~TMethod();

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

  ///Обноаление меток точек
  virtual void UpdateStatus(TTrial* trial);

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
  virtual int GetIterationCount()
  { return iteration.IterationCount; }

  /** Получить текущую оценку оптимума

  \return испытание, соответствующее текущему оптимуму
  */
  virtual TTrial* GetOptimEstimation();

  /**Сбор статистики

  Функция возвращает общее число испытаний, выполненных при решении текущей задачи и всех вложенных
  подзадач
  \return общее число испытаний
  */
  virtual int GetNumberOfTrials();

  /// сохраняем точки с уровня
  virtual void PrintLevelPoints(const std::string& fileName);

  /// Сохраняем все точки, со всех уровней, в файл
  virtual void PrintPoints(const std::string & fileName);
  /// Метод Хука-Дживса
  void HookeJeevesMethod(TTrial& point, std::vector<TTrial*>& localPoints);
  /////Локальное уточнение из текущей лучшей точки
  virtual void LocalS(TTrial& point, std::vector<TTrial*> leftLocalMinPoint, std::vector<TTrial*> RightLocalMinPoint, 
    int countPointLeft, int countPointRight);
  //virtual void LocalSearch();
  /////Случайный поиск на начальной стадии
  //virtual void RandomSearh();
  /////Сепарабельный поиск на начальной стадии
  //virtual void SeparableSearch();

  ///Возвращает Число вычислений каждой функции
  virtual std::vector<int> GetFunctionCalculationCount();

  /// Возвращает достигнутую точность
  virtual double GetAchievedAccuracy()
  {
    return AchievedAccuracy;
  };

  /** Обновление поисковой информации
  */
  virtual void ResetSearchData() {};


  /**Добавляет испытания в поисковую информацию, при этом обновляя константу Гёльдера и
  оценку оптимума

  \param[in] points точки испытаний, которые будут добавлены
  */
  void InsertPoints(const std::vector<TTrial*>& points);
  /**Добавляет испытания полученные локальным методом в поисковую информацию, при этом обновляя константу Гёльдера и
оценку оптимума

\param[in] points точки испытаний, которые будут добавлены
*/
  virtual void InsertLocalPoints(const std::vector<TTrial*>& points, TTask* task = 0);

  virtual void LocalSearch();

  /// Возвращает число точек полученное от локальныго метода
  virtual int GetLocalPointCount();

  /// Возвращает число запусков локально метода
  virtual int GetNumberLocalMethodtStart();

  /// Печатает информацию о сечениях
  virtual void PrintSection();

  /// Рисуем дерево
  virtual void PlotDecisionTrees();

  virtual void Plot3DDecisionTrees();
};

#endif
// - end of file ----------------------------------------------------------------------------------

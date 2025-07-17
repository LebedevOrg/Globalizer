/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      data.h                                                      //
//                                                                         //
//  Purpose:   Header file for search data classes                         //
//                                                                         //
//  Author(s): Sysoyev A., Barkalov K., Sovrasov V.                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __SEARCH_DATA_H__
#define __SEARCH_DATA_H__

#include "common.h"
#include "extended.h"
#include "dual_queue.h"
#include "queue.h"
#include <stack>
#include "parameters.h"

#include <string.h>

#include <list>
#include <vector>


struct TTreeNode;
class TSearchInterval;
class TSearcDataIterator;
class TTrial;
// ------------------------------------------------------------------------------------------------
class TSearchData
{
  friend class TSearcDataIterator;
protected:
  /// число функций задачи
  int NumOfFuncs;
  /// максимальный размер МСП = максимальному числу итераций метода
  int MaxSize;
  /// текущее число интервалов в дереве
  int Count;
  /// текущий индекс, используется в итераторе
  int CurIndex;
  /// Корень дерева
  TTreeNode *pRoot;
  /// Текущая вершина дерева
  TTreeNode *pCur;

  //TTreeNode *pCurIter;
  /// стек для итератора
  std::stack<TTreeNode*> Stack;
  ///очередь характеристик
  TPriorityQueueCommon *pQueue;

  /// список всех точек, для их последующего удаления
  std::vector<TTrial*> trials;

  /// истина, если нужен пересчет характеристик
  bool recalc;

  /// Лучшая точка, полученная для данной поисковой информации
  TTrial* BestTrial;

  void DeleteTree(TTreeNode *pNode);
  unsigned char GetHeight(TTreeNode *p);
  int GetBalance(TTreeNode *p);
  void FixHeight(TTreeNode *p);
  TTreeNode* RotateRight(TTreeNode *p); // правый поворот вокруг p
  TTreeNode* RotateLeft(TTreeNode *p);  // левый поворот вокруг p
  TTreeNode* Balance(TTreeNode *p);     // балансировка узла p
  TTreeNode* Maximum(TTreeNode *p) const; //поиск самого левого интервала в поддереве
  TTreeNode* Minimum(TTreeNode *p) const; //поиск самого правого интервала в поддереве
  TTreeNode* Previous(TTreeNode *p) const; //получение предыдущего и следующего за p интервалов
  TTreeNode* Next(TTreeNode *p) const;
  // вставка в дерево с корнем p (рекурсивная)
  TTreeNode* Insert(TTreeNode *p, TSearchInterval &pInterval);
  // поиск узла с нужным x в дереве с корнем p по левой границе интервала (рекурсивный)
  TTreeNode* Find(TTreeNode *p, TTrial* x) const;
  // поиск узла по правой границе интервала
  TTreeNode* FindR(TTreeNode *p, TTrial* x) const;
  // поиск узла с нужным x по левой и правой границам интервала (рекурсивный)
  //   xl() < x < xr
  TTreeNode* FindIn(TTreeNode *p, TTrial* x) const;
public:
  /// Вектор указателей на матрицы состояния поиска, для которых нужно произвести пересчет
  static std::vector<TSearchData*> pRecalcDatas;

  TSearchData(int _NumOfFuncs, int _MaxSize = DefaultSearchDataSize);
  TSearchData(int _NumOfFuncs, int _MaxSize, int _queueSize);
  ~TSearchData();

  /// Очищает и дерево и очередь интервалов
  void Clear();
  /// новый интервал (по xl)
  TSearchInterval* InsertInterval(TSearchInterval &pInterval); 
  /// обновление интервала (по xl)
  void UpdateInterval(TSearchInterval &pInterval); 
  /// Ищет интервал у которого левой точкой является x
  TSearchInterval* GetIntervalByX(TTrial* x);
  /** Поиск интервала, в котором содержится x, т.е. xl() < x < xr
     нужен для вставки прообразов при использовании множественной развертки */
  TSearchInterval* FindCoveringInterval(TTrial* x);
  /** Получение интервала с максимальной хар-кой. Интервал берется из очереди. Если очередь пуста,
     то сначала будет вызван Refill() */
  TSearchInterval* GetIntervalWithMaxR();
  /** Получение интервала с максимальной локалььной хар-кой. Интервал берется из очереди. Если очередь пуста,
  то сначала будет вызван Refill() */
  TSearchInterval* GetIntervalWithMaxLocalR();

  /** Вставка испытания в заданный интервал. Нужна для множественной развертки и
  добавления поисковой информации локального метода
  возвращает указатель на интервал с левым концом в newPoint
  */
  TSearchInterval* InsertPoint(TSearchInterval* coveringInterval, TTrial& newPoint,
    int iteration, int methodDimension);

  /// Итератор
  TSearcDataIterator GetIterator(TSearchInterval* p);
  TSearcDataIterator GetBeginIterator();

  /** Получить интервал, предыдущий к указанному
     не относится к итератору, не меняет текущий узел в итераторе
    TSearchInterval* GetPrev(TSearchInterval &pInterval);

   Для работы с очередью характеристик
   вставка, если новый элемент больше минимального в очереди
     если при этом очередь полна, то замещение минимального
  */
  void PushToQueue(TSearchInterval *pInterval);
  /// Перезаполнение очереди (при ее опустошении или при смене оценки константы Липшица)
  void RefillQueue();
  /// Удалить интервал из очереди
  void DeleteIntervalFromQueue(TSearchInterval* i);

  /// Берет из очереди один интервал
  void PopFromGlobalQueue(TSearchInterval **pInterval);
  /// Берет из очереди локальных характеристик один интервал
  void PopFromLocalQueue(TSearchInterval **pInterval);
  /// Очистить очередь интервалов
  void ClearQueue();
  /// Изменить размер очереди интервалов
  void ResizeQueue(int size);

  /// Возвращает текущее число интервалов в дереве 
  int GetCount();

  /// оценки констант Липшица
  double M[MaxNumOfFunc];
  /// минимальные значения функций задачи (для индексного метода)
  double Z[MaxNumOfFunc];

  void GetBestIntervals(TSearchInterval** intervals, int count);
  void GetBestLocalIntervals(TSearchInterval** intervals, int count);
  std::vector<TTrial*>& GetTrials()
  {
    return trials;
  }

  /// возвращает максимальный элемент без извлечения
  TSearchInterval& FindMax();

  /// истина, если нужен пересчет характеристик
  bool IsRecalc()
  {
    return recalc;
  }
  /// Задает нужно ли пересчитывать характеристики
  void SetRecalc(bool f)
  {    
    if (recalc == false)
      pRecalcDatas.push_back(this);
    recalc = f;    
  }
  /// Лучшая точка, полученная для данной поисковой информации
  TTrial* GetBestTrial()
  {
    return BestTrial;
  }
  /// Задает лучшую точку
  void SetBestTrial(TTrial* trial);

  /// Всплытие для интервала
  void TrickleUp(TSearchInterval* intervals);

  /// Возвращает размер очереди
  int GetQueueSize();


  double local_r;//вычисляемое r
};


#endif
// - end of file ----------------------------------------------------------------------------------

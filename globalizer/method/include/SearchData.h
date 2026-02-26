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

#include "Common.h"
#include "Extended.h"
#include "DualQueue.h"
#include "Queue.h"
#include <stack>
#include "Parameters.h"

#include <string.h>

#include <list>
#include <vector>


class TreeNode;
class SearchInterval;
class SearcDataIterator;
class Trial;
// ------------------------------------------------------------------------------------------------
class SearchData
{
  friend class SearcDataIterator;
protected:

  /// Максимальный размер МСП = максимальному числу итераций метода
  int MaxSize;
  /// Текущее число интервалов в дереве
  int Count;
  /// Текущий индекс, используется в итераторе
  int CurIndex;
  /// Корень дерева
  TreeNode *pRoot;
  /// Текущая вершина дерева
  TreeNode *pCur;

  /// Стек для итератора
  std::stack<TreeNode*> Stack;
  /// Очередь характеристик
  PriorityQueueCommon *pQueue;

  /// Список всех точек, для их последующего удаления
  std::vector<Trial*> trials;

  /// Истина, если нужен пересчет характеристик
  bool recalc;

  /// Лучшая точка, полученная для данной поисковой информации
  Trial* BestTrial;

  /// Удалить дерево
  void DeleteTree(TreeNode *pNode);
  /// Получить высоту
  unsigned char GetHeight(TreeNode *p);
  /// Отбалансировать
  int GetBalance(TreeNode *p);
  /// Исправить высоту
  void FixHeight(TreeNode *p);
  /// Правый поворот вокруг p
  TreeNode* RotateRight(TreeNode *p);
  /// Левый поворот вокруг p
  TreeNode* RotateLeft(TreeNode *p);
  /// Балансировка узла p
  TreeNode* Balance(TreeNode *p);
  /// Поиск самого левого интервала в поддереве
  TreeNode* Maximum(TreeNode *p) const;
  /// Поиск самого правого интервала в поддереве
  TreeNode* Minimum(TreeNode *p) const;
  /// Получение предыдущего за p интервала
  TreeNode* Previous(TreeNode *p) const;
  /// Получение следующего за p интервала
  TreeNode* Next(TreeNode *p) const;
  /// Вставка в дерево с корнем p (рекурсивная)
  TreeNode* Insert(TreeNode *p, SearchInterval &pInterval);
  /// Поиск узла с нужным x в дереве с корнем p по левой границе интервала (рекурсивный)
  TreeNode* Find(TreeNode *p, Trial* x) const;
  /// Поиск узла по правой границе интервала
  TreeNode* FindR(TreeNode *p, Trial* x) const;
  /** Поиск узла с нужным x по левой и правой границам интервала(рекурсивный)
   xl() < x < xr */
  TreeNode* FindIn(TreeNode *p, Trial* x) const;
public:
  /// Число функций задачи
  int NumOfFuncs;

  SearchData(int _NumOfFuncs, int _MaxSize = DefaultSearchDataSize);
  SearchData(int _NumOfFuncs, int _MaxSize, int _queueSize);
  ~SearchData();

  /// Очищает и дерево и очередь интервалов
  void Clear();
  /// Новый интервал (по xl)
  SearchInterval* InsertInterval(SearchInterval &pInterval); 
  /// Обновление интервала (по xl)
  void UpdateInterval(SearchInterval &pInterval); 
  /// Ищет интервал у которого левой точкой является x
  SearchInterval* GetIntervalByX(Trial* x);
  /** Поиск интервала, в котором содержится x, т.е. xl() < x < xr
     нужен для вставки прообразов при использовании множественной развертки */
  SearchInterval* FindCoveringInterval(Trial* x);
  /** Получение интервала с максимальной хар-кой. Интервал берется из очереди. Если очередь пуста,
     то сначала будет вызван Refill() */
  SearchInterval* GetIntervalWithMaxR();
  /** Получение интервала с максимальной локальной хар-кой. Интервал берется из очереди. Если очередь пуста,
  то сначала будет вызван Refill() */
  SearchInterval* GetIntervalWithMaxLocalR();

  /** Вставка испытания в заданный интервал. Нужна для множественной развертки и
  добавления поисковой информации локального метода
  возвращает указатель на интервал с левым концом в newPoint
  */
  SearchInterval* InsertPoint(SearchInterval* coveringInterval, Trial& newPoint,
    int iteration, int methodDimension);

  /// Получить итератор
  SearcDataIterator GetIterator(SearchInterval* p);
  /// Получить следующий итератор
  SearcDataIterator GetBeginIterator();

  /** Получить интервал, предыдущий к указанному
     не относится к итератору, не меняет текущий узел в итераторе
     SearchInterval* GetPrev(SearchInterval &pInterval);

    Для работы с очередью характеристик
    вставка, если новый элемент больше минимального в очереди
    если при этом очередь полна, то замещение минимального
  */
  void PushToQueue(SearchInterval *pInterval);
  /// Перезаполнение очереди (при ее опустошении или при смене оценки константы Липшица)
  void RefillQueue();
  /// Удалить интервал из очереди
  void DeleteIntervalFromQueue(SearchInterval* i);

  /// Берет из очереди один интервал
  void PopFromGlobalQueue(SearchInterval **pInterval);
  /// Берет из очереди локальных характеристик один интервал
  void PopFromLocalQueue(SearchInterval **pInterval);
  /// Очистить очередь интервалов
  void ClearQueue();
  /// Изменить размер очереди интервалов
  void ResizeQueue(int size);

  /// Возвращает текущее число интервалов в дереве 
  int GetCount();

  /// Оценки констант Липшица
  double M[MaxNumOfFunc];
  /// Минимальные значения функций задачи (для индексного метода)
  double Z[MaxNumOfFunc];

  /// Получить count лучших интервалов
  void GetBestIntervals(SearchInterval** intervals, int count);
  /// Получить count лучших локальных интервалов
  void GetBestLocalIntervals(SearchInterval** intervals, int count);
  /// Получить испытания
  std::vector<Trial*>& GetTrials()
  {
    return trials;
  }

  /// Возвращает максимальный элемент без извлечения
  SearchInterval& FindMax();

  /// Истина, если нужен пересчет характеристик
  bool IsRecalc()
  {
    return recalc;
  }
  /// Задает нужно ли пересчитывать характеристики
  void SetRecalc(bool f)
  {    
    recalc = f;    
  }
  /// Лучшая точка, полученная для данной поисковой информации
  Trial* GetBestTrial()
  {
    return BestTrial;
  }
  /// Задает лучшую точку
  void SetBestTrial(Trial* trial);

  /// Всплытие для интервала
  void TrickleUp(SearchInterval* intervals);

  /// Возвращает размер очереди
  int GetQueueSize();

  /// Вычисляемое r
  double local_r;

  /// Число функций задачи
  int GetNumOfFuncs()
  {
    return NumOfFuncs;
  }
}; // SearchData


#endif
// - end of file ----------------------------------------------------------------------------------

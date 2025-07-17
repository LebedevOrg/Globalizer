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

#ifndef __SEARCH_INTERVAL_H__
#define __SEARCH_INTERVAL_H__


#include "extended.h"
#include "baseInterval.h"
#include "Trial.h"


struct TTreeNode;

// ------------------------------------------------------------------------------------------------
class TSearchInterval : public TQueueBaseData
{
public:

  /// Метка интервала: обычный, образующий локальную область, образовный локальным методом
  enum Status
  {
    usual,
    local_area,
    educational_local_method
  };

  /// Метка интервала: обычный, образующий локальную область, образовный локальным методом
  Status status;

  /// Левая точка интервала
  TTrial* LeftPoint;
  /// Правая точка интервала
  TTrial* RightPoint;

  /// "гельдеровская" длина
  double   delta;

  /// номер итерации
  int      ind;
  /// число "вложенных" итераций
  int     K;
  /// характеристика интервала (xl, xr)
  double   R;
  /// локальная характеристика интервала (xl, xr)
  double   locR;

  /// Элемент дерева хранящий этот интервал
  TTreeNode *treeNode;



  /// левая граница интервала
  Extended xl()
  {
    return LeftPoint->X();
  }
  /// правая граница интервала
  Extended xr()
  {
    return RightPoint->X();
  }

  /// значение последнего вычисленного функционала в xl
  double   zl()
  {
    if (LeftPoint->index >= 0)
      return LeftPoint->FuncValues[LeftPoint->index];
    else
      return MaxDouble;
  };
  /// индекс последнего вычисленного фукнционала в xl
  int izl()
  {
    return LeftPoint->index;
  }
  /// значение последнего вычисленного функционала в xr
  double   zr()
  {
    if (RightPoint->index >= 0)
      return RightPoint->FuncValues[RightPoint->index];
    else
      return MaxDouble;
  };
  /// индекс последнего вычисленного фукнционала в xr
  int izr()
  {
    return RightPoint->index;
  }

  /// значения вычисленных функционалов в izl, кол-во - izl() + 1
  double* z()
  {
    //double   *z;
    return LeftPoint->FuncValues;
  }

  /// Индекс значения дискретного параметра
  int discreteValuesIndex()
  {
    if (LeftPoint->discreteValuesIndex != RightPoint->discreteValuesIndex)
      throw "Error TSearchInterval discreteValuesIndex!!!\n";
    return LeftPoint->discreteValuesIndex;
  }


  virtual bool operator == (TSearchInterval &p)
  {
    return *LeftPoint == *(p.LeftPoint);
  }

  virtual bool operator > (TSearchInterval &p)
  {
    return *LeftPoint > *(p.LeftPoint);
  }

  virtual bool operator < (TSearchInterval &p)
  {
    return *LeftPoint < *(p.LeftPoint);
  }

  void CreatePoint();

  TSearchInterval();
  TSearchInterval(const TSearchInterval &p);
  ~TSearchInterval();
};

#endif
// - end of file ----------------------------------------------------------------------------------

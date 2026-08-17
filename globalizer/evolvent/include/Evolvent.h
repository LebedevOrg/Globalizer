/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      evolvent.h                                                  //
//                                                                         //
//  Purpose:   Header file for evolvent classes                            //
//                                                                         //
//  Author(s): Barkalov K., Sysoyev A.                                     //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


/**
\file evolvent.h

\authors Баркалов К., Сысоев А.
\date 2015-2016
\copyright ННГУ им. Н.И. Лобачевского

\brief Объявление класса #Evolvent

\details Объявление класса #Evolvent и сопутствующих типов данных
*/

#ifndef __EVOLVENT_H__
#define __EVOLVENT_H__

#include "Common.h"
#include "Extended.h"
#include "EvolventInterface.h"
#include <vector>

// ------------------------------------------------------------------------------------------------

/**
\brief Класс, реализующий отображение между гиперкубом и гиперинтервалом

Класс #Evolvent предоставляет средства для преобразования координат между
гиперкубом [-1/2, 1/2]^N и гиперинтервалом D.
*/
class Evolvent : public IEvolvent
{
protected:
  /// Точность разложения гиперкуба
  int      m;
  /// Размерность задачи
  int      N;
  /// Левые границы поисковой области
  double   A[MaxDim];
  /// Правые границы поисковой области
  double   B[MaxDim];
  /// Точка из гиперкуба [-1/2, 1/2]^N
  double* y;

  /// Extended(0.0)
  const Extended extNull;
  /// = Extended(1.0)
  const Extended extOne;
  /// = Extended(0.5)
  const Extended extHalf;
  Extended nexpExtended;


  virtual void CalculateNumbr(Extended* s, long long* u, long long* v, long long* l);

  /// вычисление вспомогательного центра u(s) и соответствующих ему v(s) и l(s)
  virtual void CalculateNode(Extended is, int n, long long* u, long long* v, long long* l);
  /// Преобразование из гиперкуба P в гиперинтервал D
  virtual void transform_P_to_D();
  /// Преобразование из гиперинтервала D в гиперкуб P
  virtual void transform_D_to_P();
  /// Получить точку y по x
  virtual double* GetYOnX(const Extended& _x);
  /// Получить x по точке y
  virtual Extended GetXOnY();

public:

  /**
  \brief Конструктор класса #Evolvent
  */
  Evolvent(int _N = 2, int _m = 10);

  /**
  \brief Конструктор копирования
  */
  Evolvent(const Evolvent& evolvent);

  /// Деструктор класса #Evolvent
  virtual ~Evolvent();

  /**
  \brief Возвращает левые границы поисковой области (A)
  */
  virtual const double* getA() const { return A; }

  /**
  \brief Возвращает правые границы поисковой области (B)
  */
  virtual const double* getB() const { return B; }

  /**
  \brief Установка границ поисковой области
  */
  virtual void SetBounds(const double* _A, const double* _B);

  /**
  \brief Преобразование x в y (x -> y)
  */
  virtual void GetImage(const Extended& x, double* _y, int EvolventNum = 0);

  /**
  \brief Преобразование y в x (y -> x)
  */
  virtual void GetInverseImage(double* _y, Extended& x);

  /**
  \brief Преобразование y в x (y -> x)
  */
  virtual void GetPreimages(double* _y, Extended* x);

  /**
  \brief Оператор присваивания
  */
  virtual Evolvent& operator=(const Evolvent& evolvent);

  /**
  \brief Вычисляет функцию существования точки в развертки EvolventNum для y, <0 - существует
  */
 virtual double ZeroConstraintCalc(const double* _y, int EvolventNum = 0);
};

#endif
// - end of file ----------------------------------------------------------------------------------
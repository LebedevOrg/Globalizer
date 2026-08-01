/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      evolventinterface.h                                         //
//                                                                         //
//  Purpose:   Header file for evolvent interface classes                  //
//                                                                         //
//  Author(s): Barkalov K., Sysoyev A.                                     //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


/**
\file evolventinterface.h

\authors Баркалов К., Сысоев А.
\date 2015-2016
\copyright ННГУ им. Н.И. Лобачевского

\brief Объявление класса #Evolvent

\details Объявление класса #Evolvent и сопутствующих типов данных
*/

#ifndef __EVOLVENT_INTERFACE_H__
#define __EVOLVENT_INTERFACE_H__

#include "Common.h"
#include "Extended.h"
#include <vector>

// ------------------------------------------------------------------------------------------------

/**
\brief Класс, реализующий отображение между гиперкубом и гиперинтервалом

Класс #Evolvent предоставляет средства для преобразования координат между
гиперкубом [-1/2, 1/2]^N и гиперинтервалом D.
*/
class IEvolvent
{
//protected:
//  /// Точность разложения гиперкуба
//  int      m;
//  /// Размерность задачи
//  int      N;
//  /// Левые границы поисковой области
//  double   A[MaxDim];
//  /// Правые границы поисковой области
//  double   B[MaxDim];
//  /// Точка из гиперкуба [-1/2, 1/2]^N
//  double* y;
//
//  /// Extended(0.0)
//  const Extended extNull;
//  /// = Extended(1.0)
//  const Extended extOne;
//  /// = Extended(0.5)
//  const Extended extHalf;
//  Extended nexpExtended;
//
//
//  virtual void CalculateNumbr(Extended* s, long long* u, long long* v, long long* l) = 0;
//
//  /// вычисление вспомогательного центра u(s) и соответствующих ему v(s) и l(s)
//  virtual void CalculateNode(Extended is, int n, long long* u, long long* v, long long* l) = 0;
//  /// Преобразование из гиперкуба P в гиперинтервал D
//  virtual void transform_P_to_D() = 0;
//  /// Преобразование из гиперинтервала D в гиперкуб P
//  virtual void transform_D_to_P() = 0;
//  /// Получить точку y по x
//  virtual double* GetYOnX(const Extended& _x) = 0;
//  /// Получить x по точке y
//  virtual Extended GetXOnY() = 0;

public:

  /// Виртуальный деструктор
  virtual ~IEvolvent() {}

  /**
  \brief Возвращает левые границы поисковой области (A)
  */
	virtual const double* getA() const = 0;

  /**
  \brief Возвращает правые границы поисковой области (B)
  */
	virtual const double* getB() const = 0;

  /**
  \brief Установка границ поисковой области
  */
	virtual void SetBounds(const double* _A, const double* _B) = 0;

  /**
  \brief Преобразование x в y (x -> y)
  */
  virtual void GetImage(const Extended& x, double* _y, int EvolventNum = 0) = 0;

  /**
  \brief Преобразование y в x (y -> x)
  */
  virtual void GetInverseImage(double* _y, Extended& x) = 0;

  /**
  \brief Преобразование y в x (y -> x)
  */
  virtual void GetPreimages(double* _y, Extended* x) = 0;

  /**
	\brief Неинъективное преобразование y в x (y -> x)
	*/
  virtual int GetNoninjectivePreimages(double* _y, Extended* x) { return 0; };

  /**
  \brief Вычисляет функцию существования точки в развертки EvolventNum для y, <0 - существует
  */
  virtual double ZeroConstraintCalc(const double* _y, int EvolventNum = 0) = 0;
};

#endif
// - end of file ----------------------------------------------------------------------------------
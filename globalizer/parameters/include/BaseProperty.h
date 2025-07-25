/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2016 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      BaseProperty.h                                              //
//                                                                         //
//  Purpose:   Header file for random generator class                      //
//                                                                         //
//  Author(s): Lebedev I.                                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __BASE_PROPERTY_H__
#define __BASE_PROPERTY_H__

/**
\file BaseProperty.h

\authors ������� �.
\date 2015-2016
\copyright ���� ��. �.�. ������������

\brief ���������� ������ ������� ��� �������

*/

#include "property.h"

//

/* ======================================================================== *\
**  ���������� �������                                                      **
\* ======================================================================== */


/**
������� ����� ������� ���������
*/
template <class Owner>
class TBaseProperty : public IBaseValueClass
{
protected:

  /// ��� ������ ������� �������� ������, ��������� ����� �������� ���������� ��� ������
  typedef int (Owner::*tCheckValue)(int);

  /// �������� �������
  Owner* mOwner;
  /// ����������� �� �������� ��������� �� ���������� ������� ��� ������ �����
  bool mIsReadValue;
  /// �������� �������� �� �������������
  bool mIsPreChange;
public:
  /// ���������� ��������� ����������� ���� ��������
  virtual void operator = (TBaseProperty<Owner>& data);
  /// ���������� ��������� ����������� ��� ���������
  virtual void operator = (TBaseProperty<Owner>* data);

  /// ������� ������ data
  virtual void Clone(TBaseProperty<Owner>** data) = 0;

  TBaseProperty(Owner* owner) : mOwner(owner), mIsReadValue(false), mIsPreChange(false) {}
  virtual ~TBaseProperty() {}

  /** ������������� ��������
  \param[in] owner - ����� �������� ��������
  \param[in] checkMethod - ����� �������� ������������ ��������� ������
  \param[in] index - ����� ��������
  \param[in] separator - ����������� ��������� �������
  \param[in] size - ������ ������� ��������, ��� ����� ������ �� ����������� �������� - ������ ����� 1
  \param[in] name - ��� ��������
  \param[in] help - ��������� �� ������� �������
  \param[in] link - �������� ������ ��� �������
  \param[in] defValue - �������� �� ���������
  */
  virtual void InitializationParameterProperty(Owner * owner,
    tCheckValue checkMethod, int index, std::string separator, int size, std::string name,
    std::string help, std::string link, std::string defValue) = 0;

  /// ���������� ����������� �� �������� ��������� �� ���������� ������� ��� ������ �����
  virtual bool GetIsReadValue()
  {
    return mIsReadValue;
  }
  /// ������ ����������� �� �������� ��������� �� ���������� ������� ��� ������ �����
  virtual void SetIsReadValue(bool isReadValue)
  {
    mIsReadValue = isReadValue;
  }

  /// ������ ��� �������� ���� �������� �� �������������
  virtual void SetIsPreChange(bool val)
  {
    mIsPreChange = val;
  }

  /// ���� �� �������� �������� �� �������������
  virtual bool IsPreChange()
  {
    return mIsPreChange;
  }


};



/* ======================================================================== *\
**  ���������� ������� ������     TBaseProperty                                 **
\* ======================================================================== */

// ------------------------------------------------------------------------------------------------
///���������� ��������� ���������� ���� ��������
template <class Owner>
void TBaseProperty<Owner>::operator = (TBaseProperty<Owner>& data)
{
  mOwner = data.mOwner;
}

// ------------------------------------------------------------------------------------------------
/// ���������� ��������� ���������� ��� ���������
template <class Owner>
void TBaseProperty<Owner>::operator = (TBaseProperty<Owner>* data)
{
  mOwner = data->mOwner;
}

#endif //__BASE_PROPERTY_H__
// - end of file ----------------------------------------------------------------------------------

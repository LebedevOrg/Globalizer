/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2016 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      property.h                                                  //
//                                                                         //
//  Purpose:   Header file for random generator class                      //
//                                                                         //
//  Author(s): Lebedev I.                                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_WARNINGS

#ifndef __PROPERTY_H__
#define __PROPERTY_H__

/**
\file property.h

\authors ������� �.
\date 2015-2016
\copyright ���� ��. �.�. ������������

\brief ���������� ������ ������� ��� �������

*/

#include <string>

//

/* ======================================================================== *\
**  ���������� �������                                                      **
\* ======================================================================== */


/**
�������� �������� ������ �������� ��� � ��������
*/

class IBaseValueClass
{
public:

  /// �������� ������ �� ��������� � ���� ������
  virtual void Copy(void* data) = 0;
  /// ������ ������ ������� void* � ���� �������
  virtual void SetValue(void* data) = 0;
  /// ���������� ��������� �� ������ ���������� � �������
  virtual void* GetValue() = 0;
  /// �������� � ������
  virtual std::string ToString() = 0;
  /// ��������� �������� �� ������
  virtual void FromString(std::string val) = 0;

  virtual ~IBaseValueClass() {}

  /// ������ ����������� ��������� �������
  virtual void SetSeparator(std::string separator) = 0;
  /// ���������� ����������� ��������� �������
  virtual std::string GetSeparator() = 0;

  /// ������ ������ ������� ���������
  virtual void SetSize(int size) = 0;
  /// ���������� ������ ������� ���������
  virtual int GetSize() = 0;

  /// ��� ��������
  virtual std::string GetName() = 0;
  /// ����� �������
  virtual std::string GetHelp() = 0;
  /// �������� ��� ��� ��������� ������
  virtual std::string GetLink() = 0;

  /// ��� ��������
  virtual void SetName(std::string name) = 0;
  /// ����� �������
  virtual void SetHelp(std::string help) = 0;
  /// �������� ��� ��� ��������� ������
  virtual void SetLink(std::string link) = 0;
  /// ���������� ������� �� ��������
  virtual std::string GetHelpString() = 0;
  /// ���������� �������� ��������� ���������
  virtual std::string GetCurrentStringValue() = 0;
  /// ��������� �� ��� ��������� �� ��������� �������
  virtual bool IsNameEqual(std::string name) = 0;

  /// ���������� ����������� �� �������� ��������� �� ���������� ������� ��� ������ �����
  virtual bool GetIsReadValue() = 0;
  /// ������ ����������� �� �������� ��������� �� ���������� ������� ��� ������ �����
  virtual void SetIsReadValue(bool isReadValue) = 0;

  /// �������� �� �������� ������, � �������� �������� �� ���������.
  virtual bool IsFlag() = 0;
  /// ��������� ������������ ��������
  virtual int CheckValue() = 0;

  /// ������ ��� �������� ���� �������� �� �������������
  virtual void SetIsPreChange(bool val) = 0;
  /// ���� �� �������� �������� �� �������������
  virtual bool IsPreChange() = 0;

  /// ����� �� ��������������� (������� �� ������������)
  bool mIsEdit = false;
};

#endif //__TYPES_H__
// - end of file ----------------------------------------------------------------------------------

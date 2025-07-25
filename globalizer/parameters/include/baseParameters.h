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

#ifndef __BASE_PARAMETERS_H__
#define __BASE_PARAMETERS_H__

/**
\file baseParameters.h

\authors ������� �.
\date 2015-2016
\copyright ���� ��. �.�. ������������

\brief ���������� ������ ������� ��� �������

*/

#include "types.h"
#include "pugixml.hpp"
#include "CombinableBaseParameters.h"

/**
������� ����� ����������
��� �������� ���������� ���������� �������������� �������� OWNER_NAME
� � ������������ ������ mOwner
(mOwner = this;)
*/
template <class Owner>
class TBaseParameters : public TCombinableBaseParameters
{
#undef OWNER_NAME
#define OWNER_NAME Owner

protected:
  /// ������ ������� ������������ � ������ �������
  IBaseValueClass** mOtherOptions;
  /// ������ ������� ������������ � ���� ������
  TBaseProperty<Owner>** mOptions;
  /// ������ ������� ������������ � ���� ������ ����������� � �������� ����
  IBaseValueClass** mBaseTypeOptions;
  /// ���������� �����
  int mOptionsCount;
  /// ���������� ����� �� ������ �������
  int mOtherOptionsCount;
  /// ������������ ������ ������� ����������
  int mOptionsSize;
  /// ������������ ������ ������� ����������  ������������ � ������ �������
  int mOtherOptionsSize;

  /// �������� �������
  bool mIsPrintHelp;
  /// ������� ��������� ��������� ������
  bool mIsHaveArguments;
  /// �������� ����� ������
  Owner* mOwner;
  /// ������������������� �� ���������
  bool mIsInit;
  /// ���� �� ��������� �� ���������������� �����
  std::string mConfigPath;

  /// �������� ������������ ����� ��������� ������ ����������
  virtual int CheckValue(int index = -1);

  /**
  ������������� ���������
  \param[in] option - �������� ������� ��������������
  \param[in] sizeVal - ������ ������� ��������, ��� ����� ������ �� ����������� �������� - ������ ����� 1
  \param[in] name - ��� ��������
  \param[in] help - ��������� �� ������� �������
  \param[in] com - �������� ������ ��� �������
  \param[in] defVal - �������� �� ���������
  */
  virtual void InitializationOption(TBaseProperty<Owner>* option, std::string name, std::string defVal,
    std::string com, std::string help, int sizeVal);
  /// ��������� ����� � ����� ������
  virtual void AddOption(TBaseProperty<Owner>* option);
  /// ������� �������� �� ��������� ������� ����������
  virtual void SetBaseDefaultParameters();
  /**
  ������� �������� �� ��������� ��� ���� ����������
  ������:
  InitOption(��� ���������, �������� �� ���������, "�������� �������", "������� �� ���������", ���-�� ���������);
  *���-�� ��������� ��� �� �������� ������ ����� 1.
  InitOption(Separator,_, "-Separator", "eparator", 1);
  */
  virtual void SetDefaultParameters() = 0;
  /// ������ ���������� �� ����� ConfigPath
  virtual void ReadConfigFile();
  /// ������ ���������� ��������� ������
  virtual void ReadParameters(int argc, char* argv[]);
  /// ������ ����������� ������� ��� ���� �����
  void SetSeparator();


public:
  ///�������� ��� ��� ������� ��� ������ ��������� ������
  TBool<TBaseParameters<Owner>> IsPrintHelpWithoutArguments;
  /// ��������� �� ��� ������ ��������� ������
  TBool<TBaseParameters<Owner>> IsStartWithoutArguments;
  /// ����������� ��������� �������
  TString<TBaseParameters<Owner>> Separator;
  /// ���� �� ������ ����� ���������
  TString<TBaseParameters<Owner>> ConfigPath;
  /// ������ ������� �� ����������
  void PrintHelp();
  /// ������ ������� �������� ����������
  void PrintParameters();
  /// ��������� �� ������ ���������
  bool IsStart();

  /**
  �������� ������������ ��� ��������� ����������
  ��� ��������������� ���������� ������� ������� ����� �������� ������!
  */
  virtual int CheckValueParameters(int index = 0);

  /// ������ �������� ��������� � ������ name
  void PrintParameter(std::string name);
  /// ������� ��������� � ������ name �������� val
  void SetVal(std::string name, std::string val);
  /// ������� ��������� � ������ name �������� val
  void SetVal(std::string name, void* val);
  /// ���������� ������ � ��������� ��������� � ������ name
  std::string GetStringVal(std::string name);
  /// ���������� �������� ��������� � ������ name
  void* GetVal(std::string name);

  /**
  ������������� ���������
  \param[in] pt - ��� ���������
  \param[in] sizeVal - ������ ������� ��������, ��� ����� ������ �� ����������� �������� - ������ ����� 1
  \param[in] name - ��� ��������
  \param[in] help - ��������� �� ������� �������
  \param[in] com - �������� ������ ��� �������
  \param[in] defVal - �������� �� ���������
  */
  virtual void AddOption(EParameterType pt, std::string name, std::string defVal,
    std::string com, std::string help, int sizeVal);

  /// ���������� ��� ��������� ���� ������� � �����
  virtual void CombineOptions(IBaseValueClass** otherOptions, int count);
  /// ���������� ��������� ��������
  virtual IBaseValueClass** GetOptions();
  /// ���������� �������������� �������� ������ �� ������ ������� ����������
  virtual IBaseValueClass** GetOtherOptions();
  /// ���������� ���������� �����
  virtual int GetOptionsCount();
  /// ���������� ���������� �������������� ������� ������ �� ������ ������� ����������
  virtual int GetOtherOptionsCount();

  /// ������������� ����������
  virtual void Init(int argc, char* argv[], bool isMPIInit = false);
  TBaseParameters();
  TBaseParameters(TBaseParameters& _parameters);
  virtual ~TBaseParameters();
  /// �������� �� ����� �������
  virtual bool IsProblem();
};

// ------------------------------------------------------------------------------------------------
/// �������� ������������
template <class Owner>
int TBaseParameters<Owner>::CheckValue(int index)
{

  if (Separator.ToString().length() < 1)
  {
    Separator = std::string("_");
  }

  return 0;
}

// ------------------------------------------------------------------------------------------------
/// ������������� ���������
template <class Owner>
void TBaseParameters<Owner>::InitializationOption(TBaseProperty<Owner>* option, std::string name, std::string defVal,
  std::string com, std::string help, int sizeVal)
{
  option->InitializationParameterProperty(mOwner, &Owner::CheckValueParameters, mOptionsCount, Separator, sizeVal, name, help, com, defVal);
  AddOption(option);

  if (IsProblem())
    option->mIsEdit = true;
}

// ------------------------------------------------------------------------------------------------
/// ��������� ����� � ����� ������
template <class Owner>
void TBaseParameters<Owner>::AddOption(TBaseProperty<Owner>* option)
{
  for (int i = 0; i < mOptionsCount; i++)
    if (mOptions[i]->IsNameEqual(option->GetName()))
      return;

  mOptions[mOptionsCount] = option;
  mBaseTypeOptions[mOptionsCount] = (IBaseValueClass*)option;
  mOptionsCount++;


  if (mOptionsCount >= mOptionsSize)
  {
    TBaseProperty<Owner>** bufOptions = new TBaseProperty<Owner>*[mOptionsSize];
    for (int i = 0; i < mOptionsSize; i++)
    {
      bufOptions[i] = mOptions[i];
    }

    delete[] mOptions;
    delete[] mBaseTypeOptions;

    mOptions = new TBaseProperty<Owner>*[mOptionsSize * 2];
    mBaseTypeOptions = new IBaseValueClass*[mOptionsSize * 2];

    for (int i = 0; i < mOptionsSize * 2; i++)
    {
      mOptions[i] = 0;
      mBaseTypeOptions[i] = 0;
    }

    for (int i = 0; i < mOptionsSize; i++)
    {
      mOptions[i] = bufOptions[i];
      mBaseTypeOptions[i] = (IBaseValueClass*)mOptions[i];
    }
    mOptionsSize = mOptionsSize * 2;

    delete[] bufOptions;
  }
}

// ------------------------------------------------------------------------------------------------
/// ������� �������� �� ��������� ������� ����������
template <class Owner>
void TBaseParameters<Owner>::SetBaseDefaultParameters()
{
  int sizeVal = 1;

  Separator.InitializationParameterProperty(this, &TBaseParameters::CheckValue, mOptionsCount, Separator, sizeVal,
    "Separator", "eparator", "-Separator", "_");
  AddOption((TBaseProperty<Owner>*)(&Separator));

  IsPrintHelpWithoutArguments.InitializationParameterProperty(this, &TBaseParameters::CheckValue, mOptionsCount, Separator, sizeVal,
    "IsPrintHelpWithoutArguments", "Is print help without console arguments", "-PHWA", "false");
  AddOption((TBaseProperty<Owner>*)(&IsPrintHelpWithoutArguments));

  IsStartWithoutArguments.InitializationParameterProperty(this, &TBaseParameters::CheckValue, mOptionsCount, Separator, sizeVal,
    "IsStartWithoutArguments", "Is start without console arguments", "-SWA", "true");
  AddOption((TBaseProperty<Owner>*)(&IsStartWithoutArguments));

  ConfigPath.InitializationParameterProperty(this, &TBaseParameters::CheckValue, mOptionsCount, Separator, sizeVal,
    "ConfigPath", "The path to the configuration file of the program", "-CP", mConfigPath);
  AddOption((TBaseProperty<Owner>*)(&ConfigPath));

}

// ------------------------------------------------------------------------------------------------
/// ������ ���������� �� ����� ConfigPath
template <class Owner>
void TBaseParameters<Owner>::ReadConfigFile()
{
  if (mConfigPath != "")
    ConfigPath = mConfigPath;
  if (ConfigPath.operator std::string() != "")
  {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(ConfigPath.ToString().c_str());
    if (result.status != pugi::status_ok)
      return;
    pugi::xml_node config = doc.child("config");
    for (pugi::xml_node iter = config.first_child(); iter != 0; iter = iter.next_sibling())
    {
      std::string name = iter.name();
      for (int i = 0; i < mOptionsCount; i++)
      {
        if (mOptions[i]->IsNameEqual(name))
        {
          if (mOptions[i]->mIsEdit)
          {
            if (!mOptions[i]->IsFlag())
            {
              std::string value = iter.child_value();
              mOptions[i]->FromString(value);
              mOptions[i]->SetIsReadValue(true);
              break;

            }
            else
            {
              mOptions[i]->FromString("1");
              mOptions[i]->SetIsReadValue(true);
              break;
            }
          }
        }
      }
    }
  }
}

// ------------------------------------------------------------------------------------------------
/// ������ ���������� ��������� ������
template <class Owner>
void TBaseParameters<Owner>::ReadParameters(int argc, char* argv[])
{
  for (int i = 1; i < argc; i++)
  {
    std::string argument = argv[i];
    for (int j = 0; j < mOptionsCount; j++)
    {
      if (mOptions[j]->IsNameEqual(argument))
      {
        if (!mOptions[j]->IsPreChange())
        {
          if (mOptions[j]->mIsEdit)
          {
            if (!mOptions[j]->IsFlag())
            {
              i++;
              if (i < argc)
              {
                std::string value = argv[i];
                mOptions[j]->FromString(value);
                mOptions[j]->SetIsReadValue(true);
                break;
              }
            }
            else
            {
              mOptions[j]->FromString("1");
              mOptions[j]->SetIsReadValue(true);
              break;
            }
          }
        }
      }
    }
  }
}

// ------------------------------------------------------------------------------------------------
/// ������ ����������� ������� ��� ���� �����
template <class Owner>
void TBaseParameters<Owner>::SetSeparator()
{
  for (int i = 0; i < mOptionsCount; i++)
  {
    mOptions[i]->SetSeparator(Separator);
  }
}

// ------------------------------------------------------------------------------------------------
/// ������ ������� �� ����������
template <class Owner>
void TBaseParameters<Owner>::PrintHelp()
{
  printf("\n\nHelp:\n");
  for (int i = 0; i < mOptionsCount; i++)
  {
    if (mOptions[i]->mIsEdit)
      printf("%s\n", mOptions[i]->GetHelpString().c_str());
  }
  for (int i = 0; i < mOtherOptionsCount; i++)
  {
    if (mOtherOptions[i]->mIsEdit)
      printf("%s\n", mOtherOptions[i]->GetHelpString().c_str());
  }
}

// ------------------------------------------------------------------------------------------------
template <class Owner>
void TBaseParameters<Owner>::PrintParameters()
{
  for (int i = 0; i < mOptionsCount; i++)
  {
    if (mOptions[i]->mIsEdit)
      printf("%s\n", mOptions[i]->GetCurrentStringValue().c_str());
  }
  for (int i = 0; i < mOtherOptionsCount; i++)
  {
    if (mOtherOptions[i]->mIsEdit)
      printf("%s\n", mOtherOptions[i]->GetCurrentStringValue().c_str());
  }
}

// ------------------------------------------------------------------------------------------------
/// ��������� �� ������ ���������
template <class Owner>
bool TBaseParameters<Owner>::IsStart()
{
  if ((mIsHaveArguments == false) && (IsStartWithoutArguments == false))
    return false;
  return true;
}

// ------------------------------------------------------------------------------------------------
/**
�������� ������������ ��� ��������� ����������
��� ��������������� ���������� ������� ������� ����� �������� ������!
*/
template <class Owner>
int TBaseParameters<Owner>::CheckValueParameters(int index)
{
  if (mIsInit)
  {
    CheckValue(index);
    mIsInit = false;
    for (int i = 0; i < mOtherOptionsCount; i++)
    {
      for (int j = 0; j < mOptionsCount; j++)
      {
        if (mOtherOptions[i]->IsNameEqual(mOptions[j]->GetName()))
        {
          std::string oldBaseParameterValue = mOptions[j]->ToString();
          std::string oldOtherParameterValue = mOtherOptions[i]->ToString();

          if (oldBaseParameterValue != oldOtherParameterValue)
          {
            mOtherOptions[i]->FromString(oldBaseParameterValue);

            std::string newOtherParameterValue = mOtherOptions[i]->ToString();

            if (newOtherParameterValue == oldOtherParameterValue)
            {
              mOptions[j]->FromString(oldOtherParameterValue);
            }
          }
          break;
        }
      }
    }
    mIsInit = true;
  }
  return 0;
}

// ------------------------------------------------------------------------------------------------
/// ������ �������� ��������� � ������ name
template <class Owner>
void TBaseParameters<Owner>::PrintParameter(std::string name)
{
  for (int i = 0; i < mOptionsCount; i++)
  {
    if (mOptions[i]->IsNameEqual(name))
    {
      printf("%s\n", mOptions[i]->GetCurrentStringValue().c_str());
      break;
    }
  }
  for (int i = 0; i < mOtherOptionsCount; i++)
  {
    if (mOtherOptions[i]->IsNameEqual(name))
    {
      printf("%s\n", mOtherOptions[i]->GetCurrentStringValue().c_str());
      break;
    }
  }
}

// ------------------------------------------------------------------------------------------------
/// ������� ��������� � ������ name �������� val
template <class Owner>
void TBaseParameters<Owner>::SetVal(std::string name, std::string val)
{
  for (int i = 0; i < mOptionsCount; i++)
  {
    if (mOptions[i]->IsNameEqual(name))
    {
      mOptions[i]->FromString(val);
      break;
    }
  }

  for (int i = 0; i < mOtherOptionsCount; i++)
  {
    if (mOtherOptions[i]->IsNameEqual(name))
    {
      mOtherOptions[i]->FromString(val);
      break;
    }
  }
}

// ------------------------------------------------------------------------------------------------
/// ������� ��������� � ������ name �������� val
template <class Owner>
void TBaseParameters<Owner>::SetVal(std::string name, void* val)
{
  for (int i = 0; i < mOptionsCount; i++)
  {
    if (mOptions[i]->IsNameEqual(name))
    {
      mOptions[i]->SetValue(val);
      break;
    }
  }
  for (int i = 0; i < mOtherOptionsCount; i++)
  {
    if (mOtherOptions[i]->IsNameEqual(name))
    {
      mOtherOptions[i]->SetValue(val);
      break;
    }
  }
}

// ------------------------------------------------------------------------------------------------
/// ���������� ������ � ��������� ��������� � ������ name
template <class Owner>
std::string TBaseParameters<Owner>::GetStringVal(std::string name)
{
  for (int i = 0; i < mOptionsCount; i++)
  {
    if (mOptions[i]->IsNameEqual(name))
    {
      return mOptions[i]->ToString();
    }
  }

  for (int i = 0; i < mOtherOptionsCount; i++)
  {
    if (mOtherOptions[i]->IsNameEqual(name))
    {
      return mOtherOptions[i]->ToString();
    }
  }

  return std::string("");
}

// ------------------------------------------------------------------------------------------------
/// ���������� �������� ��������� � ������ name
template <class Owner>
void* TBaseParameters<Owner>::GetVal(std::string name)
{
  for (int i = 0; i < mOptionsCount; i++)
  {
    if (mOptions[i]->IsNameEqual(name))
    {
      return mOptions[i]->GetValue();
    }
  }

  for (int i = 0; i < mOtherOptionsCount; i++)
  {
    if (mOtherOptions[i]->IsNameEqual(name))
    {
      return mOtherOptions[i]->GetValue();
    }
  }

  return NULL;
}

// ------------------------------------------------------------------------------------------------
/**
������������� ���������
\param[in] pt - ��� ���������
\param[in] sizeVal - ������ ������� ��������, ��� ����� ������ �� ����������� �������� - ������ ����� 1
\param[in] name - ��� ��������
\param[in] help - ��������� �� ������� �������
\param[in] com - �������� ������ ��� �������
\param[in] defVal - �������� �� ���������
*/
template <class Owner>
void TBaseParameters<Owner>::AddOption(EParameterType pt, std::string name, std::string defVal,
  std::string com, std::string help, int sizeVal)
{
  TBaseProperty<Owner>* option = 0;

  if (pt == Pint)
    option = new TInt<Owner>();
  else if (pt == Pdouble)
    option = new TDouble<Owner>();
  else if (pt == Pstring)
    option = new TString<Owner>();
  else if (pt == PETypeMethod)
    option = new TETypeMethod<Owner>();
  else if (pt == PETypeProcess)
    option = new TETypeProcess<Owner>();
  else if (pt == PETypeCalculation)
    option = new TETypeCalculation<Owner>();
  else if (pt == PELocalMethodScheme)
    option = new TELocalMethodScheme<Owner>();
  else if (pt == PESeparableMethodType)
    option = new TESeparableMethodType<Owner>();
  else if (pt == PEStopCondition)
    option = new TEStopCondition<Owner>();
  else if (pt == Pbool)
    option = new TBool<Owner>();
  else if (pt == Pints)
    option = new TInts<Owner>();
  else if (pt == Pdoubles)
    option = new TDoubles<Owner>();
  else if (pt == PEMapType)
    option = new TEMapType<Owner>();

  option->InitializationParameterProperty(mOwner, 0, mOptionsCount, Separator, sizeVal,
    name, help, com, defVal);
  AddOption(option);
}

// ------------------------------------------------------------------------------------------------
/// ���������� ��� ��������� ���� ������� � �����
template <class Owner>
void TBaseParameters<Owner>::CombineOptions(IBaseValueClass** otherOptions, int count)
{
  int newOtherOptionsCount = mOtherOptionsCount + count;
  if (newOtherOptionsCount >= mOtherOptionsSize)
  {
    IBaseValueClass** bufOptions = new IBaseValueClass*[mOtherOptionsSize];
    for (int i = 0; i < mOtherOptionsSize; i++)
    {
      bufOptions[i] = mOtherOptions[i];
    }

    delete[] mOtherOptions;

    mOtherOptions = new IBaseValueClass*[mOtherOptionsSize * 2];

    for (int i = 0; i < mOtherOptionsSize * 2; i++)
    {
      mOtherOptions[i] = 0;
    }

    for (int i = 0; i < mOtherOptionsSize; i++)
    {
      mOtherOptions[i] = bufOptions[i];
    }
    mOtherOptionsSize = mOtherOptionsSize * 2;

    delete[] bufOptions;
  }

  for (int i = 0; i < count; i++)
  {
    mOtherOptions[mOtherOptionsCount] = otherOptions[i];

    for (int j = 0; j < mOptionsCount; j++)
    {
      if (mOtherOptions[mOtherOptionsCount]->IsNameEqual(mOptions[j]->GetName()))
      {
        std::string oldBaseParameterValue = mOptions[j]->ToString();
        std::string oldOtherParameterValue = mOtherOptions[mOtherOptionsCount]->ToString();
        if (!mOptions[j]->IsPreChange() && !mOtherOptions[mOtherOptionsCount]->IsPreChange())
        {
          if (oldBaseParameterValue != oldOtherParameterValue)
          {
            mOptions[j]->FromString(oldOtherParameterValue);
            std::string newBaseParameterValue = mOptions[j]->ToString();

            if (newBaseParameterValue == oldBaseParameterValue)
            {
              mOtherOptions[mOtherOptionsCount]->FromString(oldBaseParameterValue);
            }
          }
        }
        break;
      }
    }

    mOtherOptionsCount++;
  }
}

// ------------------------------------------------------------------------------------------------
/// ���������� ��������� ��������
template <class Owner>
IBaseValueClass** TBaseParameters<Owner>::GetOptions()
{
  return mBaseTypeOptions;
}

// ------------------------------------------------------------------------------------------------
/// ���������� �������������� �������� ������ �� ������ ������� ����������
template <class Owner>
IBaseValueClass** TBaseParameters<Owner>::GetOtherOptions()
{
  return mOtherOptions;
}

// ------------------------------------------------------------------------------------------------
/// ���������� ���������� �����
template <class Owner>
int TBaseParameters<Owner>::GetOptionsCount()
{
  return mOptionsCount;
}

// ------------------------------------------------------------------------------------------------
/// ���������� ���������� �������������� ������� ������ �� ������ ������� ����������
template <class Owner>
int TBaseParameters<Owner>::GetOtherOptionsCount()
{
  return mOtherOptionsCount;
}

// ------------------------------------------------------------------------------------------------
template <class Owner>
void TBaseParameters<Owner>::Init(int argc, char* argv[], bool isMPIInit)
{
  mIsMPIInit = isMPIInit;
  mIsInit = false;
  mOptionsCount = 0;
  mOtherOptionsCount = 0;
  mOptions = 0;

  mOptionsSize = 200;
  mOtherOptionsSize = 200;

  mOptions = new TBaseProperty<Owner>*[mOptionsSize];
  mOtherOptions = new IBaseValueClass*[mOtherOptionsSize];
  mBaseTypeOptions = new IBaseValueClass*[mOptionsSize];
  for (int i = 0; i < mOptionsSize; i++)
  {
    mOptions[i] = 0;
    mBaseTypeOptions[i] = 0;
  }

  for (int i = 0; i < mOtherOptionsSize; i++)
  {
    mOtherOptions[i] = 0;
  }

  // ���������� ���� �� ��������� �������

  if (argc > 0)
  {
    mArgumentCount = argc;
    mAargumentValue = argv;
  }

  if (mArgumentCount <= 1)
    mIsHaveArguments = false;
  else
    mIsHaveArguments = true;

  // ������������� ������� ���������� �� ���������
  SetBaseDefaultParameters();
  // ������������� ������� ����������
  SetDefaultParameters();
  // ������ ����������� ��� ��������
  SetSeparator();
#ifndef EXAMIN_LITE
  // ���������� ��������� �� �������
  ReadParameters(mArgumentCount, mAargumentValue);
#endif // DEBUG

  // ���������� ��������� �� ������
  ReadConfigFile();
  // �������� ����������
  CheckValue();

  if ((mIsHaveArguments == false) && (IsPrintHelpWithoutArguments == false))
    mIsPrintHelp = false;
  else if (mIsHaveArguments == true)
    mIsPrintHelp = false;
  else
    mIsPrintHelp = true;
  mIsInit = true;

  CheckValueParameters();
}

// ------------------------------------------------------------------------------------------------
template <class Owner>
TBaseParameters<Owner>::TBaseParameters()
{
#ifdef EXAMIN_LITE
  // ���������� ��������� �� �������
  mConfigPath = "Globalizer_conf.xml";
#else
  mConfigPath = "";
#endif // DEBUG

  mOwner = 0;
  mIsInit = false;
}

// ------------------------------------------------------------------------------------------------
template <class Owner>
TBaseParameters<Owner>::TBaseParameters(TBaseParameters& _parameters) : TCombinableBaseParameters()
{
  mIsInit = false;
  mOptionsCount = 0;
  mOtherOptionsCount = 0;
  mIsMPIInit = _parameters.mIsMPIInit;
  mOptionsSize = _parameters.mOptionsSize;
  mOtherOptionsSize = _parameters.mOtherOptionsSize;
  mOwner = 0;
  mIsHaveArguments = _parameters.mIsHaveArguments;
  IsPrintHelpWithoutArguments = _parameters.IsPrintHelpWithoutArguments;
  IsStartWithoutArguments = _parameters.IsStartWithoutArguments;
  mIsPrintHelp = _parameters.mIsPrintHelp;
  mOtherOptionsCount = _parameters.mOtherOptionsCount;

  mOptions = new TBaseProperty<Owner>*[mOptionsSize];
  mOtherOptions = new IBaseValueClass*[mOtherOptionsSize];
  mBaseTypeOptions = new IBaseValueClass*[mOptionsSize];
  for (int i = 0; i < mOptionsSize; i++)
  {
    mOptions[i] = 0;

    mBaseTypeOptions[i] = 0;
  }

  for (int i = 0; i < mOtherOptionsSize; i++)
  {
    mOtherOptions[i] = 0;
  }

  // ������������� ������� ���������� �� ���������
  SetBaseDefaultParameters();

  for (int i = 0; i < mOptionsCount; i++)
  {
    *mOptions[i] = *_parameters.mOptions[i];
  }

  for (int i = 0; i < mOtherOptionsCount; i++)
  {
    mOtherOptions[i] = _parameters.mOtherOptions[i];
  }

}

// ------------------------------------------------------------------------------------------------
template <class Owner>
TBaseParameters<Owner>::~TBaseParameters()
{
  delete[] mOptions;
  mOptions = 0;
}

/// �������� �� ����� �������
template <class Owner>
bool TBaseParameters<Owner>::IsProblem()
{
  return true;
}

#endif //__BASE_PARAMETERS_H__
// - end of file ----------------------------------------------------------------------------------

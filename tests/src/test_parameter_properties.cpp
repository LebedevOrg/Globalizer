/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      properties_test.cpp                                         //
//                                                                         //
//  Purpose:   Модульные тесты для классов параметров                      //
//                                                                         //
//  Author(s): Lebedev I.                                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

/**
\file properties_test.cpp

\authors Lebedev I.
\copyright ННГУ им. Н.И. Лобачевского

\brief Модульные тесты для классов параметров

*/

#include "Parameters.h"

#include <gtest/gtest.h>
#include <string>
#include <cstdlib>

using namespace std;

int PropertiesIndex = 11;

class PropertiesTest : public ::testing::Test
{

public:

  bool boolVal;

  bool GetBool() const
  {
    return boolVal;
  }
  void SetBool(bool val)
  {
    boolVal = val;
  }

  virtual int CheckValue(int index = -1)
  {
    boolVal = !boolVal;

    return 0;
  }

  virtual void TestBody()
  {  }

  PropertiesTest()
  {
    boolVal = true;
  }
};

/**
 * Проверка класса TBool
 */
TEST(Properties_TBool, can_create_default_Bool)
{
  ASSERT_NO_THROW(TBool<PropertiesTest> a);
}

///  может создать объект TBool с заданным значением
TEST(Properties_TBool, can_create_Bool)
{
  bool val = true;
  ASSERT_NO_THROW(TBool<PropertiesTest> a(val));
}

///  инициализированное значение TBool соответствует ожидаемому
TEST(Properties_TBool, is_init_Bool_value)
{
  bool val = true;
  TBool<PropertiesTest> b(val);
  ASSERT_EQ(val, (bool)b);
}

///  работа геттера и сеттера для TBool
TEST(Properties_TBool, is_getter_and_setter_working_Bool)
{
  bool val = true;
  TBool<PropertiesTest> b(!val);

  b = val;

  ASSERT_EQ(val, (bool)b);
}

///  работа SetIndex и GetIndex для TBool
TEST(Properties_TBool, is_SetIndex_and_GetIndex_working_Bool)
{
  int val = PropertiesIndex++;
  TBool<PropertiesTest> b;

  b.SetIndex(val);

  ASSERT_EQ(val, b.GetIndex());
}

///  метод GetData для TBool возвращает корректное значение
TEST(Properties_TBool, is_Bool_GetData_working)
{
  bool val = true;
  TBool<PropertiesTest> b(val);
  ASSERT_EQ(val, b.GetData());
}

///  метод GetValue для TBool возвращает корректное значение
TEST(Properties_TBool, is_Bool_GetValue_working)
{
  bool val = true;
  TBool<PropertiesTest> b(val);
  ASSERT_EQ(val, *((bool*)b.GetValue()));
}

///  метод Clone для TBool работает корректно
TEST(Properties_TBool, is_Bool_Clone_working)
{
  bool val = true;
  TBool<PropertiesTest> b(val);
  TBool<PropertiesTest>* c;

  b.Clone((BaseProperty<PropertiesTest>**) & c);

  ASSERT_EQ(val, c->GetData());
}

///  метод GetIsChange для TBool работает корректно
TEST(Properties_TBool, is_Bool_GetIsChange_working)
{
  bool val = true;
  TBool<PropertiesTest> b(!val);

  ASSERT_EQ(false, b.GetIsChange());

  b = val;

  ASSERT_EQ(true, b.GetIsChange());
}

///  метод Copy для TBool работает корректно
TEST(Properties_TBool, is_Bool_Copy_working)
{
  bool val = true;
  TBool<PropertiesTest> b(!val);
  TBool<PropertiesTest> c(val);

  b.Copy((void*)&c);

  ASSERT_EQ(val, b.GetData());
}

///  метод GetCurrentStringValue для TBool работает корректно
TEST(Properties_TBool, is_Bool_GetCurrentStringValue_working)
{
  bool val = true;
  string name = "n";
  string result = "n = true";
  TBool<PropertiesTest> b(val);

  b.SetName(name);

  ASSERT_EQ(result, b.GetCurrentStringValue());
}

///  работа SetName и GetName для TBool
TEST(Properties_TBool, is_Bool_SetName_and_GetName_working)
{
  string name = "n";
  TBool<PropertiesTest> b;

  ASSERT_NO_THROW(b.SetName(name));

  ASSERT_EQ(name, b.GetName());
}

///  метод IsNameEqual для TBool работает корректно
TEST(Properties_TBool, is_Bool_IsNameEqual_working)
{
  string name = "n";
  TBool<PropertiesTest> b;

  ASSERT_NO_THROW(b.SetName(name));

  ASSERT_EQ(true, b.IsNameEqual(name));
}

///  метод IsFlag для TBool возвращает false
TEST(Properties_TBool, is_Bool_IsFlag_working)
{
  TBool<PropertiesTest> b;

  ASSERT_EQ(false, b.IsFlag());
}

///  работа SetIsReadValue и GetIsReadValue для TBool
TEST(Properties_TBool, is_Bool_SetIsReadValue_and_GetIsReadValue_working)
{
  TBool<PropertiesTest> b;

  ASSERT_NO_THROW(b.SetIsReadValue(true));

  ASSERT_EQ(true, b.GetIsReadValue());
}

///  работа SetIsPreChange и IsPreChange для TBool
TEST(Properties_TBool, is_Bool_SetIsPreChange_and_IsPreChange_working)
{
  TBool<PropertiesTest> b;

  ASSERT_NO_THROW(b.SetIsPreChange(true));

  ASSERT_EQ(true, b.IsPreChange());
}

///  работа SetHelp и GetHelp для TBool
TEST(Properties_TBool, is_Bool_SetHelp_and_GetHelp_working)
{
  string help = "n";
  TBool<PropertiesTest> b;

  ASSERT_NO_THROW(b.SetHelp(help));

  ASSERT_EQ(help, b.GetHelp());
}

///  работа SetLink и GetLink для TBool
TEST(Properties_TBool, is_Bool_SetLink_and_GetLink_working)
{
  string link = "n";
  TBool<PropertiesTest> b;

  ASSERT_NO_THROW(b.SetLink(link));

  ASSERT_EQ(link, b.GetLink());
}

///  метод GetHelpString для TBool работает корректно
TEST(Properties_TBool, is_Bool_GetHelpString_working)
{
  string result = "b (-b) - 'This is B' default:\ttrue";
  string link = "-b";
  string help = "This is B";
  string name = "b";
  bool val = true;
  TBool<PropertiesTest> b(val);

  b.SetLink(link);
  b.SetHelp(help);
  b.SetName(name);

  ASSERT_EQ(result, b.GetHelpString());
}

///  метод Init для TBool не вызывает исключения
TEST(Properties_TBool, is_init_function_Bool)
{
  PropertiesTest a;
  TBool<PropertiesTest> b;

  ASSERT_NO_THROW(b.Init(&a, &PropertiesTest::GetBool, &PropertiesTest::SetBool,
    &PropertiesTest::CheckValue));
}


///  метод InitializationParameterProperty для TBool работает корректно
TEST(Properties_TBool, is_InitializationParameterProperty_function_Bool)
{
  int index = 12;
  string result = "b (-b) - 'This is B' default:\ttrue";
  string link = "-b";
  string help = "This is B";
  string name = "b";
  string sep = "_";
  string defVal = "true";
  bool val = true;

  PropertiesTest a;
  TBool<PropertiesTest> b;

  ASSERT_NO_THROW(b.InitializationParameterProperty(&a, &PropertiesTest::CheckValue, index,
    sep, 1, name, help, link, defVal));

  ASSERT_EQ(val, b);
  ASSERT_EQ(link, b.GetLink());
  ASSERT_EQ(help, b.GetHelp());
  ASSERT_EQ(name, b.GetName());
  ASSERT_EQ(result, b.GetHelpString());
}

///  геттер владельца для TBool работает корректно
TEST(Properties_TBool, is_owner_getter_working_Bool)
{
  bool val = true;
  PropertiesTest a;
  TBool<PropertiesTest> b(!val);

  b.Init(&a, &PropertiesTest::GetBool, &PropertiesTest::SetBool, 0);
  a.boolVal = val;

  ASSERT_EQ(val, (bool)b);
}

///  сеттер владельца для TBool работает корректно
TEST(Properties_TBool, is_owner_setter_working_Bool)
{
  bool val = true;
  PropertiesTest a;
  TBool<PropertiesTest> b(!val);

  b.Init(&a, &PropertiesTest::GetBool, &PropertiesTest::SetBool, 0);

  a.boolVal = !val;

  b = val;

  ASSERT_EQ(val, a.boolVal);
}

///  метод CheckValue для TBool работает корректно
TEST(Properties_TBool, is_CheckValue_working_Bool)
{
  bool val = true;
  PropertiesTest a;
  TBool<PropertiesTest> b;

  b.Init(&a, &PropertiesTest::GetBool, &PropertiesTest::SetBool,
    &PropertiesTest::CheckValue);

  b = !val;

  ASSERT_EQ(val, b);
}

///  метод GetAvailableData для TBool возвращает корректное значение
TEST(Properties_TBool, is_GetAvailableData_working_Bool)
{
  bool val = true;
  PropertiesTest a;
  TBool<PropertiesTest> b(val);

  b.Init(&a, &PropertiesTest::GetBool, &PropertiesTest::SetBool, 0);

  b = !val;

  ASSERT_EQ(val, b.GetAvailableData());
}

///  работа GetGetter и SetGetter для TBool
TEST(Properties_TBool, is_GetGetter_and_SetGetter_working_Bool)
{
  TBool<PropertiesTest> b;

  ASSERT_NO_THROW(b.SetGetter(&PropertiesTest::GetBool));

  ASSERT_EQ(&PropertiesTest::GetBool, b.GetGetter());
}

///  метод GetIsHaveValue для TBool работает корректно
TEST(Properties_TBool, is_GetIsHaveValue_working_Bool)
{
  bool val = true;
  TBool<PropertiesTest> b(val);

  ASSERT_EQ(true, b.GetIsHaveValue());

  ASSERT_NO_THROW(b.SetGetter(&PropertiesTest::GetBool));

  ASSERT_EQ(false, b.GetIsHaveValue());

  ASSERT_NO_THROW(b.SetGetter(0));

  ASSERT_EQ(true, b.GetIsHaveValue());

  ASSERT_NO_THROW(b.SetSetter(&PropertiesTest::SetBool));

  ASSERT_EQ(false, b.GetIsHaveValue());
}

///  метод SetIsHaveValue для TBool работает корректно
TEST(Properties_TBool, is_SetIsHaveValue_working_Bool)
{
  bool val = true;
  PropertiesTest a;
  TBool<PropertiesTest> b(val);

  b.Init(&a, &PropertiesTest::GetBool, &PropertiesTest::SetBool,
    &PropertiesTest::CheckValue);

  a.boolVal = !val;

  ASSERT_EQ(false, b.GetIsHaveValue());

  ASSERT_EQ(!val, b);

  b.SetIsHaveValue(true);

  ASSERT_EQ(true, b.GetIsHaveValue());

  ASSERT_EQ(val, b);
}

///  работа GetSetter и SetSetter для TBool
TEST(Properties_TBool, is_GetSetter_and_SetSetter_working_Bool)
{
  TBool<PropertiesTest> b;

  ASSERT_NO_THROW(b.SetSetter(&PropertiesTest::SetBool));

  ASSERT_EQ(&PropertiesTest::SetBool, b.GetSetter());
}

///  работа SetCheckValue и GetCheckValue для TBool
TEST(Properties_TBool, is_SetCheckValue_and_GetCheckValue_working_Bool)
{
  TBool<PropertiesTest> b;

  ASSERT_NO_THROW(b.SetCheckValue(&PropertiesTest::CheckValue));

  ASSERT_EQ(&PropertiesTest::CheckValue, b.GetCheckValue());
}

///  метод CheckValue (внутренний) для TBool работает корректно
TEST(Properties_TBool, is_CheckValue__working_Bool)
{
  PropertiesTest a;
  TBool<PropertiesTest> b;
  a.boolVal = false;

  b.Init(&a, &PropertiesTest::GetBool, &PropertiesTest::SetBool, &PropertiesTest::CheckValue);

  ASSERT_NO_THROW(b.CheckValue());

  ASSERT_EQ(true, a.boolVal);
}


///  метод ToString для TBool работает корректно
TEST(Properties_TBool, is_Bool_ToString_working)
{
  string result = "true";
  bool val = true;
  TBool<PropertiesTest> b(val);

  ASSERT_EQ(result, b.ToString());
}

///  метод FromString для TBool работает корректно
TEST(Properties_TBool, is_Bool_FromString_working)
{
  bool val = true;
  string sVal = "true";
  TBool<PropertiesTest> b(!val);

  ASSERT_NO_THROW(b.FromString(sVal));

  ASSERT_EQ(val, b);
}

///  оператор присваивания строки для TBool работает корректно
TEST(Properties_TBool, is_Bool_operator_FromString_working)
{
  bool val = true;
  string sVal = "true";
  TBool<PropertiesTest> b(!val);

  b = sVal;

  ASSERT_EQ(val, b);
}

///  оператор преобразования в строку для TBool работает корректно
TEST(Properties_TBool, is_Bool_operator_ToString_working)
{
  string result = "true";
  bool val = true;
  TBool<PropertiesTest> b(val);

  ASSERT_EQ(result, (string)b);
}

/**
 * Проверка класса TFlag
 */
TEST(Properties_TFlag, can_create_default_Flag)
{
  ASSERT_NO_THROW(TFlag<PropertiesTest> a);
}

///  может создать объект TFlag с заданным значением
TEST(Properties_TFlag, can_create_Flag)
{
  bool val = true;
  ASSERT_NO_THROW(TFlag<PropertiesTest> a(val));
}

///  инициализированное значение TFlag соответствует ожидаемому
TEST(Properties_TFlag, is_init_Flag_value)
{
  bool val = true;
  TFlag<PropertiesTest> b(val);
  ASSERT_EQ(val, (bool)b);
}

///  метод IsFlag для TFlag возвращает true
TEST(Properties_TFlag, is_Flag_IsFlag_working)
{
  TFlag<PropertiesTest> b;

  ASSERT_EQ(true, b.IsFlag());
}

///  оператор присваивания строки для TFlag работает корректно
TEST(Properties_TFlag, is_Flag_operator_FromString_working)
{
  bool val = true;
  string sVal = "true";
  TFlag<PropertiesTest> b(!val);

  b = sVal;

  ASSERT_EQ(val, b);
}

///  оператор преобразования в строку для TFlag работает корректно
TEST(Properties_TFlag, is_Flag_operator_ToString_working)
{
  string result = "true";
  bool val = true;
  TFlag<PropertiesTest> b(val);

  ASSERT_EQ(result, (string)b);
}

/**
 * Проверка класса TInt
 */
TEST(Properties_TInt, can_create_default_Int)
{
  ASSERT_NO_THROW(TInt<PropertiesTest> a);
}

///  может создать объект TInt с заданным значением
TEST(Properties_TInt, can_create_Int)
{
  int val = 42;
  ASSERT_NO_THROW(TInt<PropertiesTest> a(val));
}

///  инициализированное значение TInt соответствует ожидаемому
TEST(Properties_TInt, is_init_Int_value)
{
  int val = 42;
  TInt<PropertiesTest> b(val);
  ASSERT_EQ(val, (int)b);
}

///  оператор присваивания строки для TInt работает корректно
TEST(Properties_TInt, is_Int_operator_FromString_working)
{
  int val = 17;
  string sVal = "17";
  TInt<PropertiesTest> b(val + 1);

  b = sVal;

  ASSERT_EQ(val, b);
}

///  оператор преобразования в строку для TInt работает корректно
TEST(Properties_TInt, is_Int_operator_ToString_working)
{
  int val = 17;
  string result = "17";
  TInt<PropertiesTest> b(val);

  ASSERT_EQ(result, (string)b);
}

/**
 * Проверка класса TDouble
 */
TEST(Properties_TDouble, can_create_default_Double)
{
  ASSERT_NO_THROW(TDouble<PropertiesTest> a);
}

///  может создать объект TDouble с заданным значением
TEST(Properties_TDouble, can_create_Double)
{
  double val = 17.3;
  ASSERT_NO_THROW(TDouble<PropertiesTest> a(val));
}

///  инициализированное значение TDouble соответствует ожидаемому
TEST(Properties_TDouble, is_init_Double_value)
{
  double val = 17.3;
  TDouble<PropertiesTest> b(val);
  ASSERT_EQ(val, (double)b);
}

///  оператор присваивания строки для TDouble работает корректно
TEST(Properties_TDouble, is_Double_operator_FromString_working)
{
  double val = 17.3;
  string sVal = "17.3";
  TDouble<PropertiesTest> b(val + 1);

  b = sVal;

  ASSERT_EQ(val, (double)b);
}

///  оператор преобразования в строку для TDouble работает корректно
TEST(Properties_TDouble, is_Double_operator_ToString_working)
{
  double val = 17.987654;
  string result = "17.987654";
  TDouble<PropertiesTest> b(val);

  ASSERT_EQ(result, (string)b);
}

/**
 * Проверка класса TString
 */
TEST(Properties_TString, can_create_default_String)
{
  ASSERT_NO_THROW(TString<PropertiesTest> a);
}

///  может создать объект TString с заданным значением
TEST(Properties_TString, can_create_String)
{
  string val = "abc";
  ASSERT_NO_THROW(TString<PropertiesTest> a(val));
}

///  инициализированное значение TString соответствует ожидаемому
TEST(Properties_TString, is_init_String_value)
{
  string val = "abc";
  TString<PropertiesTest> b(val);
  ASSERT_EQ(val, b.GetData());
}

/**
 * Проверка класса TStrings
 */
TEST(Properties_TStrings, can_create_default_Strings)
{
  ASSERT_NO_THROW(TStrings<PropertiesTest> a);
}

///  инициализированное значение TStrings соответствует ожидаемому
TEST(Properties_TStrings, is_init_Strings_value)
{
  string val[3] = { "abc", "def", "gih" };
  TStrings<PropertiesTest> b;
  b.SetSize(3);
  b = val;
  for (int i = 0; i < 3; i++)
    ASSERT_EQ(val[i], b.GetData()[i]);
}


///  оператор присваивания строки для TStrings работает корректно
TEST(Properties_TStrings, is_Strings_operator_FromString_working)
{
  string val = "a_b_c";
  string result[] = { "a", "b", "c" };
  TStrings<PropertiesTest> b;

  b = val;
  for (int i = 0; i < 3; i++)
    ASSERT_EQ(result[i], b.GetData()[i]);
  //ASSERT_EQ(val, b);
}

///  оператор преобразования в строку для TStrings работает корректно
TEST(Properties_TStrings, is_Strings_operator_ToString_working)
{
  string sVal[] = { "a", "b", "c" };
  string result = "a_b_c";
  TStrings<PropertiesTest> b(sVal, 3);

  ASSERT_EQ(result, (string)b);
}

/**
 * Проверка класса TInts
 */
TEST(Properties_TInts, can_create_Ints)
{
  ASSERT_NO_THROW(TInts<PropertiesTest> a);
}

///  инициализированное значение TInts соответствует ожидаемому
TEST(Properties_TInts, is_init_Ints_value)
{
  int val[3] = { 1, 2, 3 };
  TInts<PropertiesTest> b;
  b.SetSize(3);
  b = val;
  for (int i = 0; i < 3; i++)
    ASSERT_EQ(val[i], b.GetData()[i]);
}

///  оператор присваивания строки для TInts работает корректно
TEST(Properties_TInts, is_Ints_operator_FromString_working)
{
  string val = "1_2_3";
  int result[] = { 1, 2, 3 };
  TInts<PropertiesTest> b;

  b = val;
  for (int i = 0; i < 3; i++)
    ASSERT_EQ(result[i], b.GetData()[i]);
}

///  оператор преобразования в строку для TInts работает корректно
TEST(Properties_TInts, is_Ints_operator_ToString_working)
{
  int sVal[] = { 1, 2, 3 };
  string result = "1_2_3";
  TInts<PropertiesTest> b(sVal, 3);

  ASSERT_EQ(result, (string)b);
}

/**
 * Проверка класса TDoubles
 */
TEST(Properties_TDoubles, can_create_Doubles)
{
  ASSERT_NO_THROW(TDoubles<PropertiesTest> a);
}

///  инициализированное значение TDoubles соответствует ожидаемому
TEST(Properties_TDoubles, is_init_Doubles_value)
{
  double val[3] = { 1.1, 2.30, 3.54 };
  ASSERT_NO_THROW(TDoubles<PropertiesTest> a);
  TDoubles<PropertiesTest> b;
  b.SetSize(3);
  b = val;
  for (int i = 0; i < 3; i++)
    ASSERT_EQ(val[i], b.GetData()[i]);
}


///  оператор присваивания строки для TDoubles работает корректно
TEST(Properties_TDoubles, is_TDoubles_operator_FromString_working)
{
  string val = "1.1_2.2_3.3";
  double result[] = { 1.1, 2.2, 3.3 };
  TDoubles<PropertiesTest> b;

  b = val;
  for (int i = 0; i < 3; i++)
    ASSERT_EQ(result[i], b.GetData()[i]);
}

///  оператор преобразования в строку для TDoubles работает корректно
TEST(Properties_TDoubles, is_TDoubles_operator_ToString_working)
{
  double sVal[] = { 1.1, 2.2, 3.3 };
  string result = "1.100000_2.200000_3.300000";
  TDoubles<PropertiesTest> b(sVal, 3);

  ASSERT_EQ(result, (string)b);
}

/**
 * Проверка класса TETypeMethod
 */
TEST(Properties_TETypeMethod, can_create_default_ETypeMethod)
{
  ASSERT_NO_THROW(TETypeMethod<PropertiesTest> a);
}


/**
 * Проверка класса TESeparableMethodType
 */
TEST(Properties_TESeparableMethodType, can_create_default_ESeparableMethodType)
{
  ASSERT_NO_THROW(TESeparableMethodType<PropertiesTest> a);
}

///  может создать объект TESeparableMethodType с заданным значением
TEST(Properties_TESeparableMethodType, can_create_ESeparableMethodType)
{
  ESeparableMethodType val = GridSearch;
  ASSERT_NO_THROW(TESeparableMethodType<PropertiesTest> a(val));
}

///  инициализированное значение TESeparableMethodType соответствует ожидаемому
TEST(Properties_TESeparableMethodType, is_init_ESeparableMethodType_value)
{
  ESeparableMethodType val = GridSearch;
  TESeparableMethodType<PropertiesTest> b(val);
  ASSERT_EQ(val, b);
}

/**
 * Проверка класса TELocalMethodScheme
 */
TEST(Properties_TELocalMethodScheme, can_create_default_ELocalMethodScheme)
{
  ASSERT_NO_THROW(TELocalMethodScheme<PropertiesTest> a);
}


/**
 * Проверка класса TEStopCondition
 */
TEST(Properties_TEStopCondition, can_create_default_EStopCondition)
{
  ASSERT_NO_THROW(TEStopCondition<PropertiesTest> a);
}

///  может создать объект TEStopCondition с заданным значением
TEST(Properties_TEStopCondition, can_create_EStopCondition)
{
  EStopCondition val = OptimumVicinity2;
  ASSERT_NO_THROW(TEStopCondition<PropertiesTest> a(val));
}

///  инициализированное значение TEStopCondition соответствует ожидаемому
TEST(Properties_TEStopCondition, is_init_EStopCondition_value)
{
  EStopCondition val = OptimumVicinity2;
  TEStopCondition<PropertiesTest> b(val);
  ASSERT_EQ(val, b);
}

/**
 * Проверка класса TETypeCalculation
 */
TEST(Properties_TETypeCalculation, can_create_default_ETypeCalculation)
{
  ASSERT_NO_THROW(TETypeCalculation<PropertiesTest> a);
}

///  может создать объект TETypeCalculation с заданным значением
TEST(Properties_TETypeCalculation, can_create_ETypeCalculation)
{
  ETypeCalculation val = CUDA;
  ASSERT_NO_THROW(TETypeCalculation<PropertiesTest> a(val));
}

///  инициализированное значение TETypeCalculation соответствует ожидаемому
TEST(Properties_TETypeCalculation, is_init_ETypeCalculation_value)
{
  ETypeCalculation val = CUDA;
  TETypeCalculation<PropertiesTest> b(val);
  ASSERT_EQ(val, b);
}

/**
 * Проверка класса TETypeProcess
 */
TEST(Properties_TETypeProcess, can_create_default_ETypeProcess)
{
  ASSERT_NO_THROW(TETypeProcess<PropertiesTest> a);
}


/**
 * Проверка класса TEMapType
 */
TEST(Properties_TEMapType, can_create_default_EMapType)
{
  ASSERT_NO_THROW(TEMapType<PropertiesTest> a);
}

///  метод GetData для TFlag возвращает корректное значение
TEST(Properties_TFlag, is_Flag_GetData_working)
{
  TFlag<PropertiesTest> b(true);
  ASSERT_EQ(true, b.GetData());
}

///  метод Clone для TFlag сохраняет флаговость
TEST(Properties_TFlag, is_Flag_Clone_working)
{
  TFlag<PropertiesTest> b(true);
  TFlag<PropertiesTest>* c;
  b.Clone((BaseProperty<PropertiesTest>**) & c);
  ASSERT_EQ(true, c->GetData());
  ASSERT_EQ(true, c->IsFlag());   // важно: клон тоже флаг
}

///  метод Copy для TFlag работает корректно
TEST(Properties_TFlag, is_Flag_Copy_working)
{
  TFlag<PropertiesTest> b(false);
  TFlag<PropertiesTest> c(true);
  b.Copy((void*)&c);
  ASSERT_EQ(true, b.GetData());
}

///  метод ToString для TFlag работает корректно
TEST(Properties_TFlag, is_Flag_ToString_working)
{
  TFlag<PropertiesTest> b(false);
  ASSERT_EQ("false", b.ToString());
}

///  метод FromString для TFlag работает корректно
TEST(Properties_TFlag, is_Flag_FromString_working)
{
  TFlag<PropertiesTest> b(false);
  b.FromString("true");
  ASSERT_EQ(true, (bool)b);
}

///  парсинг "1" даёт true
TEST(Properties_TBool, is_Bool_FromString_numeric_true)
{
  TBool<PropertiesTest> b(false);
  b = std::string("1");
  ASSERT_EQ(true, (bool)b);
}

///  парсинг "0" даёт false
TEST(Properties_TBool, is_Bool_FromString_numeric_false)
{
  TBool<PropertiesTest> b(true);
  b = std::string("0");
  ASSERT_EQ(false, (bool)b);
}

///  невалидная строка НЕ меняет значение (документируем поведение)
TEST(Properties_TBool, is_Bool_FromString_invalid_keeps_value)
{
  TBool<PropertiesTest> b(true);
  b = std::string("qwerty");
  ASSERT_EQ(true, (bool)b);   // значение осталось прежним

  TBool<PropertiesTest> c(false);
  c = std::string("");
  ASSERT_EQ(false, (bool)c);
}

///  отрицательное значение TInt
TEST(Properties_TInt, is_Int_negative_value)
{
  TInt<PropertiesTest> b(-123);
  ASSERT_EQ(-123, (int)b);
  ASSERT_EQ("-123", (string)b);
}

///  граничные значения int (min/max)
TEST(Properties_TInt, is_Int_limits)
{
  TInt<PropertiesTest> bmax(2147483647);
  ASSERT_EQ(2147483647, (int)bmax);

  TInt<PropertiesTest> bmin(-2147483647 - 1);
  ASSERT_EQ((-2147483647 - 1), (int)bmin);
}

///  round-trip: value -> string -> value
TEST(Properties_TInt, is_Int_roundtrip)
{
  TInt<PropertiesTest> a(98765);
  string s = (string)a;
  TInt<PropertiesTest> b(0);
  b = s;
  ASSERT_EQ((int)a, (int)b);
}

///  парсинг строки с мусором в хвосте берёт число из начала
TEST(Properties_TInt, is_Int_FromString_trailing_garbage)
{
  TInt<PropertiesTest> b(0);
  b = std::string("42abc");
  ASSERT_EQ(42, (int)b);
}

///  метод Clone / Copy / GetData для TInt
TEST(Properties_TInt, is_Int_Clone_and_Copy_working)
{
  TInt<PropertiesTest> b(7);
  TInt<PropertiesTest>* c;
  b.Clone((BaseProperty<PropertiesTest>**) & c);
  ASSERT_EQ(7, c->GetData());

  TInt<PropertiesTest> d(0);
  d.Copy((void*)&b);
  ASSERT_EQ(7, d.GetData());
}

///  ToString/FromString методы (не только операторы) для TInt
TEST(Properties_TInt, is_Int_ToString_FromString_methods)
{
  TInt<PropertiesTest> b(55);
  ASSERT_EQ("55", b.ToString());
  b.FromString("777");
  ASSERT_EQ(777, (int)b);
}

///  сравнение double через ASSERT_DOUBLE_EQ
TEST(Properties_TDouble, is_Double_value_precise)
{
  TDouble<PropertiesTest> b(3.14159265);
  ASSERT_DOUBLE_EQ(3.14159265, (double)b);
}

///  отрицательное и нулевое значение
TEST(Properties_TDouble, is_Double_negative_and_zero)
{
  TDouble<PropertiesTest> b(-2.5);
  ASSERT_DOUBLE_EQ(-2.5, (double)b);

  TDouble<PropertiesTest> z(0.0);
  ASSERT_EQ("0.000000", (string)z);   // формат %lf
}

///  ToString использует формат %lf (6 знаков после точки)
TEST(Properties_TDouble, is_Double_ToString_format)
{
  TDouble<PropertiesTest> b(1.5);
  ASSERT_EQ("1.500000", (string)b);   // фиксируем поведение sprintf %lf
}

///  парсинг научной нотации
TEST(Properties_TDouble, is_Double_FromString_scientific)
{
  TDouble<PropertiesTest> b(0);
  b = std::string("1.5e3");
  ASSERT_DOUBLE_EQ(1500.0, (double)b);
}

///  round-trip double (с учётом потери точности до 6 знаков)
TEST(Properties_TDouble, is_Double_roundtrip)
{
  TDouble<PropertiesTest> a(12.345678);
  string s = (string)a;
  TDouble<PropertiesTest> b(0);
  b = s;
  ASSERT_NEAR((double)a, (double)b, 1e-6);
}

///  Clone / Copy для TDouble
TEST(Properties_TDouble, is_Double_Clone_and_Copy_working)
{
  TDouble<PropertiesTest> b(9.81);
  TDouble<PropertiesTest>* c;
  b.Clone((BaseProperty<PropertiesTest>**) & c);
  ASSERT_DOUBLE_EQ(9.81, c->GetData());
}

///  оператор преобразования TString в строку
TEST(Properties_TString, is_String_operator_ToString_working)
{
  string val = "hello";
  TString<PropertiesTest> b(val);
  ASSERT_EQ(val, (string)b);
}

///  пустая строка
TEST(Properties_TString, is_String_empty_value)
{
  TString<PropertiesTest> b;
  ASSERT_EQ("", (string)b);
}

///  строка с разделителем и пробелами хранится как есть
TEST(Properties_TString, is_String_with_special_chars)
{
  string val = "a_b c";
  TString<PropertiesTest> b(val);
  ASSERT_EQ(val, b.GetData());
}

///  Clone / Copy для TString
TEST(Properties_TString, is_String_Clone_and_Copy_working)
{
  TString<PropertiesTest> b("abc");
  TString<PropertiesTest>* c;
  b.Clone((BaseProperty<PropertiesTest>**) & c);
  ASSERT_EQ("abc", c->GetData());

  TString<PropertiesTest> d;
  d.Copy((void*)&b);
  ASSERT_EQ("abc", d.GetData());
}

///  Name / Help / Link для TString (общий интерфейс)
TEST(Properties_TString, is_String_name_help_link_working)
{
  TString<PropertiesTest> b("x");
  b.SetName("s");
  b.SetHelp("help");
  b.SetLink("-s");
  ASSERT_EQ("s", b.GetName());
  ASSERT_EQ("help", b.GetHelp());
  ASSERT_EQ("-s", b.GetLink());
}


///  SetSize с нулём/отрицательным значением не меняет массив
TEST(Properties_TInts, is_Ints_SetSize_zero_ignored)
{
  int val[3] = { 1, 2, 3 };
  TInts<PropertiesTest> b(val, 3);
  b.SetSize(0);
  ASSERT_EQ(3, b.GetSize());   // размер не изменился
  b.SetSize(-5);
  ASSERT_EQ(3, b.GetSize());
}

///  SetSize с увеличением заполняет новые элементы нулями и сохраняет старые
TEST(Properties_TInts, is_Ints_SetSize_grow_preserves_data)
{
  int val[2] = { 10, 20 };
  TInts<PropertiesTest> b(val, 2);
  b.SetSize(4);
  ASSERT_EQ(4, b.GetSize());
  ASSERT_EQ(10, b.GetData()[0]);
  ASSERT_EQ(20, b.GetData()[1]);
  ASSERT_EQ(0, b.GetData()[2]);
  ASSERT_EQ(0, b.GetData()[3]);
}

///  SetSize с уменьшением усекает массив
TEST(Properties_TInts, is_Ints_SetSize_shrink)
{
  int val[4] = { 1, 2, 3, 4 };
  TInts<PropertiesTest> b(val, 4);
  b.SetSize(2);
  ASSERT_EQ(2, b.GetSize());
  ASSERT_EQ(1, b.GetData()[0]);
  ASSERT_EQ(2, b.GetData()[1]);
}

///  оператор индексации TInts работает
TEST(Properties_TInts, is_Ints_operator_index_working)
{
  int val[3] = { 7, 8, 9 };
  TInts<PropertiesTest> b(val, 3);
  ASSERT_EQ(7, b[0]);
  ASSERT_EQ(9, b[2]);
}

///  индексация за границей создаёт элемент (документируем поведение Indexer)
TEST(Properties_TInts, is_Ints_operator_index_out_of_range)
{
  TInts<PropertiesTest> b;   // пустой, mValue == 0
  ASSERT_NO_THROW(b[100]);   // Indexer вызовет SetSize(1) и вернёт [0]
  ASSERT_EQ(1, b.GetSize());
}

///  парсинг одиночного элемента без разделителя
TEST(Properties_TInts, is_Ints_FromString_single_element)
{
  string val = "42";
  TInts<PropertiesTest> b;
  b = val;
  ASSERT_EQ(1, b.GetSize());
  ASSERT_EQ(42, b.GetData()[0]);
}

///  ToString пустого массива
TEST(Properties_TInts, is_Ints_ToString_empty)
{
  TInts<PropertiesTest> b;
  ASSERT_EQ("", (string)b);
}

///  round-trip: value -> string -> value для TInts
TEST(Properties_TInts, is_Ints_roundtrip)
{
  int val[3] = { 5, 6, 7 };
  TInts<PropertiesTest> a(val, 3);
  string s = (string)a;              // "5_6_7"
  TInts<PropertiesTest> b;
  b = s;
  ASSERT_EQ(3, b.GetSize());
  for (int i = 0; i < 3; i++)
    ASSERT_EQ(val[i], b.GetData()[i]);
}

///  SetSize grow для TStrings заполняет пустыми строками
TEST(Properties_TStrings, is_Strings_SetSize_grow_fills_empty)
{
  string val[1] = { "a" };
  TStrings<PropertiesTest> b(val, 1);
  b.SetSize(3);
  ASSERT_EQ(3, b.GetSize());
  ASSERT_EQ("a", b.GetData()[0]);
  ASSERT_EQ("", b.GetData()[1]);
  ASSERT_EQ("", b.GetData()[2]);
}

///  round-trip для TStrings
TEST(Properties_TStrings, is_Strings_roundtrip)
{
  string val[3] = { "x", "y", "z" };
  TStrings<PropertiesTest> a(val, 3);
  string s = (string)a;              // "x_y_z"
  TStrings<PropertiesTest> b;
  b = s;
  ASSERT_EQ(3, b.GetSize());
  for (int i = 0; i < 3; i++)
    ASSERT_EQ(val[i], b.GetData()[i]);
}

///  оператор индексации TStrings
TEST(Properties_TStrings, is_Strings_operator_index_working)
{
  string val[2] = { "hi", "bye" };
  TStrings<PropertiesTest> b(val, 2);
  ASSERT_EQ("hi", b[0]);
  ASSERT_EQ("bye", b[1]);
}

///  ToString для TDoubles одного элемента
TEST(Properties_TDoubles, is_Doubles_ToString_single)
{
  double val[1] = { 2.5 };
  TDoubles<PropertiesTest> b(val, 1);
  ASSERT_EQ("2.500000", (string)b);
}

///  round-trip для TDoubles
TEST(Properties_TDoubles, is_Doubles_roundtrip)
{
  double val[3] = { 1.1, 2.2, 3.3 };
  TDoubles<PropertiesTest> a(val, 3);
  string s = (string)a;
  TDoubles<PropertiesTest> b;
  b = s;
  ASSERT_EQ(3, b.GetSize());
  for (int i = 0; i < 3; i++)
    ASSERT_NEAR(val[i], b.GetData()[i], 1e-6);
}

///  ToString для всех значений TETypeMethod
TEST(Properties_TETypeMethod, is_ETypeMethod_ToString_all)
{
  ASSERT_EQ("StandartMethod", (string)TETypeMethod<PropertiesTest>(StandartMethod));
  ASSERT_EQ("IntegerMethod", (string)TETypeMethod<PropertiesTest>(IntegerMethod));
  ASSERT_EQ("RSAMethod", (string)TETypeMethod<PropertiesTest>(RSAMethod));
}

///  FromString по имени для TETypeMethod
TEST(Properties_TETypeMethod, is_ETypeMethod_FromString_byName)
{
  TETypeMethod<PropertiesTest> b;
  b = std::string("RSAMethod");
  ASSERT_EQ(RSAMethod, (ETypeMethod)b);
}

///  FromString по числовому коду для TETypeMethod
TEST(Properties_TETypeMethod, is_ETypeMethod_FromString_byCode)
{
  TETypeMethod<PropertiesTest> b;
  b = std::string("2");
  ASSERT_EQ(RSAMethod, (ETypeMethod)b);
}

///  невалидная строка не меняет значение TETypeMethod
TEST(Properties_TETypeMethod, is_ETypeMethod_FromString_invalid)
{
  TETypeMethod<PropertiesTest> b(IntegerMethod);
  b = std::string("nonsense");
  ASSERT_EQ(IntegerMethod, (ETypeMethod)b);
}

///  создание TETypeMethod с явным значением не бросает исключение
TEST(Properties_TETypeMethod, can_create_ETypeMethod)
{
  ETypeMethod val = RSAMethod;
  ASSERT_NO_THROW(TETypeMethod<PropertiesTest> a(val));
}

///  значение по умолчанию у TETypeMethod == StandartMethod
TEST(Properties_TETypeMethod, is_ETypeMethod_default_value)
{
  TETypeMethod<PropertiesTest> b;
  ASSERT_EQ(StandartMethod, (ETypeMethod)b);
}

///  инициализированное значение TETypeMethod соответствует ожидаемому (для каждого значения)
TEST(Properties_TETypeMethod, is_init_ETypeMethod_value)
{
  ASSERT_EQ(StandartMethod, (ETypeMethod)TETypeMethod<PropertiesTest>(StandartMethod));
  ASSERT_EQ(IntegerMethod, (ETypeMethod)TETypeMethod<PropertiesTest>(IntegerMethod));
  ASSERT_EQ(RSAMethod, (ETypeMethod)TETypeMethod<PropertiesTest>(RSAMethod));
}

///  ToString для StandartMethod
TEST(Properties_TETypeMethod, is_ETypeMethod_ToString_StandartMethod)
{
  TETypeMethod<PropertiesTest> b(StandartMethod);
  ASSERT_EQ("StandartMethod", (string)b);
}

///  ToString для IntegerMethod
TEST(Properties_TETypeMethod, is_ETypeMethod_ToString_IntegerMethod)
{
  TETypeMethod<PropertiesTest> b(IntegerMethod);
  ASSERT_EQ("IntegerMethod", (string)b);
}

///  ToString для RSAMethod
TEST(Properties_TETypeMethod, is_ETypeMethod_ToString_RSAMethod)
{
  TETypeMethod<PropertiesTest> b(RSAMethod);
  ASSERT_EQ("RSAMethod", (string)b);
}

///  FromString по имени -> StandartMethod
TEST(Properties_TETypeMethod, is_ETypeMethod_FromString_name_StandartMethod)
{
  TETypeMethod<PropertiesTest> b(RSAMethod);   // намеренно другое стартовое значение
  b = std::string("StandartMethod");
  ASSERT_EQ(StandartMethod, (ETypeMethod)b);
}

///  FromString по имени -> IntegerMethod
TEST(Properties_TETypeMethod, is_ETypeMethod_FromString_name_IntegerMethod)
{
  TETypeMethod<PropertiesTest> b(StandartMethod);
  b = std::string("IntegerMethod");
  ASSERT_EQ(IntegerMethod, (ETypeMethod)b);
}

///  FromString по имени -> RSAMethod
TEST(Properties_TETypeMethod, is_ETypeMethod_FromString_name_RSAMethod)
{
  TETypeMethod<PropertiesTest> b(StandartMethod);
  b = std::string("RSAMethod");
  ASSERT_EQ(RSAMethod, (ETypeMethod)b);
}

///  FromString по коду "0" -> StandartMethod
TEST(Properties_TETypeMethod, is_ETypeMethod_FromString_code_0)
{
  TETypeMethod<PropertiesTest> b(RSAMethod);
  b = std::string("0");
  ASSERT_EQ(StandartMethod, (ETypeMethod)b);
}

///  FromString по коду "1" -> IntegerMethod
TEST(Properties_TETypeMethod, is_ETypeMethod_FromString_code_1)
{
  TETypeMethod<PropertiesTest> b(StandartMethod);
  b = std::string("1");
  ASSERT_EQ(IntegerMethod, (ETypeMethod)b);
}

///  FromString по коду "2" -> RSAMethod
TEST(Properties_TETypeMethod, is_ETypeMethod_FromString_code_2)
{
  TETypeMethod<PropertiesTest> b(StandartMethod);
  b = std::string("2");
  ASSERT_EQ(RSAMethod, (ETypeMethod)b);
}

///  невалидная строка НЕ меняет значение (парсер молча игнорирует)
TEST(Properties_TETypeMethod, is_ETypeMethod_FromString_invalid_keeps_value)
{
  TETypeMethod<PropertiesTest> b(IntegerMethod);
  b = std::string("nonsense");
  ASSERT_EQ(IntegerMethod, (ETypeMethod)b);   // значение осталось прежним
}

///  пустая строка НЕ меняет значение
TEST(Properties_TETypeMethod, is_ETypeMethod_FromString_empty_keeps_value)
{
  TETypeMethod<PropertiesTest> b(RSAMethod);
  b = std::string("");
  ASSERT_EQ(RSAMethod, (ETypeMethod)b);
}

///  код вне диапазона ("3") НЕ меняет значение
TEST(Properties_TETypeMethod, is_ETypeMethod_FromString_out_of_range_code)
{
  TETypeMethod<PropertiesTest> b(StandartMethod);
  b = std::string("3");
  ASSERT_EQ(StandartMethod, (ETypeMethod)b);
}

///  чувствительность к регистру: "rsamethod" не распознаётся
TEST(Properties_TETypeMethod, is_ETypeMethod_FromString_case_sensitive)
{
  TETypeMethod<PropertiesTest> b(StandartMethod);
  b = std::string("rsamethod");
  ASSERT_EQ(StandartMethod, (ETypeMethod)b);   // сравнение строгое, значение не поменялось
}

///  round-trip: value -> string -> value для всех значений
TEST(Properties_TETypeMethod, is_ETypeMethod_roundtrip)
{
  ETypeMethod values[] = { StandartMethod, IntegerMethod, RSAMethod };
  for (ETypeMethod v : values)
  {
    TETypeMethod<PropertiesTest> a(v);
    string s = (string)a;                 // enum -> string
    TETypeMethod<PropertiesTest> b(StandartMethod);
    b = s;                                // string -> enum
    ASSERT_EQ(v, (ETypeMethod)b) << "roundtrip failed for value " << (int)v;
  }
}

///  GetData возвращает хранимое значение
TEST(Properties_TETypeMethod, is_ETypeMethod_GetData_working)
{
  TETypeMethod<PropertiesTest> b(RSAMethod);
  ASSERT_EQ(RSAMethod, b.GetData());
}

///  Clone создаёт копию с тем же значением
TEST(Properties_TETypeMethod, is_ETypeMethod_Clone_working)
{
  TETypeMethod<PropertiesTest> b(IntegerMethod);
  TETypeMethod<PropertiesTest>* c;
  b.Clone((BaseProperty<PropertiesTest>**) & c);
  ASSERT_EQ(IntegerMethod, c->GetData());
}

///  Copy копирует значение из другого объекта
TEST(Properties_TETypeMethod, is_ETypeMethod_Copy_working)
{
  TETypeMethod<PropertiesTest> b(StandartMethod);
  TETypeMethod<PropertiesTest> c(RSAMethod);
  b.Copy((void*)&c);
  ASSERT_EQ(RSAMethod, b.GetData());
}

///  GetIsChange корректно отражает факт изменения
TEST(Properties_TETypeMethod, is_ETypeMethod_GetIsChange_working)
{
  TETypeMethod<PropertiesTest> b(StandartMethod);
  ASSERT_EQ(false, b.GetIsChange());
  b = RSAMethod;
  ASSERT_EQ(true, b.GetIsChange());
}

///  Name / Help / Link работают
TEST(Properties_TETypeMethod, is_ETypeMethod_name_help_link_working)
{
  TETypeMethod<PropertiesTest> b(StandartMethod);
  b.SetName("tm");
  b.SetHelp("Type of method");
  b.SetLink("-tm");
  ASSERT_EQ("tm", b.GetName());
  ASSERT_EQ("Type of method", b.GetHelp());
  ASSERT_EQ("-tm", b.GetLink());
}

///  IsFlag для TETypeMethod возвращает false
TEST(Properties_TETypeMethod, is_ETypeMethod_IsFlag_working)
{
  TETypeMethod<PropertiesTest> b;
  ASSERT_EQ(false, b.IsFlag());
}

///  GetCurrentStringValue формирует "name = value"
TEST(Properties_TETypeMethod, is_ETypeMethod_GetCurrentStringValue_working)
{
  TETypeMethod<PropertiesTest> b(RSAMethod);
  b.SetName("tm");
  ASSERT_EQ("tm = RSAMethod", b.GetCurrentStringValue());
}

///  InitializationParameterProperty для TETypeMethod
TEST(Properties_TETypeMethod, is_InitializationParameterProperty_ETypeMethod)
{
  int index = 20;
  string link = "-tm";
  string help = "Type of method";
  string name = "tm";
  string sep = "_";
  string defVal = "RSAMethod";

  PropertiesTest a;
  TETypeMethod<PropertiesTest> b;

  ASSERT_NO_THROW(b.InitializationParameterProperty(&a, &PropertiesTest::CheckValue,
    index, sep, 1, name, help, link, defVal));

  ASSERT_EQ(RSAMethod, (ETypeMethod)b);   // defVal распарсился
  ASSERT_EQ(link, b.GetLink());
  ASSERT_EQ(help, b.GetHelp());
  ASSERT_EQ(name, b.GetName());
}
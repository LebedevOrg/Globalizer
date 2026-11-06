/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//  File:      test_extended.cpp                                           //
//                                                                         //
//  Purpose:   Модульные тесты класса Extended (числа расширенной          //
//             точности) в режимах etDouble и etMPFR.                       //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include <gtest/gtest.h>

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "Extended.h"

// ВАЖНО: объект Extended должен быть создан и уничтожен при одном и том же
// типе (Extended::SetTypeID), поэтому все объекты живут внутри тела теста.
class ExtendedTest : public ::testing::TestWithParam<ExtendedTypeID>
{
protected:
  double oldPrecision = 0.01;

  void SetUp() override
  {
    oldPrecision = Extended::GetPrecision();
    Extended::SetTypeID(GetParam());
    Extended::SetPrecision(1e-12);
  }

  void TearDown() override
  {
    Extended::SetTypeID(etDouble);
    Extended::SetPrecision(oldPrecision);
  }
};

TEST_P(ExtendedTest, type_id_is_set)
{
  EXPECT_EQ(Extended::GetTypeID(), GetParam());
}

TEST_P(ExtendedTest, precision_settings)
{
  EXPECT_DOUBLE_EQ(Extended::GetPrecision(), 1e-12);
  EXPECT_GT(Extended::getPrecisionReserveFactor(), 0.0f);
  EXPECT_GT(Extended::GetStringSize(), 10);
  if (GetParam() == etDouble)
    EXPECT_EQ(Extended::GetPackedSize(), (int)sizeof(double));
  else
    EXPECT_GT(Extended::GetPackedSize(), (int)sizeof(double));
}

TEST_P(ExtendedTest, constructors_and_to_double)
{
  Extended a;
  Extended b(2.5);
  Extended c("1.25");
  Extended d(b);
  EXPECT_DOUBLE_EQ(a.toDouble(), 0.0);
  EXPECT_DOUBLE_EQ(b.toDouble(), 2.5);
  EXPECT_DOUBLE_EQ(c.toDouble(), 1.25);
  EXPECT_DOUBLE_EQ(d.toDouble(), 2.5);
}

TEST_P(ExtendedTest, assignment_operators)
{
  Extended a(1.0), b(3.0);
  a = b;
  EXPECT_DOUBLE_EQ(a.toDouble(), 3.0);
  a = 4.5;
  EXPECT_DOUBLE_EQ(a.toDouble(), 4.5);
  a = "0.75";
  EXPECT_DOUBLE_EQ(a.toDouble(), 0.75);
  a = a;  // самоприсваивание
  EXPECT_DOUBLE_EQ(a.toDouble(), 0.75);
}

TEST_P(ExtendedTest, arithmetic_with_extended)
{
  Extended a(6.0), b(1.5);
  EXPECT_DOUBLE_EQ((a + b).toDouble(), 7.5);
  EXPECT_DOUBLE_EQ((a - b).toDouble(), 4.5);
  EXPECT_DOUBLE_EQ((a * b).toDouble(), 9.0);
  EXPECT_DOUBLE_EQ((a / b).toDouble(), 4.0);

  Extended c(1.0);
  c += b;  EXPECT_DOUBLE_EQ(c.toDouble(), 2.5);
  c -= b;  EXPECT_DOUBLE_EQ(c.toDouble(), 1.0);
  c *= a;  EXPECT_DOUBLE_EQ(c.toDouble(), 6.0);
}

TEST_P(ExtendedTest, arithmetic_with_double)
{
  Extended a(6.0);
  EXPECT_DOUBLE_EQ((a + 1.0).toDouble(), 7.0);
  EXPECT_DOUBLE_EQ((a - 1.0).toDouble(), 5.0);
  EXPECT_DOUBLE_EQ((a * 2.0).toDouble(), 12.0);
  EXPECT_DOUBLE_EQ((a / 4.0).toDouble(), 1.5);

  EXPECT_DOUBLE_EQ((1.0 + a).toDouble(), 7.0);
  EXPECT_DOUBLE_EQ((10.0 - a).toDouble(), 4.0);
  EXPECT_DOUBLE_EQ((2.0 * a).toDouble(), 12.0);
  EXPECT_DOUBLE_EQ((3.0 / a).toDouble(), 0.5);

  Extended c(1.0);
  c += 2.0;  EXPECT_DOUBLE_EQ(c.toDouble(), 3.0);
  c -= 0.5;  EXPECT_DOUBLE_EQ(c.toDouble(), 2.5);
  c *= 2.0;  EXPECT_DOUBLE_EQ(c.toDouble(), 5.0);
}

TEST_P(ExtendedTest, math_functions)
{
  Extended neg(-2.25), x(2.25), whole(3.0), zero(0.0);
  EXPECT_DOUBLE_EQ(fabs(neg).toDouble(), 2.25);
  EXPECT_DOUBLE_EQ(floor(x), 2.0);
  EXPECT_DOUBLE_EQ(ceil(x, zero).toDouble(), 3.0);
  EXPECT_DOUBLE_EQ(ceil(whole, zero).toDouble(), 3.0);
  EXPECT_DOUBLE_EQ(pow(whole, 2.0).toDouble(), 9.0);
  EXPECT_NEAR(root(Extended(27.0), 3), 3.0, 1e-12);
}

TEST_P(ExtendedTest, comparisons_extended_extended)
{
  Extended a(1.0), b(2.0), a2(1.0);
  EXPECT_TRUE(a == a2);
  EXPECT_FALSE(a == b);
  EXPECT_TRUE(a != b);
  EXPECT_FALSE(a != a2);
  EXPECT_TRUE(a < b);
  EXPECT_FALSE(b < a);
  EXPECT_TRUE(b > a);
  EXPECT_FALSE(a > b);
  EXPECT_TRUE(a <= a2);
  EXPECT_TRUE(a <= b);
  EXPECT_FALSE(b <= a);
  EXPECT_TRUE(b >= a);
  EXPECT_TRUE(a >= a2);
  EXPECT_FALSE(a >= b);
}

TEST_P(ExtendedTest, comparisons_with_double)
{
  Extended a(1.0);
  EXPECT_TRUE(a == 1.0);   EXPECT_TRUE(1.0 == a);
  EXPECT_TRUE(a != 2.0);   EXPECT_TRUE(2.0 != a);
  EXPECT_FALSE(a != 1.0);  EXPECT_FALSE(1.0 != a);
  EXPECT_TRUE(a < 2.0);    EXPECT_TRUE(0.5 < a);
  EXPECT_FALSE(a < 1.0);   EXPECT_FALSE(1.0 < a);
  EXPECT_TRUE(a > 0.5);    EXPECT_TRUE(2.0 > a);
  EXPECT_FALSE(a > 1.0);   EXPECT_FALSE(1.0 > a);
  EXPECT_TRUE(a <= 1.0);   EXPECT_TRUE(1.0 <= a);
  EXPECT_FALSE(a <= 0.5);  EXPECT_FALSE(2.0 <= a);
  EXPECT_TRUE(a >= 1.0);   EXPECT_TRUE(1.0 >= a);
  EXPECT_FALSE(a >= 2.0);  EXPECT_FALSE(0.5 >= a);
}

TEST_P(ExtendedTest, pack_unpack_roundtrip)
{
  Extended src(3.141592653589793);
  std::vector<char> buffer(Extended::GetPackedSize());
  src.Pack(buffer.data());

  Extended dst(0.0);
  dst.Unpack(buffer.data());
  EXPECT_TRUE(dst == src);
  EXPECT_DOUBLE_EQ(dst.toDouble(), 3.141592653589793);
}

TEST_P(ExtendedTest, to_string_for_mpfr)
{
  if (GetParam() != etMPFR)
    GTEST_SKIP() << "toString is implemented only for etMPFR";
  Extended a(1.5);
  std::vector<char> s(Extended::GetStringSize() + 1, 0);
  a.toString(s.data());
  // Десятичный разделитель у MPFR зависит от локали ("1.5" или "1,5").
  std::string str(s.data());
  for (char& ch : str)
    if (ch == ',') ch = '.';
  EXPECT_DOUBLE_EQ(std::atof(str.c_str()), 1.5) << "toString: " << s.data();
}

TEST_P(ExtendedTest, high_precision_distinguishes_close_numbers)
{
  if (GetParam() != etMPFR)
    GTEST_SKIP() << "only meaningful for etMPFR";
  Extended::SetPrecision(1e-30);
  Extended one(1.0);
  Extended tiny(1e-25);
  Extended sum = one + tiny;
  EXPECT_TRUE(sum > one);           // в double 1 + 1e-25 == 1
  EXPECT_TRUE((sum - one) > 0.0);
}

#ifdef NO_MPREAL
#define GLOBALIZER_TEST_EXTENDED_TYPES ::testing::Values(etDouble)
#else
#define GLOBALIZER_TEST_EXTENDED_TYPES ::testing::Values(etDouble, etMPFR)
#endif

INSTANTIATE_TEST_SUITE_P(Types, ExtendedTest,
  GLOBALIZER_TEST_EXTENDED_TYPES,
  [](const ::testing::TestParamInfo<ExtendedTypeID>& info)
  { return info.param == etDouble ? std::string("Double") : std::string("MPFR"); });

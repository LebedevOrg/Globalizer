#include "Evolvent.h"
#include "Extended.h"
#include "test_config.h"

#include <gtest/gtest.h>
#include <cmath>

class EvolventTest : public ::testing::Test
{
protected:
  static constexpr int numOfPoint = 5;
  static constexpr int numOfDim = 5;
  static constexpr int numOfm = 7;
  static constexpr int maxOfDim = 10;
  double y[maxOfDim];
  double A[maxOfDim];
  double B[maxOfDim];

  void SetUp()
  {
    Extended::SetTypeID(etDouble);
  }

  void TearDown() {}
  /**
   * Создает файл содержащий значения разверток, полученных с помощью функции #GetInverseImage
   * для случайных точек в фотмате
   *
   * N = 7; m = 10
   * y[0] = -0.25304422
   * ...
   * y[N-1] = -0.25304422
   * x = 0.15112305
   */
  void CreateCheckEvolventFile_GetInverseImage()
  {
    int N[numOfDim] = {1, 2, 3, 4, 5};
    int m[numOfm] = {4, 5, 6, 7, 8, 9, 10};

    Extended x;
    FILE* pf;
    pf = fopen("evolventGetInverseImage.txt","w");

    for (int t = 0; t < maxOfDim; t++)
    {
      A[t] = -0.5;
      B[t] = 0.5;
    }

    srand(0);
    for (int i = 0; i < numOfDim; i++)
    {
      for (int j = 0; j < numOfm; j++)
      {
        Evolvent evolvent(N[i], m[j]);
        evolvent.SetBounds(A, B);

        /// Записываем numOfPoint случайных точек
        for (int k = 0; k < numOfPoint; k++)
        {
          for (int p = 0; p < N[i]; p++)
          {
            y[p] = double(rand()) / RAND_MAX - 0.5;
          }
          evolvent.GetInverseImage(y, x);
          PrintToFile(pf, N[i], m[j], y, x.toDouble());
        }

        /// Проверяем граничные ситуации
        for (int p = 0; p < N[i]; p++)
        {
           y[p] = -0.5;
        }
        evolvent.GetInverseImage(y, x);
        PrintToFile(pf, N[i], m[j], y, x.toDouble());

        for (int p = 0; p < N[i]; p++)
        {
           y[p] = 0.5;
        }
        evolvent.GetInverseImage(y, x);
        PrintToFile(pf, N[i], m[j], y, x.toDouble());
      }
    }

    fclose(pf);
  }

  /**
   * Создает файл содержащий значения разверток, полученных с помощью функции #GetImage
   * для случайных точек в фотмате
   *
   * N = 7; m = 10
   * y[0] = -0.25304422
   * ...
   * y[N-1] = -0.25304422
   * x = 0.15112305
   */
  void CreateCheckEvolventFile_GetImage()
  {
    int N[numOfDim] = {1, 2, 3, 4, 5};
    int m[numOfm] = {4, 5, 6, 7, 8, 9, 10};

    FILE* pf;
    pf = fopen("evolventGetImage.txt","w");
    for (int t = 0; t < maxOfDim; t++)
    {
      A[t] = -0.5;
      B[t] = 0.5;
    }

    srand(0);

    for (int i = 0; i < numOfDim; i++)
    {
      for (int j = 0; j < numOfm; j++)
      {
        Evolvent evolvent(N[i], m[j]);
        evolvent.SetBounds(A, B);

        /// Записываем numOfPoint случайных точек
        for (int k = 0; k < numOfPoint; k++)
        {
          double x = (double(rand()) / RAND_MAX);
          evolvent.GetImage(Extended(x), y);
          PrintToFile(pf, N[i], m[j], y, x);
        }

        /// Проверяем граничные ситуации
        evolvent.GetImage(Extended(0.0), y);
        PrintToFile(pf, N[i], m[j], y, 0.0);

        evolvent.GetImage(Extended(1.0), y);
        PrintToFile(pf, N[i], m[j], y, 1.0);
      }
    }
    fclose(pf);
  }

  void PrintToFile(FILE* pf, int N, int m, double* _y, double x)
  {
    fprintf(pf, "N = %d; m = %d\n", N, m);
    for (int i = 0; i < N; i++)
    {
      fprintf(pf, "y[%d] = %.17lf\n", i, _y[i]);
    }
    fprintf(pf, "x = %.17lf\n", x);
  }

  void ReadFromFile(FILE* pf, int& N, int& m, double* _y, double& x)
  {
    char tmp[30];
    fscanf(pf, "%s%s%d%s%s%s%d", tmp, tmp, &N, tmp, tmp, tmp, &m);
    for (int i = 0; i < N; i++)
    {
      fscanf(pf, "%s%s%lf", tmp, tmp, &_y[i]);
    }
    fscanf(pf, "%s%s%lf", tmp, tmp, &x);
  }
};

/**
 * Проверка параметра размерности N
 * 1<= N <= MaxDim
 */
TEST_F(EvolventTest, throws_when_create_with_negative_N)
{
  ASSERT_ANY_THROW(Evolvent ev(-1, 10));
}

TEST_F(EvolventTest, throws_when_create_with_too_large_N)
{
  ASSERT_ANY_THROW(Evolvent ev(MaxDim + 1, 10));
}

/**
 * Проверка параметра построения развертки m -
 * точность разложения гиперкуба
 * 2 <= m <= MaxM
 */
TEST_F(EvolventTest, throws_when_create_with_too_low_m)
{
  ASSERT_ANY_THROW(Evolvent ev(2, 1));
}

TEST_F(EvolventTest, throws_when_create_with_too_large_m)
{
  ASSERT_ANY_THROW(Evolvent ev(2, MaxM + 1));
}

/**
 * Создание задачи с корректными входными параметрами
 */
TEST_F(EvolventTest, can_create_with_correct_values)
{
  ASSERT_NO_THROW(Evolvent ev(MaxDim - 1, MaxM - 1));
}

/**
 * Проверка корректности работы метода #GetInverseImage (y-->x)
 */
TEST_F(EvolventTest, can_get_inverse_image)
{
  //CreateCheckEvolventFile_GetImage();
  FILE* pf;
  int N, m;
  double x_actual;
  Extended x_expected;

  for (int t = 0; t < maxOfDim; t++)
  {
    A[t] = -0.5;
    B[t] = 0.5;
  }

  pf = fopen( (std::string(TESTDATA_PATH) + std::string("/evolventGetImage.txt")).c_str(),"r");
  if (pf == NULL)
  {
    EXPECT_TRUE(pf != NULL) << "Missed testdata!\n";
    return;
  }
  while (!feof(pf))
  {
    ReadFromFile(pf, N, m, y, x_actual);
    Evolvent evolvent(N, m);
    evolvent.SetBounds(A, B);
    evolvent.GetInverseImage(y, x_expected);
    double eps = 1.0 / (pow(2.0, m * N));
    ASSERT_NEAR(x_expected.toDouble(), x_actual, eps);
  }
  fclose(pf);
}

/**
 * Проверка корректности работы метода #GetImage (x-->y)
 */
TEST_F(EvolventTest, can_get_image)
{
  //CreateCheckEvolventFile_GetInverseImage();
  FILE* pf;
  int N, m;
  double x;
  double y_expected[maxOfDim];
  double eps;

  for (int t = 0; t < maxOfDim; t++)
  {
    A[t] = -0.5;
    B[t] = 0.5;
  }

  pf = fopen((std::string(TESTDATA_PATH) + std::string("/evolventGetInverseImage.txt")).c_str(), "r");
  if (pf == NULL)
  {
    EXPECT_TRUE(pf != NULL) << "Missed testdata!\n";
    return;
  }
  while (!feof(pf))
  {
    ReadFromFile(pf, N, m, y, x);

    Evolvent evolvent(N, m);
    evolvent.SetBounds(A, B);
    evolvent.GetImage(Extended(x), y_expected);
    eps = 1.0 / (pow(2.0, m));
    for (int i = 0; i < N; i++)
    {
      ASSERT_NEAR(y_expected[i], y[i], eps);
    }
  }
  fclose(pf);
}

/**
 * Проверка входного параметра x функции #GetImage
 * 0 <= x <= 1
 */
TEST_F(EvolventTest, throws_when_get_image_with_negative_x)
{
  const int N = 2, m = 2;
  double _y[N];
  Evolvent evolvent(N, m);
  ASSERT_ANY_THROW(evolvent.GetImage(Extended(-1), _y));
}

TEST_F(EvolventTest, throws_when_get_image_with_too_large_x)
{
  const int N = 2, m = 2;
  double _y[N];
  Evolvent evolvent(N, m);
  ASSERT_ANY_THROW(evolvent.GetImage(Extended(2), _y));
}

/**
 * Проверить, что метод #SetBounds корректно устанавливает границы A и B.
 */
TEST_F(EvolventTest, set_bounds_works_correctly) {
  const int N = 2;
  double A_test[] = { 0.0, 1.0 };
  double B_test[] = { 2.0, 3.0 };
  Evolvent ev(N, 5);
  ev.SetBounds(A_test, B_test);

  ASSERT_EQ(ev.getA()[0], 0.0);
  ASSERT_EQ(ev.getB()[0], 2.0);
  ASSERT_EQ(ev.getA()[1], 1.0);
  ASSERT_EQ(ev.getB()[1], 3.0);
}

/**
 * Проверить, что #GetPreimages возвращает корректный x[0]
 */
TEST_F(EvolventTest, get_preimages_returns_correct_x) {
  const int N = 2, m = 5;
  double y[N] = { 0.0, 0.0 };
  Extended x[1];
  Evolvent ev(N, m);

  for (int i = 0; i < N; i++) {
      A[i] = -0.5;
      B[i] = 0.5;
    }
  ev.SetBounds(A, B);

  ev.GetPreimages(y, x);
  EXPECT_DOUBLE_EQ(x[0].toDouble(), 0.5);
}

/**
 * Проверить, что метод возвращает -1
 */
TEST_F(EvolventTest, zero_constraint_calc_returns_negative_one) {
  const int N = 3, m = 5;
  double y[] = { 0.1, 0.2, 0.3 };
  Evolvent ev(N, m);

  double result = ev.ZeroConstraintCalc(y, 0);
  EXPECT_EQ(result, -1);
}


// ================================================================
// Дополнительные include для тестирования наследников IEvolvent
// ================================================================
#include "TLinearEvolvent.h"
#include "TNoninjectiveEvolvent.h"
#include "TRotatedEvolvent.h"
#include "TShiftedEvolvent.h"
#include "TSmoothEvolvent.h"
#include "EvolventFactory.h"
#include "Parameters.h"
#include <cstring>

// ================================================================
//                    LinearEvolvent
// ================================================================

/**
 * LinearEvolvent наследует валидацию N/m у базового класса.
 */
TEST_F(EvolventTest, linear_throws_on_invalid_N)
{
  ASSERT_ANY_THROW(LinearEvolvent ev(MaxDim + 1, 10));
}

/**
 * Образ линейной развёртки при N>=2 лежит внутри заданных границ.
 */
TEST_F(EvolventTest, linear_image_within_bounds)
{
  const int N = 2, m = 8;
  double a[] = { -1.0, -1.0 }, b[] = { 1.0, 1.0 };
  LinearEvolvent ev(N, m);
  ev.SetBounds(a, b);

  double _y[2];
  for (int i = 0; i <= 100; ++i)
  {
    Extended x = static_cast<double>(i) / 100.0;
    ev.GetImage(x, _y);
    for (int c = 0; c < N; ++c)
    {
      EXPECT_GE(_y[c], a[c] - 1e-9);
      EXPECT_LE(_y[c], b[c] + 1e-9);
    }
  }
}

/**
 * Round-trip линейной развёртки: GetInverseImage(GetImage(x)) ≈ x
 * с точностью до шага сетки.
 */
TEST_F(EvolventTest, linear_roundtrip_image_inverse)
{
  const int N = 2, m = 10;
  double a[] = { -0.5, -0.5 }, b[] = { 0.5, 0.5 };
  LinearEvolvent ev(N, m);
  ev.SetBounds(a, b);

  double _y[2];
  Extended xBack;
  const double eps = 1.0 / std::pow(2.0, m);
  for (double xv : {0.1, 0.3, 0.55, 0.8})
  {
    ev.GetImage(Extended(xv), _y);
    ev.GetInverseImage(_y, xBack);
    EXPECT_NEAR(xBack.toDouble(), xv, eps) << "xv=" << xv;
  }
}

/**
 * LinearEvolvent при выходе x за границы бросает исключение.
 */
TEST_F(EvolventTest, linear_throws_on_x_out_of_range)
{
  LinearEvolvent ev(2, 6);
  double _y[2];
  ASSERT_ANY_THROW(ev.GetImage(Extended(-0.1), _y));
  ASSERT_ANY_THROW(ev.GetImage(Extended(1.5), _y));
}

// ================================================================
//                    ShiftedEvolvent
// ================================================================

/**
 * ShiftedEvolvent: L должно быть в диапазоне [0, m). L >= m -> throw.
 */
TEST_F(EvolventTest, shifted_throws_when_L_too_large)
{
  const int m = 8;
  ASSERT_ANY_THROW(ShiftedEvolvent ev(2, m, m));      // L == m
  ASSERT_ANY_THROW(ShiftedEvolvent ev(2, m, m + 1));  // L > m
}

/**
 * ShiftedEvolvent: отрицательное L недопустимо.
 */
TEST_F(EvolventTest, shifted_throws_when_L_negative)
{
  ASSERT_ANY_THROW(ShiftedEvolvent ev(2, 8, -1));
}

/**
 * ShiftedEvolvent с корректными параметрами создаётся без исключений.
 */
TEST_F(EvolventTest, shifted_creates_with_valid_L)
{
  ASSERT_NO_THROW(ShiftedEvolvent ev(2, 8, 0));
  ASSERT_NO_THROW(ShiftedEvolvent ev(2, 8, 3));
}


/**
 * ShiftedEvolvent::ZeroConstraintCalc для точки внутри области возвращает
 * значение <= 0 (точка допустима для базовой развёртки EvolventNum=0).
 */
TEST_F(EvolventTest, shifted_zero_constraint_inside_is_nonpositive)
{
  const int N = 2, m = 8, L = 2;
  double a[] = { -0.5, -0.5 }, b[] = { 0.5, 0.5 };
  ShiftedEvolvent ev(N, m, L);
  ev.SetBounds(a, b);

  // центр области отображается в центр гиперкуба -> ограничение <= 0
  double yCenter[2] = { 0.0, 0.0 };
  double z = ev.ZeroConstraintCalc(yCenter, 0);
  EXPECT_LE(z, 0.0);
}

// ================================================================
//                    RotatedEvolvent
// ================================================================

/**
 * RotatedEvolvent при N>=2 и L=0 (по умолчанию) бросает исключение,
 * т.к. требуется L >= 1.
 */
TEST_F(EvolventTest, rotated_throws_when_L_is_zero_for_multidim)
{
  ASSERT_ANY_THROW(RotatedEvolvent ev(2, 8));  // L=0 по умолчанию -> throw
}

/**
 * RotatedEvolvent: L вне диапазона [1, 2*PlaneCount+1] -> throw.
 * Для N=2 PlaneCount=1, значит максимум L = 3.
 */
TEST_F(EvolventTest, rotated_throws_when_L_out_of_range)
{
  ASSERT_ANY_THROW(RotatedEvolvent ev(2, 8, 4)); // > 2*1+1
}

/**
 * RotatedEvolvent с корректным L создаётся без исключений.
 */
TEST_F(EvolventTest, rotated_creates_with_valid_L)
{
  ASSERT_NO_THROW(RotatedEvolvent ev(2, 8, 1));
  ASSERT_NO_THROW(RotatedEvolvent ev(2, 8, 3));
}

/**
 * RotatedEvolvent при N==1 не валидирует L (ранний выход в конструкторе).
 */
TEST_F(EvolventTest, rotated_1d_does_not_validate_L)
{
  ASSERT_NO_THROW(RotatedEvolvent ev(1, 8, 0));
}

/**
 * RotatedEvolvent: с EvolventNum==0 GetImage совпадает с базовой развёрткой.
 */
TEST_F(EvolventTest, rotated_evolvent_zero_matches_base)
{
  const int N = 2, m = 8, L = 3;
  double a[] = { -0.5, -0.5 }, b[] = { 0.5, 0.5 };

  RotatedEvolvent rot(N, m, L);
  rot.SetBounds(a, b);
  Evolvent base(N, m);
  base.SetBounds(a, b);

  double yRot[2], yBase[2];
  rot.GetImage(Extended(0.37), yRot, 0);
  base.GetImage(Extended(0.37), yBase);

  for (int c = 0; c < N; ++c)
    EXPECT_DOUBLE_EQ(yRot[c], yBase[c]);
}

/**
 * RotatedEvolvent: образ при повёрнутой развёртке (EvolventNum=1)
 * остаётся внутри границ.
 */
TEST_F(EvolventTest, rotated_image_within_bounds)
{
  const int N = 2, m = 8, L = 3;
  double a[] = { -1.0, -1.0 }, b[] = { 1.0, 1.0 };
  RotatedEvolvent ev(N, m, L);
  ev.SetBounds(a, b);

  double _y[2];
  for (int i = 0; i <= 100; ++i)
  {
    Extended x = static_cast<double>(i) / 100.0;
    ev.GetImage(x, _y, 1);
    for (int c = 0; c < N; ++c)
    {
      EXPECT_GE(_y[c], a[c] - 1e-9);
      EXPECT_LE(_y[c], b[c] + 1e-9);
    }
  }
}

/**
 * RotatedEvolvent::GetPreimages заполняет L прообразов; каждый через
 * свою развёртку возвращается в исходный y.
 */
TEST_F(EvolventTest, rotated_preimages_map_back_to_same_image)
{
  const int N = 2, m = 10, L = 3;
  double a[] = { -0.5, -0.5 }, b[] = { 0.5, 0.5 };
  RotatedEvolvent ev(N, m, L);
  ev.SetBounds(a, b);

  double _y[2];
  ev.GetImage(Extended(0.4), _y, 0);

  Extended preimgs[16];
  ev.GetPreimages(_y, preimgs);

  const double coordTol = 8.0 / std::pow(2.0, m);
  double yBack[2];
  for (int k = 0; k < L; ++k)
  {
    ev.GetImage(preimgs[k], yBack, k);
    for (int c = 0; c < N; ++c)
      EXPECT_NEAR(yBack[c], _y[c], coordTol) << "preimage#" << k << " coord " << c;
  }
}

// ================================================================
//                    NoninjectiveEvolvent
// ================================================================

/**
 * NoninjectiveEvolvent наследует валидацию N/m.
 */
TEST_F(EvolventTest, noninjective_throws_on_invalid_m)
{
  ASSERT_ANY_THROW(NoninjectiveEvolvent ev(2, 1));
}

/**
 * GetMaxPreimagesNumber возвращает значение, заданное в конструкторе.
 */
TEST_F(EvolventTest, noninjective_reports_max_preimages)
{
  NoninjectiveEvolvent ev(3, 8, 32);
  EXPECT_EQ(ev.GetMaxPreimagesNumber(), 32);
}

/**
 * Для N==1 GetNoninjectivePreimages возвращает ровно 1 прообраз.
 */
TEST_F(EvolventTest, noninjective_1d_returns_single_preimage)
{
  NoninjectiveEvolvent ev(1, 8, 8);
  double a[] = { -0.5 }, b[] = { 0.5 };
  ev.SetBounds(a, b);

  double _y[1] = { 0.0 };
  Extended x[8];
  int cnt = ev.GetNoninjectivePreimages(_y, x);
  EXPECT_EQ(cnt, 1);
  EXPECT_NEAR(x[0].toDouble(), 0.5, 1e-9); // y=0 -> P=0 -> x=0.5
}

/**
 * Для N>=2 число прообразов лежит в допустимом диапазоне [1, max_preimages],
 * и все прообразы через развёртку отображаются в исходный y.
 */
TEST_F(EvolventTest, noninjective_preimages_within_limit_and_consistent)
{
  const int N = 2, m = 10, maxP = 1 << N;
  double a[] = { -0.5, -0.5 }, b[] = { 0.5, 0.5 };
  NoninjectiveEvolvent ev(N, m, maxP);
  ev.SetBounds(a, b);

  double _y[2];
  ev.GetImage(Extended(0.4), _y);

  Extended preimgs[16];
  int cnt = ev.GetNoninjectivePreimages(_y, preimgs);

  ASSERT_GE(cnt, 1);
  ASSERT_LE(cnt, maxP);

  const double coordTol = 8.0 / std::pow(2.0, m);
  double yBack[2];
  for (int k = 0; k < cnt; ++k)
  {
    ev.GetImage(preimgs[k], yBack);
    for (int c = 0; c < N; ++c)
      EXPECT_NEAR(yBack[c], _y[c], coordTol) << "preimage#" << k << " coord " << c;
  }
}

// ================================================================
//                    SmoothEvolvent
// ================================================================

/**
 * SmoothEvolvent: h вне [0,1] -> throw.
 */
TEST_F(EvolventTest, smooth_throws_on_invalid_h)
{
  ASSERT_ANY_THROW(SmoothEvolvent ev(2, 8, -0.1));
  ASSERT_ANY_THROW(SmoothEvolvent ev(2, 8, 1.5));
}

/**
 * SmoothEvolvent создаётся с корректным h.
 */
TEST_F(EvolventTest, smooth_creates_with_valid_h)
{
  ASSERT_NO_THROW(SmoothEvolvent ev(2, 8, 0.25));
  ASSERT_NO_THROW(SmoothEvolvent ev(2, 8, 0.0));
  ASSERT_NO_THROW(SmoothEvolvent ev(2, 8, 1.0));
}

/**
 * SmoothEvolvent::GetImage при x вне [0,1] бросает исключение.
 */
TEST_F(EvolventTest, smooth_throws_on_x_out_of_range)
{
  SmoothEvolvent ev(2, 8, 0.25);
  double _y[2];
  ASSERT_ANY_THROW(ev.GetImage(Extended(-0.01), _y));
  ASSERT_ANY_THROW(ev.GetImage(Extended(1.01), _y));
}

/**
 * SmoothEvolvent::GetInverseImage не реализован и должен бросать исключение.
 */
TEST_F(EvolventTest, smooth_inverse_image_throws_not_implemented)
{
  SmoothEvolvent ev(2, 8, 0.25);
  double _y[2] = { 0.0, 0.0 };
  Extended x;
  ASSERT_ANY_THROW(ev.GetInverseImage(_y, x));
}

/**
 * Образ гладкой развёртки лежит внутри границ.
 */
TEST_F(EvolventTest, smooth_image_within_bounds)
{
  const int N = 2, m = 8;
  double a[] = { -1.0, -1.0 }, b[] = { 1.0, 1.0 };
  SmoothEvolvent ev(N, m, 0.25);
  ev.SetBounds(a, b);

  double _y[2];
  for (int i = 0; i <= 50; ++i)
  {
    Extended x = static_cast<double>(i) / 50.0;
    ev.GetImage(x, _y);
    for (int c = 0; c < N; ++c)
    {
      EXPECT_GE(_y[c], a[c] - 1e-6);
      EXPECT_LE(_y[c], b[c] + 1e-6);
    }
  }
}

// ================================================================
//                    EvolventFactory (полиморфизм)
// ================================================================

/**
 * Фабрика создаёт правильный тип развёртки по parameters.MapType.
 * Проверяем через dynamic_cast и полиморфный вызов GetImage.
 */
TEST_F(EvolventTest, factory_creates_base_evolvent)
{
  parameters.MapType = mpBase;
  IEvolvent* e = EvolventFactory::CreateEvolvent(2, 8);
  ASSERT_NE(e, nullptr);
  EXPECT_NE(dynamic_cast<Evolvent*>(e), nullptr);
  delete e;
}

TEST_F(EvolventTest, factory_creates_linear_evolvent)
{
  parameters.MapType = mpLinar;
  IEvolvent* e = EvolventFactory::CreateEvolvent(2, 8);
  ASSERT_NE(e, nullptr);
  EXPECT_NE(dynamic_cast<LinearEvolvent*>(e), nullptr);
  delete e;
}

TEST_F(EvolventTest, factory_creates_noninjective_evolvent)
{
  parameters.MapType = mpNoninjective;
  IEvolvent* e = EvolventFactory::CreateEvolvent(2, 8);
  ASSERT_NE(e, nullptr);
  EXPECT_NE(dynamic_cast<NoninjectiveEvolvent*>(e), nullptr);
  delete e;
}

TEST_F(EvolventTest, factory_creates_shifted_evolvent)
{
  parameters.MapType = mpShifted;
  IEvolvent* e = EvolventFactory::CreateEvolvent(2, 8);
  ASSERT_NE(e, nullptr);
  EXPECT_NE(dynamic_cast<ShiftedEvolvent*>(e), nullptr);
  delete e;
}

TEST_F(EvolventTest, factory_creates_smooth_evolvent)
{
  parameters.MapType = mpSmooth;
  IEvolvent* e = EvolventFactory::CreateEvolvent(2, 8);
  ASSERT_NE(e, nullptr);
  EXPECT_NE(dynamic_cast<SmoothEvolvent*>(e), nullptr);
  delete e;
}

/**
 * Полиморфный контракт: базовый образ внутри границ.
 * ВНИМАНИЕ: mpRotated фабрика создаёт с L=0 (для N>=2 бросает исключение),
 * а mpShifted намеренно растягивает координаты (y*2+...), поэтому её образ
 * может легально выходить за [A,B] — обе эти развёртки исключены из строгой
 * проверки границ.
 */
TEST_F(EvolventTest, polymorphic_image_within_bounds_for_factory_types)
{
  double a[] = { -0.5, -0.5 }, b[] = { 0.5, 0.5 };
  EMapType types[] = { mpBase, mpLinar, mpNoninjective, mpSmooth };

  for (EMapType t : types)
  {
    parameters.MapType = t;
    IEvolvent* e = EvolventFactory::CreateEvolvent(2, 8);
    ASSERT_NE(e, nullptr) << "MapType=" << (int)t;
    e->SetBounds(a, b);

    double _y[2];
    e->GetImage(Extended(0.5), _y, 0);
    for (int c = 0; c < 2; ++c)
    {
      EXPECT_GE(_y[c], a[c] - 1e-6) << "MapType=" << (int)t;
      EXPECT_LE(_y[c], b[c] + 1e-6) << "MapType=" << (int)t;
    }
    delete e;
  }
}

/**
 * ShiftedEvolvent::GetPreimages заполняет L+1 прообразов.
 * Гарантированный инвариант: прообраз базовой ветви (индекс 0) при обратном
 * отображении через ту же ветвь восстанавливает исходный y.
 * (Для ветвей i>0 y может не принадлежать подобласти развёртки — проверяется
 * отдельно через ZeroConstraintCalc, поэтому строгий round-trip к ним не применяем.)
 */
TEST_F(EvolventTest, shifted_base_branch_preimage_roundtrips)
{
  const int N = 2, m = 10, L = 3;
  double a[] = { -0.5, -0.5 }, b[] = { 0.5, 0.5 };
  ShiftedEvolvent ev(N, m, L);
  ev.SetBounds(a, b);

  // Берём точку в центре области, где базовая развёртка гарантированно определена.
  double _y[2];
  ev.GetImage(Extended(0.3), _y, 0);

  Extended preimgs[16];
  ev.GetPreimages(_y, preimgs);

  // Все L+1 прообразов — конечные и лежат в [0,1].
  for (int k = 0; k <= L; ++k)
  {
    double xv = preimgs[k].toDouble();
    EXPECT_GE(xv, 0.0) << "preimage#" << k;
    EXPECT_LE(xv, 1.0) << "preimage#" << k;
  }

  // Round-trip только для базовой ветви (индекс 0).
  const double coordTol = 8.0 / std::pow(2.0, m);
  double yBack[2];
  ev.GetImage(preimgs[0], yBack, 0);
  for (int c = 0; c < N; ++c)
    EXPECT_NEAR(yBack[c], _y[c], coordTol) << "base branch coord " << c;
}

/**
 * Правильная проверка обратимости всех ветвей ShiftedEvolvent:
 * для образа, построенного по ветви i, прообраз x[i] совпадает с исходным x.
 */
TEST_F(EvolventTest, shifted_each_branch_is_invertible)
{
  const int N = 2, m = 10, L = 3;
  double a[] = { -0.5, -0.5 }, b[] = { 0.5, 0.5 };
  ShiftedEvolvent ev(N, m, L);
  ev.SetBounds(a, b);

  const double xTol = 4.0 / std::pow(2.0, m);
  const double x0 = 0.3;

  for (int i = 0; i <= L; ++i)
  {
    double _y[2];
    ev.GetImage(Extended(x0), _y, i);   // образ по ветви i

    Extended preimgs[16];
    ev.GetPreimages(_y, preimgs);       // среди прообразов x[i] == x0

    EXPECT_NEAR(preimgs[i].toDouble(), x0, xTol) << "branch " << i;
  }
}
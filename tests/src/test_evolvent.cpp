#include "evolvent.h"
#include "extended.h"
#include "test_config.h"

#include <gtest/gtest.h>
#include <cmath>

class TEvolventTest : public ::testing::Test
{
protected:
  static const int numOfPoint = 5;
  static const int numOfDim = 5;
  static const int numOfm = 7;
  static const int maxOfDim = 10;
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
        TEvolvent evolvent(N[i], m[j]);
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
        TEvolvent evolvent(N[i], m[j]);
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
TEST_F(TEvolventTest, throws_when_create_with_negative_N)
{
  ASSERT_ANY_THROW(TEvolvent ev(-1, 10));
}

TEST_F(TEvolventTest, throws_when_create_with_too_large_N)
{
  ASSERT_ANY_THROW(TEvolvent ev(MaxDim + 1, 10));
}

/**
 * Проверка параметра построения развертки m -
 * точность разложения гиперкуба
 * 2 <= m <= MaxM
 */
TEST_F(TEvolventTest, throws_when_create_with_too_low_m)
{
  ASSERT_ANY_THROW(TEvolvent ev(2, 1));
}

TEST_F(TEvolventTest, throws_when_create_with_too_large_m)
{
  ASSERT_ANY_THROW(TEvolvent ev(2, MaxM + 1));
}

/**
 * Создание задачи с корректными входными параметрами
 */
TEST_F(TEvolventTest, can_create_with_correct_values)
{
  ASSERT_NO_THROW(TEvolvent ev(MaxDim - 1, MaxM - 1));
}

/**
 * Проверка корректности работы метода #GetInverseImage (y-->x)
 */
TEST_F(TEvolventTest, can_get_inverse_image)
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
    TEvolvent evolvent(N, m);
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
TEST_F(TEvolventTest, can_get_image)
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

    TEvolvent evolvent(N, m);
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
TEST_F(TEvolventTest, throws_when_get_image_with_negative_x)
{
  const int N = 2, m = 2;
  double _y[N];
  TEvolvent evolvent(N, m);
  ASSERT_ANY_THROW(evolvent.GetImage(Extended(-1), _y));
}

TEST_F(TEvolventTest, throws_when_get_image_with_too_large_x)
{
  const int N = 2, m = 2;
  double _y[N];
  TEvolvent evolvent(N, m);
  ASSERT_ANY_THROW(evolvent.GetImage(Extended(2), _y));
}
#include "SearchInterval.h"
#include "SearchData.h"
#include "gtest/gtest.h"
#include "Trial.h"

/**
  Вспомогательный класс, помогающий задать начальную конфигурацию объекта
  класса #TSearchData, которая будет использоваться в тестах
 */
class TSearchDataTest : public ::testing::Test
{
protected:
  TSearchData* data;
  TSearchInterval interval1;
  TSearchInterval interval2;
  TSearchInterval interval3;
  void SetUp()
  {
    Extended::SetTypeID(etDouble);
    int argc = 1;
    int n = 5;
    char* argv[1];
    argv[0] = new char(8);
    parameters.Init(argc, argv);
    parameters.Dimension = n;
    data = new TSearchData(MaxNumOfFunc, DefaultSearchDataSize);
    interval1 = SetUpInterval(1.0, 2.0);
    interval2 = SetUpInterval(3.0, 4.0);
    interval3 = SetUpInterval(5.0, 6.0);
  }
  void TearDown()
  {
    delete data;
  }
  /**
  * Create interval with length = 1
  */
  TSearchInterval SetUpInterval(double xl, double R)
  {
    TSearchInterval interval;
    interval.CreatePoint();
    interval.LeftPoint->SetX(Extended(xl));
    interval.RightPoint->SetX(Extended(xl + 1));
    interval.R = R;
    return interval;
  }
};

/**
 * Проверка параметра максимальный размер МСП #MaxSize
 * MaxSize > 0
 */
TEST_F(TSearchDataTest, throws_when_create_with_null_MaxSize_of_searchData)
{
  ASSERT_ANY_THROW(TSearchData searchData(MaxNumOfFunc, 0));
}

TEST_F(TSearchDataTest, throws_when_create_with_negative_MaxSize_of_searchData)
{
  ASSERT_ANY_THROW(TSearchData searchData(MaxNumOfFunc, -1));
}

/**
 * Проверка параметра число функций задачи #NumOfFuncs
 * 0 < NumOfFuncs <= MaxNumOfFunc
 */
TEST_F(TSearchDataTest, throws_when_create_with_null_NumOfFunc)
{
  ASSERT_ANY_THROW(TSearchData searchData(0, DefaultSearchDataSize));
}

TEST_F(TSearchDataTest, throws_when_create_with_negative_NumOfFunc)
{
  ASSERT_ANY_THROW(TSearchData searchData(-1, DefaultSearchDataSize));
}

TEST_F(TSearchDataTest, throws_when_create_with_too_large_NumOfFunc)
{
  ASSERT_ANY_THROW(TSearchData searchData(MaxNumOfFunc + 1, DefaultSearchDataSize));
}

/**
 * Создание поисковой информации с корректными входными параметрами
 */
TEST_F(TSearchDataTest, can_create_searchData_with_correct_values)
{
  ASSERT_NO_THROW(TSearchData searchData(MaxNumOfFunc, DefaultSearchDataSize));
}

/**
 * Проверка корректности работы метода #InsertInterval
 */
TEST_F(TSearchDataTest, can_insert_interval)
{
  TSearchInterval* pInterval = data->InsertInterval(interval1);

  ASSERT_EQ(interval1.xl(), pInterval->xl());
}

TEST_F(TSearchDataTest, throws_when_insert_interval_which_already_exist)
{
  (void*) data->InsertInterval(interval1);

  ASSERT_ANY_THROW(data->InsertInterval(interval1));
}

TEST_F(TSearchDataTest, throws_when_insert_interval_with_null_length)
{
  TSearchInterval interval;
  interval.LeftPoint = new TTrial();
  interval.LeftPoint->SetX(Extended(1.0));
  interval.RightPoint = new TTrial();
  interval.RightPoint->SetX(Extended(1.0));
  ASSERT_ANY_THROW(data->InsertInterval(interval));
}

TEST_F(TSearchDataTest, throws_when_insert_interval_with_negative_length)
{
  TSearchInterval interval;
  interval.LeftPoint = new TTrial();
  interval.LeftPoint->SetX(Extended(2.0));
  interval.RightPoint = new TTrial();
  interval.RightPoint->SetX(Extended(1.0));
  ASSERT_ANY_THROW(data->InsertInterval(interval));
}
/**
 * Проверка корректности работы метода #UpdateInterval
 */
TEST_F(TSearchDataTest, do_nothing_when_update_interval_which_is_not)
{
  double xl = 2.0;
  double R = 2.0;
  TSearchInterval insertInterval = SetUpInterval(xl, R);
  TSearchInterval updateInterval = SetUpInterval(xl + 1, R + 1);
  TSearchInterval* pInterval = data->InsertInterval(insertInterval);

  data->UpdateInterval(updateInterval);

  ASSERT_DOUBLE_EQ(R, pInterval->R);
}

TEST_F(TSearchDataTest, can_update_interval)
{
  double xl = 2.0;
  double R = 2.0;
  double newR = 5.0;
  TSearchInterval insertInterval = SetUpInterval(xl, R);
  TSearchInterval updateInterval = SetUpInterval(xl, newR);
  TSearchInterval* pInterval = data->InsertInterval(insertInterval);

  data->UpdateInterval(updateInterval);

  ASSERT_DOUBLE_EQ(newR, pInterval->R);
}

/**
 * Проверка корректности работы метода #GetIntervalByX
 */
TEST_F(TSearchDataTest, get_NULL_by_illegal_X)
{
  TSearchInterval insertInterval = SetUpInterval(1.0, 2.0);
  (void *) data->InsertInterval(insertInterval);
  TTrial* x = new TTrial();
  x->SetX(3.0);

  TSearchInterval* pIntervalExpected = data->GetIntervalByX(x);

  ASSERT_EQ(NULL, pIntervalExpected);
}

TEST_F(TSearchDataTest, can_get_interval_by_X)
{
  TSearchInterval* pInterval = data->InsertInterval(interval1);
  pInterval = data->InsertInterval(interval2);
  pInterval = data->InsertInterval(interval3);

  TTrial* x = new TTrial();
  x->SetX(interval2.xl());
  TSearchInterval* pIntervalExpected = data->GetIntervalByX(x);

  ASSERT_DOUBLE_EQ(interval2.R, pIntervalExpected->R);
}

///**
// * Проверка корректности работы метода #FindCoveringInterval
// */
TEST_F(TSearchDataTest, return_NULL_instead_covering_interval_by_illegal_X)
{
  TSearchInterval insertInterval = SetUpInterval(1.0, 2.0);
  TSearchInterval* pInterval = data->InsertInterval(insertInterval);
  insertInterval = SetUpInterval(4.0, 5.0);
  pInterval = data->InsertInterval(insertInterval);
  TTrial* x = new TTrial();
  x->SetX(Extended(3.0));
  TSearchInterval* pIntervalExpected = data->FindCoveringInterval(x);

  ASSERT_EQ(NULL, pIntervalExpected);
}

TEST_F(TSearchDataTest, can_find_covering_interval_by_X)
{
  TSearchInterval* pInterval = data->InsertInterval(interval1);
  pInterval = data->InsertInterval(interval2);
  pInterval = data->InsertInterval(interval3);

  TTrial* x = new TTrial();
  x->SetX(interval2.xl() + 0.5);
  TSearchInterval* pIntervalExpected = data->FindCoveringInterval(x);

  ASSERT_DOUBLE_EQ(interval2.R, pIntervalExpected->R);
}

/**
 * Проверка корректности работы метода #GetIntervalWithMaxR
 */
TEST_F(TSearchDataTest, can_return_interval_with_max_R)
{
  double actualXl = 2.0;
  double actualR = 5.0;
  TSearchInterval i1 = SetUpInterval(3.0, 2.0);
  data->PushToQueue(&i1);
  TSearchInterval i2 = SetUpInterval(2.0, 5.0);
  data->PushToQueue(&i2);
  TSearchInterval i3 = SetUpInterval(1.0, 3.0);
  data->PushToQueue(&i3);

  TSearchInterval* pIntervalWithMaxR = data->GetIntervalWithMaxR();

  ASSERT_DOUBLE_EQ(actualR, pIntervalWithMaxR->R);
  ASSERT_DOUBLE_EQ(actualXl, pIntervalWithMaxR->xl().toDouble());
}

TEST_F(TSearchDataTest, can_return_interval_with_max_R_when_queue_empty)
{
  TSearchInterval* pInterval = data->InsertInterval(interval1);
  pInterval = data->InsertInterval(interval3);
  pInterval = data->InsertInterval(interval2);

  TSearchInterval* pIntervalWithMaxR = data->GetIntervalWithMaxR();

  ASSERT_DOUBLE_EQ(interval3.R, pIntervalWithMaxR->R);
}

/**
 * Проверка корректности работы метода #GetIntervalWithMaxLocalR
 * ???
 */
TEST_F(TSearchDataTest, can_return_interval_with_max_local_R)
{
  parameters.localMix = 1;
  TSearchData *pData = new TSearchData(MaxNumOfFunc, DefaultSearchDataSize);

  double actualXl = 2.0;
  double actualLocalR = 6.0;
  TSearchInterval interval = SetUpInterval(3.0, 5.0);
  interval.locR = 1.0;
  pData->PushToQueue(&interval);

  TSearchInterval interval2_ = SetUpInterval(actualXl, 2.0);
  interval2_.locR = actualLocalR;
  pData->PushToQueue(&interval2_);

  TSearchInterval interval3_ = SetUpInterval(1.0, 3.0);
  interval3_.locR = 4.0;
  pData->PushToQueue(&interval3_);

  TSearchInterval* pIntervalWithMaxLocalR = pData->GetIntervalWithMaxLocalR();

  ASSERT_DOUBLE_EQ(actualLocalR, pIntervalWithMaxLocalR->locR);
}

/**
 * Проверка корректности работы метода #PushToQueue
 */
TEST_F(TSearchDataTest, throw_when_push_to_queue_null_pointer)
{
  ASSERT_ANY_THROW(data->PushToQueue(0));
}

TEST_F(TSearchDataTest, can_push_interval_to_queue)
{
  ASSERT_NO_THROW(data->PushToQueue(new TSearchInterval()));
}

/**
 * Проверка корректности работы метода #RefillQueue
 */
TEST_F(TSearchDataTest, can_refill_queue)
{
  TSearchInterval* pInterval = data->InsertInterval(interval1);
  pInterval = data->InsertInterval(interval3);
  pInterval = data->InsertInterval(interval2);

  data->RefillQueue();

  data->PopFromGlobalQueue(&pInterval);
  ASSERT_DOUBLE_EQ(interval3.R, pInterval->R);
}

/**
 * Проверка корректности работы метода #InsertPoint
 */
TEST_F(TSearchDataTest, can_insert_new_point)
{
  Extended newXl = Extended(5.7);
  TSearchInterval* pInterval;
  TSearchInterval* pCoveringInterval;
  TTrial point;
  point = Extended(newXl);
  point.index = 0;
  pInterval = data->InsertInterval(interval1);
  pCoveringInterval = data->InsertInterval(interval3);
  pInterval = data->InsertInterval(interval2);

  TSearchInterval* pNewInterval = data->InsertPoint(pCoveringInterval, point, 1, 1);

  ASSERT_EQ(newXl, pNewInterval->xl());
}

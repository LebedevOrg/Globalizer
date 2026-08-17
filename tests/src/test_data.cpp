#include "SearchInterval.h"
#include "SearchData.h"
#include "TreeNode.h"
#include "gtest/gtest.h"
#include "Trial.h"
#include "SearcDataIterator.h"

/**
  Вспомогательный класс, помогающий задать начальную конфигурацию объекта
  класса #SearchData, которая будет использоваться в тестах
 */
class TSearchDataTest : public ::testing::Test
{
protected:
  SearchData* data;
  SearchInterval interval1;
  SearchInterval interval2;
  SearchInterval interval3;

  void SetUp()
  {
    Extended::SetTypeID(etDouble);
    parameters.Dimension = 5;      // Init уже сделан глобально
    data = new SearchData(MaxNumOfFunc, DefaultSearchDataSize);
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
  SearchInterval SetUpInterval(double xl, double R)
  {
    SearchInterval interval;
    interval.CreatePoint();
    interval.LeftPoint->SetX(Extended(xl));
    interval.RightPoint->SetX(Extended(xl + 1));
    interval.R = R;
    return interval;
  }
};

/**
 * Создание дерева с корректным входным параметром
 */
TEST_F(TSearchDataTest, can_create_TreeNode_with_correct_values)
{
    ASSERT_NO_THROW(TreeNode treeNode(interval1));
}

/**
 * Проверка параметра максимальный размер МСП #MaxSize
 * MaxSize > 0
 */
TEST_F(TSearchDataTest, throws_when_create_with_null_MaxSize_of_searchData)
{
  ASSERT_ANY_THROW(SearchData searchData(MaxNumOfFunc, 0));
}

TEST_F(TSearchDataTest, throws_when_create_with_negative_MaxSize_of_searchData)
{
  ASSERT_ANY_THROW(SearchData searchData(MaxNumOfFunc, -1));
}

/**
 * Проверка параметра число функций задачи #NumOfFuncs
 * 0 < NumOfFuncs <= MaxNumOfFunc
 */
TEST_F(TSearchDataTest, throws_when_create_with_null_NumOfFunc)
{
  ASSERT_ANY_THROW(SearchData searchData(0, DefaultSearchDataSize));
}

TEST_F(TSearchDataTest, throws_when_create_with_negative_NumOfFunc)
{
  ASSERT_ANY_THROW(SearchData searchData(-1, DefaultSearchDataSize));
}

TEST_F(TSearchDataTest, throws_when_create_with_too_large_NumOfFunc)
{
  ASSERT_ANY_THROW(SearchData searchData(MaxNumOfFunc + 1, DefaultSearchDataSize));
}

/**
 * Создание поисковой информации с корректными входными параметрами
 */
TEST_F(TSearchDataTest, can_create_searchData_with_correct_values)
{
  ASSERT_NO_THROW(SearchData searchData(MaxNumOfFunc, DefaultSearchDataSize));
}

/**
 * Проверка корректности работы метода #Clear
 */

TEST_F(TSearchDataTest, can_clear)
{
    SearchData* pData = new SearchData(MaxNumOfFunc, DefaultSearchDataSize);
    pData->InsertInterval(interval1);
    pData->InsertInterval(interval2);
    pData->InsertInterval(interval3);
    pData->Clear();

    EXPECT_EQ(pData->GetCount(), 0);
    EXPECT_EQ(pData->GetBestTrial(), nullptr);
}

/**
 * Проверка корректности работы метода #InsertInterval
 */
TEST_F(TSearchDataTest, can_insert_interval)
{
  SearchInterval* pInterval = data->InsertInterval(interval1);

  ASSERT_EQ(interval1.xl(), pInterval->xl());
}

TEST_F(TSearchDataTest, throws_when_insert_interval_which_already_exist)
{
  (void*) data->InsertInterval(interval1);

  ASSERT_ANY_THROW(data->InsertInterval(interval1));
}

TEST_F(TSearchDataTest, throws_when_insert_interval_with_null_length)
{
  SearchInterval interval;

  interval.LeftPoint = new Trial();
  interval.LeftPoint->SetX(Extended(1.0));
  interval.RightPoint = new Trial();
  interval.RightPoint->SetX(Extended(1.0));

  ASSERT_ANY_THROW(data->InsertInterval(interval));
}

TEST_F(TSearchDataTest, throws_when_insert_interval_with_negative_length)
{
  SearchInterval interval;

  interval.LeftPoint = new Trial();
  interval.LeftPoint->SetX(Extended(2.0));
  interval.RightPoint = new Trial();
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
  SearchInterval insertInterval = SetUpInterval(xl, R);
  SearchInterval updateInterval = SetUpInterval(xl + 1, R + 1);
  SearchInterval* pInterval = data->InsertInterval(insertInterval);

  data->UpdateInterval(updateInterval);

  ASSERT_DOUBLE_EQ(R, pInterval->R);
}

TEST_F(TSearchDataTest, can_update_interval)
{
  double xl = 2.0;
  double R = 2.0;
  double newR = 5.0;
  SearchInterval insertInterval = SetUpInterval(xl, R);
  SearchInterval updateInterval = SetUpInterval(xl, newR);
  SearchInterval* pInterval = data->InsertInterval(insertInterval);

  data->UpdateInterval(updateInterval);

  ASSERT_DOUBLE_EQ(newR, pInterval->R);
}

/**
 * Проверка корректности работы метода #GetIntervalByX
 */
TEST_F(TSearchDataTest, get_NULL_by_illegal_X)
{
  SearchInterval insertInterval = SetUpInterval(1.0, 2.0);
  (void *) data->InsertInterval(insertInterval);
  Trial* x = new Trial();
  x->SetX(3.0);

  SearchInterval* pIntervalExpected = data->GetIntervalByX(x);

  ASSERT_EQ(NULL, pIntervalExpected);
}

TEST_F(TSearchDataTest, can_get_interval_by_X)
{
  SearchInterval* pInterval = data->InsertInterval(interval1);
  pInterval = data->InsertInterval(interval2);
  pInterval = data->InsertInterval(interval3);

  Trial* x = new Trial();
  x->SetX(interval2.xl());
  SearchInterval* pIntervalExpected = data->GetIntervalByX(x);

  ASSERT_DOUBLE_EQ(interval2.R, pIntervalExpected->R);
}

///**
// * Проверка корректности работы метода #FindCoveringInterval
// */
TEST_F(TSearchDataTest, return_NULL_instead_covering_interval_by_illegal_X)
{
  SearchInterval insertInterval = SetUpInterval(1.0, 2.0);
  SearchInterval* pInterval = data->InsertInterval(insertInterval);
  insertInterval = SetUpInterval(4.0, 5.0);
  pInterval = data->InsertInterval(insertInterval);
  Trial* x = new Trial();
  x->SetX(Extended(3.0));
  SearchInterval* pIntervalExpected = data->FindCoveringInterval(x);

  ASSERT_EQ(NULL, pIntervalExpected);
}

TEST_F(TSearchDataTest, can_find_covering_interval_by_X)
{
  SearchInterval* pInterval = data->InsertInterval(interval1);
  pInterval = data->InsertInterval(interval2);
  pInterval = data->InsertInterval(interval3);

  Trial* x = new Trial();
  x->SetX(interval2.xl() + 0.5);
  SearchInterval* pIntervalExpected = data->FindCoveringInterval(x);

  ASSERT_DOUBLE_EQ(interval2.R, pIntervalExpected->R);
}

/**
 * Проверка корректности работы метода #GetIntervalWithMaxR
 */
TEST_F(TSearchDataTest, can_return_interval_with_max_R)
{
  double actualXl = 2.0;
  double actualR = 5.0;
  SearchInterval i1 = SetUpInterval(3.0, 2.0);
  data->PushToQueue(&i1);
  SearchInterval i2 = SetUpInterval(2.0, 5.0);
  data->PushToQueue(&i2);
  SearchInterval i3 = SetUpInterval(1.0, 3.0);
  data->PushToQueue(&i3);

  SearchInterval* pIntervalWithMaxR = data->GetIntervalWithMaxR();

  ASSERT_DOUBLE_EQ(actualR, pIntervalWithMaxR->R);
  ASSERT_DOUBLE_EQ(actualXl, pIntervalWithMaxR->xl().toDouble());
}

TEST_F(TSearchDataTest, can_return_interval_with_max_R_when_queue_empty)
{
  SearchInterval* pInterval = data->InsertInterval(interval1);
  pInterval = data->InsertInterval(interval3);
  pInterval = data->InsertInterval(interval2);

  SearchInterval* pIntervalWithMaxR = data->GetIntervalWithMaxR();

  ASSERT_DOUBLE_EQ(interval3.R, pIntervalWithMaxR->R);
}

/**
 * Проверка корректности работы метода #GetIntervalWithMaxLocalR
 * ???
 */
TEST_F(TSearchDataTest, can_return_interval_with_max_local_R)
{
  parameters.LocalMix = 1;
  SearchData *pData = new SearchData(MaxNumOfFunc, DefaultSearchDataSize);

  double actualXl = 2.0;
  double actualLocalR = 6.0;
  SearchInterval interval = SetUpInterval(3.0, 5.0);
  interval.locR = 1.0;
  pData->PushToQueue(&interval);

  SearchInterval interval2_ = SetUpInterval(actualXl, 2.0);
  interval2_.locR = actualLocalR;
  pData->PushToQueue(&interval2_);

  SearchInterval interval3_ = SetUpInterval(1.0, 3.0);
  interval3_.locR = 4.0;
  pData->PushToQueue(&interval3_);

  SearchInterval* pIntervalWithMaxLocalR = pData->GetIntervalWithMaxLocalR();

  ASSERT_DOUBLE_EQ(actualLocalR, pIntervalWithMaxLocalR->locR);
  ASSERT_DOUBLE_EQ(actualXl, pIntervalWithMaxLocalR->xl().toDouble());
}

/**
 * Проверка корректности работы метода #InsertPoint
 */
TEST_F(TSearchDataTest, can_insert_new_point)
{
    Extended newXl = Extended(5.7);
    SearchInterval* pInterval;
    SearchInterval* pCoveringInterval;
    Trial point;
    point = Extended(newXl);
    point.index = 0;
    pInterval = data->InsertInterval(interval1);
    pCoveringInterval = data->InsertInterval(interval3);
    pInterval = data->InsertInterval(interval2);

    SearchInterval* pNewInterval = data->InsertPoint(pCoveringInterval, point, 1, 1);

    ASSERT_EQ(newXl, pNewInterval->xl());
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
  ASSERT_NO_THROW(data->PushToQueue(new SearchInterval()));
}

/**
 * Проверка корректности работы метода #RefillQueue
 */
TEST_F(TSearchDataTest, can_refill_queue)
{
  SearchInterval* pInterval = data->InsertInterval(interval1);
  pInterval = data->InsertInterval(interval3);
  pInterval = data->InsertInterval(interval2);

  data->RefillQueue();

  data->PopFromGlobalQueue(&pInterval);
  ASSERT_DOUBLE_EQ(interval3.R, pInterval->R);
}

/**
 * Проверка счётчика интервалов #GetCount после вставок.
 */
TEST_F(TSearchDataTest, count_increases_on_insert)
{
  EXPECT_EQ(data->GetCount(), 0);
  data->InsertInterval(interval1);
  EXPECT_EQ(data->GetCount(), 1);
  data->InsertInterval(interval2);
  data->InsertInterval(interval3);
  EXPECT_EQ(data->GetCount(), 3);
}

/**
 * Проверка #SetBestTrial / #GetBestTrial и пересчёта Z.
 * При index == 0 значение Z[0] должно стать равным FuncValues[0].
 */
TEST_F(TSearchDataTest, can_set_and_get_best_trial)
{
  Trial* best = new Trial();
  best->index = 0;
  best->FuncValues[0] = 3.5;

  data->SetBestTrial(best);

  EXPECT_EQ(data->GetBestTrial(), best);
  EXPECT_DOUBLE_EQ(data->Z[0], 3.5);

  delete best;
}

/**
 * При index == 0 и FuncValues[0] == MaxDouble Z[0] обнуляется.
 */
TEST_F(TSearchDataTest, set_best_trial_with_maxdouble_sets_zero_Z)
{
  Trial* best = new Trial(); // по умолчанию FuncValues[i] == MaxDouble
  best->index = 0;

  data->SetBestTrial(best);

  EXPECT_DOUBLE_EQ(data->Z[0], 0.0);

  delete best;
}

/**
 * Проверка #GetBestIntervals: возвращаются интервалы в порядке убывания R.
 */
TEST_F(TSearchDataTest, can_get_best_intervals)
{
  data->InsertInterval(interval1); // R = 2.0
  data->InsertInterval(interval2); // R = 4.0
  data->InsertInterval(interval3); // R = 6.0

  SearchInterval* result[2];
  data->GetBestIntervals(result, 2);

  ASSERT_DOUBLE_EQ(result[0]->R, 6.0);
  ASSERT_DOUBLE_EQ(result[1]->R, 4.0);
}

/**
 * Проверка #FindMax: возвращает максимальный элемент без извлечения из очереди.
 */
TEST_F(TSearchDataTest, find_max_does_not_remove_element)
{
  data->InsertInterval(interval1);
  data->InsertInterval(interval2);
  data->InsertInterval(interval3);

  data->RefillQueue();

  SearchInterval& maxInterval = data->FindMax();
  ASSERT_DOUBLE_EQ(maxInterval.R, interval3.R);

  // Повторный вызов должен вернуть тот же максимум (элемент не извлечён).
  SearchInterval& maxInterval2 = data->FindMax();
  ASSERT_DOUBLE_EQ(maxInterval2.R, interval3.R);
}

/**
 * Проверка #SetRecalc / #IsRecalc.
 */
TEST_F(TSearchDataTest, can_set_and_get_recalc_flag)
{
  data->SetRecalc(true);
  EXPECT_TRUE(data->IsRecalc());
  data->SetRecalc(false);
  EXPECT_FALSE(data->IsRecalc());
}

/**
 * Проверка #GetNumOfFuncs.
 */
TEST_F(TSearchDataTest, get_num_of_funcs_returns_correct_value)
{
  EXPECT_EQ(data->GetNumOfFuncs(), MaxNumOfFunc);
}

/**
 * Проверка #GetQueueSize: возвращает максимальный размер очереди.
 */
TEST_F(TSearchDataTest, get_queue_size_returns_max_size)
{
  EXPECT_EQ(data->GetQueueSize(), DefaultQueueSize);
}

/**
 * Проверка обхода дерева итератором в порядке возрастания xl.
 */
TEST_F(TSearchDataTest, iterator_traverses_in_ascending_order)
{
  data->InsertInterval(interval3); // xl = 5
  data->InsertInterval(interval1); // xl = 1
  data->InsertInterval(interval2); // xl = 3

  SearcDataIterator it = data->GetBeginIterator();
  ASSERT_TRUE((void*)it != nullptr);
  ASSERT_DOUBLE_EQ(it->xl().toDouble(), 1.0);
  ++it;
  ASSERT_DOUBLE_EQ(it->xl().toDouble(), 3.0);
  ++it;
  ASSERT_DOUBLE_EQ(it->xl().toDouble(), 5.0);
  ++it;
  EXPECT_EQ((void*)it, nullptr);
}

/**
 * Проверка декремента итератора (#Previous).
 */
TEST_F(TSearchDataTest, iterator_decrement_moves_back)
{
  data->InsertInterval(interval1);
  data->InsertInterval(interval2);
  SearchInterval* p3 = data->InsertInterval(interval3);

  SearcDataIterator it = data->GetIterator(p3); // указывает на xl = 5
  ASSERT_DOUBLE_EQ(it->xl().toDouble(), 5.0);
  --it;
  ASSERT_DOUBLE_EQ(it->xl().toDouble(), 3.0);
  --it;
  ASSERT_DOUBLE_EQ(it->xl().toDouble(), 1.0);
}

/**
 * Проверка #DeleteIntervalFromQueue: удаление уменьшает очередь.
 */
TEST_F(TSearchDataTest, can_delete_interval_from_queue)
{
  SearchInterval* p1 = data->InsertInterval(interval1);
  SearchInterval* p2 = data->InsertInterval(interval2);
  SearchInterval* p3 = data->InsertInterval(interval3);

  data->RefillQueue();

  // Удаляем интервал с максимальным R (interval3).
  data->DeleteIntervalFromQueue(p3);

  // Теперь максимум — interval2.
  SearchInterval& maxInterval = data->FindMax();
  ASSERT_DOUBLE_EQ(maxInterval.R, interval2.R);
}

// ================================================================
// Тесты для SearchInterval
// ================================================================

/**
 * Проверка zl()/zr(): при index < 0 возвращается MaxDouble.
 */
TEST_F(TSearchDataTest, interval_returns_maxdouble_when_index_negative)
{
  SearchInterval interval = SetUpInterval(1.0, 2.0);
  // По умолчанию index == -2 для обеих точек.
  ASSERT_DOUBLE_EQ(interval.zl(), MaxDouble);
  ASSERT_DOUBLE_EQ(interval.zr(), MaxDouble);
  EXPECT_EQ(interval.izl(), -2);
  EXPECT_EQ(interval.izr(), -2);
}

/**
 * Проверка zl()/zr(): при index >= 0 возвращается FuncValues[index].
 */
TEST_F(TSearchDataTest, interval_returns_func_value_when_index_valid)
{
  SearchInterval interval = SetUpInterval(1.0, 2.0);
  interval.LeftPoint->index = 0;
  interval.LeftPoint->FuncValues[0] = -5.0;
  interval.RightPoint->index = 0;
  interval.RightPoint->FuncValues[0] = 7.0;

  ASSERT_DOUBLE_EQ(interval.zl(), -5.0);
  ASSERT_DOUBLE_EQ(interval.zr(), 7.0);
  EXPECT_EQ(interval.izl(), 0);
  EXPECT_EQ(interval.izr(), 0);
}

/**
 * Проверка discreteValuesIndex(): совпадающие индексы возвращаются корректно.
 */
TEST_F(TSearchDataTest, interval_discrete_index_returns_value_when_equal)
{
  SearchInterval interval = SetUpInterval(1.0, 2.0);
  interval.LeftPoint->discreteValuesIndex = 3;
  interval.RightPoint->discreteValuesIndex = 3;

  EXPECT_EQ(interval.discreteValuesIndex(), 3);
}

/**
 * Проверка discreteValuesIndex(): при рассогласовании индексов бросается исключение.
 */
TEST_F(TSearchDataTest, interval_discrete_index_throws_when_mismatch)
{
  SearchInterval interval = SetUpInterval(1.0, 2.0);
  interval.LeftPoint->discreteValuesIndex = 1;
  interval.RightPoint->discreteValuesIndex = 2;

  ASSERT_ANY_THROW(interval.discreteValuesIndex());
}

/**
 * Проверка конструктора копирования SearchInterval.
 */
TEST_F(TSearchDataTest, interval_copy_constructor_copies_fields)
{
  SearchInterval original = SetUpInterval(2.0, 9.0);
  original.locR = 3.3;
  original.ind = 7;
  original.K = 4;

  SearchInterval copy(original);

  EXPECT_EQ(copy.LeftPoint, original.LeftPoint);
  EXPECT_EQ(copy.RightPoint, original.RightPoint);
  ASSERT_DOUBLE_EQ(copy.R, 9.0);
  ASSERT_DOUBLE_EQ(copy.locR, 3.3);
  EXPECT_EQ(copy.ind, 7);
  EXPECT_EQ(copy.K, 4);
}

/**
 * Проверка операторов сравнения SearchInterval (==, <, >) по левой точке.
 */
TEST_F(TSearchDataTest, interval_comparison_operators)
{
  SearchInterval a = SetUpInterval(1.0, 2.0);
  SearchInterval b = SetUpInterval(3.0, 2.0);
  SearchInterval c = SetUpInterval(1.0, 5.0);

  EXPECT_TRUE(a < b);
  EXPECT_TRUE(b > a);
  EXPECT_TRUE(a == c); // сравнение по xl, R не учитывается
}

// ================================================================
// Тесты для связей точек (Trial::GetLeftPoint / GetRightPoint)
// ================================================================

/**
 * После InsertPoint новая точка получает корректные соседние точки.
 */
TEST_F(TSearchDataTest, inserted_point_has_correct_neighbours)
{
  Trial point;
  point = Extended(5.5);
  point.index = 0;

  data->InsertInterval(interval1);
  SearchInterval* pCovering = data->InsertInterval(interval3); // [5,6]
  data->InsertInterval(interval2);

  SearchInterval* pNew = data->InsertPoint(pCovering, point, 1, 1);
  ASSERT_NE(pNew, nullptr);

  // point является левой точкой правого (нового) интервала
  ASSERT_EQ(pNew->LeftPoint, &point);
  // левый интервал точки — исходный covering
  ASSERT_EQ(point.leftInterval, pCovering);
  // правый интервал точки — новый
  ASSERT_EQ(point.rightInterval, pNew);
}

/**
 * InsertPoint возвращает NULL, если точка совпадает с концом интервала.
 */
TEST_F(TSearchDataTest, insert_point_returns_null_for_existing_point)
{
  SearchInterval* pCovering = data->InsertInterval(interval1); // [1,2]

  Trial samePoint;
  samePoint = Extended(1.0); // совпадает с LeftPoint
  samePoint.index = 0;

  SearchInterval* pNew = data->InsertPoint(pCovering, samePoint, 1, 1);
  EXPECT_EQ(pNew, nullptr);
}

/**
 * Проверка GetLeftPoint/GetRightPoint у Trial без установленных интервалов.
 */
TEST_F(TSearchDataTest, trial_neighbours_null_without_intervals)
{
  Trial t;
  EXPECT_EQ(t.GetLeftPoint(), nullptr);
  EXPECT_EQ(t.GetRightPoint(), nullptr);
}
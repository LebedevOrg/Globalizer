#include <gtest/gtest.h>
#include <queue.h>

/**
  Вспомогательный класс, помогающий задать начальную конфигурацию объекта
  класса #TPriorityQueue, которая будет использоваться в тестах
 */

char values [6]= {'a', 'b', 'c', 'd', 'e', 'f'};

class TQueueTest : public ::testing::Test
{
protected:
  static const int MaxQueueSize = 67108863;
  /// Указатель на очередь
  TPriorityQueue* queue;
  static const int maxSize = 3;
  void SetUp()
  {
    queue = new TPriorityQueue(maxSize);
  }
  void TearDown()
  {
    delete queue;
  }
  void SetUpFullQueue()
  {
    queue->Push(2, 2, values + 1);
    queue->Push(4, 4, values + 3);
    queue->Push(6, 6, values + 5);
  }
};
/**
 * Проверка параметра максимальный размер очереди MaxSize
 * MaxSize = 2^k - 1 <= MaxQueueSize
 */
TEST_F(TQueueTest, throws_when_create_queue_with_size_not_divisible_by_power_of_two)
{
  ASSERT_ANY_THROW(TPriorityQueue q(10));
}

TEST_F(TQueueTest, can_create_queue_with_DefaultQueueSize)
{
  ASSERT_NO_THROW(TPriorityQueue q(DefaultQueueSize));
}

TEST_F(TQueueTest, can_create_queue_with_correct_size)
{
  ASSERT_NO_THROW(TPriorityQueue q(1023));
}

TEST_F(TQueueTest, throws_when_memory_for_queue_not_allocated)
{
  ASSERT_ANY_THROW(TPriorityQueue q((MaxQueueSize + 1) * 2 - 1));
}

/**
 * Проверка корректности работы метода #IsEmpty
 */
TEST_F(TQueueTest, can_create_an_empty_queue)
{
  ASSERT_TRUE(queue->IsEmpty());
}

/**
 * Проверка корректности работы метода #Push
 * Push to queue a element with priority.
 * In case of full queue -> push to queue if given element with key greater
 * then some element in queue (replace it)
 */
TEST_F(TQueueTest, can_push_element)
{
  double globalKey = 1;
  double localKey = 1;

  queue->Push(globalKey, localKey, values);

  ASSERT_EQ(1, queue->GetSize());
}

TEST_F(TQueueTest, not_doing_push_to_fill_queue_when_element_is_less_then_min_key)
{
  double key;
  void* value;
  SetUpFullQueue(); //fill queue {(2, 2, "b"),(4, 4, "d"),(6, 6, "f")}

  queue->Push(1, 1, values); //1 < 2

  /// get element with min global key
  /// and check, that it is not (1, 1, "a")
  for (int i = 0; i < 3; i++)
  {
    queue->Pop(&key, &value);
  }
  ASSERT_NE(1, key);
}

TEST_F(TQueueTest, can_push_to_full_queue_when_element_with_largest_key)
{
  double key;
  void* value;
  SetUpFullQueue(); //fill queue {(2, 2, "b"),(4, 4, "d"),(6, 6, "f")}

  queue->Push(7, 7, values + 3); // 7 > 6

  /// get element with max global key
  /// and check, that it is (7, 7, "g")
  queue->Pop(&key, &value);
  ASSERT_EQ(7, key);
}

TEST_F(TQueueTest, can_push_to_full_queue_when_element_is_greater_then_min_key)
{
  double key;
  void* value;
  SetUpFullQueue(); //fill queue {(2, 2, "b"),(4, 4, "d"),(6, 6, "f")}

  queue->Push(3, 3, values + 2); //3 > 2


  /// check, that element (3, 3, "c") isin queue
  for (int i = 0; i < maxSize; i++)
  {
    queue->Pop(&key, &value);
  }
  ASSERT_EQ(3, key);
}

TEST_F(TQueueTest, can_push_to_queue_when_element_is_less_then_min_key)
{
  double key;
  void* value = NULL;
  char* resValue = values;
  queue->Push(2, 2, values + 1);
  queue->Push(3, 3, values + 2);

  queue->Push(1, 1, resValue);

  for (int i = 0; i < 3; i++)
  {
    queue->Pop(&key, &value);
  }
  ASSERT_EQ(1, key);
  ASSERT_EQ(resValue, (char*)value);
}

/**
 * Проверка корректности работы метода #IsFull
 */
TEST_F(TQueueTest, can_detect_when_queue_is_full)
{
  SetUpFullQueue();

  ASSERT_TRUE(queue->IsFull());
}

TEST_F(TQueueTest, can_detect_not_full_queue_when_it_is_empty)
{
  ASSERT_FALSE(queue->IsFull());
}

TEST_F(TQueueTest, can_detect_when_queue_is_not_full)
{
  queue->Push(1, 1, values);
  queue->Push(2, 2, values + 1);

  ASSERT_FALSE(queue->IsFull());
}

/**
 * Проверка корректности работы метода #Pop
 */
TEST_F(TQueueTest, can_pop_element)
{
  double key;
  void* value;
  char* resValue = values + 1;
  queue->Push(1, 1, values);
  queue->Push(2, 2, resValue);

  queue->Pop(&key, &value);

  ASSERT_EQ(2, key);
  ASSERT_EQ(resValue, (char*)value);
}

TEST_F(TQueueTest, throws_when_pop_from_empty_queue)
{
  double key;
  void* value;

  ASSERT_ANY_THROW(queue->Pop(&key, &value));
}

/**
 * Проверка корректности работы метода #PushWithPriority
 * do not push element to queue if given key less then min key in queue
 */
TEST_F(TQueueTest, can_push_to_empty_queue)
{
  queue->PushWithPriority(1, 1, values);

  ASSERT_FALSE(queue->IsEmpty());
}

TEST_F(TQueueTest, can_push_when_element_is_greater_then_min_key)
{
  double key;
  void* value;
  queue->PushWithPriority(1, 1, values);
  queue->PushWithPriority(3, 3, values + 2);

  queue->PushWithPriority(2, 2, values + 2);

  queue->Pop(&key, &value);
  queue->Pop(&key, &value);
  ASSERT_EQ(2, key);
}

TEST_F(TQueueTest, can_push_when_element_is_equal_to_min_key)
{
  double key;
  void* value = NULL;
  char* resValue = values + 2;
  queue->PushWithPriority(1, 1, values);
  queue->PushWithPriority(2, 2, values + 1);

  queue->PushWithPriority(1, 1, resValue);

  for (int i = 0; i < 3; i++)
  {
    queue->Pop(&key, &value);
  }

  ASSERT_EQ(1, key);
  ASSERT_EQ(resValue, (char*)value);
}

TEST_F(TQueueTest, can_push_with_priority_to_full_queue_when_element_is_greater_then_min_key)
{
  double key;
  void* value;
  char* resValue = values + 2;
  SetUpFullQueue(); //{(2, 2, "b"),(4, 4, "d"),(6, 6, "f")}

  queue->PushWithPriority(3, 3, resValue);

  queue->Pop(&key, &value);
  queue->Pop(&key, &value);
  queue->Pop(&key, &value);

  ASSERT_EQ(3, key);
  ASSERT_EQ(resValue, (char*)value);
}

TEST_F(TQueueTest, not_doing_push_to_queue_when_element_is_less_then_min_key)
{
  double key;
  void* value;
  char* resValue = values;
  queue->PushWithPriority(2, 2, values + 1);
  queue->PushWithPriority(3, 3, values + 2);

  queue->PushWithPriority(1, 1, resValue);

  queue->Pop(&key, &value);
  queue->Pop(&key, &value);
  ASSERT_TRUE(queue->IsEmpty());
}
/**
 * Проверка корректности работы метода #Clear
 */
TEST_F(TQueueTest, can_clear_queue)
{
  SetUpFullQueue();

  queue->Clear();

  ASSERT_TRUE(queue->IsEmpty());
}

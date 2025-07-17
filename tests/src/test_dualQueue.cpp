#include "dual_queue.h"
#include "gtest/gtest.h"

/**
  Вспомогательный класс, помогающий задать начальную конфигурацию объекта
  класса #TPriorityQueue, которая будет использоваться в тестах
 */

char elemValues [7]= {'a', 'b', 'c', 'd', 'e', 'f', 'g'};

class TDualQueueTest : public ::testing::Test
{
protected:
  /// Указатель на очередь
  TPriorityDualQueue* queue;
  static const int MaxDualQueueSize = 33554431;
  static const int maxSize = 3;
  void SetUp()
  {
    queue = new TPriorityDualQueue(maxSize);
  }
  void TearDown()
  {
    delete queue;
  }
  void SetUpFullQueue()
  {
    queue->Push(2, 2, elemValues + 1);
    queue->Push(4, 4, elemValues + 3);
    queue->Push(6, 6, elemValues + 5);
  }
};
/**
 * Проверка параметра максимальный размер очереди MaxSize
 * MaxSize = 2^k - 1 <= MaxQueueSize
 */
TEST_F(TDualQueueTest, throws_when_create_queue_with_size_not_divisible_by_power_of_two)
{
  ASSERT_ANY_THROW(TPriorityDualQueue q(10));
}

TEST_F(TDualQueueTest, can_create_queue_with_DefaultQueueSize)
{
  ASSERT_NO_THROW(TPriorityDualQueue q(DefaultQueueSize));
}

TEST_F(TDualQueueTest, can_create_queue_with_correct_size)
{
  ASSERT_NO_THROW(TPriorityDualQueue q(1023));
}

TEST_F(TDualQueueTest, throws_when_memory_for_queue_not_allocated)
{
  ASSERT_ANY_THROW(TPriorityDualQueue q((MaxDualQueueSize + 1) * 2 - 1));
}
/**
 * Проверка корректности работы метода #GetMaxSize
 */
TEST_F(TDualQueueTest, can_get_MaxSize)
{
  int size = maxSize;
  ASSERT_EQ(size, queue->GetMaxSize());
}

/**
 * Проверка корректности работы метода #IsLocalEmpty
 */
TEST_F(TDualQueueTest, can_create_an_empty_loacal_queue)
{
  ASSERT_TRUE(queue->IsLocalEmpty());
}

/**
 * Проверка корректности работы метода #IsEmpty
 */
TEST_F(TDualQueueTest, can_create_an_empty_queue)
{
  ASSERT_TRUE(queue->IsEmpty());
}
/**
 * Проверка корректности работы метода #Push
 * Push to queue a element with priority.
 * In case of full queue -> push to queue if given element with key greater
 * then some element in queue (replace it)
 */
TEST_F(TDualQueueTest, can_push_element)
{
  double globalKey = 1;
  double localKey = 1;

  queue->Push(globalKey, localKey, elemValues);

  ASSERT_EQ(1, queue->GetSize());
  ASSERT_EQ(1, queue->GetLocalSize());
}

TEST_F(TDualQueueTest, not_doing_push_to_fill_queue_when_element_is_less_then_min_key)
{
  double key;
  void* value;
  SetUpFullQueue(); //fill queue {(2, 2, "b"),(4, 4, "d"),(6, 6, "f")}

  queue->Push(1, 5, elemValues); //1 < 2

  /// get element with min global key
  /// and check, that it is not (1, 5, "a")
  for (int i = 0; i < 3; i++)
  {
    queue->Pop(&key, &value);
  }
  ASSERT_NE(1, key);
}

TEST_F(TDualQueueTest, not_doing_push_to_fill_local_queue_when_element_is_less_then_min_key)
{
  double key;
  void* value;
  SetUpFullQueue(); //fill queue {(2, 2, "b"),(4, 4, "d"),(6, 6, "f")}

  queue->Push(5, 1, elemValues); //1 < 2

  /// get element with min local key
  /// and check, that it is not (5, 1, "a")
  for (int i = 0; i < 3; i++)
  {
    queue->PopFromLocal(&key, &value);
  }
  ASSERT_NE(1, key);
}

TEST_F(TDualQueueTest, can_push_to_full_queue_when_element_with_largest_key)
{
  double key;
  void* value;
  SetUpFullQueue(); //fill queue {(2, 2, "b"),(4, 4, "d"),(6, 6, "f")}

  queue->Push(7, 5, elemValues + 6); // 7 > 6

  /// get element with max global key
  /// and check, that it is (7, 5, "g")
  queue->Pop(&key, &value);
  ASSERT_EQ(7, key);
}

TEST_F(TDualQueueTest, can_push_to_full_local_queue_when_element_with_largest_key)
{
  double key;
  void* value;
  SetUpFullQueue(); //fill queue {(2, 2, "b"),(4, 4, "d"),(6, 6, "f")}

  queue->Push(5, 7, elemValues + 6); // 7 > 6

  /// get element with max local key
  /// and check, that it is (5, 7, "g")
  queue->PopFromLocal(&key, &value);
  ASSERT_EQ(7, key);
}

TEST_F(TDualQueueTest, can_push_to_full_queue_when_element_is_greater_then_min_key)
{
  double key;
  void* value;
  SetUpFullQueue(); //fill queue {(2, 2, "b"),(4, 4, "d"),(6, 6, "f")}

  queue->Push(3, 1, elemValues + 2); //3 > 2

  /// check, that element (3, 1, "c") is in queue
  for (int i = 0; i < maxSize; i++)
  {
    queue->Pop(&key, &value);
  }
  ASSERT_EQ(3, key);
}

TEST_F(TDualQueueTest, can_push_to_full_local_queue_when_element_is_greater_then_min_key)
{
  double key;
  void* value;
  SetUpFullQueue(); //fill queue {(2, 2, "b"),(4, 4, "d"),(6, 6, "f")}

  queue->Push(1, 3, elemValues + 2); //3 > 2

  /// check, that element (1, 3, "c") is in queue
  for (int i = 0; i < maxSize; i++)
  {
    queue->PopFromLocal(&key, &value);
  }
  ASSERT_EQ(3, key);
}

TEST_F(TDualQueueTest, can_push_to_queue_when_element_is_less_then_min_key)
{
  double key;
  void* value = NULL;
  char* resValue = elemValues;
  queue->Push(2, 2, elemValues + 1);
  queue->Push(3, 3, elemValues + 2);

  queue->Push(1, 4, resValue);

  for (int i = 0; i < 3; i++)
  {
    queue->Pop(&key, &value);
  }
  ASSERT_EQ(1, key);
  ASSERT_EQ(resValue, (char*)value);
}

TEST_F(TDualQueueTest, can_push_to_local_queue_when_element_is_less_then_min_key)
{
  double key;
  void* value = NULL;
  char* resValue = elemValues;
  queue->Push(2, 2, elemValues + 1);
  queue->Push(3, 3, elemValues + 2);

  queue->Push(4, 1, resValue);

  for (int i = 0; i < 3; i++)
  {
    queue->PopFromLocal(&key, &value);
  }
  ASSERT_EQ(1, key);
  ASSERT_EQ(resValue, (char*)value);
}

/**
 * Проверка корректности работы метода #IsFull
 */
TEST_F(TDualQueueTest, can_detect_when_queue_is_full)
{
  SetUpFullQueue();

  ASSERT_TRUE(queue->IsFull());
}

TEST_F(TDualQueueTest, can_detect_not_full_queue_when_it_is_empty)
{
  ASSERT_FALSE(queue->IsFull());
}

TEST_F(TDualQueueTest, can_detect_when_queue_is_not_full)
{
  queue->Push(1, 1, elemValues);
  queue->Push(2, 2, elemValues + 1);

  ASSERT_FALSE(queue->IsFull());
}

/**
 * Проверка корректности работы метода #IsLocalFull
 */
TEST_F(TDualQueueTest, can_detect_when_local_queue_is_full)
{
  SetUpFullQueue();

  ASSERT_TRUE(queue->IsLocalFull());
}

TEST_F(TDualQueueTest, can_detect_not_full_local_queue_when_it_is_empty)
{
  ASSERT_FALSE(queue->IsLocalFull());
}

TEST_F(TDualQueueTest, can_detect_when_local_queue_is_not_full)
{
  queue->Push(1, 1, elemValues);
  queue->Push(2, 2, elemValues + 1);

  ASSERT_FALSE(queue->IsLocalFull());
}

/**
 * Проверка корректности работы метода #Pop
 */
TEST_F(TDualQueueTest, can_pop_element)
{
  double key;
  void* value;
  char* resValue = elemValues + 1;
  queue->Push(1, 2, elemValues);
  queue->Push(3, 4, resValue);

  queue->Pop(&key, &value);

  ASSERT_EQ(3, key);
  ASSERT_EQ(resValue, (char*)value);
}

TEST_F(TDualQueueTest, method_pop_can_delete_link_element_from_local_queue)
{
  double key;
  void* value;
  char* resValue = elemValues + 1;
  queue->Push(1, 2, elemValues);
  queue->Push(3, 4, resValue);

  queue->Pop(&key, &value);

  ASSERT_EQ(1, queue->GetLocalSize());
}

TEST_F(TDualQueueTest, throws_when_pop_from_empty_queue)
{
  double key;
  void* value;

  ASSERT_ANY_THROW(queue->Pop(&key, &value));
}

/**
 * Проверка корректности работы метода #PopFromLocal
 */
TEST_F(TDualQueueTest, can_Pop_element_from_local_queue)
{
  double key;
  void* value;
  char* resValue = elemValues + 1;
  queue->Push(1, 2, elemValues);
  queue->Push(3, 4, resValue);

  queue->PopFromLocal(&key, &value);

  ASSERT_EQ(4, key);
  ASSERT_EQ(resValue, (char*)value);
}

TEST_F(TDualQueueTest, method_PopFromLocal_can_delete_link_element_from_global_queue)
{
  double key;
  void* value;
  char* resValue = elemValues + 1;
  queue->Push(1, 2, elemValues);
  queue->Push(3, 4, resValue);

  queue->PopFromLocal(&key, &value);

  ASSERT_EQ(1, queue->GetSize());
}

TEST_F(TDualQueueTest, throws_when_pop_from_empty_local_queue)
{
  double key;
  void* value;

  ASSERT_ANY_THROW(queue->PopFromLocal(&key, &value));
}
/**
 * Проверка корректности работы метода #PushWithPriority
 * do not push element to queue if given key less then min key in queue
 */
TEST_F(TDualQueueTest, can_push_to_empty_queue)
{
  queue->PushWithPriority(1, 1, elemValues);

  ASSERT_FALSE(queue->IsEmpty());
  ASSERT_FALSE(queue->IsLocalEmpty());
}

TEST_F(TDualQueueTest, can_push_to_queue_when_element_is_greater_then_min_key)
{
  double key;
  void* value;
  queue->PushWithPriority(1, 1, elemValues);
  queue->PushWithPriority(3, 3, elemValues + 2);

  queue->PushWithPriority(2, 1, elemValues + 1);

  queue->Pop(&key, &value);
  queue->Pop(&key, &value);
  ASSERT_EQ(2, key);
}

TEST_F(TDualQueueTest, can_push_to_local_queue_when_element_is_greater_then_min_key)
{
  double key;
  void* value;
  queue->PushWithPriority(1, 1, elemValues);
  queue->PushWithPriority(3, 3, elemValues + 2);

  queue->PushWithPriority(1, 2, elemValues + 2);

  queue->PopFromLocal(&key, &value);
  queue->PopFromLocal(&key, &value);
  ASSERT_EQ(2, key);
}

TEST_F(TDualQueueTest, can_push_to_queue_when_element_is_equal_to_min_key)
{
  double key;
  void* value = NULL;
  char* resValue = elemValues + 2;
  queue->PushWithPriority(1, 1, elemValues);
  queue->PushWithPriority(2, 2, elemValues + 1);

  queue->PushWithPriority(1, 3, resValue);

  for (int i = 0; i < 3; i++)
  {
    queue->Pop(&key, &value);
  }

  ASSERT_EQ(1, key);
  ASSERT_EQ(resValue, (char*)value);
}

TEST_F(TDualQueueTest, can_push_to_local_queue_when_element_is_equal_to_min_key)
{
  double key;
  void* value = NULL;
  char* resValue = elemValues + 2;
  queue->PushWithPriority(1, 1, elemValues);
  queue->PushWithPriority(2, 2, elemValues + 1);

  queue->PushWithPriority(3, 1, resValue);

  for (int i = 0; i < 3; i++)
  {
    queue->PopFromLocal(&key, &value);
  }

  ASSERT_EQ(1, key);
  ASSERT_EQ(resValue, (char*)value);
}

TEST_F(TDualQueueTest, can_push_to_queue_with_priority_to_full_queue_when_element_is_greater_then_min_key)
{
  double key;
  void* value;
  char* resValue = elemValues + 2;
  SetUpFullQueue(); //{(2, 2, "b"),(4, 4, "d"),(6, 6, "f")}

  queue->PushWithPriority(3, 1, resValue);

  queue->Pop(&key, &value);
  queue->Pop(&key, &value);
  queue->Pop(&key, &value);

  ASSERT_EQ(3, key);
  ASSERT_EQ(resValue, (char*)value);
}

TEST_F(TDualQueueTest, can_push_to_local_queue_with_priority_to_full_queue_when_element_is_greater_then_min_key)
{
  double key;
  void* value;
  char* resValue = elemValues + 2;
  SetUpFullQueue(); //{(2, 2, "b"),(4, 4, "d"),(6, 6, "f")}

  queue->PushWithPriority(1, 3, resValue);

  queue->PopFromLocal(&key, &value);
  queue->PopFromLocal(&key, &value);
  queue->PopFromLocal(&key, &value);

  ASSERT_EQ(3, key);
  ASSERT_EQ(resValue, (char*)value);
}

TEST_F(TDualQueueTest, not_doing_push_to_queue_when_element_is_less_then_min_key)
{
  double key;
  void* value;
  char* resValue = elemValues;
  queue->PushWithPriority(2, 2, elemValues + 1);
  queue->PushWithPriority(3, 3, elemValues + 2);

  queue->PushWithPriority(1, 4, resValue);

  queue->Pop(&key, &value);
  queue->Pop(&key, &value);
  ASSERT_TRUE(queue->IsEmpty());
}

TEST_F(TDualQueueTest, not_doing_push_to_local_queue_when_element_is_less_then_min_key)
{
  double key;
  void* value;
  char* resValue = elemValues;
  queue->PushWithPriority(2, 2, elemValues + 1);
  queue->PushWithPriority(3, 3, elemValues + 2);

  queue->PushWithPriority(4, 1, resValue);

  queue->PopFromLocal(&key, &value);
  queue->PopFromLocal(&key, &value);
  ASSERT_TRUE(queue->IsLocalEmpty());
}

/**
 * Проверка корректности работы метода #Clear
 */
TEST_F(TDualQueueTest, can_clear_queue)
{
  SetUpFullQueue();

  queue->Clear();

  ASSERT_TRUE(queue->IsEmpty());
  ASSERT_TRUE(queue->IsLocalEmpty());
}

/**
 * Проверка корректности работы метода #Resize
 */
TEST_F(TDualQueueTest, can_resize_queue)
{
  SetUpFullQueue();

  queue->Resize(maxSize + 1);

  ASSERT_EQ(maxSize + 1, queue->GetMaxSize());
  ASSERT_TRUE(queue->IsEmpty());
  ASSERT_TRUE(queue->IsLocalEmpty());
}

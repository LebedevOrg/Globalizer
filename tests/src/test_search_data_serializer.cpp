/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//  File:      test_search_data_serializer.cpp                             //
//                                                                         //
//  Purpose:   Модульные тесты для класса SearchDataSerializer             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

/**
\file test_search_data_serializer.cpp
\brief Тесты сериализации/десериализации поисковых данных в JSON.

Замечания по окружению:
  * Глобальный объект parameters инициализируется через Init(..., isMPIInit=false).
  * Для сериализации нужны И SearchData, И Task (TrialToJson -> pTask->TransformPoint).
  * Тесты создают временные .json файлы и удаляют их в TearDown.
  * SearchData сам владеет добавленными в него Trial* (удаляет в деструкторе),
    поэтому точки, помещённые в pData->GetTrials(), НЕ удаляем вручную.
*/

#include "SearchDataSerializer.h"
#include "SearchData.h"
#include "Task.h"
#include "Trial.h"
#include "TrialFactory.h"
#include "ProblemFromFunctionPointers.h"

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <fstream>
#include <cstdio>
#include <functional>

using namespace std;

// ------------------------------------------------------------------------------------------------
// Fixture: готовит parameters, задачу, Task, SearchData и сериализатор.
// ------------------------------------------------------------------------------------------------
class SearchDataSerializerTest : public ::testing::Test
{
protected:
  static constexpr int kDim = 2;
  static constexpr int kFunc = 1; // 1 критерий, 0 ограничений

  IProblem* problem = nullptr;
  Task* task = nullptr;
  SearchData* data = nullptr;
  SearchDataSerializer* serializer = nullptr;

  std::vector<std::string> tempFiles; // для очистки

  void SetUp() override
  {
    // 1. Инициализируем глобальные параметры.
    parameters.Init(0, nullptr, /*isMPIInit=*/false);
    parameters.Dimension = kDim;

    // 2. Простая непрерывная задача (сфера) в [-1, 1]^2 с известным оптимумом 0 в нуле.
    problem = new ProblemFromFunctionPointers(
      kDim,
      std::vector<double>(kDim, -1.0),   // lower
      std::vector<double>(kDim, 1.0),   // upper
      std::vector<std::function<double(const double*)>>(1, [](const double* y) {
        return y[0] * y[0] + y[1] * y[1];
        }),
      true, 0.0, std::vector<double>(kDim, 0.0));

    task = new Task(problem, 0);

    // 3. Поисковые данные.
    data = new SearchData(kFunc);

    // 4. Сериализатор с привязанными data и task.
    serializer = new SearchDataSerializer();
    serializer->SetSearchData(data);
    serializer->SetTask(task);
  }

  void TearDown() override
  {
    delete serializer;
    delete data;      // SearchData удалит принадлежащие ему Trial*
    delete task;
    delete problem;

    for (const auto& f : tempFiles)
      std::remove(f.c_str());
  }

  // Уникальное имя временного файла (регистрируется для удаления).
  std::string TempFile(const std::string& name)
  {
    std::string full = "test_sds_" + name + ".json";
    tempFiles.push_back(full);
    return full;
  }

  // Утилита: добавить в data точку и вернуть указатель.
  Trial* AddTrial(double x, double y0, double y1, double funcVal, int index = 0)
  {
    Trial* t = TrialFactory::CreateTrial();
    t->SetX(Extended(x));
    t->y[0] = y0;
    t->y[1] = y1;
    t->FuncValues[0] = funcVal;
    t->index = index;
    t->K = 1;
    data->GetTrials().push_back(t); // владение переходит к SearchData
    return t;
  }

  // Утилита: прочитать весь файл в строку.
  static std::string ReadWholeFile(const std::string& path)
  {
    std::ifstream f(path.c_str());
    if (!f.is_open()) return "";
    return std::string((std::istreambuf_iterator<char>(f)),
      std::istreambuf_iterator<char>());
  }

  static bool Contains(const std::string& hay, const std::string& needle)
  {
    return hay.find(needle) != std::string::npos;
  }
};

const int SearchDataSerializerTest::kDim;
const int SearchDataSerializerTest::kFunc;

/* ======================================================================== *\
**  Конструирование / базовое состояние                                     **
\* ======================================================================== */

///  сериализатор создаётся без исключений
TEST(SearchDataSerializer_Ctor, can_create)
{
  ASSERT_NO_THROW({ SearchDataSerializer s; });
}

///  SerializeFullState без SearchData возвращает "{}"
TEST(SearchDataSerializer_Ctor, serialize_without_data_returns_empty_object)
{
  SearchDataSerializer s;
  ASSERT_EQ("{}", s.SerializeFullState());
}

///  SaveProgress без SearchData возвращает false
TEST(SearchDataSerializer_Ctor, save_without_data_returns_false)
{
  SearchDataSerializer s;
  ASSERT_FALSE(s.SaveProgress("dummy.json"));
}

/* ======================================================================== *\
**  SerializeFullState — структура JSON                                     **
\* ======================================================================== */

///  полная сериализация содержит обязательные ключи верхнего уровня
TEST_F(SearchDataSerializerTest, full_state_contains_top_level_keys)
{
  AddTrial(0.5, 0.1, -0.2, 0.05);

  std::string json = serializer->SerializeFullState();

  EXPECT_TRUE(Contains(json, "\"version\""));
  EXPECT_TRUE(Contains(json, "\"timestamp\""));
  EXPECT_TRUE(Contains(json, "\"mode\": \"full\""));
  EXPECT_TRUE(Contains(json, "\"method_parameters\""));
  EXPECT_TRUE(Contains(json, "\"search_data\""));
  EXPECT_TRUE(Contains(json, "\"trials\""));
  EXPECT_TRUE(Contains(json, "\"best_trial\""));
}

///  search_data содержит NumOfFuncs, Count, M, Z, local_r
TEST_F(SearchDataSerializerTest, full_state_contains_search_data_fields)
{
  std::string json = serializer->SerializeFullState();

  EXPECT_TRUE(Contains(json, "\"NumOfFuncs\""));
  EXPECT_TRUE(Contains(json, "\"Count\""));
  EXPECT_TRUE(Contains(json, "\"M\""));
  EXPECT_TRUE(Contains(json, "\"Z\""));
  EXPECT_TRUE(Contains(json, "\"local_r\""));
}

///  version в JSON совпадает с parameters.version
TEST_F(SearchDataSerializerTest, full_state_version_matches_parameters)
{
  std::string json = serializer->SerializeFullState();
  EXPECT_TRUE(Contains(json, parameters.version));
}

///  best_trial == null, если лучшая точка не задана
TEST_F(SearchDataSerializerTest, full_state_best_trial_null_when_absent)
{
  // BestTrial по умолчанию nullptr
  std::string json = serializer->SerializeFullState();
  EXPECT_TRUE(Contains(json, "\"best_trial\": null"));
}

///  при заданной лучшей точке best_trial — это объект, а не null
TEST_F(SearchDataSerializerTest, full_state_best_trial_object_when_set)
{
  Trial* t = AddTrial(0.3, 0.0, 0.0, 0.0);
  data->SetBestTrial(t);

  std::string json = serializer->SerializeFullState();
  EXPECT_FALSE(Contains(json, "\"best_trial\": null"));
  // объект содержит поля точки
  EXPECT_TRUE(Contains(json, "\"best_trial\": {"));
}

/* ======================================================================== *\
**  TrialToJson (косвенно через SerializeFullState)                          **
\* ======================================================================== */

///  сериализованная точка содержит все ожидаемые поля
TEST_F(SearchDataSerializerTest, trial_json_contains_all_fields)
{
  AddTrial(0.42, 0.1, 0.2, 0.05);

  std::string json = serializer->SerializeFullState();

  EXPECT_TRUE(Contains(json, "\"x\":"));
  EXPECT_TRUE(Contains(json, "\"discreteValuesIndex\":"));
  EXPECT_TRUE(Contains(json, "\"y\":["));
  EXPECT_TRUE(Contains(json, "\"FuncValues\":["));
  EXPECT_TRUE(Contains(json, "\"index\":"));
  EXPECT_TRUE(Contains(json, "\"K\":"));
  EXPECT_TRUE(Contains(json, "\"lowAndUpPoints\":"));
  EXPECT_TRUE(Contains(json, "\"TypeColor\":"));
}

///  недопустимое (MaxDouble) значение функции сериализуется как null
TEST_F(SearchDataSerializerTest, trial_json_maxdouble_func_becomes_null)
{
  Trial* t = TrialFactory::CreateTrial();
  t->SetX(Extended(0.5));
  t->FuncValues[0] = MaxDouble;   // невычисленное значение
  t->index = -1;
  data->GetTrials().push_back(t);

  std::string json = serializer->SerializeFullState();
  EXPECT_TRUE(Contains(json, "null"));
}

/* ======================================================================== *\
**  SaveProgress — первое (полное) сохранение в файл                        **
\* ======================================================================== */

///  первый SaveProgress создаёт непустой файл
TEST_F(SearchDataSerializerTest, save_progress_creates_file)
{
  AddTrial(0.5, 0.1, 0.2, 0.05);
  std::string file = TempFile("create");

  ASSERT_TRUE(serializer->SaveProgress(file));

  std::string content = ReadWholeFile(file);
  EXPECT_FALSE(content.empty());
  EXPECT_TRUE(Contains(content, "\"trials\""));
}

///  повторный вызов SaveProgress не бросает (переход в режим дозаписи)
TEST_F(SearchDataSerializerTest, save_progress_second_call_appends)
{
  Trial* t1 = AddTrial(0.3, 0.0, 0.0, 0.0);
  std::string file = TempFile("append");

  ASSERT_TRUE(serializer->SaveProgress(file));

  // Добавляем новую точку и дозаписываем.
  Trial* t2 = AddTrial(0.6, 0.5, 0.5, 0.5);
  std::vector<Trial*> newTrials = { t2 };

  ASSERT_NO_THROW(serializer->SaveProgress(file, newTrials,
    std::vector<SearchInterval*>(), t2));

  std::string content = ReadWholeFile(file);
  EXPECT_FALSE(content.empty());
}

///  ResetFirstSave заставляет следующий SaveProgress снова писать полный файл
TEST_F(SearchDataSerializerTest, reset_first_save_restarts_full_save)
{
  AddTrial(0.5, 0.1, 0.2, 0.05);
  std::string file = TempFile("reset");

  ASSERT_TRUE(serializer->SaveProgress(file));   // полное сохранение
  serializer->ResetFirstSave();                  // сброс
  ASSERT_TRUE(serializer->SaveProgress(file));   // снова полное (не append)

  std::string content = ReadWholeFile(file);
  EXPECT_TRUE(Contains(content, "\"mode\": \"full\""));
}

/* ======================================================================== *\
**  LoadFromFile — базовые случаи                                           **
\* ======================================================================== */

///  загрузка несуществующего файла возвращает false
TEST_F(SearchDataSerializerTest, load_nonexistent_file_returns_false)
{
  SearchDataSerializer::LoadedFileData out;
  ASSERT_FALSE(serializer->LoadFromFile("no_such_file_12345.json", out));
}

///  загрузка полностью пустого/битого файла не приводит к падению
TEST_F(SearchDataSerializerTest, load_garbage_file_no_crash)
{
  std::string file = TempFile("garbage");
  {
    std::ofstream f(file.c_str());
    f << "this is not json at all {{{";
  }

  SearchDataSerializer::LoadedFileData out;
  // Функция может вернуть false (нет search_data), но не должна падать.
  ASSERT_NO_THROW(serializer->LoadFromFile(file, out));
}

/* ======================================================================== *\
**  Round-trip: сохранить -> загрузить                                       **
\* ======================================================================== */

///  round-trip восстанавливает версию, режим и метаданные
TEST_F(SearchDataSerializerTest, roundtrip_restores_metadata)
{
  AddTrial(0.25, 0.1, 0.2, 0.05);
  std::string file = TempFile("meta");

  ASSERT_TRUE(serializer->SaveProgress(file));

  SearchDataSerializer::LoadedFileData out;
  ASSERT_TRUE(serializer->LoadFromFile(file, out));

  EXPECT_EQ(parameters.version, out.version);
  EXPECT_EQ("full", out.mode);
}

///  round-trip восстанавливает search_data (NumOfFuncs, Count, M/Z, local_r)
TEST_F(SearchDataSerializerTest, roundtrip_restores_search_data)
{
  AddTrial(0.25, 0.1, 0.2, 0.05);
  data->M[0] = 3.5;
  data->Z[0] = -1.25;
  data->local_r = 2.0;

  std::string file = TempFile("sdata");
  ASSERT_TRUE(serializer->SaveProgress(file));

  SearchDataSerializer::LoadedFileData out;
  ASSERT_TRUE(serializer->LoadFromFile(file, out));

  EXPECT_EQ(kFunc, out.searchData.NumOfFuncs);
  ASSERT_FALSE(out.searchData.M.empty());
  ASSERT_FALSE(out.searchData.Z.empty());
  EXPECT_NEAR(3.5, out.searchData.M[0], 1e-9);
  EXPECT_NEAR(-1.25, out.searchData.Z[0], 1e-9);
  EXPECT_NEAR(2.0, out.searchData.local_r, 1e-9);
}

///  round-trip восстанавливает число загруженных точек
TEST_F(SearchDataSerializerTest, roundtrip_restores_trials_count)
{
  AddTrial(0.2, 0.1, 0.1, 0.02);
  AddTrial(0.4, 0.2, 0.2, 0.08);
  AddTrial(0.6, 0.3, 0.3, 0.18);

  std::string file = TempFile("trials");
  ASSERT_TRUE(serializer->SaveProgress(file));

  SearchDataSerializer::LoadedFileData out;
  ASSERT_TRUE(serializer->LoadFromFile(file, out));

  EXPECT_EQ(3u, out.trials.size());

  // Загруженные точки принадлежат out (LoadedFileData) — освобождаем их вручную,
  // т.к. деструктор LoadedFileData в текущей реализации их не удаляет.
  for (Trial* t : out.trials)
    delete t;
  // bestTrial: если он входит в trials — уже удалён; если отдельный — освободим,
  // но только если он НЕ из числа out.trials (здесь совпадает — не трогаем).
}

///  round-trip восстанавливает координаты и значение функции точки
TEST_F(SearchDataSerializerTest, roundtrip_restores_trial_values)
{
  AddTrial(0.5, 0.3, -0.4, 0.25, /*index=*/0);

  std::string file = TempFile("vals");
  ASSERT_TRUE(serializer->SaveProgress(file));

  SearchDataSerializer::LoadedFileData out;
  ASSERT_TRUE(serializer->LoadFromFile(file, out));

  ASSERT_EQ(1u, out.trials.size());
  Trial* loaded = out.trials[0];

  EXPECT_NEAR(0.3, loaded->y[0], 1e-6);
  EXPECT_NEAR(-0.4, loaded->y[1], 1e-6);
  EXPECT_NEAR(0.25, loaded->FuncValues[0], 1e-6);
  EXPECT_EQ(0, loaded->index);

  for (Trial* t : out.trials)
    delete t;
}

///  round-trip восстанавливает лучшую точку
TEST_F(SearchDataSerializerTest, roundtrip_restores_best_trial)
{
  Trial* best = AddTrial(0.5, 0.0, 0.0, 0.0, /*index=*/0);
  data->SetBestTrial(best);

  std::string file = TempFile("best");
  ASSERT_TRUE(serializer->SaveProgress(file));

  SearchDataSerializer::LoadedFileData out;
  ASSERT_TRUE(serializer->LoadFromFile(file, out));

  ASSERT_NE(nullptr, out.bestTrial);
  EXPECT_NEAR(0.0, out.bestTrial->FuncValues[0], 1e-6);

  // Освобождаем загруженные точки; bestTrial здесь совпадает с одной из trials
  // (LoadBestTrial возвращает найденный candidate), поэтому отдельно не удаляем.
  for (Trial* t : out.trials)
    delete t;
}

/* ======================================================================== *\
**  LoadedFileData — семантика перемещения                                   **
\* ======================================================================== */

///  move-конструктор LoadedFileData переносит данные и обнуляет источник
TEST_F(SearchDataSerializerTest, loaded_file_data_move_transfers_ownership)
{
  AddTrial(0.5, 0.1, 0.2, 0.05);
  std::string file = TempFile("move");
  ASSERT_TRUE(serializer->SaveProgress(file));

  SearchDataSerializer::LoadedFileData src;
  ASSERT_TRUE(serializer->LoadFromFile(file, src));

  size_t n = src.trials.size();
  Trial* srcBest = src.bestTrial;

  SearchDataSerializer::LoadedFileData dst(std::move(src));

  EXPECT_EQ(n, dst.trials.size());
  EXPECT_EQ(srcBest, dst.bestTrial);
  EXPECT_EQ(nullptr, src.bestTrial);   // источник обнулён move-конструктором

  for (Trial* t : dst.trials)
    delete t;
}

/* ======================================================================== *\
**  JSONParser (через публичный вложенный класс)                            **
\* ======================================================================== */

///  ParseObject разбирает простой объект в пары ключ-значение
TEST(SearchDataSerializer_JSONParser, parse_simple_object)
{
  SearchDataSerializer::JSONParser p("{\"a\": 1, \"b\": \"hello\"}");
  std::map<std::string, std::string> obj = p.ParseObject();

  ASSERT_EQ(2u, obj.size());
  EXPECT_EQ("1", obj["a"]);
  EXPECT_EQ("hello", obj["b"]);
}

///  ParseDoubleArray разбирает массив чисел
TEST(SearchDataSerializer_JSONParser, parse_double_array)
{
  SearchDataSerializer::JSONParser p("[1.5, -2.0, 3.25]");
  std::vector<double> arr = p.ParseDoubleArray();

  ASSERT_EQ(3u, arr.size());
  EXPECT_NEAR(1.5, arr[0], 1e-9);
  EXPECT_NEAR(-2.0, arr[1], 1e-9);
  EXPECT_NEAR(3.25, arr[2], 1e-9);
}

///  ParseIntArray разбирает массив целых
TEST(SearchDataSerializer_JSONParser, parse_int_array)
{
  SearchDataSerializer::JSONParser p("[10, 20, 30]");
  std::vector<int> arr = p.ParseIntArray();

  ASSERT_EQ(3u, arr.size());
  EXPECT_EQ(10, arr[0]);
  EXPECT_EQ(20, arr[1]);
  EXPECT_EQ(30, arr[2]);
}

///  ParseArray разбирает массив объектов
TEST(SearchDataSerializer_JSONParser, parse_array_of_objects)
{
  SearchDataSerializer::JSONParser p("[{\"k\": 1}, {\"k\": 2}]");
  std::vector<std::map<std::string, std::string>> arr = p.ParseArray();

  ASSERT_EQ(2u, arr.size());
  EXPECT_EQ("1", arr[0]["k"]);
  EXPECT_EQ("2", arr[1]["k"]);
}

///  ParseObject на пустой строке возвращает пустой объект (без падения)
TEST(SearchDataSerializer_JSONParser, parse_empty_string)
{
  SearchDataSerializer::JSONParser p("");
  std::map<std::string, std::string> obj = p.ParseObject();
  EXPECT_TRUE(obj.empty());
}

///  вложенный объект возвращается как строка (ParseObjectAsString через значение)
TEST(SearchDataSerializer_JSONParser, parse_nested_object_as_string_value)
{
  SearchDataSerializer::JSONParser p("{\"outer\": {\"inner\": 5}}");
  std::map<std::string, std::string> obj = p.ParseObject();

  ASSERT_EQ(1u, obj.size());
  // значение "outer" содержит сырую строку вложенного объекта
  EXPECT_TRUE(obj["outer"].find("inner") != std::string::npos);
}
#include "SerializeToDashBoard.h"
#include "../../lib/nlohmann_json/json.hpp"
#include "Task.h"
#include "SearcDataIterator.h"
#include <fstream>
#include <iomanip>
#include <ctime>
#include "TrialFactory.h"
#include "GlobalizerBenchmarksProblem.h"

using json = nlohmann::json;

// ------------------------------------------------------------------------------------------------
SerializeToDashBoard::SerializeToDashBoard()
{
}

// ------------------------------------------------------------------------------------------------
json SerializeToDashBoard::TrialToJson(Trial* trial, Task* pTask)
{
  json jTrial;

  // Количество непрерывных и дискретных переменных
  int numCont = pTask->GetNumberOfContinuousVariable();
  int numDiscr = pTask->GetNumberOfDiscreteVariable();

  // Непрерывные переменные
  std::vector<double> floatVars(trial->y, trial->y + numCont);
  jTrial["float_variables"] = floatVars;

  auto problem1 = pTask->getProblem();

  if (numDiscr > 0) 
  {
#ifdef _GLOBALIZER_BENCHMARKS
    GlobalizerBenchmarksProblem* problem2 = dynamic_cast<GlobalizerBenchmarksProblem*> (problem1);
    if (problem2 != nullptr)
    {
      std::vector<double> y;
      std::vector<std::string> u;

      problem2->XtoYU(trial->y, y, u);
      jTrial["discrete_variables"] = u;
    }
    else
#endif
    {

      // Значения дискретных параметров из массива y
      std::vector<double> discrVars(trial->y + numCont, trial->y + numCont + numDiscr);
      jTrial["discrete_variables"] = discrVars;
    }
  }
  else
  {
    // Дискретные переменные
    jTrial["discrete_variables"] = json::array();
  }

  // Значения функций
  json jFuncValues = json::array();
  for (int i = 0; i < pTask->GetNumOfFunc(); ++i)
  {
    json jFunc;
    jFunc["value"] = trial->FuncValues[i];
    jFunc["type"] = 1; // Все как целевые функции
    jFunc["functionID"] = std::to_string(i);
    jFuncValues.push_back(jFunc);
  }
  jTrial["function_values"] = jFuncValues;

  // Дополнительные параметры
  jTrial["x"] = trial->X().toDouble();
  jTrial["delta"] = trial->leftInterval ? trial->leftInterval->delta : 0.0;
  jTrial["globalR"] = trial->leftInterval ? trial->leftInterval->R : 0.0;
  jTrial["localR"] = 0.0; // Не используется в текущей реализации
  jTrial["index"] = trial->index;
  jTrial["discrete_value_index"] = trial->discreteValuesIndex;
  jTrial["__z"] = trial->FuncValues[trial->index];
  jTrial["creation_time"] = trial->creationTime;
  jTrial["iterationNumber"] = trial->iterationNumber;

  jTrial["localR"] = -1.0;

  return jTrial;
}

// ------------------------------------------------------------------------------------------------
json SerializeToDashBoard::ParametersToJson(Parameters& parameters)
{
  json jParams;
  jParams["eps"] = parameters.Epsilon.operator double();
  jParams["r"] = parameters.r.operator double();
  jParams["iters_limit"] = parameters.MaxNumOfPoints.operator int();

  jParams["start_point"] = json::array();
  if (parameters.StartPoint.GetSize() > 0 && parameters.IsUseStartPoint)
  {
    for (int i = 0; i < parameters.StartPoint.GetSize(); i++)
    {

      double value = parameters.StartPoint[i];
      jParams["start_point"].push_back(value);
    }
  }

  jParams["number_of_parallel_points"] = parameters.NumPoints.operator int();
  return jParams;
}

// ------------------------------------------------------------------------------------------------
json SerializeToDashBoard::SearchDataToJson(SearchData* pSearchData)
{
  json jSearchData;
  jSearchData["NumOfFuncs"] = pSearchData->NumOfFuncs;
  jSearchData["Count"] = pSearchData->GetCount();

  // Массив M
  json jM = json::array();
  for (int i = 0; i < pSearchData->NumOfFuncs; ++i)
    jM.push_back(pSearchData->M[i]);
  jSearchData["M"] = jM;

  // Массив Z
  json jZ = json::array();
  for (int i = 0; i < pSearchData->NumOfFuncs; ++i)
    jZ.push_back(pSearchData->Z[i]);
  jSearchData["Z"] = jZ;

  return jSearchData;
}

// ------------------------------------------------------------------------------------------------
double SerializeToDashBoard::CalculateAchievedAccuracy(Task* pTask, Trial* bestTrial)
{
  if (bestTrial == nullptr || !pTask) {
    return HUGE_VAL;
  }

  double AchievedAccuracy = HUGE_VAL;
  int numOfOptima = 1;
  double* allOptimumPoints = new double[MAX_TRIAL_DIMENSION * MAX_NUM_MIN];

  // Для оптимального значения функции
  if (pTask->GetIsOptimumValueDefined() && pTask->GetNumOfFunc() > bestTrial->index) {
    AchievedAccuracy = fabs(bestTrial->FuncValues[bestTrial->index] - pTask->GetOptimumValue());
  }

  // Для оптимальной точки в пространстве
  if (pTask->GetIsOptimumPointDefined()) {
    double PointDifference = 0.0;
    for (int i = 0; i < pTask->GetN(); i++) {
      double diff = fabs(bestTrial->y[i] - pTask->GetOptimumPoint()[i]);
      PointDifference = GLOBALIZER_MAX(diff, PointDifference);
    }

    if (pTask->getProblem()->GetAllOptimumPoint(allOptimumPoints, numOfOptima) != IProblem::UNDEFINED) {
      PointDifference = HUGE_VAL;
      for (int j = 0; j < numOfOptima; j++) {
        double currentPointDifference = 0.0;
        for (int i = 0; i < pTask->GetN(); i++) {
          double diff = fabs(bestTrial->y[i] - allOptimumPoints[j * pTask->GetN() + i]);
          currentPointDifference = GLOBALIZER_MAX(diff, currentPointDifference);
        }
        if (currentPointDifference < PointDifference) {
          PointDifference = currentPointDifference;
        }
      }
    }

    // Возвращаем точность по точке, если она определяется
    AchievedAccuracy = PointDifference;
  }

  delete[] allOptimumPoints;
  return AchievedAccuracy;
}

// ------------------------------------------------------------------------------------------------
json SerializeToDashBoard::SolutionResultToJson(SolutionResult& solutionResult, Task* pTask)
{
  json jSolution;
  jSolution["number_of_trials"] = solutionResult.TrialCount;
  jSolution["number_of_global_trials"] = solutionResult.IterationCount; // Иногда сохраняют как количество итераций
  jSolution["number_of_local_trials"] = 0; // В текущей реализации не используется
  jSolution["solving_time"] = solutionResult.SolvingTime;        
  jSolution["solution_accuracy"] = CalculateAchievedAccuracy(pTask, solutionResult.BestTrial);

  jSolution["num_iteration_best_trial"] = json::array();
  // Лучшая итерация
  if (solutionResult.BestTrial) {
    jSolution["num_iteration_best_trial"].push_back(solutionResult.BestTrial->iterationNumber);
  }
  else {
    jSolution["num_iteration_best_trial"].push_back(0);
  }

  return jSolution;
}

// ------------------------------------------------------------------------------------------------
bool SerializeToDashBoard::SaveFullState(const std::string& filename,
  SearchData* pSearchData,
  SolutionResult& solutionResult,
  Task* pTask,
  Parameters& parameters,
  const std::vector<Trial*>& trials,
  const std::vector<Trial*>& bestTrials)
{
  json jRoot;

  // Заголовочная информация
  jRoot["version"] = "1.0";
  jRoot["timestamp"] = time(nullptr);
  jRoot["mode"] = "full";


  // Все точки испытаний
  jRoot["SearchDataItem"] = json::array();
  for (Trial* trial : trials) {
    jRoot["SearchDataItem"].push_back(TrialToJson(trial, pTask));
  }

  // Параметры метода
  jRoot["Parameters"] = json::array();
  jRoot["Parameters"].push_back(ParametersToJson(parameters));

  // Поисковые данные
  jRoot["SearchData"] = SearchDataToJson(pSearchData);


  // Лучшие точки
  jRoot["best_trials"] = json::array();
  for (Trial* bestTrial : bestTrials) {
    jRoot["best_trials"].push_back(TrialToJson(bestTrial, pTask));
  }

  // Информация о решении
  jRoot["solution"] = json::array();
  jRoot["solution"].push_back(SolutionResultToJson(solutionResult, pTask));

  // Информация о задаче
  jRoot["Task"] = json::array();
  jRoot["Task"].push_back(ProblemToJson(*pTask));

  // Запись в файл
  try {
    std::ofstream file(filename);
    file << std::setw(1) << jRoot;
    return true;
  }
  catch (...) {
    return false;
  }
  return false;
}

// ------------------------------------------------------------------------------------------------
bool SerializeToDashBoard::LoadFromFile(const std::string& filename,
  std::vector<Trial*>& outTrials,
  Trial*& outBestTrial,
  Task* pTask)
{
  try
  {
    std::ifstream file(filename);
    if (!file.is_open())
      return false;

    json jRoot = json::parse(file);

    // Получаем количество непрерывных и дискретных переменных если задача задана
    int numCont = 0;
    int numDiscr = 0;

#ifdef _GLOBALIZER_BENCHMARKS
    GlobalizerBenchmarksProblem* gbProblem = nullptr;
#endif

    if (pTask != nullptr)
    {
      numCont = pTask->GetNumberOfContinuousVariable();
      numDiscr = pTask->GetNumberOfDiscreteVariable();

#ifdef _GLOBALIZER_BENCHMARKS
      gbProblem = dynamic_cast<GlobalizerBenchmarksProblem*>(pTask->getProblem());
#endif
    }

    // Определяем формат файла:
    // "новый" формат (SaveFullState) использует "SearchDataItem" и "best_trials"
    // "старый" формат (qqq.json)     использует "trials"          и "best_trial"
    const bool isNewFormat = jRoot.contains("SearchDataItem");
    const bool isOldFormat = jRoot.contains("trials");

    // ----------------------------------------------------------------
    // Лямбда для загрузки одного испытания из json
    // ----------------------------------------------------------------
    auto LoadTrialFromJson = [&](const json& jTrial) -> Trial*
      {
        Trial* trial = TrialFactory::CreateTrial();

        // ---- Координаты ------------------------------------------------
        if (isNewFormat && jTrial.contains("float_variables"))
        {
          // Новый формат: "float_variables": [x0, x1, x2, ...]
          const auto& floatVars = jTrial["float_variables"];
          for (size_t i = 0; i < floatVars.size(); ++i)
            trial->y[i] = floatVars[i].get<double>();
        }
        else if (jTrial.contains("y"))
        {
          // Старый формат: "y": [x0, x1, x2, ...]
          const auto& yArr = jTrial["y"];
          for (size_t i = 0; i < yArr.size(); ++i)
            trial->y[i] = yArr[i].get<double>();
        }

        // ---- Дискретные переменные -------------------------------------
        if (isNewFormat && jTrial.contains("discrete_variables") && numDiscr > 0)
        {
          const auto& discrVars = jTrial["discrete_variables"];

#ifdef _GLOBALIZER_BENCHMARKS
          if (gbProblem != nullptr && !discrVars.empty() && discrVars[0].is_string())
          {
            std::vector<std::string> u;
            for (size_t i = 0; i < discrVars.size(); ++i)
              u.push_back(discrVars[i].get<std::string>());

            std::vector<double> y(trial->y, trial->y + numCont);
            gbProblem->YUtoX(y, u, trial->y);
          }
          else
#endif
          {
            for (size_t i = 0; i < discrVars.size(); ++i)
              trial->y[numCont + i] = discrVars[i].get<double>();
          }
        }

        // ---- Значения функций ------------------------------------------
        if (isNewFormat && jTrial.contains("function_values"))
        {
          // Новый формат: "function_values": [{"value": 1.23, ...}, ...]
          const auto& funcValues = jTrial["function_values"];
          for (size_t i = 0; i < funcValues.size(); ++i)
            trial->FuncValues[i] = funcValues[i]["value"].get<double>();
        }
        else if (jTrial.contains("FuncValues"))
        {
          // Старый формат: "FuncValues": [1.23, null, ...]
          const auto& funcValues = jTrial["FuncValues"];
          for (size_t i = 0; i < funcValues.size(); ++i)
          {
            if (funcValues[i].is_null())
              trial->FuncValues[i] = std::numeric_limits<double>::quiet_NaN();
            else
              trial->FuncValues[i] = funcValues[i].get<double>();
          }
        }

        // ---- Координата x ----------------------------------------------
        if (jTrial.contains("x"))
          trial->SetX(Extended(jTrial["x"].get<double>()));

        // ---- index -----------------------------------------------------
        if (jTrial.contains("index"))
          trial->index = jTrial["index"].get<int>();

        // ---- discreteValuesIndex / discrete_value_index ----------------
        if (jTrial.contains("discrete_value_index"))
          trial->discreteValuesIndex = jTrial["discrete_value_index"].get<int>();
        else if (jTrial.contains("discreteValuesIndex"))
          trial->discreteValuesIndex = jTrial["discreteValuesIndex"].get<int>();

        // ---- K ---------------------------------------------------------
        if (jTrial.contains("K"))
          trial->K = jTrial["K"].get<int>();

        // ---- TypeColor -------------------------------------------------
        if (jTrial.contains("TypeColor"))
          trial->TypeColor = jTrial["TypeColor"].get<int>();

        // ---- iterationNumber -------------------------------------------
        if (jTrial.contains("iterationNumber"))
          trial->iterationNumber = jTrial["iterationNumber"].get<int>();

        // ---- creation_time ---------------------------------------------
        if (jTrial.contains("creation_time"))
          trial->creationTime = jTrial["creation_time"].get<double>();

        return trial;
      };

    // ----------------------------------------------------------------
    // Загрузка точек испытаний
    // ----------------------------------------------------------------
    if (isNewFormat && jRoot.contains("SearchDataItem"))
    {
      for (const auto& jTrial : jRoot["SearchDataItem"])
        outTrials.push_back(LoadTrialFromJson(jTrial));
    }
    else if (isOldFormat)
    {
      for (const auto& jTrial : jRoot["trials"])
        outTrials.push_back(LoadTrialFromJson(jTrial));
    }

    // ----------------------------------------------------------------
    // Загрузка лучшей точки
    // ----------------------------------------------------------------
    if (isNewFormat && jRoot.contains("best_trials") && !jRoot["best_trials"].empty())
    {
      // Новый формат: "best_trials": [{...}, ...]
      outBestTrial = LoadTrialFromJson(jRoot["best_trials"][0]);
    }
    else if (jRoot.contains("best_trial") && !jRoot["best_trial"].is_null())
    {
      // Старый формат: "best_trial": {...}
      outBestTrial = LoadTrialFromJson(jRoot["best_trial"]);
    }

    return true;
  }
  catch (const std::exception& e)
  {
    return false;
  }
}

// ------------------------------------------------------------------------------------------------
json SerializeToDashBoard::ProblemToJson(Task& task)
{
  json jProblem;

  // Версия формата задачи
  jProblem["version"] = "2.0";

  // Проверка инициализации
  if (!task.IsInit()) {
    throw std::runtime_error("Task is not initialized in ProblemToJson");
  }

  // Берем границы области поиска
  const double* a = task.GetA();
  const double* b = task.GetB();
  // Количество непрерывных и дискретных переменных
  int numCont = task.GetNumberOfContinuousVariable();
  int numDiscr = task.GetNumberOfDiscreteVariable();

  // Формат float_variables из log_Rastrigin.json
  jProblem["float_variables"] = json::array();
  
  for (int i = 0; i < numCont; i++)
  {
    json float_variables;
    float_variables[std::to_string(i)] = {a[i], b[i]};
    jProblem["float_variables"].push_back(float_variables);
  }
  

  if (numDiscr > 0)
  {
#ifdef _GLOBALIZER_BENCHMARKS
    GlobalizerBenchmarksProblem* problem2 = dynamic_cast<GlobalizerBenchmarksProblem*> (task.getProblem());
    
    if (problem2 != nullptr)
    {
      std::vector<std::vector<std::string>> vals = problem2->GetDiscretesValues();
      jProblem["discrete_variables"] = json::array();
      for (int i = 0; i < vals.size(); i++)
      {
        json discrete_variables;
        discrete_variables[std::to_string(i)] = vals[i];
        jProblem["discrete_variables"].push_back(discrete_variables);
      }
    }
    else
#endif
    {
      // Дискретные переменные
      jProblem["discrete_variables"] = json::array();
    }
  }
  else
  {
    // Дискретные переменные
    jProblem["discrete_variables"] = json::array();
  }


  // TODO: Реализовать получение имени задачи
  jProblem["name"] = "Unnamed";

  // Существующее детальное описание
  json jSearchSpace;

  // ... существующий код (discrete_parameters, continuous_parameters и т.д.)

  jProblem["problem"] = jSearchSpace;

  // Существующие метаданные
  json jMetadata;
  // ... существующий код метаданных
  jProblem["metadata"] = jMetadata;

  return jProblem;
}
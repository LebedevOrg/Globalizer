#pragma once

#include "Trial.h"
#include "Parameters.h"
#include "Task.h"
#include "SearchData.h"
#include "SolutionResult.h"

#include <vector>
#include <string>
#include "../../lib/nlohmann_json/json.hpp"

using json = nlohmann::json;

class SerializeToDashBoard
{
public:
  SerializeToDashBoard();

  json TrialToJson(Trial* trial, Task* pTask);
  json ParametersToJson(Parameters& parameters);
  json SearchDataToJson(SearchData* pSearchData);
  json SolutionResultToJson(SolutionResult& solutionResult, Task* pTask);
  json ProblemToJson(Task& task);

  // Метод для записи полного состояния в файл
  bool SaveFullState(const std::string& filename,
    SearchData* pSearchData,
    SolutionResult& solutionResult,
    Task* pTask,
    Parameters& parameters,
    const std::vector<Trial*>& trials,
    const std::vector<Trial*>& bestTrials);

  // Метод для загрузки состояния из файла
  bool LoadFromFile(const std::string& filename,
    std::vector<Trial*>& outTrials,
    Trial*& outBestTrial,
    Task* pTask = nullptr);

  /** Расчитывает достигнутую точность при решении задачи
*
*/
  double CalculateAchievedAccuracy(Task* pTask, Trial* bestTrial);
};
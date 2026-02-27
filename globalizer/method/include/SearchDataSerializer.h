/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      method.h                                                    //
//                                                                         //
//  Purpose:   Header file for method class                                //
//                                                                         //
//  Author(s): Lebedev.i                                                   //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


#ifndef __SEARCH_DATA_SERIALIZER_H__
#define __SEARCH_DATA_SERIALIZER_H__

#include "SearchData.h"
#include "Trial.h"
#include "SearchInterval.h"
#include "SearcDataIterator.h"
#include "Parameters.h"
#include "TrialFactory.h"
#include "SearchIntervalFactory.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <vector>
#include <stack>
#include <cmath>
#include <cctype>
#include <map>
#include <algorithm>
#include "Task.h"

// ------------------------------------------------------------------------------------------------
/**
 * \brief Класс для сериализации поисковых данных в JSON формат
 */
class SearchDataSerializer
{
private:
  /// Поисковая информация
  SearchData* pSearchData;

  /// текущая задача
  Task* pTask;

  /// Рабочее имя файла
  std::string currentFileName;
  /// Произведено ли начальное сохранение
  bool isFirstSave;

  /// Экранирование специальных символов в строке для JSON
  static std::string EscapeJsonString(const std::string& str);

  /// Форматирование числа с плавающей точкой для JSON
  static std::string FormatDouble(double value);

  /// Получение текущего времени в ISO формате
  static std::string GetCurrentTimeISO();

  /// Преобразование int в строку
  static std::string IntToString(int value);

  /// Преобразование строки в double
  static double StringToDouble(const std::string& str);

  /// Преобразование строки в int
  static int StringToInt(const std::string& str);

  /// Удаление пробелов в начале и конце строки
  static std::string Trim(const std::string& str);

  /// Разделение строки по разделителю
  static std::vector<std::string> Split(const std::string& str, char delimiter);

  /// Сериализация Trial в JSON строку
  std::string TrialToJson(Trial* trial);

  /// Сериализация SearchInterval в JSON строку
  std::string IntervalToJson(SearchInterval* interval);

  /// Создание Trial из JSON данных
  Trial* CreateTrialFromJSON(const std::map<std::string, std::string>& data);

  /// Создание SearchInterval из JSON данных
  SearchInterval* CreateIntervalFromJSON(const std::map<std::string, std::string>& data,
    Trial* leftPoint, Trial* rightPoint);

  /// Сохранение полного состояния в файл
  bool SaveFullState(const std::string& filename);

  /// Добавление новых точек в существующий файл
  bool AppendNewPoints(const std::vector<Trial*>& newTrials,
    const std::vector<SearchInterval*>& newIntervals,
    Trial* newBestTrial);

public:
  SearchDataSerializer();

  /// Задаем поисковую информацию
  void SetSearchData(SearchData* data);

  /// Задачем задачу
  void SetTask(Task* task);

  /// Сериализация полного состояния в JSON строку
  std::string SerializeFullState();

  /// Сохранение состояния (первый раз - полное, потом - добавление)
  bool SaveProgress(const std::string& filename,
    const std::vector<Trial*>& newTrials = std::vector<Trial*>(),
    const std::vector<SearchInterval*>& newIntervals = std::vector<SearchInterval*>(),
    Trial* newBestTrial = nullptr);


  /// Загрузка состояния из файла
  bool LoadFromFile(const std::string& filename);

  /// Очищаем информацию о файле
  void ResetFirstSave();


/**
 * \brief Простой JSON парсер для чтения данных из файла
 */
  class JSONParser
  {
  private:
    /// Весь файл в строке
    std::string content;

    /// текущая позиция
    size_t pos;

    void SkipWhitespace();

    std::string ParseString();

    double ParseNumber();

    std::string ParseWord();

    std::string ParseArrayAsString();

    std::string ParseObjectAsString();

  public:
    JSONParser(const std::string& str);

    void Reset();

    std::map<std::string, std::string> ParseObject();

    std::vector<std::map<std::string, std::string> > ParseArray();

    std::vector<double> ParseDoubleArray();

    std::vector<int> ParseIntArray();
  };
};

#endif // __SEARCH_DATA_SERIALIZER_H__
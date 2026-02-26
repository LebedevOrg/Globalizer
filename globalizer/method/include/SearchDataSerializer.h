/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2025 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      SearchDataSerializer.h                                      //
//                                                                         //
//  Purpose:   Serialization of search data to JSON format                 //
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

// ------------------------------------------------------------------------------------------------
/**
 * \brief Класс для сериализации поисковых данных в JSON формат
 */
class SearchDataSerializer
{
private:
  /// Поисковая информация
  SearchData* pSearchData;
  /// Рабочее имя файла
  std::string currentFileName;
  /// Произведено ли начальное сохранение
  bool isFirstSave;

  /// Экранирование специальных символов в строке для JSON
  static std::string EscapeJsonString(const std::string& str)
  {
    std::ostringstream oss;
    for (size_t i = 0; i < str.length(); ++i)
    {
      char c = str[i];
      switch (c)
      {
      case '"': oss << "\\\""; break;
      case '\\': oss << "\\\\"; break;
      case '\b': oss << "\\b"; break;
      case '\f': oss << "\\f"; break;
      case '\n': oss << "\\n"; break;
      case '\r': oss << "\\r"; break;
      case '\t': oss << "\\t"; break;
      default:
        if (static_cast<unsigned char>(c) <= 0x1f)
        {
          oss << "\\u" << std::hex << std::setw(4) << std::setfill('0')
            << static_cast<int>(c);
        }
        else
        {
          oss << c;
        }
      }
    }
    return oss.str();
  }

  /// Форматирование числа с плавающей точкой для JSON
  static std::string FormatDouble(double value)
  {
    if (std::isinf(value) || std::isnan(value))
    {
      return "null";
    }
    std::ostringstream oss;
    oss.precision(15);
    oss << std::fixed << value;
    return oss.str();
  }

  /// Получение текущего времени в ISO формате
  static std::string GetCurrentTimeISO()
  {
    time_t now = time(nullptr);
    struct tm tmbuf;
#ifdef _WIN32
    localtime_s(&tmbuf, &now);
#else
    localtime_r(&now, &tmbuf);
#endif
    char buffer[32];
    strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", &tmbuf);
    return std::string(buffer);
  }

  /// Преобразование int в строку
  static std::string IntToString(int value)
  {
    std::ostringstream oss;
    oss << value;
    return oss.str();
  }

  /// Преобразование строки в double
  static double StringToDouble(const std::string& str)
  {
    if (str == "null" || str.empty())
    {
      return MaxDouble;
    }
    std::istringstream iss(str);
    double value;
    iss >> value;
    return value;
  }

  /// Преобразование строки в int
  static int StringToInt(const std::string& str)
  {
    std::istringstream iss(str);
    int value;
    iss >> value;
    return value;
  }

  /// Удаление пробелов в начале и конце строки
  static std::string Trim(const std::string& str)
  {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos)
      return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
  }

  /// Разделение строки по разделителю
  static std::vector<std::string> Split(const std::string& str, char delimiter)
  {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter))
    {
      tokens.push_back(Trim(token));
    }
    return tokens;
  }

  // ==================== JSON парсер ====================

  /**
   * \brief Простой JSON парсер для чтения данных из файла
   */
  class JSONParser
  {
  private:
    std::string content;
    size_t pos;

    void SkipWhitespace()
    {
      while (pos < content.length() &&
        (content[pos] == ' ' || content[pos] == '\t' ||
          content[pos] == '\n' || content[pos] == '\r'))
      {
        pos++;
      }
    }

    std::string ParseString()
    {
      if (pos >= content.length() || content[pos] != '"')
      {
        return "";
      }

      pos++; // Пропускаем открывающую кавычку
      std::string result;

      while (pos < content.length())
      {
        char c = content[pos];
        if (c == '"')
        {
          pos++; // Пропускаем закрывающую кавычку
          break;
        }
        else if (c == '\\')
        {
          pos++;
          if (pos < content.length())
          {
            switch (content[pos])
            {
            case '"': result += '"'; break;
            case '\\': result += '\\'; break;
            case '/': result += '/'; break;
            case 'b': result += '\b'; break;
            case 'f': result += '\f'; break;
            case 'n': result += '\n'; break;
            case 'r': result += '\r'; break;
            case 't': result += '\t'; break;
            default: result += content[pos];
            }
          }
        }
        else
        {
          result += c;
        }
        pos++;
      }

      return result;
    }

    double ParseNumber()
    {
      size_t start = pos;
      while (pos < content.length() &&
        (isdigit(content[pos]) || content[pos] == '.' ||
          content[pos] == '-' || content[pos] == '+' ||
          content[pos] == 'e' || content[pos] == 'E'))
      {
        pos++;
      }
      std::string numStr = content.substr(start, pos - start);
      return StringToDouble(numStr);
    }

    std::string ParseWord()
    {
      size_t start = pos;
      while (pos < content.length() && isalpha(content[pos]))
      {
        pos++;
      }
      return content.substr(start, pos - start);
    }

    std::string ParseArrayAsString()
    {
      if (pos >= content.length() || content[pos] != '[')
      {
        return "";
      }

      size_t start = pos;
      int depth = 1;
      pos++;

      while (pos < content.length() && depth > 0)
      {
        if (content[pos] == '[') depth++;
        else if (content[pos] == ']') depth--;
        pos++;
      }

      return content.substr(start, pos - start);
    }

    std::string ParseObjectAsString()
    {
      if (pos >= content.length() || content[pos] != '{')
      {
        return "";
      }

      size_t start = pos;
      int depth = 1;
      pos++;

      while (pos < content.length() && depth > 0)
      {
        if (content[pos] == '{') depth++;
        else if (content[pos] == '}') depth--;
        pos++;
      }

      return content.substr(start, pos - start);
    }

  public:
    JSONParser(const std::string& str) : content(str), pos(0) {}

    void Reset()
    {
      pos = 0;
    }

    std::map<std::string, std::string> ParseObject()
    {
      std::map<std::string, std::string> result;

      SkipWhitespace();
      if (pos >= content.length() || content[pos] != '{')
      {
        return result;
      }
      pos++; // Пропускаем '{'

      while (pos < content.length())
      {
        SkipWhitespace();
        if (content[pos] == '}')
        {
          pos++;
          break;
        }

        // Парсим ключ
        std::string key = ParseString();
        SkipWhitespace();

        if (pos < content.length() && content[pos] == ':')
        {
          pos++; // Пропускаем ':'
        }
        SkipWhitespace();

        // Парсим значение
        std::string value;
        if (pos < content.length())
        {
          if (content[pos] == '"')
          {
            value = ParseString();
          }
          else if (content[pos] == '{')
          {
            value = ParseObjectAsString();
          }
          else if (content[pos] == '[')
          {
            value = ParseArrayAsString();
          }
          else if (isdigit(content[pos]) || content[pos] == '-')
          {
            size_t start = pos;
            while (pos < content.length() &&
              (isdigit(content[pos]) || content[pos] == '.' ||
                content[pos] == '-' || content[pos] == '+' ||
                content[pos] == 'e' || content[pos] == 'E'))
            {
              pos++;
            }
            value = content.substr(start, pos - start);
          }
          else if (isalpha(content[pos]))
          {
            value = ParseWord();
          }
        }

        result[key] = value;

        SkipWhitespace();
        if (pos < content.length() && content[pos] == ',')
        {
          pos++; // Пропускаем ','
        }
      }

      return result;
    }

    std::vector<std::map<std::string, std::string> > ParseArray()
    {
      std::vector<std::map<std::string, std::string> > result;

      SkipWhitespace();
      if (pos >= content.length() || content[pos] != '[')
      {
        return result;
      }
      pos++; // Пропускаем '['

      while (pos < content.length())
      {
        SkipWhitespace();
        if (content[pos] == ']')
        {
          pos++;
          break;
        }

        if (content[pos] == '{')
        {
          std::map<std::string, std::string> obj = ParseObject();
          result.push_back(obj);
        }

        SkipWhitespace();
        if (pos < content.length() && content[pos] == ',')
        {
          pos++; // Пропускаем ','
        }
      }

      return result;
    }

    std::vector<double> ParseDoubleArray()
    {
      std::vector<double> result;

      std::string arrayStr = ParseArrayAsString();
      if (arrayStr.empty())
      {
        return result;
      }

      // Убираем скобки
      if (arrayStr.length() >= 2 && arrayStr[0] == '[' && arrayStr[arrayStr.length() - 1] == ']')
      {
        arrayStr = arrayStr.substr(1, arrayStr.length() - 2);
      }

      std::vector<std::string> tokens = Split(arrayStr, ',');
      for (size_t i = 0; i < tokens.size(); ++i)
      {
        result.push_back(StringToDouble(tokens[i]));
      }

      return result;
    }

    std::vector<int> ParseIntArray()
    {
      std::vector<int> result;

      std::string arrayStr = ParseArrayAsString();
      if (arrayStr.empty())
      {
        return result;
      }

      // Убираем скобки
      if (arrayStr.length() >= 2 && arrayStr[0] == '[' && arrayStr[arrayStr.length() - 1] == ']')
      {
        arrayStr = arrayStr.substr(1, arrayStr.length() - 2);
      }

      std::vector<std::string> tokens = Split(arrayStr, ',');
      for (size_t i = 0; i < tokens.size(); ++i)
      {
        result.push_back(StringToInt(tokens[i]));
      }

      return result;
    }
  };

  /// Сериализация Trial в JSON строку
  std::string TrialToJson(Trial* trial)
  {
    std::ostringstream json;
    json << "{";

    // Координата на одномерном отрезке
    json << "\"x\":" << FormatDouble(trial->X().toDouble()) << ",";

    // Индекс дискретного параметра
    json << "\"discreteValuesIndex\":" << trial->discreteValuesIndex << ",";

    // Многомерная точка
    json << "\"y\":[";
    for (int i = 0; i < parameters.Dimension; ++i)
    {
      if (i > 0) json << ",";
      json << FormatDouble(trial->y[i]);
    }
    json << "],";

    // Значения функций
    json << "\"FuncValues\":[";
    for (int i = 0; i < pSearchData->NumOfFuncs; ++i)
    {
      if (i > 0) json << ",";
      if (std::isfinite(trial->FuncValues[i]) && trial->FuncValues[i] < MaxDouble / 2)
      {
        json << FormatDouble(trial->FuncValues[i]);
      }
      else
      {
        json << "null";
      }
    }
    json << "],";

    // Индекс точки
    json << "\"index\":" << trial->index << ",";

    // Число вложенных итераций
    json << "\"K\":" << trial->K << ",";

    // Количество точек для локального метода
    json << "\"lowAndUpPoints\":" << trial->lowAndUpPoints << ",";

    // Цвет точки
    json << "\"TypeColor\":" << trial->TypeColor;

    json << "}";

    return json.str();
  }

  /// Сериализация SearchInterval в JSON строку
  std::string IntervalToJson(SearchInterval* interval)
  {
    std::ostringstream json;
    json << "{";

    // Левая и правая точки (по x координате)
    json << "\"left_x\":" << FormatDouble(interval->xl().toDouble()) << ",";
    json << "\"right_x\":" << FormatDouble(interval->xr().toDouble()) << ",";

    // Характеристики интервала
    json << "\"R\":" << FormatDouble(interval->R) << ",";
    json << "\"localR\":" << FormatDouble(interval->locR) << ",";
    json << "\"delta\":" << FormatDouble(interval->delta) << ",";
    json << "\"iterationNumber\":" << interval->ind << ",";
    json << "\"K\":" << interval->K;

    json << "}";

    return json.str();
  }

  /// Создание Trial из JSON данных
  Trial* CreateTrialFromJSON(const std::map<std::string, std::string>& data)
  {
    Trial* trial = TrialFactory::CreateTrial();

    // Парсим x
    std::map<std::string, std::string>::const_iterator it = data.find("x");
    if (it != data.end())
    {
      double xVal = StringToDouble(it->second);
      trial->SetX(Extended(xVal));
    }

    // Парсим discreteValuesIndex
    it = data.find("discreteValuesIndex");
    if (it != data.end())
    {
      trial->discreteValuesIndex = StringToInt(it->second);
    }

    // Парсим y (многомерная точка)
    it = data.find("y");
    if (it != data.end())
    {
      std::string yStr = it->second;
      // Убираем скобки
      if (yStr.length() >= 2 && yStr[0] == '[' && yStr[yStr.length() - 1] == ']')
      {
        yStr = yStr.substr(1, yStr.length() - 2);
      }

      std::vector<std::string> tokens = Split(yStr, ',');
      for (size_t i = 0; i < tokens.size() && i < (size_t)parameters.Dimension; ++i)
      {
        trial->y[i] = StringToDouble(tokens[i]);
      }
    }

    // Парсим FuncValues
    it = data.find("FuncValues");
    if (it != data.end())
    {
      std::string fvStr = it->second;
      if (fvStr.length() >= 2 && fvStr[0] == '[' && fvStr[fvStr.length() - 1] == ']')
      {
        fvStr = fvStr.substr(1, fvStr.length() - 2);
      }

      std::vector<std::string> tokens = Split(fvStr, ',');
      for (size_t i = 0; i < tokens.size() && i < (size_t)pSearchData->NumOfFuncs; ++i)
      {
        if (tokens[i] != "null")
        {
          trial->FuncValues[i] = StringToDouble(tokens[i]);
        }
        else
        {
          trial->FuncValues[i] = MaxDouble;
        }
      }
    }

    // Парсим index
    it = data.find("index");
    if (it != data.end())
    {
      trial->index = StringToInt(it->second);
    }

    // Парсим K
    it = data.find("K");
    if (it != data.end())
    {
      trial->K = StringToInt(it->second);
    }

    // Парсим lowAndUpPoints
    it = data.find("lowAndUpPoints");
    if (it != data.end())
    {
      trial->lowAndUpPoints = StringToInt(it->second);
    }

    // Парсим TypeColor
    it = data.find("TypeColor");
    if (it != data.end())
    {
      trial->TypeColor = StringToInt(it->second);
    }

    return trial;
  }

  /// Создание SearchInterval из JSON данных
  SearchInterval* CreateIntervalFromJSON(const std::map<std::string, std::string>& data,
    Trial* leftPoint, Trial* rightPoint)
  {
    SearchInterval* interval = SearchIntervalFactory::CreateSearchInterval();

    interval->LeftPoint = leftPoint;
    interval->RightPoint = rightPoint;

    // Парсим R
    std::map<std::string, std::string>::const_iterator it = data.find("R");
    if (it != data.end())
    {
      interval->R = StringToDouble(it->second);
    }

    // Парсим localR
    it = data.find("localR");
    if (it != data.end())
    {
      interval->locR = StringToDouble(it->second);
    }

    // Парсим delta
    it = data.find("delta");
    if (it != data.end())
    {
      interval->delta = StringToDouble(it->second);
    }

    // Парсим iterationNumber
    it = data.find("iterationNumber");
    if (it != data.end())
    {
      interval->ind = StringToInt(it->second);
    }

    // Парсим K
    it = data.find("K");
    if (it != data.end())
    {
      interval->K = StringToInt(it->second);
    }

    return interval;
  }

  /// Сохранение полного состояния в файл
  bool SaveFullState(const std::string& filename)
  {
    std::ofstream file(filename.c_str());
    if (!file.is_open())
    {
      return false;
    }

    file << SerializeFullState();
    file.close();

    currentFileName = filename;
    isFirstSave = false;

    return true;
  }

  /// Добавление новых точек в существующий файл
  bool AppendNewPoints(const std::vector<Trial*>& newTrials,
    const std::vector<SearchInterval*>& newIntervals,
    Trial* newBestTrial)
  {
    if (currentFileName.empty())
    {
      return false;
    }

    // Читаем существующий файл
    std::ifstream infile(currentFileName.c_str());
    if (!infile.is_open())
    {
      return false;
    }

    std::string content;
    std::string line;
    while (std::getline(infile, line))
    {
      content += line + "\n";
    }
    infile.close();

    // ==================== ОБНОВЛЕНИЕ search_data ====================

    // 1. Обновляем Count
    size_t countPos = content.find("\"Count\": ");
    if (countPos != std::string::npos)
    {
      size_t countEnd = content.find(",", countPos);
      if (countEnd == std::string::npos)
      {
        countEnd = content.find("\n", countPos);
      }
      if (countEnd != std::string::npos)
      {
        std::string newCount = "\"Count\": " + IntToString(pSearchData->GetCount());
        content.replace(countPos, countEnd - countPos, newCount);
      }
    }

    // 2. Обновляем массив M (оценки констант Липшица)
    size_t mPos = content.find("\"M\": [");
    if (mPos != std::string::npos)
    {
      size_t mEnd = content.find("]", mPos);
      if (mEnd != std::string::npos)
      {
        std::string newM = "\"M\": [";
        for (int i = 0; i < pSearchData->NumOfFuncs; ++i)
        {
          if (i > 0) newM += ",";
          newM += FormatDouble(pSearchData->M[i]);
        }
        newM += "]";
        content.replace(mPos, mEnd - mPos + 1, newM);
      }
    }

    // 3. Обновляем массив Z (минимальные значения функций)
    size_t zPos = content.find("\"Z\": [");
    if (zPos != std::string::npos)
    {
      size_t zEnd = content.find("]", zPos);
      if (zEnd != std::string::npos)
      {
        std::string newZ = "\"Z\": [";
        for (int i = 0; i < pSearchData->NumOfFuncs; ++i)
        {
          if (i > 0) newZ += ",";
          newZ += FormatDouble(pSearchData->Z[i]);
        }
        newZ += "]";
        content.replace(zPos, zEnd - zPos + 1, newZ);
      }
    }

    // 4. Обновляем local_r
    size_t localRPos = content.find("\"local_r\": ");
    if (localRPos != std::string::npos)
    {
      size_t localREnd = content.find(",", localRPos);
      if (localREnd == std::string::npos)
      {
        localREnd = content.find("\n", localRPos);
      }
      if (localREnd != std::string::npos)
      {
        std::string newLocalR = "\"local_r\": " + FormatDouble(pSearchData->local_r);
        content.replace(localRPos, localREnd - localRPos, newLocalR);
      }
    }

    // 5. Обновляем общее количество интервалов (если есть отдельное поле)
    size_t intervalsCountPos = content.find("\"intervals_count\": ");
    if (intervalsCountPos != std::string::npos)
    {
      size_t intervalsCountEnd = content.find(",", intervalsCountPos);
      if (intervalsCountEnd == std::string::npos)
      {
        intervalsCountEnd = content.find("\n", intervalsCountPos);
      }
      if (intervalsCountEnd != std::string::npos)
      {
        std::string newIntervalsCount = "\"intervals_count\": " + IntToString(pSearchData->GetCount() - 1);
        content.replace(intervalsCountPos, intervalsCountEnd - intervalsCountPos, newIntervalsCount);
      }
    }

    // ==================== ОБНОВЛЕНИЕ best_trial ====================

    // Обновляем лучшую точку
    size_t bestTrialPos = content.find("\"best_trial\": ");
    if (bestTrialPos != std::string::npos && newBestTrial != nullptr)
    {
      size_t bestTrialEnd = content.find("\n", bestTrialPos);
      size_t commaPos = content.find(",", bestTrialPos);
      if (commaPos < bestTrialEnd)
      {
        bestTrialEnd = commaPos;
      }

      std::string newBestStr = "\"best_trial\": " + TrialToJson(newBestTrial);
      content.replace(bestTrialPos, bestTrialEnd - bestTrialPos, newBestStr);
    }

    // ==================== ДОБАВЛЕНИЕ НОВЫХ ТОЧЕК ====================

    // Добавляем новые точки
    if (newTrials.size() > 0)
    {
      size_t trialsPos = content.find("\"trials\": [");
      if (trialsPos != std::string::npos)
      {
        size_t trialsEnd = content.find("]", trialsPos + 10);
        if (trialsEnd != std::string::npos)
        {
          std::string newTrialsStr;
          for (size_t i = 0; i < newTrials.size(); ++i)
          {
            if (i > 0) newTrialsStr += ",";
            newTrialsStr += "\n    " + TrialToJson(newTrials[i]);
          }

          // Проверяем, не пустой ли массив
          bool isEmptyArray = true;
          for (size_t i = trialsPos + 10; i < trialsEnd; ++i)
          {
            if (!isspace(content[i]) && content[i] != '[' && content[i] != ']')
            {
              isEmptyArray = false;
              break;
            }
          }

          if (!isEmptyArray)
          {
            content.insert(trialsEnd, "," + newTrialsStr);
          }
          else
          {
            content.insert(trialsEnd, newTrialsStr + "\n  ");
          }
        }
      }
    }

    // ==================== ДОБАВЛЕНИЕ НОВЫХ ИНТЕРВАЛОВ ====================

    // Добавляем новые интервалы
    if (newIntervals.size() > 0)
    {
      size_t intervalsPos = content.find("\"intervals\": [");
      if (intervalsPos != std::string::npos)
      {
        size_t intervalsEnd = content.find("]", intervalsPos + 13);
        if (intervalsEnd != std::string::npos)
        {
          std::string newIntervalsStr;
          for (size_t i = 0; i < newIntervals.size(); ++i)
          {
            if (i > 0) newIntervalsStr += ",";
            newIntervalsStr += "\n    " + IntervalToJson(newIntervals[i]);
          }

          bool isEmptyArray = true;
          for (size_t i = intervalsPos + 13; i < intervalsEnd; ++i)
          {
            if (!isspace(content[i]) && content[i] != '[' && content[i] != ']')
            {
              isEmptyArray = false;
              break;
            }
          }

          if (!isEmptyArray)
          {
            content.insert(intervalsEnd, "," + newIntervalsStr);
          }
          else
          {
            content.insert(intervalsEnd, newIntervalsStr + "\n  ");
          }
        }
      }
    }

    // ==================== ОБНОВЛЕНИЕ ВРЕМЕННОЙ МЕТКИ ====================

    // Обновляем временную метку
    size_t timestampPos = content.find("\"timestamp\": \"");
    if (timestampPos != std::string::npos)
    {
      size_t timestampEnd = content.find("\"", timestampPos + 14);
      if (timestampEnd != std::string::npos)
      {
        std::string newTimestamp = "\"timestamp\": \"" + GetCurrentTimeISO() + "\"";
        content.replace(timestampPos, timestampEnd - timestampPos + 1, newTimestamp);
      }
    }

    // Записываем обновленный файл
    std::ofstream outfile(currentFileName.c_str());
    if (!outfile.is_open())
    {
      return false;
    }

    outfile << content;
    outfile.close();

    return true;
  }

public:
  SearchDataSerializer() : pSearchData(nullptr), isFirstSave(true) {}

  void SetSearchData(SearchData* data)
  {
    pSearchData = data;
  }

  /// Сериализация полного состояния в JSON строку
  std::string SerializeFullState()
  {
    if (!pSearchData)
    {
      return "{}";
    }

    std::ostringstream json;
    json << "{\n";

    // Версия и метаданные
    json << "  \"version\": \"1.0\",\n";
    json << "  \"timestamp\": \"" << GetCurrentTimeISO() << "\",\n";
    json << "  \"mode\": \"full\",\n";

    // Параметры метода
    json << "  \"method_parameters\": {\n";
    json << "    \"eps\": " << FormatDouble(parameters.Epsilon) << ",\n";
    json << "    \"r\": " << FormatDouble(parameters.r) << ",\n";
    json << "    \"iters_limit\": " << parameters.MaxNumOfPoints << ",\n";
    json << "    \"number_of_parallel_points\": " << parameters.NumPoints << ",\n";
    json << "    \"start_point\": [";
    for (int i = 0; i < parameters.Dimension; ++i)
    {
      if (i > 0) json << ",";
      json << FormatDouble(parameters.startPoint[i]);
    }
    json << "]\n";
    json << "  },\n";

    // Информация о поисковых данных
    json << "  \"search_data\": {\n";
    json << "    \"NumOfFuncs\": " << pSearchData->NumOfFuncs << ",\n";
    json << "    \"Count\": " << pSearchData->GetCount() << ",\n";

    json << "    \"M\": [";
    for (int i = 0; i < pSearchData->NumOfFuncs; ++i)
    {
      if (i > 0) json << ",";
      json << FormatDouble(pSearchData->M[i]);
    }
    json << "],\n";

    json << "    \"Z\": [";
    for (int i = 0; i < pSearchData->NumOfFuncs; ++i)
    {
      if (i > 0) json << ",";
      json << FormatDouble(pSearchData->Z[i]);
    }
    json << "],\n";

    json << "    \"local_r\": " << FormatDouble(pSearchData->local_r) << "\n";
    json << "  },\n";

    // Все точки испытаний
    json << "  \"trials\": [\n";
    std::vector<Trial*>& trials = pSearchData->GetTrials();
    for (size_t i = 0; i < trials.size(); ++i)
    {
      if (i > 0) json << ",\n";
      json << "    " << TrialToJson(trials[i]);
    }
    json << "\n  ],\n";

    // Все интервалы
    json << "  \"intervals\": [\n";
    SearcDataIterator it = pSearchData->GetBeginIterator();
    bool first = true;
    while (it.operator void* () != NULL)
    {
      SearchInterval* interval = it.operator*();
      if (interval != NULL)
      {
        if (!first) json << ",\n";
        json << "    " << IntervalToJson(interval);
        first = false;
      }
      ++it;
    }
    json << "\n  ],\n";

    // Лучшая точка
    json << "  \"best_trial\": ";
    Trial* best = pSearchData->GetBestTrial();
    if (best)
    {
      json << TrialToJson(best);
    }
    else
    {
      json << "null";
    }
    json << "\n";

    json << "}\n";

    return json.str();
  }

  /// Сохранение состояния (первый раз - полное, потом - добавление)
  bool SaveProgress(const std::string& filename,
    const std::vector<Trial*>& newTrials = std::vector<Trial*>(),
    const std::vector<SearchInterval*>& newIntervals = std::vector<SearchInterval*>(),
    Trial* newBestTrial = nullptr)
  {
    if (!pSearchData)
    {
      return false;
    }

    if (isFirstSave)
    {
      // Первый раз - сохраняем всё
      return SaveFullState(filename);
    }
    else
    {
      // Последующие разы - добавляем новые данные
      return AppendNewPoints(newTrials, newIntervals, newBestTrial);
    }
  }

  // ==================== ПОЛНАЯ РЕАЛИЗАЦИЯ LoadFromFile ====================

  /// Загрузка состояния из файла
  bool LoadFromFile(const std::string& filename)
  {
    if (!pSearchData)
    {
      return false;
    }

    // Открываем файл
    std::ifstream file(filename.c_str());
    if (!file.is_open())
    {
      return false;
    }

    // Читаем всё содержимое
    std::string content((std::istreambuf_iterator<char>(file)),
      std::istreambuf_iterator<char>());
    file.close();

    // Очищаем текущие данные
    pSearchData->Clear();

    // Парсим JSON
    JSONParser parser(content);

    // Парсим основной объект
    std::map<std::string, std::string> root = parser.ParseObject();

    // Проверяем версию
    std::map<std::string, std::string>::const_iterator verIt = root.find("version");
    if (verIt != root.end())
    {
      std::string version = verIt->second;
      // Можно проверить версию, но пока просто игнорируем
    }

    // Парсим search_data
    std::map<std::string, std::string> searchDataObj;
    std::map<std::string, std::string>::const_iterator sdIt = root.find("search_data");
    if (sdIt != root.end())
    {
      // Создаем парсер для вложенного объекта
      JSONParser sdParser(sdIt->second);
      searchDataObj = sdParser.ParseObject();

      // Восстанавливаем NumOfFuncs
      std::map<std::string, std::string>::const_iterator it = searchDataObj.find("NumOfFuncs");
      if (it != searchDataObj.end())
      {
        pSearchData->NumOfFuncs = StringToInt(it->second);
      }

      // Восстанавливаем M
      it = searchDataObj.find("M");
      if (it != searchDataObj.end())
      {
        JSONParser mParser(it->second);
        std::vector<double> mValues = mParser.ParseDoubleArray();
        for (size_t i = 0; i < mValues.size() && i < (size_t)pSearchData->NumOfFuncs; ++i)
        {
          pSearchData->M[i] = mValues[i];
        }
      }

      // Восстанавливаем Z
      it = searchDataObj.find("Z");
      if (it != searchDataObj.end())
      {
        JSONParser zParser(it->second);
        std::vector<double> zValues = zParser.ParseDoubleArray();
        for (size_t i = 0; i < zValues.size() && i < (size_t)pSearchData->NumOfFuncs; ++i)
        {
          pSearchData->Z[i] = zValues[i];
        }
      }

      // Восстанавливаем local_r
      it = searchDataObj.find("local_r");
      if (it != searchDataObj.end())
      {
        pSearchData->local_r = StringToDouble(it->second);
      }
    }

    // Парсим все точки trials
    std::map<double, Trial*> trialMap;
    std::map<std::string, std::string>::const_iterator trialsIt = root.find("trials");
    if (trialsIt != root.end())
    {
      JSONParser trialsParser(trialsIt->second);
      std::vector<std::map<std::string, std::string> > trialObjs = trialsParser.ParseArray();

      for (size_t i = 0; i < trialObjs.size(); ++i)
      {
        Trial* trial = CreateTrialFromJSON(trialObjs[i]);
        pSearchData->GetTrials().push_back(trial);
        trialMap[trial->X().toDouble()] = trial;
      }
    }

    // Парсим все интервалы intervals
    std::map<std::string, std::string>::const_iterator intervalsIt = root.find("intervals");
    if (intervalsIt != root.end())
    {
      JSONParser intervalsParser(intervalsIt->second);
      std::vector<std::map<std::string, std::string> > intervalObjs = intervalsParser.ParseArray();

      // Вектор для хранения интервалов перед вставкой
      std::vector<SearchInterval*> intervals;

      for (size_t i = 0; i < intervalObjs.size(); ++i)
      {
        double leftX = 0.0, rightX = 0.0;

        std::map<std::string, std::string>::const_iterator it = intervalObjs[i].find("left_x");
        if (it != intervalObjs[i].end())
        {
          leftX = StringToDouble(it->second);
        }

        it = intervalObjs[i].find("right_x");
        if (it != intervalObjs[i].end())
        {
          rightX = StringToDouble(it->second);
        }

        // Находим соответствующие точки
        Trial* leftPoint = NULL;
        Trial* rightPoint = NULL;

        std::map<double, Trial*>::const_iterator trialIt = trialMap.find(leftX);
        if (trialIt != trialMap.end())
        {
          leftPoint = trialIt->second;
        }

        trialIt = trialMap.find(rightX);
        if (trialIt != trialMap.end())
        {
          rightPoint = trialIt->second;
        }

        if (leftPoint != NULL && rightPoint != NULL)
        {
          SearchInterval* interval = CreateIntervalFromJSON(intervalObjs[i], leftPoint, rightPoint);
          intervals.push_back(interval);
        }
      }

      // Вставляем интервалы в правильном порядке
      // Сначала сортируем по left_x
      std::sort(intervals.begin(), intervals.end(),
        [](SearchInterval* a, SearchInterval* b) {
          return a->xl().toDouble() < b->xl().toDouble();
        });

      // Вставляем первый интервал особым образом
      if (intervals.size() >= 2)
      {
        // Для первых двух интервалов используем insert_first_data_item
        // В текущей реализации SearchData нет такого метода, поэтому используем InsertInterval
        pSearchData->InsertInterval(*intervals[0]);
        pSearchData->InsertInterval(*intervals[1]);

        // Связываем точки с интервалами
        intervals[0]->LeftPoint->rightInterval = intervals[0];
        intervals[0]->RightPoint->leftInterval = intervals[0];
        intervals[1]->LeftPoint->rightInterval = intervals[1];
        intervals[1]->RightPoint->leftInterval = intervals[1];

        // Остальные интервалы
        for (size_t i = 2; i < intervals.size(); ++i)
        {
          pSearchData->InsertInterval(*intervals[i]);
          intervals[i]->LeftPoint->rightInterval = intervals[i];
          intervals[i]->RightPoint->leftInterval = intervals[i];
        }
      }
    }

    // Восстанавливаем лучшую точку best_trial
    std::map<std::string, std::string>::const_iterator bestIt = root.find("best_trial");
    if (bestIt != root.end() && bestIt->second != "null")
    {
      JSONParser bestParser(bestIt->second);
      std::map<std::string, std::string> bestObj = bestParser.ParseObject();

      double bestX = 0.0;
      std::map<std::string, std::string>::const_iterator xIt = bestObj.find("x");
      if (xIt != bestObj.end())
      {
        bestX = StringToDouble(xIt->second);
      }

      // Ищем эту точку среди загруженных
      std::map<double, Trial*>::const_iterator trialIt = trialMap.find(bestX);
      if (trialIt != trialMap.end())
      {
        pSearchData->SetBestTrial(trialIt->second);
      }
    }

    // Перестраиваем очередь характеристик
    pSearchData->RefillQueue();

    currentFileName = filename;
    isFirstSave = false;

    return true;
  }

  void ResetFirstSave()
  {
    isFirstSave = true;
    currentFileName.clear();
  }
};

#endif // __SEARCH_DATA_SERIALIZER_H__
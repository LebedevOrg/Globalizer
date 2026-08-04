#include "SearchDataSerializer.h"



// ------------------------------------------------------------------------------------------------
std::string SearchDataSerializer::EscapeJsonString(const std::string& str)
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

// ------------------------------------------------------------------------------------------------
std::string SearchDataSerializer::FormatDouble(double value)
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

// ------------------------------------------------------------------------------------------------
std::string SearchDataSerializer::GetCurrentTimeISO()
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

// ------------------------------------------------------------------------------------------------
std::string SearchDataSerializer::IntToString(int value)
{
  std::ostringstream oss;
  oss << value;
  return oss.str();
}

// ------------------------------------------------------------------------------------------------
double SearchDataSerializer::StringToDouble(const std::string& str)
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

// ------------------------------------------------------------------------------------------------
int SearchDataSerializer::StringToInt(const std::string& str)
{
  std::istringstream iss(str);
  int value;
  iss >> value;
  return value;
}

// ------------------------------------------------------------------------------------------------
std::string SearchDataSerializer::Trim(const std::string& str)
{
  size_t first = str.find_first_not_of(" \t\n\r");
  if (first == std::string::npos)
    return "";
  size_t last = str.find_last_not_of(" \t\n\r");
  return str.substr(first, last - first + 1);
}

// ------------------------------------------------------------------------------------------------
std::vector<std::string> SearchDataSerializer::Split(const std::string& str, char delimiter)
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

// ------------------------------------------------------------------------------------------------
void SearchDataSerializer::JSONParser::SkipWhitespace()
{
  while (pos < content.length() &&
    (content[pos] == ' ' || content[pos] == '\t' ||
      content[pos] == '\n' || content[pos] == '\r'))
  {
    pos++;
  }
}

// ------------------------------------------------------------------------------------------------
std::string SearchDataSerializer::JSONParser::ParseString()
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

// ------------------------------------------------------------------------------------------------
double SearchDataSerializer::JSONParser::ParseNumber()
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

// ------------------------------------------------------------------------------------------------
std::string SearchDataSerializer::JSONParser::ParseWord()
{
  size_t start = pos;
  while (pos < content.length() && isalpha(content[pos]))
  {
    pos++;
  }
  return content.substr(start, pos - start);
}

// ------------------------------------------------------------------------------------------------
std::string SearchDataSerializer::JSONParser::ParseArrayAsString()
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

// ------------------------------------------------------------------------------------------------
std::string SearchDataSerializer::JSONParser::ParseObjectAsString()
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


// ------------------------------------------------------------------------------------------------
SearchDataSerializer::JSONParser::JSONParser(const std::string& str) : content(str), pos(0) {}


// ------------------------------------------------------------------------------------------------
void SearchDataSerializer::JSONParser::Reset()
{
  pos = 0;
}

// ------------------------------------------------------------------------------------------------
std::map<std::string, std::string> SearchDataSerializer::JSONParser::ParseObject()
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

// ------------------------------------------------------------------------------------------------
std::vector<std::map<std::string, std::string> > SearchDataSerializer::JSONParser::ParseArray()
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

// ------------------------------------------------------------------------------------------------
std::vector<double> SearchDataSerializer::JSONParser::ParseDoubleArray()
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

// ------------------------------------------------------------------------------------------------
std::vector<int> SearchDataSerializer::JSONParser::ParseIntArray()
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


// ------------------------------------------------------------------------------------------------
std::string SearchDataSerializer::TrialToJson(Trial* trial)
{
  std::ostringstream json;
  json << "{";

  // Координата на одномерном отрезке
  json << "\"x\":" << FormatDouble(trial->X().toDouble()) << ",";

  // Индекс дискретного параметра
  json << "\"discreteValuesIndex\":" << trial->discreteValuesIndex << ",";

  // Многомерная точка
  json << "\"y\":[";
  int dim = std::max(parameters.StartPoint.GetSize(), parameters.Dimension.GetData());
  double* point = new double[dim];
  pTask->TransformPoint(point, trial->y);
  for (int i = 0; i < dim; ++i)
  {
    if (i > 0) json << ",";
    json << FormatDouble(point[i]);
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

  delete[] point;

  return json.str();
}

// ------------------------------------------------------------------------------------------------
std::string SearchDataSerializer::IntervalToJson(SearchInterval* interval)
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

// ------------------------------------------------------------------------------------------------
Trial* SearchDataSerializer::CreateTrialFromJSON(const std::map<std::string, std::string>& data)
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
        //trial->FuncValues[i] = MaxDouble;
        delete trial;
        return nullptr;
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

// ------------------------------------------------------------------------------------------------
SearchInterval* SearchDataSerializer::CreateIntervalFromJSON(const std::map<std::string, std::string>& data,
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

// ------------------------------------------------------------------------------------------------
bool SearchDataSerializer::SaveFullState(const std::string& filename)
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

// ------------------------------------------------------------------------------------------------
bool SearchDataSerializer::ReadFileContent(const std::string& filename, std::string& content)
{
  std::ifstream infile(filename.c_str());
  if (!infile.is_open())
  {
    return false;
  }

  content.clear();
  std::string line;
  while (std::getline(infile, line))
  {
    content += line + "\n";
  }
  infile.close();

  return true;
}

// ------------------------------------------------------------------------------------------------
bool SearchDataSerializer::WriteFileContent(const std::string& filename, const std::string& content)
{
  std::ofstream outfile(filename.c_str());
  if (!outfile.is_open())
  {
    return false;
  }

  outfile << content;
  outfile.close();

  return true;
}

// ------------------------------------------------------------------------------------------------
bool SearchDataSerializer::FindTrialsPosition(const std::string& content,
  size_t& trialsPos,
  size_t& trialsEnd)
{
  trialsPos = content.find("\"trials\": [");
  if (trialsPos == std::string::npos)
  {
    return false;
  }

  trialsEnd = content.rfind("]");
  if (trialsEnd == std::string::npos || trialsEnd < trialsPos)
  {
    return false;
  }

  return true;
}

// ------------------------------------------------------------------------------------------------
bool SearchDataSerializer::HasExistingPoints(const std::string& content,
  size_t trialsPos,
  size_t trialsEnd)
{
  for (size_t i = trialsPos + 10; i < trialsEnd; ++i)
  {
    if (!isspace(content[i]) && content[i] != '[' && content[i] != ']')
    {
      return true;
    }
  }
  return false;
}


// ------------------------------------------------------------------------------------------------
std::string SearchDataSerializer::BuildNewTrialsString(const std::vector<Trial*>& newTrials,
  bool hasExistingPoints)
{
  std::string result;

  for (size_t i = 0; i < newTrials.size(); ++i)
  {
    std::string trialJson = TrialToJson(newTrials[i]);

    // Добавляем запятую и перевод строки перед каждой новой точкой
    if (hasExistingPoints || i > 0)
    {
      result += ",\n    ";
    }
    else
    {
      result += "\n    ";
    }

    result += trialJson;
  }

  return result;
}


// ------------------------------------------------------------------------------------------------
bool SearchDataSerializer::FindTrialsEnd(const std::string& content,
  size_t trialsPos,
  size_t& trialsEnd)
{
  // Ищем от конца файла последнюю закрывающую скобку
  // которая находится после trialsPos
  size_t lastBracket = content.rfind(']');
  if (lastBracket == std::string::npos || lastBracket < trialsPos)
  {
    return false;
  }

  trialsEnd = lastBracket;
  return true;
}



// ------------------------------------------------------------------------------------------------
bool SearchDataSerializer::AppendNewTrials(std::string& content,
  const std::vector<Trial*>& newTrials)
{
  if (newTrials.empty())
  {
    return true;
  }

  // Находим позицию массива trials
  std::string searchStr = "\"trials\": [";
  size_t trialsPos = content.find(searchStr);
  if (trialsPos == std::string::npos)
  {
    return false;
  }

  // Ищем закрывающую скобку ИМЕННО ЭТОГО МАССИВА
  // Начинаем с позиции после открывающей скобки
  size_t currentPos = trialsPos + searchStr.length();
  int bracketCount = 1;
  size_t trialsEnd = std::string::npos;

  while (currentPos < content.length() && bracketCount > 0)
  {
    char c = content[currentPos];
    if (c == '[')
    {
      bracketCount++;
    }
    else if (c == ']')
    {
      bracketCount--;
      if (bracketCount == 0)
      {
        trialsEnd = currentPos;
        break;
      }
    }
    currentPos++;
  }

  if (trialsEnd == std::string::npos)
  {
    return false;
  }

  // Проверяем, есть ли уже точки в массиве
  bool hasExistingPoints = false;
  for (size_t i = trialsPos + searchStr.length(); i < trialsEnd; ++i)
  {
    char c = content[i];
    if (c != ' ' && c != '\t' && c != '\n' && c != '\r' && c != '[' && c != ']')
    {
      hasExistingPoints = true;
      break;
    }
  }

  // Формируем строку с новыми точками
  std::string newTrialsStr;
  for (size_t i = 0; i < newTrials.size(); ++i)
  {
    std::string trialJson = TrialToJson(newTrials[i]);

    // Добавляем запятую и перевод строки перед каждой новой точкой
    if (hasExistingPoints || i > 0)
    {
      newTrialsStr += ",\n    ";
    }
    else
    {
      newTrialsStr += "\n    ";
    }

    newTrialsStr += trialJson;
  }

  if (content[trialsEnd - 3] == '\n')
    trialsEnd = trialsEnd - 3;
  else
  {
    // Добавляем перевод строки после последней точки если нужно
    if (!newTrialsStr.empty() && newTrialsStr[newTrialsStr.length() - 1] != '\n')
    {
      newTrialsStr += "\n  ";
    }
  }
  // Вставляем новые точки ПЕРЕД закрывающей скобкой
  content.insert(trialsEnd, newTrialsStr);

  return true;
}




// ------------------------------------------------------------------------------------------------
bool SearchDataSerializer::FindCountPosition(const std::string& content,
  size_t& countPos,
  size_t& countEnd)
{
  countPos = content.find("\"Count\": ");
  if (countPos == std::string::npos)
  {
    return false;
  }

  countEnd = content.find_first_of(",\n", countPos);
  return (countEnd != std::string::npos);
}


// ------------------------------------------------------------------------------------------------
bool SearchDataSerializer::UpdateCount(std::string& content, int newCount)
{
  size_t countPos, countEnd;
  if (!FindCountPosition(content, countPos, countEnd))
  {
    return false;
  }

  std::string newCountStr = "\"Count\": " + IntToString(newCount);
  content.replace(countPos, countEnd - countPos, newCountStr);

  return true;
}


// ------------------------------------------------------------------------------------------------
bool SearchDataSerializer::FindBestTrialPosition(const std::string& content,
  size_t& bestTrialPos,
  size_t& bestTrialEnd,
  bool& hasComma)
{
  bestTrialPos = content.find("\"best_trial\": ");
  if (bestTrialPos == std::string::npos)
  {
    return false;
  }

  // Ищем конец строки - следующий перевод строки
  size_t lineEnd = content.find("\n", bestTrialPos);
  if (lineEnd == std::string::npos)
  {
    lineEnd = content.length();
  }

  // Проверяем, есть ли запятая в конце строки
  hasComma = false;
  for (size_t i = bestTrialPos; i < lineEnd; ++i)
  {
    if (content[i] == ',')
    {
      hasComma = true;
      break;
    }
  }

  bestTrialEnd = lineEnd;

  return true;
}


// ------------------------------------------------------------------------------------------------
bool SearchDataSerializer::UpdateBestTrial(std::string& content, Trial* newBestTrial)
{
  if (newBestTrial == nullptr)
  {
    return true;
  }

  // ШАГ 1: Находим всю строку с best_trial
  size_t bestTrialPos = content.find("\"best_trial\":");
  if (bestTrialPos == std::string::npos)
  {
    return false;
  }

  // ШАГ 2: Находим конец этой строки (следующий перевод строки)
  size_t lineEndPos = content.find("\n", bestTrialPos);
  if (lineEndPos == std::string::npos)
  {
    lineEndPos = content.length();
  }

  // ШАГ 3: Сохраняем старую строку
  std::string oldLine = content.substr(bestTrialPos, lineEndPos - bestTrialPos);

  // ШАГ 4: Формируем новую строку
  std::string newLine = "\"best_trial\": " + TrialToJson(newBestTrial);

  // ШАГ 5: Проверяем, была ли запятая в старой строке
  //if (oldLine.find(',') != std::string::npos)
  //{
  //  newLine += ",";
  //}

  // ШАГ 6: Добавляем перевод строки
  //newLine += "\n";

  // ШАГ 7: Заменяем
  content.replace(bestTrialPos, lineEndPos - bestTrialPos, newLine);

  return true;
}


// ------------------------------------------------------------------------------------------------
bool SearchDataSerializer::FindTimestampPosition(const std::string& content,
  size_t& timestampStart,
  size_t& timestampEnd)
{
  size_t timestampPos = content.find("\"timestamp\": \"");
  if (timestampPos == std::string::npos)
  {
    return false;
  }

  timestampStart = timestampPos + 13;
  timestampEnd = content.find("\"", timestampStart);

  return (timestampEnd != std::string::npos);
}


// ------------------------------------------------------------------------------------------------
bool SearchDataSerializer::UpdateTimestamp(std::string& content)
{
  // Ищем строку с timestamp
  size_t timestampPos = content.find("\"timestamp\": \"");
  if (timestampPos == std::string::npos)
  {
    return false;
  }

  // Находим конец строки (до следующей запятой или перевода строки)
  size_t lineEnd = content.find_first_of(",\n", timestampPos);
  if (lineEnd == std::string::npos)
  {
    lineEnd = content.length();
  }

  // Формируем новую строку целиком
  std::string newTimestampLine = "\"timestamp\": \"" + GetCurrentTimeISO() + "\"";

  // Проверяем, была ли запятая в конце
  //if (lineEnd < content.length() && content[lineEnd] == ',')
  //{
  //  newTimestampLine += ",";
  //}

  // Заменяем всю строку
  content.replace(timestampPos, lineEnd - timestampPos, newTimestampLine);

  return true;
}


// ------------------------------------------------------------------------------------------------
bool SearchDataSerializer::FindMPosition(const std::string& content,
  size_t& mPos,
  size_t& mEnd)
{
  mPos = content.find("\"M\": [");
  if (mPos == std::string::npos)
  {
    return false;
  }

  mEnd = content.find("]", mPos);
  return (mEnd != std::string::npos);
}


// ------------------------------------------------------------------------------------------------
bool SearchDataSerializer::UpdateMArray(std::string& content,
  const double* M,
  int numOfFuncs)
{
  size_t mPos, mEnd;

  if (!FindMPosition(content, mPos, mEnd))
  {
    return false;
  }

  std::string newM = "\"M\": [";
  for (int i = 0; i < numOfFuncs; ++i)
  {
    if (i > 0) newM += ",";
    newM += FormatDouble(M[i]);
  }
  newM += "]";

  content.replace(mPos, mEnd - mPos + 1, newM);

  return true;
}


// ------------------------------------------------------------------------------------------------
bool SearchDataSerializer::FindZPosition(const std::string& content,
  size_t& zPos,
  size_t& zEnd)
{
  zPos = content.find("\"Z\": [");
  if (zPos == std::string::npos)
  {
    return false;
  }

  zEnd = content.find("]", zPos);
  return (zEnd != std::string::npos);
}


// ------------------------------------------------------------------------------------------------
bool SearchDataSerializer::UpdateZArray(std::string& content,
  const double* Z,
  int numOfFuncs)
{
  size_t zPos, zEnd;

  if (!FindZPosition(content, zPos, zEnd))
  {
    return false;
  }

  std::string newZ = "\"Z\": [";
  for (int i = 0; i < numOfFuncs; ++i)
  {
    if (i > 0) newZ += ",";
    newZ += FormatDouble(Z[i]);
  }
  newZ += "]";

  content.replace(zPos, zEnd - zPos + 1, newZ);

  return true;
}


// ------------------------------------------------------------------------------------------------
bool SearchDataSerializer::FindLocalRPosition(const std::string& content,
  size_t& localRPos,
  size_t& localREnd)
{
  localRPos = content.find("\"local_r\": ");
  if (localRPos == std::string::npos)
  {
    return false;
  }

  localREnd = content.find_first_of(",\n", localRPos);
  return (localREnd != std::string::npos);
}


// ------------------------------------------------------------------------------------------------
bool SearchDataSerializer::UpdateLocalR(std::string& content, double local_r)
{
  size_t localRPos, localREnd;

  if (!FindLocalRPosition(content, localRPos, localREnd))
  {
    return false;
  }

  std::string newLocalR = "\"local_r\": " + FormatDouble(local_r);
  content.replace(localRPos, localREnd - localRPos, newLocalR);

  return true;
}

// ------------------------------------------------------------------------------------------------
bool SearchDataSerializer::FindStartParameterNumberPosition(const std::string& content,
  size_t& paramPos,
  size_t& paramEnd)
{
  paramPos = content.find("\"start_parameter_number\": ");
  if (paramPos == std::string::npos)
  {
    return false;
  }

  paramEnd = content.find_first_of(",\n", paramPos);
  return (paramEnd != std::string::npos);
}

// ------------------------------------------------------------------------------------------------
bool SearchDataSerializer::UpdateStartParameterNumber(std::string& content, int value)
{
  size_t paramPos, paramEnd;

  if (!FindStartParameterNumberPosition(content, paramPos, paramEnd))
  {
    return false;
  }

  std::string newParam = "\"start_parameter_number\": " + IntToString(value);
  content.replace(paramPos, paramEnd - paramPos, newParam);

  return true;
}


// ------------------------------------------------------------------------------------------------
bool SearchDataSerializer::AppendNewPoints(const std::vector<Trial*>& newTrials,
  const std::vector<SearchInterval*>& /*newIntervals*/,
  Trial* newBestTrial)
{
  if (currentFileName.empty() || !pSearchData)
  {
    return false;
  }

  // Проверяем изменения в массивах
  UpdateMChanges();
  UpdateZChanges();
  UpdateLocalRChanges();
  UpdateStartParameterNumberChanges();

  // Читаем содержимое файла
  std::string content;
  if (!ReadFileContent(currentFileName, content))
  {
    return false;
  }

  // Добавляем новые точки
  if (!AppendNewTrials(content, newTrials))
  {
    return false;
  }

  // Обновляем Count
  if (!UpdateCount(content, pSearchData->GetCount()))
  {
    return false;
  }

  // Обновляем best_trial
  if (!UpdateBestTrial(content, newBestTrial))
  {
    return false;
  }

  // Обновляем временную метку
  if (!UpdateTimestamp(content))
  {
    return false;
  }

  if (startParameterNumberChanged)
  {
    UpdateStartParameterNumber(content, parameters.startParameterNumber);
  }

  // Обновляем M если изменился
  if (mArrayChanged)
  {
    UpdateMArray(content, pSearchData->M, pSearchData->NumOfFuncs);
  }

  // Обновляем Z если изменился
  if (zArrayChanged)
  {
    UpdateZArray(content, pSearchData->Z, pSearchData->NumOfFuncs);
  }

  // Обновляем local_r если изменился
  if (localRChanged)
  {
    UpdateLocalR(content, pSearchData->local_r);
  }

  // Записываем обновленное содержимое в файл
  return WriteFileContent(currentFileName, content);
}

// ------------------------------------------------------------------------------------------------
void SearchDataSerializer::UpdateStartParameterNumberChanges()
{
  if (!pSearchData)
  {
    startParameterNumberChanged = false;
    return;
  }

  int currentValue = parameters.startParameterNumber;

  // Сравниваем с предыдущим значением
  if (previousStartParameterNumber != currentValue)
  {
    startParameterNumberChanged = true;
    previousStartParameterNumber = currentValue;
  }
  else
  {
    startParameterNumberChanged = false;
  }
}

// ------------------------------------------------------------------------------------------------
SearchDataSerializer::SearchDataSerializer()
  : pSearchData(nullptr)
  , pTask(nullptr)
  , isFirstSave(true)
  , previousLocalR(0.0)
  , previousStartParameterNumber(0)
  , mArrayChanged(false)
  , zArrayChanged(false)
  , localRChanged(false)
  , startParameterNumberChanged(false)
{
}

/// Обновление флага изменений массива M
// ------------------------------------------------------------------------------------------------
void SearchDataSerializer::UpdateMChanges()
{
  if (!pSearchData)
  {
    mArrayChanged = false;
    return;
  }

  // Если предыдущий вектор пуст, инициализируем его
  if (previousM.empty())
  {
    previousM.resize(pSearchData->NumOfFuncs);
    for (int i = 0; i < pSearchData->NumOfFuncs; ++i)
    {
      previousM[i] = pSearchData->M[i];
    }
    mArrayChanged = true; // Первый раз - считаем что изменилось
    return;
  }

  // Проверяем, изменились ли значения
  mArrayChanged = false;
  for (int i = 0; i < pSearchData->NumOfFuncs; ++i)
  {
    // Сравниваем с погрешностью
    if (fabs(previousM[i] - pSearchData->M[i]) > 1e-12)
    {
      mArrayChanged = true;
      break;
    }
  }

  // Обновляем предыдущие значения
  if (mArrayChanged)
  {
    previousM.resize(pSearchData->NumOfFuncs);
    for (int i = 0; i < pSearchData->NumOfFuncs; ++i)
    {
      previousM[i] = pSearchData->M[i];
    }
  }
}


// ------------------------------------------------------------------------------------------------
void SearchDataSerializer::UpdateZChanges()
{
  if (!pSearchData)
  {
    zArrayChanged = false;
    return;
  }

  // Если предыдущий вектор пуст, инициализируем его
  if (previousZ.empty())
  {
    previousZ.resize(pSearchData->NumOfFuncs);
    for (int i = 0; i < pSearchData->NumOfFuncs; ++i)
    {
      previousZ[i] = pSearchData->Z[i];
    }
    zArrayChanged = true; // Первый раз - считаем что изменилось
    return;
  }

  // Проверяем, изменились ли значения
  zArrayChanged = false;
  for (int i = 0; i < pSearchData->NumOfFuncs; ++i)
  {
    // Сравниваем с погрешностью
    if (fabs(previousZ[i] - pSearchData->Z[i]) > 1e-12)
    {
      zArrayChanged = true;
      break;
    }
  }

  // Обновляем предыдущие значения
  if (zArrayChanged)
  {
    previousZ.resize(pSearchData->NumOfFuncs);
    for (int i = 0; i < pSearchData->NumOfFuncs; ++i)
    {
      previousZ[i] = pSearchData->Z[i];
    }
  }
}


// ------------------------------------------------------------------------------------------------
void SearchDataSerializer::UpdateLocalRChanges()
{
  if (!pSearchData)
  {
    localRChanged = false;
    return;
  }

  // Сравниваем с погрешностью
  if (fabs(previousLocalR - pSearchData->local_r) > 1e-12)
  {
    localRChanged = true;
    previousLocalR = pSearchData->local_r;
  }
  else
  {
    localRChanged = false;
  }
}


// ------------------------------------------------------------------------------------------------
void SearchDataSerializer::ResetChangeFlags()
{
  mArrayChanged = false;
  zArrayChanged = false;
  localRChanged = false;
  startParameterNumberChanged = false;
}



// ------------------------------------------------------------------------------------------------
void SearchDataSerializer::SetSearchData(SearchData* data)
{
  pSearchData = data;

  // Сбрасываем предыдущие значения при новой поисковой информации
  previousM.clear();
  previousZ.clear();
  previousLocalR = 0.0;
  previousStartParameterNumber = 0;
  ResetChangeFlags();
}

// ------------------------------------------------------------------------------------------------
void SearchDataSerializer::SetTask(Task* task)
{
  pTask = task;
}

// ------------------------------------------------------------------------------------------------
std::string SearchDataSerializer::SerializeFullState()
{
  if (!pSearchData)
  {
    return "{}";
  }

  std::ostringstream json;
  json << "{\n";

  // Версия и метаданные
  json << "  \"version\": \"" + parameters.version + "\",\n";
  json << "  \"timestamp\": \"" << GetCurrentTimeISO() << "\",\n";
  json << "  \"mode\": \"full\",\n";

  // Параметры метода
  json << "  \"method_parameters\": {\n";
  json << "    \"eps\": " << FormatDouble(parameters.Epsilon) << ",\n";
  json << "    \"r\": " << FormatDouble(parameters.r) << ",\n";
  json << "    \"iters_limit\": " << parameters.MaxNumOfPoints << ",\n";
  json << "    \"number_of_parallel_points\": " << parameters.NumPoints << ",\n";
  json << "    \"start_parameter_number\": " << parameters.startParameterNumber << ",\n";
  json << "    \"start_point\": [";
  for (int i = 0; i < parameters.Dimension; ++i)
  {
    if (i > 0) json << ",";
    json << FormatDouble(parameters.StartPoint[i]);
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
    if (i > 0)
      json << ",\n";
    json << "    " << TrialToJson(trials[i]);
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

// ------------------------------------------------------------------------------------------------
bool SearchDataSerializer::SaveProgress(const std::string& filename,
  const std::vector<Trial*>& newTrials,
  const std::vector<SearchInterval*>& /*newIntervals*/,
  Trial* newBestTrial)
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
    // Последующие разы - добавляем только новые точки
    // Интервалы не сохраняем, так как их можно восстановить из точек
    return AppendNewPoints(newTrials, std::vector<SearchInterval*>(), newBestTrial);
  }
}


// ------------------------------------------------------------------------------------------------
bool SearchDataSerializer::LoadVersion(const std::map<std::string, std::string>& root, std::string& version)
{
  std::map<std::string, std::string>::const_iterator it = root.find("version");
  if (it != root.end())
  {
    version = it->second;
    return true;
  }
  return false;
}

// ------------------------------------------------------------------------------------------------
bool SearchDataSerializer::LoadMode(const std::map<std::string, std::string>& root, std::string& mode)
{
  std::map<std::string, std::string>::const_iterator it = root.find("mode");
  if (it != root.end())
  {
    mode = it->second;
    return true;
  }
  return false;
}

// ------------------------------------------------------------------------------------------------
bool SearchDataSerializer::LoadMethodParameters(const std::map<std::string, std::string>& root, LoadedMethodParameters& params)
{
  std::map<std::string, std::string>::const_iterator it = root.find("method_parameters");
  if (it == root.end())
  {
    return false;
  }

  JSONParser parser(it->second);
  std::map<std::string, std::string> paramsObj = parser.ParseObject();

  // Парсим eps
  it = paramsObj.find("eps");
  if (it != paramsObj.end())
  {
    params.eps = StringToDouble(it->second);
  }

  // Парсим r
  it = paramsObj.find("r");
  if (it != paramsObj.end())
  {
    params.r = StringToDouble(it->second);
  }

  // Парсим iters_limit
  it = paramsObj.find("iters_limit");
  if (it != paramsObj.end())
  {
    params.iters_limit = StringToInt(it->second);
  }

  // Парсим number_of_parallel_points
  it = paramsObj.find("number_of_parallel_points");
  if (it != paramsObj.end())
  {
    params.number_of_parallel_points = StringToInt(it->second);
  }

  // Парсим start_parameter_number
  it = paramsObj.find("start_parameter_number");
  if (it != paramsObj.end())
  {
    params.start_parameter_number = StringToInt(it->second);
  }

  // Парсим start_point
  it = paramsObj.find("start_point");
  if (it != paramsObj.end())
  {
    JSONParser arrayParser(it->second);
    params.start_point = arrayParser.ParseDoubleArray();
  }

  return true;
}

// ------------------------------------------------------------------------------------------------
bool SearchDataSerializer::LoadSearchData(const std::map<std::string, std::string>& root, LoadedSearchData& searchData)
{
  std::map<std::string, std::string>::const_iterator it = root.find("search_data");
  if (it == root.end())
  {
    return false;
  }

  JSONParser parser(it->second);
  std::map<std::string, std::string> searchDataObj = parser.ParseObject();

  // Парсим NumOfFuncs
  it = searchDataObj.find("NumOfFuncs");
  if (it != searchDataObj.end())
  {
    searchData.NumOfFuncs = StringToInt(it->second);
  }

  // Парсим Count
  it = searchDataObj.find("Count");
  if (it != searchDataObj.end())
  {
    searchData.Count = StringToInt(it->second);
  }

  // Парсим M
  it = searchDataObj.find("M");
  if (it != searchDataObj.end())
  {
    JSONParser mParser(it->second);
    searchData.M = mParser.ParseDoubleArray();
  }

  // Парсим Z
  it = searchDataObj.find("Z");
  if (it != searchDataObj.end())
  {
    JSONParser zParser(it->second);
    searchData.Z = zParser.ParseDoubleArray();
  }

  // Парсим local_r
  it = searchDataObj.find("local_r");
  if (it != searchDataObj.end())
  {
    searchData.local_r = StringToDouble(it->second);
  }

  return true;
}

// ------------------------------------------------------------------------------------------------
bool SearchDataSerializer::LoadTrials(const std::map<std::string, std::string>& root,
  std::vector<Trial*>& trials,
  std::map<double, Trial*>& trialMap)
{
  std::map<std::string, std::string>::const_iterator it = root.find("trials");
  if (it == root.end())
  {
    return false;
  }

  JSONParser trialsParser(it->second);
  std::vector<std::map<std::string, std::string> > trialObjs = trialsParser.ParseArray();

  for (size_t i = 0; i < trialObjs.size(); ++i)
  {
    Trial* trial = CreateTrialFromJSON(trialObjs[i]);
    if (trial)
    {
      trials.push_back(trial);
      trialMap[trial->X().toDouble()] = trial;
    }
  }

  return true;
}

// ------------------------------------------------------------------------------------------------
bool SearchDataSerializer::LoadBestTrial(const std::map<std::string, std::string>& root,
  const std::map<double, Trial*>& trialMap,
  Trial*& bestTrial)
{
  std::map<std::string, std::string>::const_iterator it = root.find("best_trial");
  if (it == root.end() || it->second == "null")
  {
    bestTrial = nullptr;
    return false;
  }

  JSONParser bestParser(it->second);
  std::map<std::string, std::string> bestObj = bestParser.ParseObject();

  // Создаем временную точку из данных best_trial
  Trial* tempTrial = CreateTrialFromJSON(bestObj);
  if (!tempTrial) {
    return false;
  }

  // Используем все поля точки для поиска
  for (const auto& entry : trialMap) 
  {
    Trial* candidate = entry.second;

    // Сравниваем все поля точки
    if (candidate->X() == tempTrial->X() &&
      candidate->discreteValuesIndex == tempTrial->discreteValuesIndex &&
      candidate->index == tempTrial->index) 
    {

      // Дополнительное сравнение значений функций
      bool valuesMatch = true;
      for (int i = 0; i < pSearchData->NumOfFuncs; i++) {
        if (fabs(candidate->FuncValues[i] - tempTrial->FuncValues[i]) > 1e-12) {
          valuesMatch = false;
          break;
        }
      }

      if (valuesMatch) {
        bestTrial = candidate;
        delete tempTrial;
        return true;
      }
    }
  }

  // Если не нашли, возвращаем временную точку как наилучшую
  bestTrial = tempTrial;
  return true;
}



// ------------------------------------------------------------------------------------------------
bool SearchDataSerializer::LoadFromFile(const std::string& filename, LoadedFileData& outData)
{
  std::ifstream file(filename.c_str());
  if (!file.is_open())
  {
    return false;
  }

  std::string content((std::istreambuf_iterator<char>(file)),
    std::istreambuf_iterator<char>());
  file.close();

  JSONParser parser(content);
  std::map<std::string, std::string> root = parser.ParseObject();


  // Загружаем все компоненты
  LoadVersion(root, outData.version);
  LoadMode(root, outData.mode);
  LoadMethodParameters(root, outData.methodParams);

  if (!LoadSearchData(root, outData.searchData))
  {
    return false;
  }

  // Создаем map для точек, который будем использовать при загрузке
  std::map<double, Trial*> trialMap;

  if (!LoadTrials(root, outData.trials, trialMap))
  {
    // Очистка произойдет в деструкторе LoadedFileData
    return false;
  }


  LoadBestTrial(root, trialMap, outData.bestTrial);

  ResetFirstSave();

  return true;
}





// ------------------------------------------------------------------------------------------------
void SearchDataSerializer::ResetFirstSave()
{
  isFirstSave = true;
  currentFileName.clear();
}

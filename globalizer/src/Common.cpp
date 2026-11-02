#include "Common.h"
#include <cmath>
#include <algorithm>
#include <vector>


// ------------------------------------------------------------------------------------------------
std::string GetFileExtension(const std::string& filename, bool include_dot)
{
  // Находим последнюю точку
  size_t dot_pos = filename.find_last_of('.');

  // Если точки нет - нет расширения
  if (dot_pos == std::string::npos)
  {
    return "";
  }

  // Находим последний разделитель пути (для Unix и Windows)
  size_t sep_pos = filename.find_last_of("/\\");

  // Если точка находится перед разделителем или это первая позиция
  if (sep_pos != std::string::npos && sep_pos > dot_pos)
  {
    return ""; // Точка в пути
  }

  if (dot_pos == filename.length() - 1)
  {
    return ""; // Файл заканчивается точкой
  }

  // Возвращаем расширение с точкой или без
  return include_dot ? filename.substr(dot_pos) : filename.substr(dot_pos + 1);
}

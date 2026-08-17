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


// ------------------------------------------------------------------------------------------------
void FindXY(int& x, int& y, int z, int C)
{
  double approx = std::sqrt(static_cast<double>(z) / (2.0 * C));
  int y0 = static_cast<int>(approx);

  int bestX = 0, bestY = 0;
  long long bestDiff = std::numeric_limits<long long>::max();

  int startY = std::max(1, y0 - 10);
  int endY = y0 + 10;

  for (int curY = startY; curY <= endY; ++curY)
  {
    int minX = curY + 1;
    int targetX = static_cast<int>(static_cast<double>(z) / (curY * C));

    for (int curX = minX; curX <= targetX + 5; ++curX)
    {
      if (curX <= 5 * curY) continue;

      long long product = static_cast<long long>(curX) * curY * C;
      long long diff = std::abs(product - z);

      if (diff < bestDiff)
      {
        bestDiff = diff;
        bestX = curX;
        bestY = curY;
      }
      else if (diff == bestDiff)
      {

        if (curX < bestX)
        {
          bestX = curX;
          bestY = curY;
        }
      }
    }
  }

  if (bestX == 0)
  {
    bestY = 1;
    bestX = 3;
  }

  if (bestX < 5)
  {
    bestX = bestX * bestY;
    bestY = 1;
  }

  x = bestX;
  y = bestY;
}

// ------------------------------------------------------------------------------------------------
bool IsBelowGraph(int x, int y)
{
  std::vector<int> x_points = { 2, 3, 4, 5 };
  std::vector<double> y_points = { 4732.27, 5347.42, 25636.46, 161094.88 };

  double result = 0.0;

  for (size_t i = 0; i < x_points.size(); i++)
  {
    double term = y_points[i];

    for (size_t j = 0; j < x_points.size(); j++)
    {
      if (j != i)
      {
        term = term * (static_cast<double>(x) - x_points[j]) /
          (x_points[i] - x_points[j]);
      }
    }

    result += term;
  }
  // Сравниваем y с полученным значением функции
  return static_cast<double>(y) < result;
}
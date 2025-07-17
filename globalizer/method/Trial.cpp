#include "Trial.h"

#include "SearchInterval.h"

TTrial* TTrial::GetLeftPoint()
{
  if (this->leftInterval != 0)
  {
    if (this->leftInterval->LeftPoint != 0)
    {
      return this->leftInterval->LeftPoint;
    }
  }
  return 0;
}

TTrial* TTrial::GetRightPoint()
{
  if (this->rightInterval != 0)
  {
    if (this->rightInterval->RightPoint != 0)
    {
      return this->rightInterval->RightPoint;
    }
  }
  return 0;
}
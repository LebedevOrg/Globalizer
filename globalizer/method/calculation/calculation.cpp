#include "calculation.h"

int TCalculation::countCalculation = 0;

TCalculation* TCalculation::leafCalculation = 0;
TCalculation* TCalculation::firstCalculation = 0;

bool TCalculation::isStartComputingAway = true;
TResultForCalculation TCalculation::resultCalculation;
TInformationForCalculation TCalculation::inputCalculation;


TCalculation::TCalculation(TTask& _pTask) : pTask(&_pTask)
{  }

void TCalculation::SetCountCalculation(int c)
{
  countCalculation = c;
  isStartComputingAway = false;
}


void TCalculation::SetTask(TTask* _pTask)
{
  pTask = _pTask;
}

void TCalculation::SetSearchData(TSearchData* _pData)
{
  pData = _pData;
}

void TCalculation::Reset()
{
}

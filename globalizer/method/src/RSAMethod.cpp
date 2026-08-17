/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2026 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      method RSA.h                                                //
//                                                                         //
//  Purpose:   Header file for method class                                //
//                                                                         //
//  Author(s): Zaitsev A.                                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "RSAMethod.hpp"
#include "LocalMethod.h"
#include "Exception.h"
#include "Common.h"
#include "OutputSystem.h"
#include "SearcDataIterator.h"
#include "TaskFactory.h"
#include "TrialFactory.h"
#include "CalculationFactory.h"
#include "ParallelHookeJeevesMethod.h"

#include "OMPCalculation.h"


#include <cmath>
#include <vector>
#include <fstream>
#include <algorithm>


#include <string>
#include <cmath>



// ------------------------------------------------------------------------------------------------
Method_RSA::Method_RSA(Task& _pTask, SearchData& _pData,
    Calculation& _Calculation, IEvolvent& _Evolvent) :
    pTask(_pTask), pData(&_pData),
    calculation(_Calculation), evolvent(_Evolvent)
{
    isFoundOptimalPoint = false;

    MaxNumOfTrials = parameters.MaxNumOfPoints;
    if (MaxNumOfTrials < 1)
    {
        throw EXCEPTION("MaxNumOfTrials is out of range");
    }

    if (parameters.Epsilon <= 0.0)
    {
        throw EXCEPTION("Epsilon is out of range");
    }

    if (parameters.r <= 1.0)
    {
        throw EXCEPTION("r is out of range");
    }

    if ((parameters.rEps < 0.0) || (parameters.rEps > 0.5))
    {
        throw EXCEPTION("Epsilon reserv parameter is out of range");
    }

    alfa = parameters.LocalAlpha; // пока локальная адаптация - фиксированная



    if (parameters.NumPoints <= 0)
    {
        throw EXCEPTION("NumPoints parameter <= 0");
    }


    iteration.IterationCount = 0;

    AchievedAccuracy = MaxDouble;
    // Массив для текущих итераций
    iteration.pCurTrials.resize(parameters.NumPoints);

    //===========================================================================================================================================
    Xmax = new double[pTask.GetNumOfFunc()];
    for (int i = 0; i < pTask.GetNumOfFunc(); i++)
        Xmax[i] = 0;
    //===========================================================================================================================================


    if (pTask.GetNumberOfContinuousVariable() == 1)
        StartLocalIteration = 5;
    else
        StartLocalIteration = pTask.GetNumberOfContinuousVariable() * 70 / parameters.NumPoints;

    functionCalculationCount.resize(pTask.GetNumOfFunc());
    for (int i = 0; i < pTask.GetNumOfFunc(); i++)
        functionCalculationCount[i] = 0;

    isFindInterval = false;

    inputSet.trials.resize(parameters.NumPoints);

    isSetInLocalMinimumInterval = false;

    isStop = false;

    isSearchXMax = true;

    calculation.SetSearchData(&_pData);

    isLocalZUpdate = false;

    /// количество точек вычисленных локальным методом
    localPointCount = 0;
    /// число запусков локально метода
    numberLocalMethodtStart = 0;
}

// ------------------------------------------------------------------------------------------------
Method_RSA::~Method_RSA()
{
}



// ------------------------------------------------------------------------------------------------
SearchData* Method_RSA::GetSearchData(Trial* trial)
{
    return pData;
}

// ------------------------------------------------------------------------------------------------
bool Method_RSA::IsIntervalInSegment(SearchInterval* basicInterval, SearchInterval* newInterval)
{
    if (basicInterval == NULL || newInterval == NULL)
    {
        return false;
    }
    if (basicInterval->LeftPoint == NULL || newInterval->LeftPoint == NULL)
    {
        return false;
    }
    double start = basicInterval->LeftPoint->GetFloor();

    if (basicInterval->LeftPoint->GetFloor() != newInterval->LeftPoint->GetFloor())
        return false;
    double end = start + 1;
    if ((newInterval->xl() >= start) && (newInterval->xl() <= end) &&
        (newInterval->xr() >= start) && (newInterval->xr() <= end))
        return true;
    return false;
}


double Method_RSA::Update_r(int iter, int procLevel)
{
    double baseR = parameters.r;
    double iterationCount = 0;
    if (iter <= 1)
        iterationCount = (double)(iteration.IterationCount);
    else
        iterationCount = iter;

    if (iterationCount <= 0)
        iterationCount = 1;

    double p = 1.0 / pTask.GetNumberOfContinuousVariable();
    double resR = baseR + parameters.rDynamic / pow(iterationCount, p);

    return resR;
}

// ------------------------------------------------------------------------------------------------
void Method_RSA::CalculateImage(Trial& pCurTrialsj)
{
    evolvent.GetImage(pCurTrialsj.X(), pCurTrialsj.y);
}

// ------------------------------------------------------------------------------------------------
/// Вычисляет координаты на отрезке 0..1 для всех разверток по образу проведенного испытания
void Method_RSA::CalculateCurrentPoint(Trial& pCurTrialsj, SearchInterval* BestIntervalsj)
{
    // Вычисляем x
    //Extended delta_x = BestIntervalsj->xr() - BestIntervalsj->xl();
    Extended left_x = BestIntervalsj->xl(), right_x = BestIntervalsj->xr();
    double left_z = BestIntervalsj->LeftPoint->GetValue(), right_z = BestIntervalsj->RightPoint->GetValue();
    Extended delta_x = right_x - left_x;
    double sign = right_z - left_z < 0.0 ? -1.0 : 1.0;
    Extended point_x = 0.5 * (left_x + right_x);// -sign * 0.25 * pow(abs(right_z - left_z) / (right_z + left_z), pTask.GetN()) * delta_x;
    pCurTrialsj.SetX(point_x);
    //pCurTrialsj.SetX(0.5 * (BestIntervalsj->xl() + BestIntervalsj->xr()));

    pCurTrialsj.leftInterval = BestIntervalsj;
    pCurTrialsj.rightInterval = BestIntervalsj;

    // Вычисляем y
    // Вычисляем образ точки итерации - образ записывается в начальные позиции массива y
    CalculateImage(pCurTrialsj);
}


// ------------------------------------------------------------------------------------------------
void Method_RSA::LoadPoint()
{
    std::string pointsPath = parameters.FirstPointFilePath;
    std::string pointsPathExtension = GetFileExtension(pointsPath);
    int numberLoadedPoints = 0;
    std::vector<Trial*> newPoint;
    SearchDataSerializer::LoadedFileData fd;
    if (pointsPathExtension == "json")
    {
        parameters.serializer->LoadFromFile(pointsPath, fd);
        newPoint = fd.trials;
        numberLoadedPoints = newPoint.size();
        for (auto trial : newPoint)
            pData->GetTrials().push_back(trial);
    }
    else
    {

        std::vector<std::vector<double>> points;
        std::vector<double> pointVal;
        std::vector<int> typeColor;

        std::ifstream input;
        std::string currentLine(512, ' ');
        size_t numberOfPoints = 0;

        input.open(pointsPath, std::ios_base::in);

        if (input.is_open())
        {
            input.getline(&currentLine[0], currentLine.size());
            numberOfPoints = std::stoi(currentLine, NULL);
            points.reserve(numberOfPoints + 2);
            typeColor.reserve(numberOfPoints + 2);


            while (!input.eof()) {
                size_t nextPosition = 0;
                std::vector<double> currentPoint(pTask.GetNumberOfContinuousVariable());
                double curVal;
                int s = currentLine.size();
                input.getline(&currentLine[0], currentLine.size());
                int t = currentLine.find('|');
                int l = currentLine.length();

                const char* cstr = currentLine.c_str();

                if (cstr[0] == '\0')
                    continue;
                if (currentLine == "\n")
                    continue;
                if (t == -1 || currentLine == "" || l == 0)
                    continue;

                std::string curStr = currentLine;

                currentPoint[0] = std::stod(curStr, &nextPosition);

                for (int iDim = 1; iDim < pTask.GetNumberOfContinuousVariable(); iDim++)
                {
                    curStr = curStr.substr(nextPosition);
                    currentPoint[iDim] = std::stod(curStr, &nextPosition);
                }

                std::string a = currentLine.substr(t + 1);
                curVal = std::stod(a);

                t = a.find('|');
                if (t == -1)
                {
                    typeColor.push_back(0);
                    continue;
                }
                else
                {
                    std::string b = a.substr(t + 1);
                    typeColor.push_back(std::stod(b));
                }

                points.push_back(currentPoint);
                pointVal.push_back(curVal);

            }
            input.close();

            numberLoadedPoints = points.size();
            newPoint.resize(numberLoadedPoints);

            for (int i = 0; i < numberLoadedPoints; i++)
            {
                newPoint[i] = TrialFactory::CreateTrial();
                for (int iDim = 0; iDim < pTask.GetNumberOfContinuousVariable(); iDim++)
                {
                    newPoint[i]->y[iDim] = points[i][iDim];
                }

                newPoint[i]->FuncValues[0] = pointVal[i];

                newPoint[i]->K = 1;
                newPoint[i]->index = 0;

                Extended genX(0.0);
                evolvent.GetInverseImage(newPoint[i]->y, genX);

                newPoint[i]->SetX(genX);

                pData->GetTrials().push_back(newPoint[i]);
            }
        }
    }
    if (numberLoadedPoints > 0)
    {
        this->InsertPoints(newPoint);

        //this->iteration.IterationCount += numberLoadedPoints;
        parameters.iterationNumber = iteration.IterationCount;
    }
}

// ------------------------------------------------------------------------------------------------
void Method_RSA::FirstIteration()
{
    // Задаем границы интервалов изменения параметров
    // Указатель на границы интервалов в подзадаче - это указатель на исходные границы,
    //   смещенный на число фиксированных размерностей
    evolvent.SetBounds(pTask.GetA(), pTask.GetB());

    // Это первая итерация, сбрасываем счетчик
    iteration.IterationCount = 1;
    // И сбрасываем достигнутую точность
    AchievedAccuracy = 1.0;
    // И лучшую итерацию

    SearchInterval** NewInterval = new SearchInterval * [1];
    //SearchIntervalFactory::CreateSearchInterval();
    for (int e = 0; e < 1; e++)
    {
        NewInterval[e] = SearchIntervalFactory::CreateSearchInterval();
        NewInterval[e]->ind = iteration.IterationCount;
        NewInterval[e]->K = 0;
        NewInterval[e]->CreatePoint();
        // Гельдеровская длина
        NewInterval[e]->delta = 1.0;
        //Добавляем интервал
        SearchInterval* p = pData->InsertInterval(*(NewInterval[e]));
        delete NewInterval[e];
        NewInterval[e] = p;
        pData->GetTrials().push_back(p->LeftPoint);
        pData->GetTrials().push_back(p->RightPoint);
        ///Необходимо сосчитать значения на границах
        CalculateImage(*p->LeftPoint);


        CalculateImage(*p->RightPoint);

        if (pData->GetBestTrial() == 0)
            pData->SetBestTrial(p->LeftPoint);

        //====================================================================
        if ((parameters.IsCalculationInBorderPoint == true) || (parameters.LocalTuningType != 0))
        {
            //if (pTask.GetNumberOfContinuousVariable() == 1)
            {
                // Эта функция вызывается только в листе дерева - поэтому вычисляем функционалы здесь
                for (int j = 0; j < pTask.GetNumOfFunc(); j++)
                {
                    p->LeftPoint->FuncValues[j] = MaxDouble;
                    p->RightPoint->FuncValues[j] = MaxDouble;
                }
                p->LeftPoint->K = 1;
                //p->RightPoint->K = 1;

                InformationForCalculation inputlocal;
                TResultForCalculation outputlocal;
                inputlocal.Resize(2);
                outputlocal.Resize(2);

                inputlocal.trials[0] = p->LeftPoint;

                inputlocal.trials[1] = p->RightPoint;

                Calculation* Calculation_ = CalculationFactory::CreateCalculation(pTask, &evolvent);

                Calculation_->Calculate(inputlocal, outputlocal);

                for (int j = 0; j < pTask.GetNumOfFunc(); j++)
                {
                    functionCalculationCount[j] = outputlocal.countCalcTrials[j];
                }
                UpdateOptimumEstimation(*(p->RightPoint));
                UpdateOptimumEstimation(*(p->LeftPoint));

            }
        }

    }
    //====================================================================

    // На первой итерации - единственный лучший интервал

    // Флаг пересчета - поднят
    pData->SetRecalc(true);

    // Точки первой итерации выбираются по особому правилу
    // Равномерно ставим NumPoints точек c шагом h
    // А надо бы случайно...
    double h = 1.0 / (parameters.NumPoints + 1);
    if (parameters.StartPoint.GetIsChange() && parameters.IsUseStartPoint) //берем начальную точку из параметров
    {

        int firstPointCount = parameters.NumPoints - 1;


        std::vector<Trial*> newPoint(parameters.NumPoints);
        newPoint[0] = TrialFactory::CreateTrial();

        pTask.CopyPoint(parameters.StartPoint.GetData(), newPoint[0]);

        InformationForCalculation inputlocal;
        TResultForCalculation outputlocal;
        int sfipi = 0;

        if (parameters.StartPointValues.GetIsChange())
        {
            for (int ifv = 0; ifv < parameters.StartPointValues.GetSize(); ifv++)
            {
                newPoint[0]->FuncValues[ifv] = parameters.StartPointValues[ifv];
                if ((ifv == (pTask.GetNumOfFunc() - 1)) || (newPoint[0]->FuncValues[ifv] > 0))
                {
                    newPoint[0]->index = ifv;
                }
            }
            inputlocal.Resize(firstPointCount);
            outputlocal.Resize(firstPointCount);
            sfipi = 0;
        }
        else
        {
            inputlocal.Resize(parameters.NumPoints);
            outputlocal.Resize(parameters.NumPoints);
            inputlocal.trials[0] = newPoint[0];
            sfipi = 1;
        }

        h = 1.0 / (firstPointCount + 1);
        for (int ifip = 0; ifip < firstPointCount; ifip++)
        {
            int ind = ifip + 1;

            newPoint[ind] = TrialFactory::CreateTrial();

            pData->GetTrials().push_back(newPoint[ind]);
            newPoint[ind]->SetX((ifip + 1) * h);

            // Вычисляем образ точки итерации - образ записывается в начальные позиции массива y
            CalculateImage(*newPoint[ind]);

            newPoint[ind]->leftInterval = NewInterval[0];
            newPoint[ind]->rightInterval = NewInterval[0];
            inputlocal.trials[sfipi + ifip] = newPoint[ind];
        }

        calculation.Calculate(inputlocal, outputlocal);

        for (int j = 0; j < pTask.GetNumOfFunc(); j++)
        {
            functionCalculationCount[j] = outputlocal.countCalcTrials[j];
        }


        for (int ifip = 0; ifip < newPoint.size(); ifip++)
        {
            UpdateOptimumEstimation(*(newPoint[ifip]));
            newPoint[ifip]->K = 1;

            Extended genX(0.0);
            evolvent.GetInverseImage(newPoint[ifip]->y, genX);

            newPoint[ifip]->SetX(genX);

            //pData->GetTrials().push_back(newPoint[ifip]);
        }

        this->InsertPoints(newPoint);

        this->iteration.IterationCount += 1;
        parameters.iterationNumber = iteration.IterationCount;
    }
    else if (!parameters.IsLoadFirstPointFromFile) // равномерно распределяем начальные точки
    {
        for (int q = 0; q < parameters.NumPoints; q++)
        {

            if (parameters.TypeDistributionStartingPoints == Evenly)
            {
                int ind = q;
                iteration.pCurTrials[ind] = TrialFactory::CreateTrial();

                pData->GetTrials().push_back(iteration.pCurTrials[ind]);
                iteration.pCurTrials[ind]->SetX((q + 1) * h);

                // Вычисляем образ точки итерации - образ записывается в начальные позиции массива y
                CalculateImage(*iteration.pCurTrials[ind]);

                iteration.pCurTrials[ind]->leftInterval = NewInterval[0];
                iteration.pCurTrials[ind]->rightInterval = NewInterval[0];
            }
            else
            {
                int ind = q;
                iteration.pCurTrials[ind] = TrialFactory::CreateTrial();
                pData->GetTrials().push_back(iteration.pCurTrials[ind]);

                for (size_t iCNP = 0; iCNP < pTask.GetNumberOfContinuousVariable(); iCNP++)
                {
                    iteration.pCurTrials[ind]->y[iCNP] = pTask.GetA()[iCNP] + ((double(q) + 1.0) * h) * (pTask.GetB()[iCNP] - pTask.GetA()[iCNP]);
                }

                Extended genX(0.0);
                evolvent.GetInverseImage(iteration.pCurTrials[ind]->y, genX);
                iteration.pCurTrials[ind]->SetX(genX);

                iteration.pCurTrials[ind]->leftInterval = NewInterval[0];
                iteration.pCurTrials[ind]->rightInterval = NewInterval[0];
            }
        }

    }
    else // читаем из файла FirstPointFilePath
    {

        LoadPoint();

    }

}

// ------------------------------------------------------------------------------------------------
void Method_RSA::Recalc()
{
    if (pData->IsRecalc())
    {
        pData->ClearQueue();
        for (SearcDataIterator it = pData->GetBeginIterator(); it; ++it)
        {
            it->R = CalculateGlobalR(*it);
            //it->locR = CalculateLocalR(*it);

            pData->PushToQueue(*it);
        }
        // После пересчета флаг опускаем
        pData->SetRecalc(false);
    }
}

// ------------------------------------------------------------------------------------------------
void Method_RSA::CalculateIterationPoints()
{
    if (iteration.IterationCount == 1)
    {
        return;
    }

    // Если поднят флаг - то пересчитать все характеристики
    Recalc();

    // Здесь надо взять NumPoints лучших характеристик из очереди
    // Очередь пока одна - очередь глобальных характеристик
    // В ней должно быть нужное количество интервалов, т.к. на первом шаге проводится NumPoints
    // испытаний
    std::vector<SearchInterval*> BestIntervals(parameters.NumPoints);

    int LocalMix = parameters.LocalMix;

    if (GetIterationType(iteration.IterationCount, LocalMix) == Global)
    {

        pData->GetBestIntervals(BestIntervals.data(), parameters.NumPoints);

    }
    else
        pData->GetBestLocalIntervals(BestIntervals.data(), parameters.NumPoints);
    // Пока заполняем одновременно вектор CurTrials, и вектор интервалов

    CalculateCurrentPoints(BestIntervals);
}

// ------------------------------------------------------------------------------------------------
void Method_RSA::CalculateCurrentPoints(std::vector<SearchInterval*>& BestIntervals)
{
    for (unsigned int i = 0; i < BestIntervals.size(); i++)
    {
        iteration.pCurTrials[i] = TrialFactory::CreateTrial();
        pData->GetTrials().push_back(iteration.pCurTrials[i]);
        CalculateCurrentPoint(*iteration.pCurTrials[i], BestIntervals[i]);
    }
}

// ------------------------------------------------------------------------------------------------
bool Method_RSA::CheckStopCondition()
{
    bool res = false;
    int numOfOptima = 1;
    double allOptimumPoints[MAX_TRIAL_DIMENSION * MAX_NUM_MIN];

    double CurrentAccuracy = 0.1;
    if (CurrentAccuracy < AchievedAccuracy)
        AchievedAccuracy = CurrentAccuracy;

    if (pTask.IsLeaf()) //если метод не в корне, то остановка только по точности
    {
        //if (AchievedAccuracy < parameters.Epsilon)
        //  res = true;
        double fm = parameters.Epsilon;
        if ((isSetInLocalMinimumInterval == true) || (AchievedAccuracy < fm))
            res = true;
    }
    else
    {
        switch (parameters.StopCondition)
        {
        case Accuracy:
            if (AchievedAccuracy < parameters.Epsilon)
                res = true;
            break;
        case OptimumVicinity:
        {
            res = true;
            //numOfOptima = ;
            if (pData->GetBestTrial()->GetValue() > parameters.Epsilon)
            {
                res = false;
                break;
            }
        }
        break;
        case OptimumVicinity2:
        {
            res = true;
            if (pData->GetBestTrial()->GetValue() > parameters.Epsilon)
            {
                res = false;
                break;
            }
        }
        break;
        case OptimumValue:
            if (pData->GetBestTrial()->index == pTask.GetNumOfFunc() - 1 &&
                pData->GetBestTrial()->FuncValues[pData->GetBestTrial()->index] < parameters.Epsilon)
                res = true;
            break;

        case InLocalArea:
        {
            double fm = parameters.Epsilon;
            if ((isSetInLocalMinimumInterval == true) || (AchievedAccuracy < fm))
                res = true;
            break;
        }

        case MaxIterWithoutImprovement:
        {
            if ((iteration.IterationCount - LastIterationBestUpdate) > parameters.MaxIterationsWithoutImprovement)
                res = true;
            break;
        }
        break;
        }
    }

    if (iteration.IterationCount >= MaxNumOfTrials)
        res = true;

    isStop = res;
    return res;
}


// ------------------------------------------------------------------------------------------------
void Method_RSA::CalculateFunctionals()
{
    std::vector<SearchInterval*> intervalArr(iteration.pCurTrials.size());

    // Эта функция вызывается только в листе дерева - поэтому вычисляем функционалы здесь
    for (unsigned int i = 0; i < iteration.pCurTrials.size(); i++)
    {
        if (iteration.pCurTrials[i] == 0)
            continue;
        // Записываем значения MaxDouble
        for (int j = 0; j < pTask.GetNumOfFunc(); j++)
            iteration.pCurTrials[i]->FuncValues[j] = MaxDouble;

        // Так как вычисление в листе дерева, то вложенных итераций нет
        iteration.pCurTrials[i]->K = 1;
        inputSet.trials[i] = iteration.pCurTrials[i];

        intervalArr[i] = iteration.pCurTrials[i]->leftInterval;
    }


    calculation.Calculate(inputSet, outputSet);

    for (unsigned int i = 0; i < iteration.pCurTrials.size(); i++)
    {
        if (outputSet.trials[i] == 0)
            iteration.pCurTrials[i] = 0;
    }

    for (int j = 0; j < pTask.GetNumOfFunc(); j++)
    {
        functionCalculationCount[j] += outputSet.countCalcTrials[j];
    }


}


// ------------------------------------------------------------------------------------------------
std::vector<int> Method_RSA::GetFunctionCalculationCount()
{
    return functionCalculationCount;
}

double Method_RSA::GetAchievedAccuracy()
{
    return AchievedAccuracy;
}

// ------------------------------------------------------------------------------------------------
void Method_RSA::InsertLocalPoints(const std::vector<Trial*>& points, Task* task)
{
    for (size_t j = 0; j < points.size(); j++)
    {
        Trial* currentPoint = points[j];
        Extended  x;
        evolvent.GetInverseImage(&(currentPoint->y[0]), x);
        currentPoint->SetX(x);
        SearchInterval* CoveringInterval = pData->FindCoveringInterval(currentPoint);

        if (!CoveringInterval)
            throw EXCEPTION("Covering interval does not exists");
        if (!(currentPoint->X() < CoveringInterval->xr() || currentPoint->X() > CoveringInterval->xl()))
            throw EXCEPTION("Wrong covering interval");

        if (points[j]->K <= 0)
            points[j]->K = 1;

        SearchInterval* p = pData->InsertPoint(CoveringInterval, *currentPoint,
            iteration.IterationCount, pTask.GetNumberOfContinuousVariable());

        UpdateOptimumEstimation(*currentPoint);

        if (parameters.TypeAddLocalPoint == 0)
        {
            if (AchievedAccuracy > CoveringInterval->delta)
                AchievedAccuracy = CoveringInterval->delta;
            if (p)
                if (AchievedAccuracy > p->delta)
                    AchievedAccuracy = p->delta;
        }

        if (!p)
        {
            continue;
        }
        else
        {
            pData->DeleteIntervalFromQueue(CoveringInterval);
        }

        // Если полный пересчет не нужен - обновляем только очереди характеристик
        if (!pData->IsRecalc())
        {
            // Удалять интервалы из очереди не надо - они уже удалены в GetBestIntervals
            // Вставляем два новых интервала
            p->R = CalculateGlobalR(p);
            //p->locR = CalculateLocalR(p);
            pData->PushToQueue(p);

            CoveringInterval->R = CalculateGlobalR(CoveringInterval);
            //CoveringInterval->locR = CalculateLocalR(CoveringInterval);

            pData->PushToQueue(CoveringInterval);
        }


    }
}


// ------------------------------------------------------------------------------------------------
void Method_RSA::InsertPoints(const std::vector<Trial*>& points)
{
    for (size_t j = 0; j < points.size(); j++)
    {
        Trial* currentPoint = points[j];
        Extended  x;

        for (int j = 0; j <= currentPoint->index; j++)
        {
            functionCalculationCount[j] += 1;
        }

        evolvent.GetInverseImage(currentPoint->y, x);
        currentPoint->SetX(x);
        SearchInterval* CoveringInterval = pData->FindCoveringInterval(currentPoint);

        if (AchievedAccuracy > CoveringInterval->delta)
            AchievedAccuracy = CoveringInterval->delta;

        if (!CoveringInterval)
            throw EXCEPTION("Covering interval does not exists");
        if (!(currentPoint->X() < CoveringInterval->xr() || currentPoint->X() > CoveringInterval->xl()))
            throw EXCEPTION("Wrong covering interval");

        SearchInterval* p = pData->InsertPoint(CoveringInterval, *currentPoint,
            iteration.IterationCount, pTask.GetNumberOfContinuousVariable());

        UpdateOptimumEstimation(*currentPoint);

        if (p)
        {
            this->iteration.IterationCount++;
        }

    }
}

// ------------------------------------------------------------------------------------------------
bool Method_RSA::UpdateOptimumEstimation(Trial& trial)
{
    if (trial.index > pData->GetBestTrial()->index || trial.index == pData->GetBestTrial()->index &&
        trial.FuncValues[pData->GetBestTrial()->index] < pData->GetBestTrial()->FuncValues[pData->GetBestTrial()->index])
    {
        pData->SetBestTrial(trial.Clone());

        // Оптимум обновился - нужен пересчет
        pData->SetRecalc(true);
        isLocalZUpdate = true;
        LastIterationBestUpdate = iteration.IterationCount;
        return true;
    }
    return false;
}

// ------------------------------------------------------------------------------------------------
void Method_RSA::SavePoints()
{
    if (static_cast<std::string>(parameters.IterPointsSavePath).size() > 0)
    {
        if (parameters.IterPointsSavePath.ToString() != "")
        {
            SearcDataIterator it = pData->GetBeginIterator();

            for (++it; it; ++it)
            {
                printPoints.push_back((*it)->LeftPoint->Clone());
            }
        }
    }

}


// ------------------------------------------------------------------------------------------------
double Method_RSA::CalculateGlobalR(SearchInterval* p)
{
    double deltax = p->delta;
    double value = 0;
    int v = 0;
    if ((p->izl() == -2) && (p->izr() == -2))
    {
        if (parameters.GetProcRank() == 3)
        {
            print << "Bad Interval\tProcRank=" << parameters.GetProcRank() << "\n";

            value = 0;
            print << "iteration.IterationCount = " << iteration.IterationCount << "\n";

            print << "NewInterval! xl() = " << p->xl().toDouble() << " xr() = "
                << p->xr().toDouble() << " izl() = " << p->izl() << " izr() = "
                << p->izr() << " zl() = " << p->zl() << " zr() = " << p->zr()
                << " R = " << p->R << "\n";

            printf("Z[%d] = %lf M = %lf alfa = %lf\n", v, pData->Z[v], pData->Z[v], alfa);
            printf("val = %lf\n", value);

        }
    }
    else if ((p->izl() == -3) || (p->izr() == -3))
    {
        return MinDouble;
    }
    else if (p->izl() == p->izr())
    {
        value = -(p->zr() + p->zl()) / deltax;
    }
    else if (p->izr() > p->izl())
    {
        value = -(p->zr() + pData->Z[p->izr()]) / deltax;
    }
    else //if (p->izr() < p->izl)
    {
        value = -(p->zl() + pData->Z[p->izl()]) / deltax;
    }

    //Характеристика интервала должна быть конечной, иначе - ошибка
#ifdef WIN32
    if (!_finite(value))
#else
    if (!std::isfinite(value))
#endif
    {
        throw EXCEPTION("Infinite R!");
    }

    return value;
}

// ------------------------------------------------------------------------------------------------
double Method_RSA::CalculateLocalR(SearchInterval* p)
{
    double value;
    int v;
    if ((p->izl() == -3) || (p->izr() == -3))
    {
        return MinDouble;
    }
    if (p->izl() == p->izr())
    {
        v = p->izl();
        value =
            //p->R / (sqrt((p->zr() - pData->Z[v]) * (p->zl() - pData->Z[v])) / pData->M[v] + pow(1.5, -alfa));   
            p->R / (sqrt((p->zr() - pData->Z[v]) * (p->zl() - pData->Z[v])) / pData->M[v] + pow(10, -alfa));
    }
    else if (p->izl() > p->izr())
    {
        v = p->izl();
        //value = p->R / ((p->zl() - pData->Z[v]) / pData->M[v] + pow(1.5, -alfa));
        value = p->R / ((p->zl() - pData->Z[v]) / pData->M[v] + pow(10, -alfa));
    }
    else //if (p->izl() < p->izr)
    {
        v = p->izr();
        //value = p->R / ((p->zr() - pData->Z[v]) / pData->M[v] + pow(1.5, -alfa));
        value = p->R / ((p->zr() - pData->Z[v]) / pData->M[v] + pow(10, -alfa));
    }

    //Характеристика интервала должна быть конечной, иначе - ошибка
#ifdef WIN32
    if (!_finite(value))
#else
    if (!std::isfinite(value))
#endif
    {
        throw EXCEPTION("Infinite R!");
    }

    return value;
}

// ------------------------------------------------------------------------------------------------
/// Добавление основных (из основной\единственной развертки) точек испытания в базу, возвращиет правый
SearchInterval* Method_RSA::AddCurrentPoint(Trial& pCurTrialsj, SearchInterval* BestIntervalsj)
{
    // правый подинтервал
    SearchInterval* NewInterval = SearchIntervalFactory::CreateSearchInterval();
    if (isFindInterval || BestIntervalsj == 0)//если нужно искать интервал
    {
        //то ищем
        BestIntervalsj = pData->FindCoveringInterval(&pCurTrialsj);
        pCurTrialsj.leftInterval = BestIntervalsj;
        pCurTrialsj.rightInterval = BestIntervalsj;
    }
    else if ((pCurTrialsj.X() > BestIntervalsj->xr()) || (pCurTrialsj.X() < (BestIntervalsj)->xl()))
        //иначе проверяем что точка вне интервала интервала
    {
        //ищем подходящий интервал
        (BestIntervalsj) = pData->FindCoveringInterval(&pCurTrialsj);
        pCurTrialsj.leftInterval = BestIntervalsj;
        pCurTrialsj.rightInterval = BestIntervalsj;
    }

    if (!((BestIntervalsj)->xl() < pCurTrialsj.X() &&
        pCurTrialsj.X() < (BestIntervalsj)->xr()))
    {
        //CoveringPointCount0++;
        if ((BestIntervalsj)->delta < AchievedAccuracy)
        {
            AchievedAccuracy = (BestIntervalsj)->delta;
        }
        return 0;
    }

    // Заполнение интервала
    NewInterval->ind = iteration.IterationCount;
    // Запоминаем число вложенных итераций
    NewInterval->K = pCurTrialsj.K;
    // Левая точка интервала - это точка очередного испытания
    NewInterval->LeftPoint = &pCurTrialsj;



    // Правая точка интервала - это правая точка интервала, в котором проведено испытание
    NewInterval->RightPoint = BestIntervalsj->RightPoint;

    //Правая точка должна быть больше левой, если - меньше, ошибка!!!
    if (NewInterval->xr() <= NewInterval->xl())
    {
        throw EXCEPTION("Interval with negative length!");
    }

    // Гельдеровская длина интервала
    NewInterval->delta = root(NewInterval->xr() - NewInterval->xl(), pTask.GetNumberOfContinuousVariable());

    // Корректируем существующий интервал
    (BestIntervalsj)->RightPoint = NewInterval->LeftPoint;


    // Обновляем достигнутую точность - по старой гельдеровской длине лучшего интервала
    if ((BestIntervalsj)->delta < AchievedAccuracy)
    {
        AchievedAccuracy = (BestIntervalsj)->delta;
    }
    // После чего вычисляем новую гельдеровскую длину лучшего интервала
    (BestIntervalsj)->delta = root((BestIntervalsj)->xr() - (BestIntervalsj)->xl(), pTask.GetNumberOfContinuousVariable());
    //(BestIntervalsj)->delta = root((BestIntervalsj)->xr() - (BestIntervalsj)->xl(), pTask.GetNumberOfContinuousVariable());
    //    (*BestIntervalsj)->delta = pow((*BestIntervalsj)->dx,1.0/pTask.GetNumberOfContinuousVariable());

    int j = BestIntervalsj->izr();
    if (BestIntervalsj->izl() > j)
        j = BestIntervalsj->izl();

    if (Xmax[j] < (BestIntervalsj)->delta)
    {
        Xmax[j] = (BestIntervalsj)->delta;
    }

    j = NewInterval->izr();
    if (NewInterval->izl() > j)
        j = NewInterval->izl();

    if (Xmax[j] < (NewInterval)->delta)
    {
        Xmax[j] = (NewInterval)->delta;
    }

    // Интервал сформирован - можно добавлять
    // Вставка завершается корректно, или же выбрасывает исключение
    SearchInterval* p = pData->InsertInterval(*NewInterval);

    delete NewInterval;

    pCurTrialsj.leftInterval = (BestIntervalsj);
    pCurTrialsj.rightInterval = p;

    pCurTrialsj.leftInterval->LeftPoint->rightInterval = (BestIntervalsj);
    pCurTrialsj.rightInterval->RightPoint->leftInterval = p;

    return p;

}


// ------------------------------------------------------------------------------------------------
void Method_RSA::RenewSearchData()
{
    for (unsigned int j = 0; j < iteration.pCurTrials.size(); j++)
    {
        SearchInterval* interval = iteration.pCurTrials[j]->leftInterval;
        pData->PushToQueue((interval));
    }

    for (unsigned int j = 0; j < iteration.pCurTrials.size(); j++)
    {
        if (iteration.pCurTrials[j] == 0)
            continue;

        SearchInterval* p;
        SearchInterval* interval;

        if (parameters.MapType == mpNoninjective) {
            Extended* x_ = new Extended[1 << pTask.GetNumberOfContinuousVariable()];
            int kpp = evolvent.GetNoninjectivePreimages(iteration.pCurTrials[j]->y, x_);

            std::vector<Trial*> tmp(kpp+1);

            for (int i = 0; i < kpp+1; ++i) 
            {
                tmp[i] = TrialFactory::CreateTrial(iteration.pCurTrials[j]);
                pData->GetTrials().push_back(tmp[i]);
            }

            for (int i = 0; i < kpp+1; ++i)
            {
                if (i < kpp)
                    tmp[i]->SetX(x_[i]);
                p = 0;
                interval = 0;
                p = AddCurrentPoint(*tmp[i], 0);
                interval = tmp[i]->leftInterval;

                if (p == 0)
                    continue;

                //Обработка началной итерации
                if (iteration.IterationCount == 1)
                {
                    pData->SetRecalc(true);
                }

                // Если полный пересчет не нужен - обновляем только очереди характеристик
                p->R = CalculateGlobalR(p);
                pData->PushToQueue(p);

                (interval)->R = CalculateGlobalR((interval));

                pData->TrickleUp(interval);

            }

            delete[] x_;
        }
        else if(parameters.MapType == mpLinar){
            p = 0;
            interval = iteration.pCurTrials[j]->leftInterval;
            p = AddCurrentPoint(*iteration.pCurTrials[j], interval);

            if (p == 0)
                continue;

            if (interval == 0)
                interval = iteration.pCurTrials[j]->leftInterval;

            //Обработка началной итерации
            if (iteration.IterationCount == 1)
            {
                pData->SetRecalc(true);
            }

            // Если полный пересчет не нужен - обновляем только очереди характеристик
            if (!pData->IsRecalc())
            {
                // Удалять интервалы из очереди не надо - они уже удалены в GetBestIntervals
                // Вставляем два новых интервала
                p->R = CalculateGlobalR(p);
                pData->PushToQueue(p);

                (interval)->R = CalculateGlobalR((interval));

                pData->PushToQueue((interval));
            }
        }
        else {
            p = 0;
            interval = iteration.pCurTrials[j]->leftInterval;
            p = AddCurrentPoint(*iteration.pCurTrials[j], interval);

            if (p == 0)
                continue;

            if (interval == 0)
                interval = iteration.pCurTrials[j]->leftInterval;

            //Обработка началной итерации
            if (iteration.IterationCount == 1)
            {
                pData->SetRecalc(true);
            }

            // Если полный пересчет не нужен - обновляем только очереди характеристик
            if (!pData->IsRecalc())
            {
                // Удалять интервалы из очереди не надо - они уже удалены в GetBestIntervals
                // Вставляем два новых интервала
                p->R = CalculateGlobalR(p);
                pData->PushToQueue(p);

                (interval)->R = CalculateGlobalR((interval));

                pData->PushToQueue((interval));
            }
        }
    }
    isFindInterval = false;

    // Сохраняем состояние после первой итерации
    SaveCurrentProgress();
}

// ------------------------------------------------------------------------------------------------
bool Method_RSA::EstimateOptimum()
{
    bool isOptimumUpdated = false;
    // Сравниваем значение в текущем оптимуме с текущими точками
    for (unsigned int j = 0; j < iteration.pCurTrials.size(); j++)
    {
        if (iteration.pCurTrials[j] == 0)
            continue;
        isOptimumUpdated = UpdateOptimumEstimation(*iteration.pCurTrials[j]);
    }
    return isOptimumUpdated;
}


// ------------------------------------------------------------------------------------------------
void Method_RSA::SetNumPoints(int newNP)
{
    if (newNP <= 0)
        newNP = 1;

    if (iteration.pCurTrials.size() != newNP)
    {
        for (unsigned int i = 0; i < iteration.pCurTrials.size(); i++)
            iteration.pCurTrials[i] = 0;

        iteration.pCurTrials.resize(newNP);
    }

    inputSet.Resize(newNP);
    outputSet.Resize(newNP);
}

// ------------------------------------------------------------------------------------------------
void Method_RSA::FinalizeIteration()
{
    iteration.IterationCount++;
    parameters.iterationNumber = iteration.IterationCount;

    for (unsigned int i = 0; i < iteration.pCurTrials.size(); i++)
        iteration.pCurTrials[i] = 0;

    if (parameters.TypeCalculation == AsyncMPI)
    {
        SetNumPoints(1);
        parameters.NumPoints = 1;
    }

    if (isLocalZUpdate)//Если нужен пересчет - обновился минимум
    {
        LocalSearch();
    }
    isLocalZUpdate = false;
}

// ------------------------------------------------------------------------------------------------
int Method_RSA::GetIterationCount()
{
    return iteration.IterationCount;
}



// ------------------------------------------------------------------------------------------------
IterationType Method_RSA::GetIterationType(int iterationNumber, int LocalMixParameter)
{
    if (iterationNumber < StartLocalIteration)
        return   Global;

    IterationType type;
    if (LocalMixParameter > 0) {
        LocalMixParameter++;

        if (iterationNumber % LocalMixParameter != 0)
            type = Global;
        else
            type = Local;
    }
    else if (LocalMixParameter < 0) {
        LocalMixParameter = -LocalMixParameter;
        LocalMixParameter++;

        if (iterationNumber % LocalMixParameter != 0)
            type = Local;
        else
            type = Global;
    }
    else //LocalMixParameter == 0
        type = Global;

    return type;
}

// ------------------------------------------------------------------------------------------------
int Method_RSA::IsBoundary(SearchInterval* p)
{
    int ans = 0;
    if (p->izl() == -2)
        ans = 1;
    else if (p->izr() == -2)
        ans = 2;
    else if (p->LeftPoint->leftInterval == NULL)
        ans = 1;
    else if (p->RightPoint->rightInterval == NULL)
        ans = 2;

    return ans;
}

// ------------------------------------------------------------------------------------------------
Trial* Method_RSA::GetOptimEstimation()
{
    return pData->GetBestTrial();
}

// ------------------------------------------------------------------------------------------------
int Method_RSA::GetNumberOfTrials()
{

    // Число итераций равно числу интервалов в таблице - 1
    return pData->GetCount() - 1;

}

// ------------------------------------------------------------------------------------------------
void Method_RSA::PrintSection()
{
    if (parameters.IsPrintSectionPoint == true)
    {
        // Если задача верхнего уровня - надо просуммировать все данные из интервалов
        int NumberOfTrials = 0;
        int count = 0;
        for (SearcDataIterator it = pData->GetBeginIterator(); it; ++it)
        {
            NumberOfTrials += it->K;
            if (it->K > 1)
                print << "Section " << it->xl().toDouble() << "\t iteration = " << it->K << "\n";
            count++;
        }

        double sum = double(NumberOfTrials) / count;
        print << "Section average iteration = " << sum << "\n";
    }
}

// ------------------------------------------------------------------------------------------------
void Method_RSA::SaveCurrentProgress()
{
    if (parameters.FileSerializer.ToString().empty()) return;

    // Получаем новые точки и интервалы с последнего сохранения
    std::vector<Trial*> newTrials;
    std::vector<SearchInterval*> newIntervals;

    std::vector<Trial*>& allTrials = pData->GetTrials();
    for (int i = lastSavedTrialsCount; i < (int)allTrials.size(); ++i)
    {
        newTrials.push_back(allTrials[i]);
    }

    // Собираем новые интервалы
    int currentIntervalsCount = pData->GetCount();
    int intervalCounter = 0;
    for (SearcDataIterator it = pData->GetBeginIterator(); it; ++it)
    {
        if (intervalCounter >= lastSavedTrialsCount - 1)
        {
            newIntervals.push_back(*it);
        }
        intervalCounter++;
    }

    parameters.serializer->SaveProgress(parameters.FileSerializer.ToString(), newTrials, newIntervals, pData->GetBestTrial());

    lastSavedTrialsCount = allTrials.size();
}

// ------------------------------------------------------------------------------------------------
void Method_RSA::PrintLevelPoints(const std::string& fileName)
{
    std::ofstream fout;
    fout.open(fileName, std::ios_base::out);
    double* tmpPoint = new double[pTask.GetN()];
    fout << pData->GetCount() - 1 << " " <<
        pTask.GetNumOfFunc() << "\n";
    SearcDataIterator it = pData->GetBeginIterator();
    int t = 0;
    for (++it; it; ++it)
    {
        //evolvent.GetImage((*it)->xl(), tmpPoint);
        tmpPoint = (*it)->LeftPoint->y;
        for (int i = 0; i < pTask.GetN(); i++)
            fout << tmpPoint[i] << " ";
        fout << "| ";
        for (int i = 0; i < (*it)->izl() + 1; i++)
            fout << (*it)->z()[i] << " ";
        fout << "| " << (*it)->LeftPoint->TypeColor << " ";
        fout << "\n";
        t++;
    }

    for (int i = 0; i < pTask.GetN(); i++)
        fout << pData->GetBestTrial()->y[i] << " ";
    fout << "| ";
    for (int i = 0; i < pData->GetBestTrial()->index + 1; i++)
        fout << pData->GetBestTrial()->FuncValues[i] << " ";
    if (pTask.GetIsOptimumPointDefined())
    {
        fout << "\n" << pTask.GetOptimumPoint()[0];
        for (int i = 1; i < pTask.GetN(); i++)
            fout << " " << pTask.GetOptimumPoint()[i];
        fout << " | ";
        fout << pTask.GetOptimumValue() << " ";
    }
    fout.close();


}

// ------------------------------------------------------------------------------------------------
void Method_RSA::PrintPoints(const std::string& fileName)
{
    std::ofstream fout;
    fout.open(fileName, std::ios_base::out);
    double* tmpPoint = new double[pTask.GetN()];
    fout << pData->GetCount() - 1 + printPoints.size() << " " <<
        pTask.GetNumOfFunc() << "\n";
    SearcDataIterator it = pData->GetBeginIterator();
    int t = 0;
    for (++it; it; ++it)
    {
        //evolvent.GetImage((*it)->xl(), tmpPoint);
        tmpPoint = (*it)->LeftPoint->y;
        for (int i = 0; i < pTask.GetN(); i++)
            fout << tmpPoint[i] << " ";
        fout << "| ";
        for (int i = 0; i < (*it)->izl() + 1; i++)
            fout << (*it)->z()[i] << " ";
        fout << "| " << (*it)->LeftPoint->TypeColor << " ";
        fout << "\n";
        t++;
    }

    for (int k = 0; k < printPoints.size(); k++)
    {
        tmpPoint = printPoints[k]->y;
        for (int i = 0; i < pTask.GetN(); i++)
            fout << tmpPoint[i] << " ";
        fout << "| ";
        for (int i = 0; i < printPoints[k]->index + 1; i++)
            fout << printPoints[k]->FuncValues[i] << " ";
        fout << "| " << printPoints[k]->TypeColor << " ";
        fout << "\n";
    }

    for (int i = 0; i < pTask.GetN(); i++)
        fout << pData->GetBestTrial()->y[i] << " ";
    fout << "| ";
    for (int i = 0; i < pData->GetBestTrial()->index + 1; i++)
        fout << pData->GetBestTrial()->FuncValues[i] << " ";
    if (pTask.GetIsOptimumPointDefined())
    {
        fout << "\n" << pTask.GetOptimumPoint()[0];
        for (int i = 1; i < pTask.GetN(); i++)
            fout << " " << pTask.GetOptimumPoint()[i];
        fout << " | ";
        fout << pTask.GetOptimumValue() << " ";
    }
    fout.close();
}

// ------------------------------------------------------------------------------------------------
void Method_RSA::HookeJeevesMethod(Trial& point, std::vector<Trial*>& localPoints)
{
    int addAllLocalPoints = 2;
    LocalMethod* localMethod = nullptr;

    int oldNP = parameters.NumPoints;

    parameters.NumPoints = 2 * parameters.NumPoints;

    switch (parameters.TypeLocalMethod) {
    case ParallelHookeJeeves:
    {
        Calculation* LocalCalculation = new OMPCalculation(pTask);

        localMethod = new ParallelHookeJeevesMethod(&pTask, point, *LocalCalculation, addAllLocalPoints);
        break;
    }
    default:
    {
        localMethod = new LocalMethod(&pTask, point, addAllLocalPoints);
        break;
    }
    }



    double initialStep = 0;
    for (int i = 0; i < pTask.GetN(); i++)
        initialStep += pTask.GetB()[i] - pTask.GetA()[i];
    initialStep /= pTask.GetNumberOfContinuousVariable();
    // начальный шаг равен среднему размеру стороны гиперкуба, умноженному на коэффициент
    localMethod->SetEps(parameters.LocalVerificationEpsilon);
    localMethod->SetInitialStep(0.07 * initialStep);
    localMethod->SetMaxTrials(parameters.LocalIteration);
    Trial newpoint2 = localMethod->StartOptimization();
    Trial* newpoint = TrialFactory::CreateTrial(&newpoint2);

    parameters.NumPoints = oldNP;

    std::vector<Trial> points = localMethod->GetSearchSequence();

    int s = points.size();
    for (int i = 0; i < s; i++)
    {
        bool isOutOfRange = false;
        for (int g = 0; g < pTask.GetN(); g++)
        {
            if (points[i].y[g] > pTask.GetB()[g] || points[i].y[g] < pTask.GetA()[g])
                isOutOfRange = true;
        }

        if (isOutOfRange)
            continue;

        Trial* temp = TrialFactory::CreateTrial(&points[i]);
        Extended x;
        evolvent.GetInverseImage(&(temp->y[pTask.GetProcLevel()]), x);
        temp->SetX(x);
        temp->FuncValues[0] = pTask.CalculateFuncs(temp->y, 0);
        temp->TypeColor = 3;
        localPoints.push_back(temp);
    }

    InsertLocalPoints(localPoints);
}



// ------------------------------------------------------------------------------------------------
void Method_RSA::LocalSearch()
{
    if (((parameters.LocalRefineSolution == FinalStart && isStop) ||
        (parameters.LocalRefineSolution == UpdatedMinimum))
        && GetOptimEstimation()->index == pTask.GetNumOfFunc() - 1)
    {

        int oldNP = parameters.NumPoints;
        int oldNT = parameters.NumThread;

        if (parameters.LocalVerificationNumPoint <= 0)
        {
            parameters.LocalVerificationNumPoint = parameters.NumPoints;
        }

        parameters.NumPoints = parameters.LocalVerificationNumPoint.GetData();
        parameters.NumThread = parameters.LocalVerificationNumPoint.GetData();

        numberLocalMethodtStart++;
        std::vector<Trial*> localPoints;

        Trial* point = GetOptimEstimation();

        //HookeJeevesMethod(point, localPoints);

        LocalMethod* localMethod = nullptr;

        int addAllLocalPoints = std::max(0, int(parameters.TypeAddLocalPoint) - 2);
        switch (parameters.TypeLocalMethod) {
        case ParallelHookeJeeves:
        {

            //Calculation* newCalculation = new OMPCalculation(pTask);

            //localMethod = new ParallelHookeJeevesMethod(&(pTask), *point, *newCalculation, addAllLocalPoints);
            localMethod = new ParallelHookeJeevesMethod(&(pTask), *point, calculation, addAllLocalPoints);


            //localMethod = new ParallelHookeJeevesMethod(&(pTask), *point, this->calculation, addAllLocalPoints);
            break;
        }
        default:
            localMethod = new LocalMethod(&(pTask), *point, addAllLocalPoints);
            break;
        }

        double initialStep = 0;
        for (int i = 0; i < pTask.GetN(); i++)
            initialStep += pTask.GetB()[i] - pTask.GetA()[i];
        initialStep /= pTask.GetNumberOfContinuousVariable();
        // начальный шаг равен среднему размеру стороны гиперкуба, умноженному на коэффициент
        localMethod->SetEps(parameters.LocalVerificationEpsilon);

        localMethod->SetInitialStep(0.07 * initialStep);

        localMethod->SetMaxTrials(parameters.LocalIteration);
        Trial point2 = localMethod->StartOptimization();
        Trial* newpoint = TrialFactory::CreateTrial(&point2);

        std::vector<Trial> points = localMethod->GetSearchSequence();

        int s = points.size();
        for (int i = 0; i < s; i++)
        {
            bool isOutOfRange = false;
            for (int g = 0; g < pTask.GetN(); g++)
            {
                if (points[i].y[g] > pTask.GetB()[g] || points[i].y[g] < pTask.GetA()[g])
                    isOutOfRange = true;
            }

            if (isOutOfRange)
                continue;

            Trial* temp = TrialFactory::CreateTrial(&points[i]);
            Extended x;
            evolvent.GetInverseImage(temp->y, x);
            temp->SetX(x);
            temp->FuncValues[0] = pTask.CalculateFuncs(temp->y, 0);
            temp->TypeColor = 3;
            localPoints.push_back(temp);
        }
        parameters.NumPoints = oldNP;
        parameters.NumThread = oldNT;
        InsertLocalPoints(localPoints);


        localPointCount += localMethod->GetTrialsCounter();

        if (parameters.LocalRefineSolution == UpdatedMinimum)
            pData->SetRecalc(true);

    }
}

// ------------------------------------------------------------------------------------------------
int Method_RSA::GetLocalPointCount()
{
    return localPointCount;
}

// ------------------------------------------------------------------------------------------------
int Method_RSA::GetNumberLocalMethodtStart()
{
    return numberLocalMethodtStart;
}



// - end of file ----------------------------------------------------------------------------------

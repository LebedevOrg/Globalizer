/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      parameters.cpp                                              //
//                                                                         //
//  Purpose:   Source file for parameters class                            //
//                                                                         //
//  Author(s): Lebedev I.                                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include <mpi.h>

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <omp.h>


#ifdef WIN32
#include <windows.h>
#define DEFAULT_LIB rastrigin.dll
#else
#include <unistd.h>
#define DEFAULT_LIB ./librastrigin.so
#endif

#include "exception.h"
#include "parameters.h"


TParameters parameters;

// ------------------------------------------------------------------------------------------------
void TParameters::SetDefaultParameters()
{
  InitOption(HELP, 0, "-HELP", "Print Help", 1);
  InitOption(IsPlot, 0, "-PLOT", "Plot level line", 1); 
  InitOption(PlotGridSize, 300, "-PGS", "Drawing mesh precision", 1);
  InitOption(IsCalculateNumPoint, 0, "-ICNP", "Number of trials will be calculated at each iteration", 1);
  Separator = std::string("_"); //Переопределяем сепаратор на значение по умолчанию
  SetSeparator();
  InitOption(NumPoints, 1, "-np", "the number of points per iteration", 1);
  InitOption(StepPrintMessages, 1000, "-spm", "StepPrintMessages", 1);
  InitOption(StepSavePoint, 1000000, "-ssp", "After how many iterations to save points", 1);

  InitOption(TypeMethod, StandartMethod, "-tm", "HybridMethod or StandartMethod or ManyNumPointMethod", 1);
  InitOption(TypeCalculation, OMP, "-tc", "OMP or CUDA", 1);
  InitOption(TypeProcess, SynchronousProcess, "-tp", "TypeProcess", 1);
  InitOption(NumThread, 1, "-nt", "Num OpenMP Thread", 1);
  InitOption(SizeInBlock, 32, "-sb", "Size In CUDA Block", 1);
  InitOption(IsPrintFile, false, "-IsPF", "Is Print report to File", 1);
  
  InitOption(Dimension, -1, "-N", "Dimension", 1);
  
  InitOption(r, 2.3, "-r", "r", 1);
  
  InitOption(rDynamic, 0, "-rd", "Additive when dynamics change r, r = r + rDynamic / (Iteration ^ (1 / N))", 1);
  InitOption(rEps, 0.01, "-rE", "eps-reserv", 1);
  InitOption(rs, 2.3_2.3_2.3_2.3, "-rs", "r by levels", 4);
  InitOption(Eps, 0.01_0.01_0.01_0.01, "-Eps", "Epsilon", 4);
  InitOption(Comment, 000, "-Comment", "Comment", 1);//ResulLog
  InitOption(ResulLog, 000, "-ResulLog", "ResulLog", 1);
  InitOption(Epsilon, 0.01, "-E", "Epsilon", 1);
  
  InitOption(M_constant, 1, "-M_constant", "Initial M_constant estimations for each function", 1);
  InitOption(m, 10, "-m", "Number of evolnents", 1);
  InitOption(deviceCount, -1, "-dc", "Device count, def: -1 auto", 1);
  InitOption(MapType, mpBase, "-mt", "MapType", 1);
  InitOption(TypeDistributionStartingPoints, Evenly, "-tdsp",
    "Type of distribution of starting points ", 1);

  InitOption(NumOfTaskLevels, 1, "-nl", "NumOfTaskLevels", 1); // максимальное число уровней в дереве - 5
  InitOption(DimInTaskLevel, 0_0_0_0, "-dl", "DimInTaskLevel", 4);

  InitOption(DebugAsyncCalculation, 0, "-dac", "Helps debug in async calculation", 1); // Должен существовать файл: ../_build/async.txt
  InitOption(FullOrShort, 0, "-fs", "Full calculation in Parallel Block Scheme or just function", 1); // 0 - полное вычисление; 1 - вычисление только значения функции
  InitOption(ChildInProcLevel, 0_0_0_0, "-cl", "ChildInProcLevel", 4); // последний уровень - листья
  InitOption(MapInLevel, 1_1_1_1, "-ml", "MapInLevel", 4); // пока по одной развертке на уровень
  InitOption(MapProcInLevel, 1_1_1_1, "-mpl", "MapProcInLevel", 4); // пока по одной развертке на уровень
  InitOption(IsPrintSectionPoint, false, "-IsPSP", "Whether to print section information in a Block Scheme", 1);

  InitOption(MaxNumOfPoints, 7000000_2_2_2, "-MaxNP", "MaxNumOfPoints", 4);
  
  InitOption(IsSetDevice, false, "-sd", "Assign each process their device", 1);
  InitOption(deviceIndex, -1, "-di", "Device Index, def: -1 auto", 1);

  InitOption(localVerificationType, None, "-doLV", "Enables or disables starting local method after the global one finished", 1);
  InitOption(TypeLocalMethod, LeastSquareMethod, "-tlm", "Type Local Method, 0-Huck-Jivs, 1 - Qvadric, 2 - Gold", 1);
  InitOption(localVerificationIteration, 10000, "-lvi", "Number of local method iterations", 1);
  InitOption(localVerificationEpsilon, 0.0001, "-lve", "Local Method Accuracy", 1);
  InitOption(localVerificationNumPoint, -1, "-lvnp", "local Verification NumPoint", 1);
  
  InitOption(localMix, 0, "-lm", "local mix parameter", 1);
  InitOption(localAlpha, 15, "-la", "parameter alpha in mixed algorithm", 1);
  InitOption(sepS, Off, "-sepS", "enables separable optimization on start", 1);
  InitOption(rndS, false, "-rndS", "enables random optimization on start", 1);
  InitOption(libPath, DEFAULT_LIB, "-lib", "path to a library with the optimization problem", 1);
  
  InitOption(libConfigPath, \0, "-libConf", "path to config a of library with the optimization problem", 1);
  
  InitOption(stopCondition, Accuracy, "-stopCond", "stop condition type", 1);
  InitOption(isStopByAnyLevel, true, "-isbal", "Is Stop By Any Level", 1);
  InitOption(iterPointsSavePath, \0, "-sip", "path to save iterations points", 1);
  InitOption(printAdvancedInfo, 0, "-advInf", "print advanced statistics", 1);
  InitOption(disablePrintParameters, 0, "-dpp", "disable print parameters", 1);
  InitOption(logFileNamePrefix, examin_log, "-logFName", "prefix in log file name", 1);

  InitOption(calculationsArray, -1_0, "-ca", "ChildInProcLevel", 2);

  InitOption(TypeSolver, SingleSearch, "-ts", "TypeSolver ", 1);
  InitOption(DimInTask, 0_0_0_0, "-dt", "DimInSeparableTask", 4);

  InitOption(mpiBlockSize, 1, "-mbs", "Size of blocks in mpi calculation", 1);
  
  InitOption(isUseTaskR, false, "-iutr", "isUseTaskR", 1);
  InitOption(isUseFullRecount, false, "-iufr", "isUseFullRecount", 1);

  InitOption(isUseIntervalR, false, "-iuir", "isUseIntervalR", 1);
  InitOption(isUseGlobalZ, false, "-iugz", "isUseGlobalZ", 1);
  InitOption(isNotUseZ, false, "-inuz", "isNotUseZ", 1);
  
  InitOption(isUseLocalUpdate, false, "-iulu", "Whether to use the local method when finding the local minimum", 1);
  InitOption(countPointInLocalMinimum, 5, "-cpilm", "Count Point In Local Minimum", 1); 
  InitOption(Localr, 16, "-lr", "Local refinement enable parameter (the length of the local subdomain is 1 / Localr of the maximum length)", 1);

  InitOption(TypeStartLocalMethod, EqualNumberOfPoints, "-tlsm", "Type of criterion for starting the local method (0 - found any countPointInLocalMinimum points forming a paraboloid, 1 found countPointInLocalMinimum / 2 points left and right)", 1);
  InitOption(TypeAddLocalPoint, NotTakenIntoAccountInStoppingCriterion, "-talp", "The type of adding local refinement points (0 - as normal points, 1 - local method points are not counted in the precision stopping criterion)", 1);
  InitOption(maxCountLocalPoint, 5, "-mclp", "Maximum number of points set by the local method", 1);
  InitOption(PointTakingType, 0, "-ptt", "Type of taking points: 0 - take np points at once, 1 - take into account the neighborhood of local minima", 1);
  InitOption(StatusIntervalChangeType, 0, "-sict", "Interval status change type: 0 - do not change in descendants, 1 - change.", 1);
  InitOption(isCalculationInBorderPoint, false, "-icibp", "Is Calculation Function In Border Point", 1);
  InitOption(LocalTuningType, WithoutLocalTuning, "-ltt", "Type of local tuning: 0 - without it, 1 - LT, 2 - LTA, 3 - LTMA", 1);
  InitOption(ltXi, 1e-6, "-ltXi", "Parameter of local tuning", 1);
  InitOption(TypeLocalMinIntervale, NPoints, "-tlmi", "Type Local Min Intervale Search", 1); 
  InitOption(DecisionTreesMaxDepth, 6, "-dtmd", "Decision Trees Max Depth The maximum possible depth of the tree. That is the training algorithms attempts to split a node while its depth is less than maxDepth. The root node has zero depth. The actual depth may be smaller if the other termination criteria are met (see the outline of the training procedure @ref ml_intro_trees here), and/or if the tree is pruned", 1);
  InitOption(DecisionTreesRegressionAccuracy, 0.01, "-dtra", "Decision Trees Regression Accuracy Termination criteria for regression trees.     If all absolute differences between an estimated value in a node and values of train samples in this node are less than this parameter then the node will not be split further", 1);
  
  
  InitOption(NumPointsForApproximation, -1, "-npfa", "Num Points For Approximation", 1);
  
  //MCO
  InitOption(Lamda, -1, "-ld", "LamdaValue", 1);
  InitOption(rLamda, -1, "-rld", "resue Lamda", 1);
  InitOption(numberOfLamda, 50, "-nld", "number of Lamda", 1);
  InitOption(isCriteriaScaling, false, "-isCS", "is criteria scaling at convolution", 1);
  InitOption(itrEps, 0, "-itrEps", "itertion berfor solution", 1);
  InitOption(MCO_N_Criteria, 0, "-mcoN_crit", "number of criteria at geniral model of MCO problem", 1);
  InitOption(MCO_N_Constraint, 0, "-mcoN_const", "number of constraint at geniral model of MCO problem", 1);
  InitOption(MCO_Criteria_perm, 0, "-mcoCr_perm", "Permutation of criteria at geniral model of MCO problem", 1);
  InitOption(MCO_Constraint_perm, 0, "-mcoCo_perm", "Permutation of criteria at geniral model of MCO problem", 1);
  InitOption(MCO_q, 0, "-mco_q", "Tolerance vector at geniral model of MCO problem", 1);

  InitOption(isLoadFirstPointFromFile, false, "-islfp", "is load first point from file", 1);
  InitOption(FirstPointFilePath, \0, "-fpf", "path from first point file", 1);

  InitOption(ProcRank, -1, "-ProcRank", "Rank of process, def: -1 auto", 1);

  InitOption(functionSignMultiplier, 1.0_1.0_1.0_1.0 , "-fsm", "The multiplier in front of the function that determines whether we minimize or maximize the function", 4);

  ProcRank.SetGetter(&TParameters::GetProcRank);
  ProcRank.SetIsHaveValue(false);
  //TInt<TParameters> ProcRank;
  iterationNumber = 0;

  libConfigPath.mIsEdit = true;
  libPath.mIsEdit = true;
  localVerificationType.mIsEdit = true;
  localVerificationIteration.mIsEdit = true;
  localVerificationEpsilon.mIsEdit = true;
  MaxNumOfPoints.mIsEdit = true;
  Epsilon.mIsEdit = true;
  r.mIsEdit = true;
  Dimension.mIsEdit = true;
  IsPrintFile.mIsEdit = true;
  functionSignMultiplier.mIsEdit = true;
}

// ------------------------------------------------------------------------------------------------
int TParameters::CheckValueParameters(int index)
{
  TBaseParameters<TParameters>::CheckValueParameters(index);
  // Проверка на ошибки
  if (mIsInit)
  {
    mIsInit = false;

    // Проверка валидности вводимых данных
    if (NumPoints <= 0)
      NumPoints = 1;

    if (!Epsilon.GetIsChange() && Eps.GetIsChange())
      Epsilon = Eps[0];
    if (Epsilon.GetIsChange() && !Eps.GetIsChange())
    {
      Eps.SetSize(NumOfTaskLevels);
      for (int i = 0; i < NumOfTaskLevels; i++)
        Eps[i] = Epsilon;
    }

    if (Eps.GetSize() < NumOfTaskLevels)
    {
      if (Eps.GetSize() == 1)
      {
        double e = Eps[0];
        Eps.SetSize(NumOfTaskLevels);
        for (int i = 0; i < NumOfTaskLevels; i++)
          Eps[i] = e;
      }
      else
      {
        int oldSize = Eps.GetSize();
        Eps.SetSize(NumOfTaskLevels);
        for (int i = oldSize; i < NumOfTaskLevels; i++)
          Eps[i] = Epsilon;
      }
    }

    if (!DimInTaskLevel.GetIsChange())
      DimInTaskLevel[0] = Dimension;

    int sumDim = 0;
    for (int i = 0; i < NumOfTaskLevels; i++)
      sumDim += DimInTaskLevel[i];
    if (sumDim != Dimension)
    {
      if ((NumOfTaskLevels == 1) && (GetProcNum() == 1))
      {
        for (int i = 0; i < DimInTaskLevel.GetSize(); i++)
        {
          DimInTaskLevel[i] = 0;
        }
        DimInTaskLevel[0] = Dimension;
      }
    }


    if (!r.GetIsChange() && rs.GetIsChange())
      r = rs[0];
    if (r.GetIsChange() && !rs.GetIsChange())
    {
      rs.SetSize(NumOfTaskLevels);
      for (int i = 0; i < NumOfTaskLevels; i++)
        rs[i] = r;
    }

    if (rs.GetSize() < NumOfTaskLevels)
    {
      if (rs.GetSize() == 1)
      {
        double e = rs[0];
        rs.SetSize(NumOfTaskLevels);
        for (int i = 0; i < NumOfTaskLevels; i++)
          rs[i] = e;
      }
      else
      {
        int oldSize = rs.GetSize();
        rs.SetSize(NumOfTaskLevels);
        for (int i = oldSize; i < NumOfTaskLevels; i++)
          rs[i] = r;
      }
    }

    mNeedMPIProcessorCount = MapProcInLevel[0];
    //int ChildInProcLevel0[] = {2, 3, 0, 0}; // последний уровень - листья
    //// Общее число процессов = 1 (корень) + 2 (дети корня) + 2 * 3 (внуки корня) + ...
    int oldSize = MapProcInLevel[0];
    for (int i = 0; i < NumOfTaskLevels - 1; i++)
    {
      mNeedMPIProcessorCount += ChildInProcLevel[i] * MapProcInLevel[i + 1] * oldSize;
      oldSize = ChildInProcLevel[i] * MapProcInLevel[i + 1] * oldSize;
    }
    //if (GetProcRank() == 0)
      //printf("\nproc = %d\tNeed MPI processes - %d\n",0, mNeedMPIProcessorCount);

    //isCalculationsArray = false;
    //if (masSize[inccalculationsArray] == mNeedMPIProcessorCount)
    //  isCalculationsArray = true;

    if (calculationsArray.GetSize() < mNeedMPIProcessorCount)
    {
      int tempSize = calculationsArray.GetSize();
      int val = -1;
      if (calculationsArray.GetIsChange())
        val = (int)TypeCalculation;

      calculationsArray.SetSize(mNeedMPIProcessorCount);
      for (int i = tempSize; i < mNeedMPIProcessorCount; i++)
        calculationsArray[i] = val;
    }

    //дефолтные параметры для многопроцессорного запуска
    /*if (((!NumOfTaskLevels.GetIsChange() || !DimInTaskLevel.GetIsChange() ||
      !ChildInProcLevel.GetIsChange()) && (GetProcNum() > 1)) && (MapType != mpRotated))
    {
      if (GetProcRank() == 0)
        printf("\nRequired parameters are not specified!!!\nCalculated parameters: NumOfTaskLevels, DimInTaskLevel, ChildInProcLevel\n");
      NumOfTaskLevels = 2;
      DimInTaskLevel.SetSize(NumOfTaskLevels);
      ChildInProcLevel.SetSize(NumOfTaskLevels - 1);

      DimInTaskLevel[0] = Dimension / 2;
      DimInTaskLevel[1] = Dimension - DimInTaskLevel[0];

      ChildInProcLevel[0] = GetProcNum() - 1;
    }*/

    // TODO::dmsi Убрать, если асинхронная схема научится работать с пачками точек
    if (TypeCalculation == 8) {
      mpiBlockSize = 1;
    }
    //Если запуск на mpi (синхронном или асинхронном), но не блочная схема, то NumPoints может принимать только одно значение
    if ((TypeCalculation == 7) && (GetProcNum() > 1) && (GetProcRank() == 0)) {
      int val = (GetProcNum() - 1) * mpiBlockSize;
      if (val != NumPoints) {
        NumPoints = (GetProcNum() - 1) * mpiBlockSize;
      }
    }

    if (MapType == mpRotated)
    {
      for (int i = 0; i < NumOfTaskLevels; i++)
      {
        int L = MapInLevel[i];
        int PlaneCount = GLOBALIZER_MAX(Dimension * (Dimension - 1) / 2, 1);
        if ((L < 1) || (L > 2 * PlaneCount + 1))
        {
          if (GetProcRank() == 2)
          {
            printf("\n\nL=%d\n\n", L);
            printf("Error MapInLevel count!!!\n max PlaneCount = %d\n", PlaneCount);
            _ERROR_(ERROR_ARGUMENT_SIZE);
          }
        }
      }
    }

    MapCount = new int[NumOfTaskLevels];
    for (int i = 0; i < NumOfTaskLevels; i++)
      MapCount[i] = MapInLevel[i] / MapProcInLevel[i];

    if (MaxNumOfPoints.GetSize() < NumOfTaskLevels)
    {
      int a = MaxNumOfPoints.GetSize();
      MaxNumOfPoints.SetSize(NumOfTaskLevels);
      for (int t = a; t < NumOfTaskLevels; t++)
      {
        MaxNumOfPoints[t] = MaxNumOfPoints[0];
      }
    }

    //if (DimInTaskLevel.GetSize() < NumOfTaskLevels)
    //  _ERROR_(ERROR_ARGUMENT_SIZE);
    //if (ChildInProcLevel.GetSize() < (NumOfTaskLevels - 1))
    //  _ERROR_(ERROR_ARGUMENT_SIZE);
    //if (MapInLevel.GetSize() < NumOfTaskLevels)
    //  _ERROR_(ERROR_ARGUMENT_SIZE);
    //if (MapProcInLevel.GetSize() < NumOfTaskLevels)
    //  _ERROR_(ERROR_ARGUMENT_SIZE);
    //if (MaxNumOfPoints.GetSize() < NumOfTaskLevels)
    //  _ERROR_(ERROR_ARGUMENT_SIZE);



    sumDim = 0;
    for (int i = 0; i < NumOfTaskLevels; i++)
      sumDim += DimInTaskLevel[i];
    //if (sumDim != Dimension)
    //  _ERROR_(ERROR_DIM_IN_TASK_LEVEL);

    mIsInit = true;
  }
  return 0;
}

// ------------------------------------------------------------------------------------------------
/// Печать текущих значений параметров
void TParameters::PrintParameters()
{
#ifndef EXAMIN_LITE
#ifndef USE_OneAPI

  //Печать параметров командной строки
  printf("\nNeed MPI processes - %d\n", mNeedMPIProcessorCount);
#pragma omp parallel
  {
    if (omp_get_thread_num() == 0)
      printf("\nOMP Thread Num - %d\n", omp_get_num_threads());
  }
#endif
#endif
  if (!disablePrintParameters)
    TBaseParameters<TParameters>::PrintParameters();
  printf("\n\n");

}

// ------------------------------------------------------------------------------------------------
  ///Печать текущих значений параметров в файл
void TParameters::PrintParametersToFile(FILE* pf)
{
#ifndef EXAMIN_LITE
#ifndef USE_OneAPI

  //Печать параметров командной строки
  fprintf(pf, "\nNeed MPI processes - %d\n", mNeedMPIProcessorCount);
#pragma omp parallel
  {
    if (omp_get_thread_num() == 0)
      fprintf(pf, "\nOMP Thread Num - %d\n", omp_get_num_threads());
  }
#endif
#endif
  if (!disablePrintParameters)
  {
    for (int i = 0; i < mOptionsCount; i++)
    {
      if (mOptions[i]->mIsEdit)
        fprintf(pf, "%s\n", mOptions[i]->GetCurrentStringValue().c_str());
    }
    for (int i = 0; i < mOtherOptionsCount; i++)
    {
      if (mOtherOptions[i]->mIsEdit)
        fprintf(pf, "%s\n", mOtherOptions[i]->GetCurrentStringValue().c_str());
    }
  }
  fprintf(pf, "\n\n");

}

// ------------------------------------------------------------------------------------------------
/// Возвращает имя файла для сохранения картинки построенных линий уровней
std::string TParameters::GetPlotFileName()
{
  if (ConfigPath.ToString() == "")
  {
    return "globalizer_" + this->libPath.ToString() + ".png";
    //return "ExaMin.png";
  }
  else
  {
    std::string res = "";
    int i = ConfigPath.ToString().find('.');
    res = ConfigPath.ToString().substr(0, i);
    res += ".png";
    return res;
  }
}

// ------------------------------------------------------------------------------------------------
/// Инициализация параметров
void TParameters::Init(int argc, char* argv[], bool isMPIInit)
{
  //Определить номер текущего процесса и общее число процессов
  if (isMPIInit)
    DetermineProc();
  else
  {
    mProcRank = 0;
    mProcNum = 1;
  }

  TBaseParameters<TParameters>::Init(argc, argv, isMPIInit);

  deviceIndex = -1;

  //Печать справки
  if ((mIsPrintHelp) || (HELP))
    if (mProcRank == 0)
      PrintHelp();

  if (mProcRank == 0)

#ifdef CUDA_VALUE_DOUBLE_PRECISION
    printf("\nDOUBLE PRECISION\n");
#else
    printf("\nSINGLE PRECISION\n");
#endif //CUDA_VALUE_DOUBLE_PRECISION

  //printf("ProcRank=%d\n", mProcRank);
  if (IsSetDevice)
    SetDeviceIndex();

  parallel_tree.InitTree();
}

// ------------------------------------------------------------------------------------------------
TParameters::TParameters() : TBaseParameters<TParameters>::TBaseParameters()
{
  mOwner = this;
}

// ------------------------------------------------------------------------------------------------
TParameters::TParameters(TParameters& _parameters) : TBaseParameters<TParameters>::TBaseParameters(_parameters)
{
  mIsInit = false;
  mOwner = this;
  //deviceIndex = parameters.deviceIndex;
  mProcRank = _parameters.mProcRank;
  mProcNum = _parameters.mProcNum;

  //MapCount = parameters.MapCount;

  // Инициализация рабочих параметров
  SetDefaultParameters();

  for (int i = 0; i < mOptionsCount; i++)
  {
    *mOptions[i] = *_parameters.mOptions[i];
  }
  for (int i = mOptionsCount; i < _parameters.mOptionsCount; i++)
  {
    _parameters.mOptions[i]->Clone(&mOptions[i]);
  }
  mOptionsCount = _parameters.mOptionsCount;
  mIsInit = true;

  MapCount = new int[_parameters.NumOfTaskLevels];
  for (int i = 0; i < _parameters.NumOfTaskLevels; i++)
    MapCount[i] = _parameters.MapCount[i];

  MyLevel = _parameters.MyLevel;

  MyMap = _parameters.MyMap;
}

// ------------------------------------------------------------------------------------------------
TParameters::~TParameters()
{}

bool TParameters::IsProblem()
{
  return false;
}

// ------------------------------------------------------------------------------------------------
void TParameters::DetermineProc()
{
  if (MPI_Comm_size(MPI_COMM_WORLD, &mProcNum) != MPI_SUCCESS)
  {
    throw EXCEPTION("Error in MPI_Comm_size call");
  }
  if (MPI_Comm_rank(MPI_COMM_WORLD, &mProcRank) != MPI_SUCCESS)
  {
    throw EXCEPTION("Error in MPI_Comm_rank call");
  }
}

// ------------------------------------------------------------------------------------------------
void TParameters::SetDeviceIndex()
{
  //определение устройства для процесса
  //printf("ProcRank=%d\n", mProcRank);

  MPI_Status status;
  unsigned long size = 256;
  char* CompName = 0;



#ifdef WIN32
  LPWSTR buffer = new wchar_t[size];
  for (unsigned long i = 0; i < size; i++)
    buffer[i] = 0;
  GetComputerNameW(buffer, &size);
  CompName = new char[size + 1];
  for (unsigned long i = 0; i < size; i++)
    CompName[i] = (char)buffer[i];
  CompName[size] = 0;
  size++;
#else
  char* hostname = new char[size];
  for (unsigned long i = 0; i < size; i++)
    hostname[i] = 0;
  gethostname(hostname, 256);
  size = (unsigned long)strlen(hostname);
  CompName = new char[size + 1];
  for (unsigned long i = 0; i < size; i++)
    CompName[i] = (char)hostname[i];
  CompName[size] = 0;
  size++;
#endif

  printf("%s\tProcRank=%d\tProcNum=%d\n", CompName, mProcRank, mProcNum);
  int err = 0;
  //printf( "\n\n");
  if (mProcRank == 0)
  {
    //printf("pn=%d\n", GetProcNum());
    char** allCompName = new char*[GetProcNum() + 1];

    for (int i = 1; i < GetProcNum(); i++)
    {
      MPI_Recv(&size, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status);

      allCompName[i] = new char[size];
      //printf(" %d\t%lu\n", i, size);


      MPI_Recv(allCompName[i], size, MPI_CHAR, i, 0,
        MPI_COMM_WORLD, &status);

      //printf(" %d\t%s\n", i, allCompName[i]);
    }

    int* deviceIndex_ = new int[GetProcNum()];
    bool* isProcessed = new bool[GetProcNum()];

    for (int i = 0; i < GetProcNum(); i++)
    {
      deviceIndex_[i] = -1;
      isProcessed[i] = false;
    }

    std::string curComp = "";
    int curCID = 0;

    for (int i = 1; i < GetProcNum(); i++)
    {
      if (isProcessed[i])
        continue;
      curComp = allCompName[i];
      curCID = 0;
      deviceIndex_[i] = curCID;
      isProcessed[i] = true;
      for (int j = i + 1; j < GetProcNum(); j++)
      {
        if (isProcessed[j])
          continue;
        if (curComp == allCompName[j])
        {
          curCID++;
          deviceIndex_[j] = curCID;
          isProcessed[j] = true;
          //err = MPI_Send(&curCID, j, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
        }
      }
    }

    for (int i = 1; i < GetProcNum(); i++)
    {
      err = MPI_Send(&deviceIndex_[i], 1, MPI_INT, i, 0, MPI_COMM_WORLD);
    }

    delete[] isProcessed;
    delete[] deviceIndex_;
  }
  else
  {
    err = MPI_Send(&size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    err = MPI_Send(CompName, size, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
    int deviceIndex_ = -1;
    MPI_Recv(&deviceIndex_, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

    printf("%s\tProcRank = %d\tdeviceIndex = %d\n", CompName, GetProcRank(), deviceIndex_);

    deviceIndex = deviceIndex_; //is it an error?
  }
}

// ------------------------------------------------------------------------------------------------
/// Возвращает номер текущего процесса
int TParameters::GetProcRank() const
{
  return mProcRank;
}

// ------------------------------------------------------------------------------------------------
/// Возвращает общее число процессов
int TParameters::GetProcNum()
{
  return mProcNum;
}
// - end of file ----------------------------------------------------------------------------------

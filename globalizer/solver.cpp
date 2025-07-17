#include <cstring>

#include "solver.h"

#include "Pareto.h"

#include "method_factory.h"

#include "TaskFactory.h"

#include "TrialFactory.h"

#include "CalculationFactory.h"

#include "omp_calculation.h"
#include "cuda_calculation.h"

#include "separable_method.h"

#ifdef USE_PYTHON

#include "ml_wrapper.h"
#include <numpy/arrayobject.h>

#endif

// ------------------------------------------------------------------------------------------------
void TSolver::ClearData()
{
  //if (mProcess != 0)
  //  delete mProcess;
  if (pTask != 0)
  {

    delete pTask;
    pTask = 0;

    if (pData != 0)
      delete pData;

    pData = 0;

  }
  mProcess = 0;
}

#ifdef USE_PYTHON
PyObject* makeFloatList(const double* array, int size)
{
  PyObject* l = PyList_New(size);
  for (int i = 0; i != size; i++)
    PyList_SET_ITEM(l, i, PyFloat_FromDouble(array[i]));
  return l;
}
#endif

void TSolver::SeparationVariablesSolver()
{
//  int globalParam = parameters.Dimension;
//  int localParam = 0;
//  
//  TTask* tempTask = TTaskFactory::CreateTask(parameters.Dimension, parameters.DimInTaskLevel[0],
//    mProblem, 0);
//  TSearchData* tempData = new TSearchData(mProblem->GetNumberOfFunctions());
//  int qSize = GLOBALIZER_MAX((int)pow(2.0, (int)(log((double)parameters.MaxNumOfPoints[tempTask->GetProcLevel()])
//    / log(2.0) - 2)) - 1, 1023);
//  tempData->ResizeQueue(qSize);
//
//  // Инициализиуем числа с расширенной точностью
//  //InitAutoPrecision();
//
//  if (Extended::GetPrecision() == 0.01)
//  {
//    if (parameters.m * (tempTask->GetFreeN() - tempTask->GetNumberOfDiscreteVariable()) <= 50)
//    {
//      Extended::SetTypeID(etDouble);
//    }
//    Extended::SetPrecision(1 / (::pow(2., parameters.m * (tempTask->GetFreeN() -
//      tempTask->GetNumberOfDiscreteVariable()))));
//  }
//
//
//  TProcess* tempProcess = new TProcess(*tempData, *tempTask);
//
//  // local method Hyck-Jivs
//  int user_want_iter = parameters.MaxNumOfPoints[0];
//  parameters.MaxNumOfPoints[0] = 2;
//  tempProcess->Solve();
//
//  TTrial* startPoint = tempProcess->GetOptimEstimation();
//
//  int numPointsForSep = 10;
//  double* X_mass = new double[tempTask->GetN() * numPointsForSep];
//  double* y_train = new double[tempTask->GetN() * numPointsForSep];
//
//  for (int i = 0; i < tempTask->GetN(); i++) {
//    double leftBnd = tempTask->GetA()[i];
//    double rightBnd = tempTask->GetB()[i];
//    double* x0 = new double[tempTask->GetN()];
//    for (int k = 0; k < tempTask->GetN(); k++) {
//      x0[k] = startPoint->y[k];
//    }
//    double currPointt = leftBnd;
//    double step = (rightBnd - leftBnd) / (numPointsForSep - 1);
//    for (int k = 0; k < numPointsForSep; k++) {
//      x0[i] = currPointt + k * step;
//      X_mass[i * numPointsForSep + k] = x0[i];
//      y_train[i * numPointsForSep + k] = mProblem->CalculateFunctionals(x0, 0);
//    }
//  }

//#ifdef USE_PYTHON
//
//  PyObject* mPFunc;
//
//  //setenv("PYTHONHOME", "../../miniconda3/bin", true);
//  //setenv("PYTHONPATH", "../scripts", true); //Указываем где лежит скрипт
//  //setenv("PYTHONPATH", "scripts", true);
//
//  //std::cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAA1!!!!\n";
//  init_numpy(); //Инициализируем библиотеку numpy
//  //std::cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAA2!!!!\n";
//  auto pName = PyUnicode_FromString("make_separation"); // открываем скрипт
//  //std::cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAA3!!!!\n";
//  PyErr_Print();// печатаем ошибку если не получилось открыть скрипт
//  //std::cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAA4!!!!\n";  
//  auto pModule = PyImport_Import(pName); // импортируем функции из скрипта
//  //std::cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAA5!!!!\n";  
//  if (pModule == nullptr) // проверяем успешность импорта
//  {
//    PyErr_Print();
//    std::exit(1);
//  }
//  //std::cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAA6!!!!\n";
//  assert(pModule != nullptr);
//
//  //std::cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAA7!!!!\n";
//  auto pDict = PyModule_GetDict(pModule);
//
//  mPFunc = PyDict_GetItemString(pDict, "make_separation");//Достаем из скрипта указатель на указанную функцию
//  
//  assert(mPFunc != nullptr);
//  assert(PyCallable_Check(mPFunc));
//
//  auto py_arg_X = makeFloatList(X_mass, tempTask->GetN() * numPointsForSep);
//  auto py_arg_y = makeFloatList(y_train, tempTask->GetN() * numPointsForSep);
//  auto numPoints = PyLong_FromLong(numPointsForSep);
//  auto dim = PyLong_FromLong(parameters.Dimension);
//  auto porog = PyFloat_FromDouble(0.8);
//
//  /*double lb[MAX_TRIAL_DIMENSION], ub[MAX_TRIAL_DIMENSION];
//  mProblem->GetBounds(lb, ub);
//  auto py_lb = makeFloatList(lb, parameters.Dimension.GetData());
//  auto py_ub = makeFloatList(ub, parameters.Dimension.GetData());*/
//
//  auto arglist = PyTuple_Pack(5, py_arg_X, py_arg_y, numPoints, dim, porog);
//  /////
//
//  //auto* result = (PyArrayObject*)PyObject_CallObject(mPFunc, arglist);
//  //auto* result = PyObject_CallObject(mPFunc, arglist); /// Вызываем python функцию - отправляем точку, получаем массив
//  //double retval = PyFloat_AsDouble(result);
//
//
//  PyArrayObject* result = (PyArrayObject*)PyObject_CallObject(mPFunc, arglist);
//  npy_intp vsize = parameters.Dimension + 1;
//  std::vector<int> out(vsize);
//  std::memcpy(out.data(), PyArray_DATA(result), sizeof(int) * vsize);
//
//  ///// Конвертируем полученный массив в c-шный вектор
//  //npy_intp vsize = PyArray_SIZE(result);
//  //std::vector<double> out(vsize);
//  //std::memcpy(out.data(), PyArray_DATA(result), sizeof(double) * vsize);
//  /////
//  /// все закрываме
//  //Py_DECREF(py_arg);
//  Py_DECREF(dim);
//  //Py_DECREF(numPoint);
//  Py_DECREF(result);
//  Py_DECREF(arglist);
//  Py_DECREF(pDict);
//  Py_DECREF(pModule);
//  Py_DECREF(pName);
//
//  globalParam = (int)out[0]; // количество глобальных переменных
//  localParam = parameters.Dimension - out[0]; // количество локальных переменных
//
//
//
//  std::vector<int> permutations;
//
//  for (int i = 1; i < parameters.Dimension + 1; i++) {
//    permutations.push_back(out[i]);
//  }
//
//  std::cout << "Global: " << globalParam << "\nLocal: " << localParam << std::endl;
//
//  std::cout << "\nPermutations vector: ";
//  for (auto const& element : permutations) {
//    std::cout << element + 1 << " ";
//  }
//
//
//  if (localParam != 0)/// Если локальные переменные определились, то формируем запуск многошаговой схемы
//  {
//    parameters.NumOfTaskLevels = 2; //два уровня в дереве
//
//    parameters.TypeCalculation = BlockScheme;//определяем что у наспоследовательная блочная многошаговая схема
//    parameters.DimInTaskLevel[0] = globalParam; // на пером уровне глобальные переменные
//    parameters.DimInTaskLevel[1] = localParam; // на втором локальные
//
//    parameters.ChildInProcLevel[0] = 1; //один потомок выходит из корня он же лист
//  }
//
//  /// Решаем
//
//  parameters.MaxNumOfPoints[0] = 20000;
//  TTaskFactory::permutations = permutations;
//
//  ClearData();
//  CreateProcess();
//  mProcess->Solve();
//#endif
}

// ------------------------------------------------------------------------------------------------
TSolver::TSolver(IProblem* problem)
{
  mProblem = problem;

  mProcess = 0;

  pTask = 0;
  pData = 0;

  result = 0;
}

// ------------------------------------------------------------------------------------------------
int TSolver::CheckParameters()
{
  double optimumValue;
  if (mProblem->GetOptimumValue(optimumValue) == IProblem::UNDEFINED &&
    parameters.stopCondition == OptimumValue)
  {
    print << "Stop by reaching optimum value is unsupported by this problem\n";
    return 1;
  }

  double optimumPoint[MaxDim * MaxNumOfGlobMinima];
  int n;
  if (mProblem->GetOptimumPoint(optimumPoint) == IProblem::UNDEFINED &&
    mProblem->GetAllOptimumPoint(optimumPoint, n) == IProblem::UNDEFINED &&
    parameters.stopCondition == OptimumVicinity)
  {
    print << "Stop by reaching optimum vicinity is unsupported by this problem\n";
    return 1;
  }

  return 0;
}

// ------------------------------------------------------------------------------------------------
void TSolver::BlockSchemeCalculation()
{
  int isFinish = 0;
  int parent = parameters.parallel_tree.ProcParent;
  int procLevel = parameters.MyLevel;
  bool isLeaf = procLevel == (parameters.Dimension - 1);
  while (isFinish == 0)
  {
    MPI_Status status;
    MPI_Recv(&isFinish, 1, MPI_INT, parent, TagChildSolved, MPI_COMM_WORLD, &status);
    if (isFinish) {
      if (!isLeaf) {
        int childNum = parameters.ChildInProcLevel[procLevel];
        int curr_child = 0;
        for (unsigned int i = 0; i < childNum; i++) {
          curr_child = parameters.parallel_tree.ProcChild[i];
          int finish = 1;
          MPI_Send(&finish, 1, MPI_INT, curr_child, TagChildSolved, MPI_COMM_WORLD);
        }
      }
      break;
    }
    parameters.TypeCalculation = isLeaf ? OMP : ParallelBlockScheme;
    TTask* task = TTaskFactory::CreateTask();
    TTrial* trail = TTrialFactory::CreateTrial();

    int fixedN = 0;
    for (int i = 0; i < procLevel; i++)
       fixedN += parameters.DimInTaskLevel[i];

    int freeN = parameters.Dimension - fixedN;

    //print << "  Proc rank = " << parameters.GetProcRank() << "  newProcLevel = " << 
    //  newProcLevel<< "\tfreeN = "<< freeN << "\tfixedN = " << fixedN << "\t  y = ";
    MPI_Recv(trail->y, parameters.Dimension, MPI_DOUBLE, parent, TagChildSolved, MPI_COMM_WORLD, &status);
    //for (int q = 0; q < parameters.Dimension; q++)
    //{
    //  print << trail->y[q] << " ";
    //}
    //print << "\n";

    //print << "AAA-1!!!!\t" << task << "\n";
    task->Init(parameters.Dimension, freeN, mProblem, procLevel);

    //print << "AAA0!!!!\n";
    task->SetFixed(fixedN, trail->y);
    Solve(task);

    TTrial* trailResult = GetSolutionResult()->BestTrial;

    memcpy(trail->FuncValues, trailResult->FuncValues,
      MaxNumOfFunc * sizeof(double));
    memcpy(trail->y, trailResult->y,
      parameters.Dimension * sizeof(double));

    trail->K = GetSolutionResult()->TrialCount;

    MPI_Send(trail->y, parameters.Dimension, MPI_DOUBLE, parent, TagChildSolved, MPI_COMM_WORLD);
    MPI_Send(trail->FuncValues, MaxNumOfFunc, MPI_DOUBLE, parent, TagChildSolved, MPI_COMM_WORLD);
    MPI_Send(&(trail->K), 1, MPI_INT, parent, TagChildSolved, MPI_COMM_WORLD);
  }
}

// ------------------------------------------------------------------------------------------------
void TSolver::MpiCalculation()
{
  int isFinish = 0;
  while (isFinish == 0)
  {
    MPI_Status status;
    //Принимаем данные из mpi_calculation
    //Проверяем, что еще работаем
    MPI_Recv(&isFinish, 1, MPI_INT, 0, TagChildSolved, MPI_COMM_WORLD, &status);
    if (isFinish == 1)
      break;

    /// Входные данные для вычислителя, формирубтся в CalculateFunctionals()
    TInformationForCalculation inputSet;
    /// Выходные данные вычислителя, обрабатывается в CalculateFunctionals()
    TResultForCalculation outputSet;

    TTrial* trail = TTrialFactory::CreateTrial();

    inputSet.Resize(parameters.mpiBlockSize);
    outputSet.Resize(parameters.mpiBlockSize);

    for (unsigned int j = 0; j < parameters.mpiBlockSize; j++) {
      inputSet.trials[j] = TTrialFactory::CreateTrial();
      //Получаем координаты точки
      MPI_Recv(trail->y, parameters.Dimension, MPI_DOUBLE, 0, TagChildSolved, MPI_COMM_WORLD, &status);
      trail->index = -1;

      for (int k = 0; k < parameters.Dimension; k++)
        inputSet.trials[j]->y[k] = trail->y[k];

      if (inputSet.tasks[j] == 0)
        inputSet.tasks[j] = TTaskFactory::CreateTask();
    }

    IProblem* _problem = mProblem;
    TTask* _pTask = TTaskFactory::CreateTask(parameters.Dimension, parameters.DimInTaskLevel[0],
      _problem, 0);;
    TCalculation* calculation;
    if (parameters.calculationsArray[1] == OMP) {
      calculation = new TOMPCalculation(*_pTask);
    }
    else if (parameters.calculationsArray[1] == CUDA) {
      calculation = new TCUDACalculation(*_pTask);
    }

    calculation->Calculate(inputSet, outputSet);

    for (unsigned int j = 0; j < parameters.mpiBlockSize; j++) {
      //Отправляем обратно значение функции
      MPI_Send(inputSet.trials[j]->FuncValues, MaxNumOfFunc, MPI_DOUBLE, 0, TagChildSolved, MPI_COMM_WORLD);
    }
  }
}

// ------------------------------------------------------------------------------------------------
void TSolver::AsyncCalculation()
{
  int isFinish = 0;
  while (isFinish == 0)
  {
    MPI_Status status;
    //Принимаем данные из mpi_calculation
    //Проверяем, что еще работаем
    MPI_Recv(&isFinish, 1, MPI_INT, 0, TagChildSolved, MPI_COMM_WORLD, &status);
    if (isFinish == 1)
      break;

    TTrial* trail = TTrialFactory::CreateTrial();

    //Получаем координаты точки
    MPI_Recv(trail->y, parameters.Dimension, MPI_DOUBLE, 0, TagChildSolved, MPI_COMM_WORLD, &status);
    trail->index = -1;

    int fNumber = 0;

    IProblem* _problem = mProblem;
    TTask* _pTask = TTaskFactory::CreateTask(parameters.Dimension, parameters.DimInTaskLevel[0],
      _problem, 0);

    if (parameters.DebugAsyncCalculation != 0) {
      while (true) {
#ifdef WIN32
        Sleep(5);
#endif
        std::ifstream fin("../_build/async.txt");
        int i;
        fin >> i;
        fin >> i;
        if (i == parameters.GetProcRank() || i == parameters.GetProcNum()) {
          fin.close();
          break;
        }
      }
    }

    //Вычисляем значение функции
    while ((trail->index == -1) && (fNumber < _pTask->GetNumOfFunc()))
    {
      trail->FuncValues[fNumber] = _pTask->CalculateFuncs(trail->y, fNumber);

#ifdef WIN32
      if (!_finite(trail->FuncValues[fNumber]))
#else
      if (!std::isfinite(trail->FuncValues[fNumber]))
#endif
      {
        //throw EXCEPTION("Infinite trail->FuncValues[fNumber]!");
        trail->index = -2;
        std::cout << " CalculateFuncs Error!!!\n";
      }
      else
        if ((fNumber == (_pTask->GetNumOfFunc() - 1)) || (trail->FuncValues[fNumber] > 0))
        {
          trail->index = fNumber;
        }
      fNumber++;
    }

    //Отправляем индекс точки
    MPI_Send(&(trail->index), 1, MPI_INT, 0, TagChildSolved, MPI_COMM_WORLD);
    //Отправляем точку
    MPI_Send(trail->y, parameters.Dimension, MPI_DOUBLE, 0, TagChildSolved, MPI_COMM_WORLD);
    //Отправляем обратно значение функции
    MPI_Send(trail->FuncValues, MaxNumOfFunc, MPI_DOUBLE, 0, TagChildSolved, MPI_COMM_WORLD);
  }
}


// ------------------------------------------------------------------------------------------------
int TSolver::Solve()
{
  try
  {
    if (CheckParameters())
      return 1;

    if (parameters.TypeSolver == SeparationVariables) /// Если автоматически разделяем переменные
    {     
      SeparationVariablesSolver();
    }
    else /// во всех остальных случаях
    {
      if ((parameters.calculationsArray[0] == ParallelBlockScheme) && (parameters.NumOfTaskLevels > 1) && (parameters.GetProcNum() > 1) && (parameters.GetProcRank() > 0))
      {
        BlockSchemeCalculation();
      }
      else if ((parameters.calculationsArray[0] == MPICalculation) && (parameters.GetProcNum() > 1) && (parameters.GetProcRank() > 0))
      {
        MpiCalculation();
      }
      else if ((parameters.TypeCalculation == AsyncMPI) && (parameters.GetProcNum() > 1) && (parameters.GetProcRank() > 0))
      {
        AsyncCalculation();
      }
      else
      {
        ClearData();
        CreateProcess();
        mProcess->Solve();
      }
    }

  }
  catch (const TException& e)
  {
    std::string excFileName = std::string("exception_") +
      toString(parameters.GetProcRank()) + ".txt";
    e.Print(excFileName.c_str());

    for (int i = 0; i < parameters.GetProcNum(); i++)
      if (i != parameters.GetProcRank())
        MPI_Abort(MPI_COMM_WORLD, i);
    return 1;
  }
  catch (...)
  {
    print << "\nUNKNOWN EXCEPTION !!!\n";
    std::string excFileName = std::string("exception_") +
      toString(parameters.GetProcRank()) + ".txt";
    TException e("UNKNOWN FILE", -1, "UNKNOWN FUCNTION", "UNKNOWN EXCEPTION");
    e.Print(excFileName.c_str());

    for (int i = 0; i < parameters.GetProcNum(); i++)
      if (i != parameters.GetProcRank())
        MPI_Abort(MPI_COMM_WORLD, i);
    return 1;
  }
  if (parameters.GetProcRank() == 0)
  {
    if (parameters.GetProcNum() > 1) {
      int childNum = parameters.ChildInProcLevel[parameters.MyLevel];
      int curr_child = 0;
      for (unsigned int i = 0; i < childNum; i++) {
        curr_child = parameters.parallel_tree.ProcChild[i];
        int finish = 1;
        MPI_Send(&finish, 1, MPI_INT, curr_child, TagChildSolved, MPI_COMM_WORLD);
      }
    }
  }

  return 0;
}

// ------------------------------------------------------------------------------------------------
int TSolver::Solve(TTask* task)
{
  try
  {
    pTask = task;

    CreateProcess();

    mProcess->Solve();
  }
  catch (const TException& e)
  {
    std::string excFileName = std::string("exception_") +
      toString(parameters.GetProcRank()) + ".txt";
    e.Print(excFileName.c_str());

    for (int i = 0; i < parameters.GetProcNum(); i++)
      if (i != parameters.GetProcRank())
        MPI_Abort(MPI_COMM_WORLD, i);
    return 1;
  }
  catch (...)
  {
    print << "\nUNKNOWN EXCEPTION !!!\n";
    std::string excFileName = std::string("exception_") +
      toString(parameters.GetProcRank()) + ".txt";
    TException e("UNKNOWN FILE", -1, "UNKNOWN FUCNTION", "UNKNOWN EXCEPTION");
    e.Print(excFileName.c_str());

    for (int i = 0; i < parameters.GetProcNum(); i++)
      if (i != parameters.GetProcRank())
        MPI_Abort(MPI_COMM_WORLD, i);
    return 1;
  }
  return 0;
}

// ------------------------------------------------------------------------------------------------
TSolver::~TSolver()
{
  ClearData();
}

// ----------------------------------------------------------------------------
void TSolver::InitAutoPrecision()
{
  // if user has not set the precision by the command line,
  // then set it to 1 / (2^((m + 1) * N) - 1)
  if (Extended::GetPrecision() == 0.01)
  {
    if (parameters.m * (pTask->GetFreeN() - pTask->GetNumberOfDiscreteVariable()) <= 50)
    {
      Extended::SetTypeID(etDouble);
    }
    Extended::SetPrecision(1 / (::pow(2., parameters.m * (pTask->GetFreeN() -
      pTask->GetNumberOfDiscreteVariable()))));
  }
}

// ------------------------------------------------------------------------------------------------
int TSolver::CreateProcess()
{
  // В случае если не совпадают(задача пришла снаружи берем новые) иначе все равно
  IProblem* _problem = mProblem;

  /// Создание задачи(TTask) // перенести в фабрику
  if (pTask == 0)
  {
    pTask = TTaskFactory::CreateTask(parameters.Dimension, parameters.DimInTaskLevel[0],
      _problem, 0);
  }
  /// Создаем данные для поисковой информации
  if ((parameters.TypeMethod == AdaptivMethod) ||
    (parameters.TypeMethod == MultievolventsMethod) ||
    (parameters.TypeMethod == ParallelMultievolventsMethod))
  {
    TAdaptiveTask* task = static_cast<TAdaptiveTask*>(pTask);
    if (task->data != 0)
      pData = task->data;
    else
    {
      pData = new TSearchData(_problem->GetNumberOfFunctions());
      task->data = pData;
      int qSize = GLOBALIZER_MAX((int)pow(2.0, (int)(log((double)parameters.MaxNumOfPoints[pTask->GetProcLevel()])
        / log(2.0) - 2)) - 1, 1023);
      pData->ResizeQueue(qSize);
    }
  }
  else
  {
    //delete pData;
    //pData = new TSearchData(_problem->GetNumberOfFunctions());
    //int qSize = max((int)pow(2.0, (int)(log((double)parameters.MaxNumOfPoints[pTask->GetProcLevel()])
    //  / log(2.0) - 2)) - 1, 1023);
    //pData->ResizeQueue(qSize);

    if (pData == 0)
    {
      pData = new TSearchData(_problem->GetNumberOfFunctions());
      int qSize = GLOBALIZER_MAX((int)pow(2.0, (int)(log((double)parameters.MaxNumOfPoints[pTask->GetProcLevel()])
        / log(2.0) - 2)) - 1, 1023);
      pData->ResizeQueue(qSize);
    }
    else
    {
      pData->Clear();
    }
  }
  // Инициализиуем числа с расширенной точностью
  InitAutoPrecision();

  if (mProcess != 0)
    //delete mProcess;
    mProcess->Reset(pData, pTask);
  else
    mProcess = new TProcess(*pData, *pTask);

  return 0;
}

// ------------------------------------------------------------------------------------------------
void TSolver::SetProblem(IProblem* problem)
{
  mProblem = problem;
}

// ------------------------------------------------------------------------------------------------
IProblem* TSolver::GetProblem()
{
  return mProblem;
}

// ------------------------------------------------------------------------------------------------
TSolutionResult* TSolver::GetSolutionResult()  /// best point
{
  //if (pTask->GetProcLevel() == 0)  printf("DD20_21! Proc=%d \n", parameters.GetProcRank());
  if (result != 0)
    delete result;
  //if (pTask->GetProcLevel() == 0)  printf("DD20_22! Proc=%d \n", parameters.GetProcRank());
  result = new TSolutionResult();
  //if (pTask->GetProcLevel() == 0)  printf("DD20_23! Proc=%d \n", parameters.GetProcRank());
  result->BestTrial = mProcess->GetOptimEstimation();
  //result->BestTrial->FuncValues = mProcess-> 
  //if (pTask->GetProcLevel() == 0)  printf("DD20_24! Proc=%d \n", parameters.GetProcRank());
  result->IterationCount = mProcess->GetIterationCount();
  //if (pTask->GetProcLevel() == 0)  printf("DD20_25! Proc=%d \n", parameters.GetProcRank());
  result->TrialCount = mProcess->GetNumberOfTrials();
  //if (pTask->GetProcLevel() == 0)  printf("DD20_26! Proc=%d \n", parameters.GetProcRank());
  return result;
}

// ------------------------------------------------------------------------------------------------
int TSeparableSolver::Solve()
{
  //Шаг 2
  // класс с поиском по определенной координате (сепарабельный поиск):separable_method.h и class TSeparableMethod
  //void TSeparableMethod::StartGlobalSearch(int varIndex)
  ClearData();
  TSolver::CreateProcess();

  // запуск сепарабельного поиска - инициализация и запуск работы сепарабельного поиска
  TSeparableMethod SeparableMethod(pTask);
  TTrial SepTrial = SeparableMethod.StartOptimization();

  /*
  В основном методе TSeparableMethod::StartOptimization() происходит выбор из возможных значения перечисления
  GridSearch = 1 и GlobalMethod = 2 (он нам и нужен и указан выше).
  */

  // формируем множества локальных и глобальных переменных
  std::vector<int> glob_indexes;
  std::vector<int> local_indexes;

  // выявление типа переменных
  for (int i = 0; i < pTask->GetN(); i++) {
    // отсортированный набор точек, формируемых при поиске y_i
    std::set<Trial> currentSearchInformation = SeparableMethod.GetSearchInformation(i);
    // проверка функции y_i(x) на унимодальность
    bool monoton = true;
    bool unimod = true;
    std::set<Trial>::iterator pointIt = currentSearchInformation.begin();
    auto nextPointIt = currentSearchInformation.begin();
    ++nextPointIt;
    while (nextPointIt != currentSearchInformation.end()) {
      double curr_val = (*pointIt).GetZ();
      double next_val = (*nextPointIt).GetZ();
      if ((next_val > curr_val) && (monoton == true)) {
        monoton = false;
        continue;
      }
      else if ((next_val < curr_val) && (monoton == false)) {
        // функция не унимодальная, переменная считается глобальной
        glob_indexes.push_back(i);
        unimod = false;
        // monoton = true;
        break;
      }
      ++pointIt;
      ++nextPointIt;
    }

    // функция либо монотонна, либо унимодальная, переменная считается локальной
    if (monoton == true || unimod == true) local_indexes.push_back(i);

  }
  /*
  запуск сепарабельного поиска - инициализация и запуск работы сепарабельного поиска
  if (parameters.sepS)
  pMethod->SeparableSearch();

  void TMethod::SeparableSearch()
  {
  TSeparableMethod SeparableMethod(parameters, pTask);
  TTrial SepTrial = SeparableMethod.StartOptimization();

  UpdateOptimumEstimation(SepTrial);

  std::vector<TTrial> localPoints = SeparableMethod.GetSearchSequence();
  InsertPoints(localPoints);
  //После сепарабельного поиска - поднять флаг пересчета
  recalc = true;
  }
  */

  // инициализация параметров задачи (parameters), с добавлением обнаруженной информации

  int DimensionGlobalParameters = glob_indexes.size();                        // вычисленное число глобальных переменных
  int DimensionLocalParameters = local_indexes.size();                        // вычисленное число локальных переменных

  std::cout << "Dimension Global Parameters: " << DimensionGlobalParameters;
  std::cout << "\nDimension Local Parameters: " << DimensionLocalParameters;

  std::vector<int> permutations = glob_indexes;
  permutations.insert(permutations.end(), local_indexes.begin(), local_indexes.end());

  std::cout << "\nGlobal vars: ";
  for (auto const& element : glob_indexes) {
    std::cout << element << " ";
  }

  std::cout << "\nLocal vars: ";
  for (auto const& element : local_indexes) {
    std::cout << element << " ";
  }

  std::cout << "\nPermutations vector: ";
  for (auto const& element : permutations) {
    std::cout << element << " ";
  }

  //ClearData();

  parameters.DimInTaskLevel[0] = DimensionGlobalParameters;                   // Размерность по верхнему уровню
  parameters.DimInTaskLevel[1] = DimensionLocalParameters;                    // Размерность по нижнему уровню

  parameters.TypeSolver = SeparableSearch;

  TTaskFactory::permutations = permutations;
  //Шаг 1
  ClearData();
  TSolver::CreateProcess();
  //printf("AA-1! %d\n", parameters.GetProcRank());
  mProcess->Solve();

  // Если зацикливать вс, то тут
  // TTrial* tr = this->GetSolutionResult()->BestTrial;

  return 0;
}

// ------------------------------------------------------------------------------------------------
int TSeparableSolver::CreateProcess(TProcess **
  process, IProblem * problem)
{
  return 0;
}

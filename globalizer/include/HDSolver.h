#pragma once

#include "Solver.h"
#include "HDTask.h"
#include "SearchDataSerializer.h"

/**
 Базовый класс для решателя задач большой размерности
**/
class HDSolver : public ISolver
{
protected:

  /// Решатели для оптимизации по группам параметров
  std::vector< Solver*> solvers;
  /// Задачи для оптимизации по группам параметров
  std::vector <HDTask*> tasks;

  /// Решатель для объединения всех остальных Решателей
  Solver* finalSolver;

  /// Размерности групп параметров,
  /// по умолчанию по 1,
  /// дискретные включены в последнюю группу
  std::vector<int> dimensions;

  /// Решение задачи оптимизации
  SolutionResult* solutionResult;

  /// Размерность исходной задачи
  int originalDimension;
  /// Количество дискретных параметров исходной задачи
  int originalDiscreteParamsNum;

  /// Общее описание задачи
  Task* pTask;
  /// Задача оптимизации
  IProblem* problem;
  /// База данных (поисковая информация)
  SearchData* pData;

  /// Входной файл с поисковой информацией
  SearchDataSerializer::LoadedFileData fd;

  /// Количество итераций без улучшений решения
  int countIterationsWithoutImprovement = 0;

  /// Альтернативная стартовая точка на случай, если не удалось улучшить решение
  std::vector<double> alternativeStartingPoint;

  /// Задает размерности по группам параметров
  void SetDimentions(std::vector<int> _dimentions);

  /// Создает начальную точку решения задач
  void CreateStartPoint();

  /// Заполняет основные поля класса
  void Construct();

  /// Добавляет вычисленные точки в общий массив с точками
  void AddPoint(Solver* solver, int i, std::vector<Trial*>& points, int startParameterNumber);

  /// Обновляет стартовую точку
  void UpdateStartPoint(SolutionResult* solution, double& bestValue, int curDimensions,
    int startParameterNumber, std::vector<Trial*>& points, HDTask* curTask);

  /// Загружает точки испытаний из файла
  void LoadPoint();

  /// Подготавливает данные (дерево и очередь интервалов) для поисковой информации
  void CreateData();

public:
  HDSolver(IProblem* problem, int discreteParamsNum = 0, std::vector<int> _dimentions = {});
#ifdef _GLOBALIZER_BENCHMARKS
  HDSolver(IGlobalOptimizationProblem* problem, std::vector<int> _dimentions = {});
#endif

  virtual ~HDSolver();

  /// Решение задачи по умолчанию
  virtual int Solve();

  /// Возвращает полученное решение
  SolutionResult* GetSolutionResult();

  /// Добавляет точки испытаний
  virtual void SetPoint(std::vector<Trial*>& points);

  /// Возвращает все имеющиеся точки испытаний
  virtual std::vector<Trial*>& GetAllPoint();

  /// Выполняет автоматическую настройку параметров решателя
  virtual void AutoConfig();
};

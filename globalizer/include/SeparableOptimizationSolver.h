﻿#pragma once

#include "Solver.h"
#include "SeparableOptimizationTask.h"


/**
 Базовый класс для сепарабельной оптимизации
**/
class SeparableOptimizationSolver : public ISolver
{
protected:

  /// Решатели для оптимизации по группам параметров
  std::vector< Solver*> solvers;
  /// Решатель для объединения всех остальных Решателей
  Solver* finalSolver;
  /// Размерости групп парамеров, по умолчанию по 1
  std::vector<int> dimensions;
  /// Решение задачи оптимизации
  SolutionResult* solutionResult;
  /// Размерность исходной задачи
  int originalDimension;
  /// Задачи для оптимизации по группам параметров
  std::vector <SeparableOptimizationTask*> tasks;

  /// задача оптимизации
  IProblem* problem;

  /// Задачть размерности
  void SetDimentions(std::vector<int> _dimentions);

  /// Создать начальную точку решения задач
  void CreateStartPoint();

  /// заполняет основные поля класса
  void Construct();

public:
  SeparableOptimizationSolver(IProblem* problem, std::vector<int> _dimentions = {});

#ifdef _GLOBALIZER_BENCHMARKS

  SeparableOptimizationSolver(IGlobalOptimizationProblem* problem, std::vector<int> _dimentions = {});

#endif

  virtual ~SeparableOptimizationSolver();

  /// Решение задачи по умолчанию
  virtual int Solve();



  SolutionResult* GetSolutionResult();

  /// Добавляет точки испытаний
  virtual void SetPoint(std::vector<Trial*>& points);
  /// Возврящает все имеющиеся точки испытаний
  virtual std::vector<Trial*>& GetAllPoint();
};
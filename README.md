<p align="center">
  <img src="/docs/Globalizer_logo.png" width="200" height="150"/>
</p>

[![License: BSD 3-Clause](https://img.shields.io/badge/License-BSD%203--Clause-green)](LICENSE)
[![python: 3.11](https://img.shields.io/badge/python-3.11-44cc12?style=flat-square&logo=python)](https://www.python.org/downloads/release/python-3110/)
[![C++](https://img.shields.io/badge/C++-17-44cc12?style=flat-square&logo=c%2B%2B)](https://isocpp.org/)
[![docs: ](https://readthedocs.org/projects/ebonite/badge/?style=flat-square)](https://globalizer-documentation.readthedocs.io/en/latest/)
[![build:](https://github.com/UNN-ITMM-Software/iOpt/actions/workflows/python-app.yml/badge.svg)](https://github.com/OptimLLab/Globalizer/actions)



# Globalizer — программная система для глобальной оптимизации параметров моделей искусственного интеллекта, а также других сложных систем

## Краткое описание
Программная система `Globalizer` предназначена для решения трудоемких задач многомерной многоэкстремальной (глобальной) оптимизации.

Методы глобальной оптимизации, реализованные в составе программной системы Globalizer, предназначены для выбора значений параметров математических моделей сложных объектов и процессов. Система позволяет проводить точную настройку параметров моделей и методов из различных прикладных областей. Характерными примерами таких задач являются задачи настройки гиперпараметров методов искусственного интеллекта и машинного обучения.

В Globalizer поддерживаются следующие технологии параллельных вычислений: `OpenMP`, `MPI`, `CUDA`.

# **Основные возможности**
- Автоматический выбор значений параметров математических моделей и методов искусственного интеллекта и машинного обучения.
- Интеллектуальное управление процессом выбора оптимальных гиперпараметров.
- Интеграция с внешними фреймворками ИИ и МО, а также предметно-ориентированными библиотеками.
- Визуализация процесса выбора оптимальных параметров.


# **Установка и настройка**

## Сборка Globalizer со встроенными задачами:

```
git clone https://github.com/OptimLLab/Globalizer.git
cd Globalizer/
mkdir build
cd build
cmake ..
cmake --build ../build --config Release
```

# **Начать работать**

Использование фреймворка Globalizer для минимизации функции Стронгина с тремя ограничениями.

```\examples\SimpleMain.cpp

#include "Globalizer.h"

double StronginC3Functionals(const double* y, int fNumber)
{
  double res = 0.0;
  double x1 = y[0], x2 = y[1];
  switch (fNumber)
  {
  case 0: // ограничение 1
    res = 0.01 * ((x1 - 2.2) * (x1 - 2.2) + (x2 - 1.2) * (x2 - 1.2) - 2.25);
    break;
  case 1: // ограничение 2
    res = 100.0 * (1.0 - ((x1 - 2.0) / 1.2) * ((x1 - 2.0) / 1.2) -
      (x2 / 2.0) * (x2 / 2.0));
    break;
  case 2: // ограничение 3
    res = 10.0 * (x2 - 1.5 - 1.5 * sin(6.283 * (x1 - 1.75)));
    break;
  case 3: // критерий
  {
    double t1 = pow(0.5 * x1 - 0.5, 4.0);
    double t2 = pow(x2 - 1.0, 4.0);
    res = 1.5 * x1 * x1 * exp(1.0 - x1 * x1 - 20.25 * (x1 - x2) * (x1 - x2));
    res = res + t1 * t2 * exp(2.0 - t1 - t2);
    res = -res;
  }
  break;
  }

  return res;
}

// ------------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
  GlobalizerInitialization(argc, argv);

  parameters.Dimension = 2;
  IProblem* problem = new ProblemFromFunctionPointers(parameters.Dimension, // размерность задачи
    { 0.0, -1.0 }, // нижняя граница
    { 4.0, 3.0 }, // верхняя граница
    StronginC3Functionals, // задача
    4 // количество функций (3 ограничения + 1 критерий)
  );

  problem->Initialize();

  // Решатель
  Solver solver(problem);

  // Решаем задачу
  if (solver.Solve() != SYSTEM_OK)
    throw EXCEPTION("Error: solver.Solve crash!!!");

  delete problem;

  return 0;
}
```

## Python: интерфейс, совместимый с scipy.optimize

Пакет `globalizer.optimize` повторяет функции глобальной оптимизации `scipy.optimize`
(`differential_evolution`, `dual_annealing`, `shgo`, `direct`, `basinhopping`, `minimize`).
Чтобы перевести код со scipy на Globalizer, достаточно заменить имя библиотеки:

```python
from globalizer.optimize import differential_evolution   # вместо scipy.optimize

res = differential_evolution(func, bounds, constraints=..., integrality=...)
print(res.x, res.fun)
```

Подробнее: [PYGlobalizer/globalizer/README.md](PYGlobalizer/globalizer/README.md).

## Python: пошаговое решение

`PYDGlobalizer.IterativeSolver` — аналог примера [IterationMain.cpp](examples/IterationMain.cpp):
итерации выполняются по одной, между ними можно менять данные задачи, смотреть текущий
результат и решать, продолжать ли поиск.

```python
import PYDGlobalizer
from PYProblem import PYProblem

params = PYDGlobalizer.SolverParameters()
params.stop_condition = "iterations"   # "accuracy" (по умолчанию) или "iterations"
params.max_iterations = 300

solver = PYDGlobalizer.IterativeSolver(problem, params)   # Solver + Initialize()
while not solver.do_iteration():                          # DoIteration(finished)
    ...                                                   # данные задачи можно менять
print(solver.result())          # best_value, best_point, iterations, trials, feasible, finished

for state in PYDGlobalizer.IterativeSolver(problem, params):   # одна итерация на шаг
    print(state["iterations"], state["best_value"])
```

Также есть `run(max_steps=-1)` (несколько итераций подряд), `print_result()` и свойства
`finished`, `iterations`, `is_active`. Активным может быть только один решатель: `solve()`
или новый `IterativeSolver` заново инициализируют глобальные параметры Globalizer, после
чего незавершённый решатель выдаёт `RuntimeError`. Итог завершённого остаётся доступен.
Пример: [examples/example_iterative.py](examples/example_iterative.py).

# **Примеры**
Список примеров для подбора параметров архитектуры и обучения нейросетевых моделей:
- [Классификация сигналов ЭКГ по типу ритма](docs/examples/ECGClassification.md)
- [Сегментация элементов кардиоцикла в односекундном участке ЭКГ](docs/examples/ECGSegmentation.md)
- [Классификация сигналов ЭКГ по наличию\отсутствию кардиостимулятора](docs/examples/ECGPacemaker.md)
- [Классификация сигналов ЭЭГ по наличию\отсутствию эпилептиформной активности](docs/examples/EEGClassification.md)
- [Сегментация изображений, записанных с дрона, для распознования объектов на земле](docs/examples/AirObjectDetection.md)

# **Структура проекта**

Последняя стабильная версия Globalizer находится в [ветке main](https://github.com/OptimLLab/Globalizer/tree/main). 

Репозиторий включает в себя следующие директории::
- Пакет [globalizer](https://github.com/OptimLLab/Globalizer/tree/main/globalizer) содержит ядро фреймворка  в виде  классов на языке C++.
- Пакет [examples](https://github.com/OptimLLab/Globalizer/tree/main/examples) содержит примеры применения фреймворка для модельных задач.
- Модульные тесты размещены в каталоге [test](https://github.com/OptimLLab/Globalizer/tree/main/tests).
- Исходные файлы документации находятся в каталоге [docs](https://github.com/OptimLLab/Globalizer/tree/main/docs).

# **Документация**

Подробное описание API фреймворка Globalizer доступно в разделе [Read the Docs](https://globalizer-documentation.readthedocs.io/en/latest/).

# **Поддержка**

Исследование проводится при поддержке : Работа выполнена при поддержке программы стратегического академического лидерства Приоритет 2030.

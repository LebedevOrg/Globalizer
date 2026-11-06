# globalizer.optimize — интерфейс Globalizer в стиле scipy.optimize

Пакет повторяет функции глобальной оптимизации `scipy.optimize`. Чтобы решить
задачу Globalizer'ом вместо scipy, достаточно заменить имя библиотеки:

```python
from scipy.optimize import differential_evolution        # было
from globalizer.optimize import differential_evolution   # стало

import scipy.optimize as opt      ->  import globalizer.optimize as opt
from scipy import optimize        ->  from globalizer import optimize
```

Сигнатуры (имена и порядок параметров), форма результата (`OptimizeResult`
с полями `x`, `fun`, `success`, `status`, `message`, `nfev`, `nit`) и
классы ограничений совпадают со scipy (проверено для scipy 1.17).

## Установка

1. Соберите модуль `PYDGlobalizer`:
   `gen\StartGCC-no_mp-No_MPI_PYGlobalizer.bat` (GCC) или
   `gen\StartVS-no_mp-No_MPI_PYGlobalizer.bat` (Visual Studio).
   Модуль попадёт в `_bin`.
2. Добавьте папку `PYGlobalizer` в путь поиска модулей:
   `set PYTHONPATH=<Globalizer>\PYGlobalizer` (или `sys.path.insert`).
   Модуль `PYDGlobalizer` пакет найдёт сам в `_bin`; если он лежит в другом
   месте, укажите папку в переменной окружения `GLOBALIZER_BIN`.

Зависимость одна — `numpy`. scipy не нужен.

## Что поддерживается

| scipy.optimize | Используемые параметры |
|---|---|
| `differential_evolution` | `func, bounds, args, maxiter, popsize` (лимит `(maxiter+1)*popsize*N`), `callback`, `disp`, `polish`, `workers`, `constraints`, `x0`, `integrality`, `vectorized` |
| `dual_annealing` | `func, bounds, args, maxfun, no_local_search, callback(x, f, context), x0` |
| `shgo` | `func, bounds, args, constraints, callback(xk), workers`, `options['maxfev']`, `options['disp']` |
| `direct` | `func, bounds, args, maxfun` (по умолчанию `1000*N`), `f_min, f_min_rtol, callback(xk)` |
| `basinhopping` | `func, x0, callback(x, f, accept)`, из `minimizer_kwargs` — `bounds` (обязательно), `args`, `constraints` |
| `minimize` | `fun, x0, args, bounds` (обязательно), `constraints, callback`, `options['maxfev'/'maxfun'/'disp']` |
| `Bounds`, `NonlinearConstraint`, `LinearConstraint` | все конструкторы; принимаются и объекты самого scipy |
| ограничения-словари | `{'type': 'ineq' \| 'eq', 'fun': ..., 'args': ...}` |

Параметры, которые задают внутреннее устройство алгоритмов scipy (`strategy`,
`mutation`, `recombination`, `initial_temp`, `sampling_method`, `method`,
`jac`, `hess`, `seed`/`rng`, ...), принимаются и не используются: во всех
функциях задачу решает индексный метод Globalizer. Решение детерминировано.

Callback можно задавать в обоих стилях scipy: `callback(intermediate_result)`
и прежний (`callback(xk, convergence)` и т. п.). Он вызывается при каждом
улучшении лучшей допустимой точки; остановить поиск можно, вернув `True` или
подняв `StopIteration` (тогда `status = 2`).

## Настройки Globalizer

Для одного вызова — аргумент `globalizer_options`, для всех вызовов —
`set_options(...)` (сброс: `set_options()`):

```python
import globalizer.optimize as opt
opt.set_options(r=5.0, eps=0.001)               # для всех вызовов
res = opt.differential_evolution(f, bounds, globalizer_options={"maxfev": 5000})
```

| Ключ | Смысл | По умолчанию |
|---|---|---|
| `r` | параметр надёжности метода (> 1): больше — надёжнее, но дольше | 4.0 |
| `eps` | точность остановки в долях диапазона переменных | 0.01 |
| `maxfev` | лимит испытаний (перекрывает лимит из параметров scipy) | см. таблицу выше, не больше 100 000 |
| `local_search` | локальное уточнение (Хук–Дживс) | как в scipy: `polish`, `not no_local_search`, иначе включено |
| `local_iterations` | итераций локального метода | 100 |
| `eq_tol` | допуск для равенств: `abs(h(x)) <= eq_tol` | 0.01 |
| `threads` | число потоков (перекрывает `workers`) | 1 |
| `disp` | печать итогов решателя | `disp` вызова |

## Отличия от scipy

- **Нужны конечные границы всех переменных.** Globalizer ищет глобальный
  минимум в параллелепипеде; поэтому `minimize` и `basinhopping` требуют
  `bounds`, а `x0` — лишь дополнительный кандидат в решение.
- **Точность.** Поиск останавливается при точности `eps` (1% диапазона),
  после чего решение уточняется локальным методом. В задачах без ограничений
  координаты обычно точны до ~1e-4 диапазона. Если минимум лежит на границе
  нелинейного ограничения, локальный метод почти не помогает и точность
  ≈ `eps`. Нужна выше — уменьшите `eps` (например, до 1e-3).
- **Равенства поддерживаются приближённо** — как полоса `abs(h(x)) <= eq_tol`
  (по умолчанию 0.01). Слишком узкую полосу метод может не найти.
- **Результат** — лучшая допустимая точка среди всех вычислений.
  `success = False` и `status = 3`, если допустимых точек не найдено.
  `status = 1` (лимит испытаний исчерпан) считается успешным завершением,
  если допустимая точка есть. При ограничениях в результате есть поля
  `maxcv` и `constr_violation`.
- **Размерность** до 50 (ограничение `PYProblem`).
- Для `shgo` поля `xl` и `funl` содержат одну найденную точку. Для
  `basinhopping` поле `lowest_optimization_result` — копия результата.
- `brute` не реализован.

## Тесты и пример

- `integration_tests/test_globalizer_optimize.py` — тесты (unittest). При
  установленном scipy проверяют и совместимость сигнатур, и совпадение
  ответов со scipy. В CTest: `python_scipy_interface`.
- `examples/example_scipy_interface.py` — пример; с аргументом `scipy` те же
  задачи решает scipy.

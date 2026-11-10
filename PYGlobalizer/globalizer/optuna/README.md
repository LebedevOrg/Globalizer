# globalizer.optuna — интерфейс Globalizer в стиле Optuna

Пакет повторяет API [Optuna](https://optuna.org) (study / trial / `suggest_*`),
но точки испытаний выбирает индексный метод глобальной оптимизации Globalizer.
Чтобы решить задачу Globalizer'ом вместо Optuna, достаточно заменить импорт:

```python
import optuna                          # было
import globalizer.optuna as optuna     # стало

def objective(trial):
    x = trial.suggest_float("x", -10, 10)
    lr = trial.suggest_float("lr", 1e-5, 1e-1, log=True)
    kind = trial.suggest_categorical("kind", ["a", "b"])
    return (x - 2) ** 2 + (lr - 1e-3) ** 2 + (kind == "a")

study = optuna.create_study()
study.optimize(objective, n_trials=200)
print(study.best_params, study.best_value)
```

Это третий Python-интерфейс Globalizer. Есть ещё собственный
(`PYDGlobalizer.solve` + `PYProblem`) и интерфейс в стиле scipy
(`globalizer.optimize`, см. [../README.md](../README.md)).

## Установка

1. Соберите модуль `PYDGlobalizer`: `gen\StartGCC-no_mp-No_MPI_PYGlobalizer.bat`
   (GCC) или `gen\StartVS-no_mp-No_MPI_PYGlobalizer.bat` (Visual Studio).
   Модуль попадёт в `_bin`.
2. Добавьте папку `PYGlobalizer` в путь поиска модулей
   (`set PYTHONPATH=<Globalizer>\PYGlobalizer` или `sys.path.insert`).
   Модуль `PYDGlobalizer` пакет найдёт сам в `_bin`, либо укажите его папку
   в переменной окружения `GLOBALIZER_BIN`.

Зависимость одна — `numpy`. Сама Optuna не нужна.

## Что поддерживается

| Optuna | Поддержка |
|---|---|
| `create_study(direction=..., sampler=..., pruner=..., study_name=...)` | да; только хранение в памяти (`storage=None`), одна цель |
| `study.optimize(func, n_trials, timeout, catch, callbacks)` | да; `n_jobs > 1` игнорируется (испытания идут последовательно) |
| `study.ask(fixed_distributions)` / `study.tell(trial, values, state)` | да |
| `study.enqueue_trial`, `study.add_trial(s)`, `create_trial` | да |
| `study.best_trial / best_value / best_params / trials / get_trials / stop` | да |
| `study.trials_dataframe()` | да, если установлен pandas |
| `trial.suggest_float(log, step)`, `suggest_int(log, step)`, `suggest_categorical` | да (и устаревшие `suggest_uniform` и др.) |
| условное пространство поиска (define-by-run) | да |
| `trial.report` / `trial.should_prune` / `TrialPruned` | да; `MedianPruner` (по умолчанию, как в Optuna), `PercentilePruner`, `NopPruner` |
| ограничения: `trial.set_constraint(...)` (Optuna 5) и `constraints_func` сэмплера | да; `best_trial` выбирает лучшее допустимое испытание |
| `trial.set_user_attr`, `FixedTrial`, `FrozenTrial`, `TrialState` | да |
| `optuna.logging.set_verbosity(...)` | да; вывод о каждом испытании в формате Optuna |
| сэмплеры Optuna (`TPESampler`, `RandomSampler`, `CmaEsSampler`, ...) | имена приняты для совместимости, но точки выбирает Globalizer; используются только `seed` и `constraints_func` |
| многокритериальные задачи, `storage`, `load_study`, `visualization`, `importance` | нет |

Распределения и объекты самой Optuna (`optuna.distributions.FloatDistribution`,
`StudyDirection`, сэмплеры) тоже принимаются.

## Как это устроено

Optuna устроена по схеме «спроси–сообщи» (ask/tell): целевую функцию вызывает
пользователь. Globalizer, наоборот, сам вызывает функции задачи. Чтобы
совместить эти схемы, метод (`PYDGlobalizer.IterativeSolver`) работает в
отдельном потоке. Функция задачи, которую вызывает метод, отдаёт точку
в `study.ask()` и ждёт значения из `study.tell()`. `study.optimize()` — это
цикл ask → objective → tell, как в самой Optuna.

Пространство поиска строится по вызовам `suggest_*`:

| Параметр | Переменная Globalizer |
|---|---|
| `suggest_float(low, high)` | непрерывная на [low, high] |
| `suggest_float(..., log=True)` | непрерывная на [ln low, ln high] |
| `suggest_float(..., step=s)`, `suggest_int(...)` | непрерывная с округлением до узла сетки |
| `suggest_int(..., log=True)` | непрерывная на [ln low, ln(high+1)] с округлением вниз |
| `suggest_categorical(choices)` | дискретная переменная Globalizer (перебор значений), пока число сочетаний категорий не больше `max_discrete_combinations` (64); иначе непрерывная с округлением номера |

- **Первое испытание** получает центр диапазонов (у категорий — первое
  значение). Так метод узнаёт пространство поиска.
- **Новый параметр** или изменённое распределение расширяет пространство:
  после этого испытания метод перезапускается.
- **Повторных испытаний нет.** Уже вычисленные точки метод при перезапуске
  берёт из кэша, без вызова целевой функции. Точка, в которой совпадают все
  параметры, запрошенные прошлым испытанием, тоже берётся из кэша. Поэтому в
  условном пространстве испытания, отличающиеся только неиспользуемыми
  параметрами, не повторяются. Это предполагает, что целевая функция
  детерминирована: при тех же значениях параметров она даёт тот же результат.
- **Сколько испытаний.** Метод останавливается, достигнув точности `eps`. Если
  испытания ещё нужны, он перезапускается с точностью `eps * eps_decay`; путь
  до прежней точки повторяется бесплатно, по кэшу. Когда точность меньше
  `eps_min` или новых точек больше нет (небольшое дискретное пространство
  перебрано целиком), `optimize` завершается раньше `n_trials`, как Optuna с
  `GridSampler`.
- **Значения для метода.** Для `maximize` метод минимизирует `-value`. У
  прерванного испытания (PRUNED) значение — последнее промежуточное. У
  неудачного (FAIL, NaN, исключение из `catch`) — штраф: заведомо хуже всех
  полученных значений.
- **Ограничения** (`set_constraint` или `constraints_func`) становятся
  функциями-ограничениями задачи Globalizer `g(x) <= 0`. Индексный метод
  учитывает их отдельно от критерия.
- **Параллельные ask().** Метод последователен: пока испытание из точки метода
  ждёт `tell()`, следующие `ask()` получают случайные точки (зерно —
  `seed` сэмплера).
- **Один решатель на процесс.** Ядро Globalizer хранит параметры глобально,
  поэтому одновременно работает только один метод. Если чередовать испытания
  нескольких исследований, методы по очереди останавливаются и
  перезапускаются; кэш делает это дешёвым. Не вызывайте `PYDGlobalizer.solve` /
  `globalizer.optimize` внутри целевой функции.

## Настройки Globalizer

```python
sampler = optuna.samplers.GlobalizerSampler(r=4.0, eps=0.1, local_refine_mode="updated")
study = optuna.create_study(sampler=sampler)
```

| Параметр | Смысл | По умолчанию |
|---|---|---|
| `r` | параметр надёжности метода (> 1): больше — надёжнее, но больше испытаний | 4.0 |
| `eps` | точность первого запуска метода в долях диапазона | 0.1 |
| `eps_decay`, `eps_min` | уменьшение точности при перезапуске; нижняя граница | 0.5, 1e-6 |
| `local_search` | локальное уточнение (Хук–Дживс) | True |
| `local_refine_mode` | `"updated"` — локальный поиск при каждом улучшении минимума; `"final"` — один раз после глобального поиска | `"updated"` |
| `local_iterations` | итераций локального метода | 300 |
| `max_discrete_combinations` | сколько сочетаний категорий перебирать дискретно | 64 |
| `constraints_func` | ограничения, как в Optuna | None |
| `seed` | зерно запасной случайной выборки | None |

Значения по умолчанию подобраны для типичных бюджетов Optuna (100–1000
испытаний). С крупной стартовой точностью и локальным поиском при каждом
улучшении метод быстрее получает точный ответ. Режим `local_refine_mode`
добавлен в `PYDGlobalizer.SolverParameters`: со старой сборкой модуля
используется `"final"`.

## Тесты, примеры, сравнение

- `integration_tests/test_globalizer_optuna.py` — тесты (unittest), в CTest:
  `python_optuna_interface`. Если Optuna установлена, они проверяют ещё и
  совпадение сигнатур и то, что одна целевая функция работает в обеих
  библиотеках.
- `examples/example_optuna_interface.py` — основной пример: типы параметров,
  ограничения, условное пространство, ask/tell. С аргументом `optuna` те же
  задачи решает Optuna.
- `examples/example_optuna_ml.py` — выбор модели scikit-learn и прунинг.
- `examples/compare_optimizers.py` — сравнение Globalizer (все три интерфейса),
  scipy.optimize и Optuna при равном бюджете вычислений. Отчёт пишется в
  `comparison_results/report.md` и `results.csv`. Короткий прогон в CTest:
  `python_optimizer_comparison_smoke`.

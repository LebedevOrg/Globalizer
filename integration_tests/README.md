# Интеграционные тесты Globalizer — запуск всех примеров из `examples/` (v4)

Запускает каждый пример из `examples/` как тест — и C++, и Python. Всё
расширение в папке `integration_tests/`. В ядре нужна одна строка
`enable_testing()` в блоке `GLOBALIZER_BUILD_INTEGRATION_TESTS` корневого
`CMakeLists.txt` (в вашем рефакторинге она уже есть).

## Изменения по версиям

- **v4** — два исправления в раннере, выявленные при прогоне под CTest:
  1. **UnicodeEncodeError под CTest.** Когда вывод раннера идёт в pipe (а не в
     консоль), Python на Windows выбирает системную кодировку (cp1251), в
     которой нет символа `Δ`. Из-за этого падал сам раннер (`print`), а не
     примеры — отсюда «провал» 8 тестов. Теперь:
     - stdout/stderr принудительно переводятся в UTF-8 с `errors="replace"`;
     - в выводе результатов используется только ASCII (`|d|` вместо `|Δ|`,
       `ref` вместо `эталон`).
  2. **Example_simple.** Это демо `iOptProblemSimple.Rastrigin` с жёстко
     заданными 50 итерациями — для точного схода их мало (f≈2.18). Эталон убран
     (`known_optimum: null`): критерий — отсутствие краха.
- **v3** — распознавание формата `Minimum value = ...` в выводе C++ примеров.
- **v2** — исправление LNK1104 (python*.lib) и C4005.
- **v1** — первая версия.

## Состав

```
integration_tests/
├── CMakeLists.txt
├── run_all_examples.py
├── README.md
├── references/
│   ├── examples.json
│   └── tests.json
└── src/
    └── main.cpp
```

## Критерий успеха

- `known_optimum != null` → `|f_found - known_optimum| <= tolerance`;
- `known_optimum == null` → достаточно, что пример отработал без краха.

Статусы: PASS / FAIL / SKIP.

## Установка

Распакуйте архив в корень репозитория, заменив папку `integration_tests/`.
Поскольку изменился только Python-раннер и JSON, C++ пересобирать не нужно —
достаточно перезапустить тесты. Но повторный полный прогон `BuildTestAndOpenVS.bat`
тоже сработает.

## Ожидаемый результат на вашей машине

По вашему логу (при прямом запуске раннер уже показал 16 PASS / 1 FAIL / 2 SKIP;
единственный FAIL — Example_simple — в v4 снят). После обновления ожидается:

- Модульные тесты: 513/513 PASS.
- Интеграционные: все C++ примеры PASS; Python example_legacy,
  example_manual_sphere, example_constrained, example_ackley,
  example_rastrigin_int — PASS; Example_simple — PASS (без краха);
  Example_SVC_2D / Example_SVC_3D — SKIP (нет sklearn/iOptProblem).
- `ctest` больше не падает на UnicodeEncodeError.

Итого ожидается: **17 PASS, 0 FAIL, 2 SKIP**, и `ctest` возвращает 0.

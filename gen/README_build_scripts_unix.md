# Globalizer — скрипты сборки для Linux и macOS

POSIX-аналоги Windows-bat-файлов. Один скрипт работает и на Linux, и на macOS:
ОС определяется автоматически через `uname`.

## Файлы

| Скрипт | Аналог bat | Назначение |
|---|---|---|
| `build_test_and_open.sh` | `BuildTestAndOpenVS.bat` | Полный цикл: сабмодули → conda-окружение → pip → конфигурация → сборка всего → модульные тесты → интеграционные тесты → открыть каталог сборки |
| `configure_no_mp_no_mpi.sh` | второй bat (`build_64_nmmp`) | Лёгкая конфигурация без Python/MP/MPI + открытие (без сборки и тестов) |

## Чем отличается от Windows-версии (по необходимости платформы)

- **Нет Visual Studio / vswhere.** Генератор выбирается автоматически:
  `Ninja` (если установлен) → иначе `Unix Makefiles`. На macOS с ключом
  `--xcode` используется генератор `Xcode`.
- **«Открыть Visual Studio»** заменено на открытие каталога сборки штатным
  средством: `open` (macOS) или `xdg-open` (Linux). С `--xcode` на macOS
  открывается сгенерированный `.xcodeproj`.
- **Python** берётся из `build_64/Globalizer_env/bin/python` (на Windows —
  `...\python.exe`).
- **Библиотеки задач**: `.so` (Linux) / `.dylib` (macOS) вместо `.dll`;
  пути прокидываются через `LD_LIBRARY_PATH` и `DYLD_LIBRARY_PATH`.
- **OneAPI/MPI не используются** — как и в v2 bat: `GLOBALIZER_USE_MP=OFF`,
  `GLOBALIZER_USE_MPI=OFF`.
- Набор CMake-опций идентичен bat-файлам 1:1 (те же цели и имена).

## Требования

- **Linux**: `g++` или `clang`, `cmake`, `git`, conda (Miniconda/Anaconda/Miniforge),
  желательно `ninja`.
- **macOS**: Xcode Command Line Tools (`xcode-select --install`), `cmake`,
  `git`, conda. Установка через Homebrew: `brew install cmake ninja`.

conda ищется в PATH и в типичных местах (`~/miniconda3`, `~/anaconda3`,
`~/miniforge3`, `/opt/...`, `/opt/homebrew/anaconda3`).

## Запуск

```bash
chmod +x gen/build_test_and_open.sh gen/configure_no_mp_no_mpi.sh

# полный цикл (Release), затем откроется каталог сборки
./gen/build_test_and_open.sh

# варианты
./gen/build_test_and_open.sh --debug          # конфигурация Debug
./gen/build_test_and_open.sh --notest          # только собрать
./gen/build_test_and_open.sh --noopen          # не открывать каталог
./gen/build_test_and_open.sh --clean           # очистить кэш CMake
./gen/build_test_and_open.sh --jobs 8          # число потоков сборки
./gen/build_test_and_open.sh --xcode           # (macOS) проект Xcode

# лёгкая конфигурация без Python/MP/MPI
./gen/configure_no_mp_no_mpi.sh
./gen/configure_no_mp_no_mpi.sh --build        # ещё и собрать
```

## Этапы `build_test_and_open.sh`

1. `git submodule update --init --recursive`
2. conda-окружение `Globalizer_env` (Python 3.12) с запасными зеркалами
   (default → conda-forge → yandex → aliyun → tsinghua)
3. `pip install` зависимостей + `pybind11`, `numpy`; вычисление `pybind11_DIR`
4. выбор тулчейна/генератора (Ninja / Unix Makefiles / Xcode)
5. выбор cmake
6. конфигурация всех проектов
7. сборка всего + отдельно `PYDGlobalizer`
8. модульные тесты (`GLOBALIZER_tests`), затем интеграционные (`ctest`,
   при неудаче — прямой вызов `run_all_examples.py`)
9. открытие каталога сборки (или `.xcodeproj` при `--xcode`)

Коды выхода: `0` — успех; `1` — ошибка на любом этапе (с понятным сообщением).

## Примечание про тесты

Скрипты рассчитаны на пакет интеграционных тестов `integration_tests/` версии
v4 и новее (там раннер корректно работает с кодировкой вывода под `ctest`).

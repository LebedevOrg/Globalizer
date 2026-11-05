@echo off
REM ============================================================================
REM  BuildTestAndOpenVS_v2.bat
REM
REM  Единый скрипт для Globalizer: подтягивает всё необходимое, собирает ВСЕ
REM  проекты решения, прогоняет сначала МОДУЛЬНЫЕ тесты, затем ИНТЕГРАЦИОННЫЕ,
REM  и в конце открывает Visual Studio.
REM
REM  ОТЛИЧИЯ ОТ v1:
REM    * Полностью удалён блок активации Intel OneAPI. Он вызывал сбой
REM      "Непредвиденное появление: .." на шаге 4: скрипты Intel setvars.bat
REM      не совместимы с EnableDelayedExpansion. OneAPI не нужен, т.к. сборка
REM      идёт с GLOBALIZER_USE_MP=OFF и GLOBALIZER_USE_MPI=OFF (без MPFR/MPI).
REM    * Детект VS и всё остальное — без изменений.
REM
REM  Шаги:
REM    [1/9] git submodule update --init --recursive
REM    [2/9] conda-окружение Globalizer_env (Python) c запасными зеркалами
REM    [3/9] pip install зависимостей (+ pybind11/numpy для Python-интерфейса)
REM    [4/9] детект Visual Studio (vswhere) и выбор генератора
REM    [5/9] детект подходящего cmake
REM    [6/9] CMake-конфигурация ВСЕХ проектов
REM    [7/9] сборка всего решения (ALL_BUILD) + PYDGlobalizer
REM    [8/9] МОДУЛЬНЫЕ тесты, затем ИНТЕГРАЦИОННЫЕ (ctest / раннер примеров)
REM    [9/9] открытие Visual Studio
REM
REM  Ключи:
REM    (без ключей)  — полный цикл: конфигурация, сборка, тесты, открыть VS
REM    /clean        — очистить кэш CMake перед конфигурацией
REM    /noopen       — не открывать Visual Studio в конце
REM    /notest       — не запускать тесты (только собрать)
REM    /debug        — собирать и тестировать конфигурацию Debug (по умолч. Release)
REM ============================================================================
setlocal EnableExtensions EnableDelayedExpansion

REM ---------------------------------------------------------------------------
REM Параметры и пути
REM ---------------------------------------------------------------------------
set "PY_VER=3.12"
set "PY_MODULE=PYDGlobalizer"
set "BUILD_CONFIG=Release"
set "DO_CLEAN=0"
set "DO_OPEN=1"
set "DO_TEST=1"

for %%A in (%*) do (
    if /i "%%~A"=="/clean"  set "DO_CLEAN=1"
    if /i "%%~A"=="/noopen" set "DO_OPEN=0"
    if /i "%%~A"=="/notest" set "DO_TEST=0"
    if /i "%%~A"=="/debug"  set "BUILD_CONFIG=Debug"
)

set "START_DIR=%cd%"
for %%I in ("%~dp0..") do set "ROOT_DIR=%%~fI"
set "BUILD_DIR=%ROOT_DIR%\build_64"
set "ENV_DIR=%BUILD_DIR%\Globalizer_env"
set "PY_EXE=%ENV_DIR%\python.exe"
set "BIN_DIR=%ROOT_DIR%\_bin"
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
set "TMP_OUT=%TEMP%\globalizer_setup_%RANDOM%.txt"

echo.
echo Root dir : %ROOT_DIR%
echo Build dir: %BUILD_DIR%
echo Env dir  : %ENV_DIR%
echo Config   : %BUILD_CONFIG%

cd /d "%ROOT_DIR%"
if errorlevel 1 ( set "FAIL_MSG=Cannot cd to root directory." & goto error )
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

REM ---------------------------------------------------------------------------
echo.
echo ========================================
echo [1/9] Git submodules
echo ========================================
where git >nul 2>&1
if errorlevel 1 (
    echo [WARNING] git not found - skipping submodules.
) else (
    git -C "%ROOT_DIR%" submodule update --init --recursive
    if errorlevel 1 echo [WARNING] submodule update failed - third_party may be incomplete.
)

REM ---------------------------------------------------------------------------
echo.
echo ========================================
echo [2/9] Conda environment (Python %PY_VER%)
echo ========================================
call :find_conda
if errorlevel 1 ( set "FAIL_MSG=conda not found. Run from Anaconda Prompt or add condabin to PATH." & goto error )
call conda init >nul 2>&1

if exist "%PY_EXE%" (
    echo [OK] Environment already exists - reusing it.
) else (
    for %%S in (default conda-forge yandex aliyun tsinghua) do (
        if not exist "!PY_EXE!" call :create_env %%S
    )
)
if not exist "%PY_EXE%" ( set "FAIL_MSG=Failed to create conda environment from all channels." & goto error )

call conda activate "%ENV_DIR%"
if errorlevel 1 ( set "FAIL_MSG=conda activate failed." & goto error )

REM ---------------------------------------------------------------------------
echo.
echo ========================================
echo [3/9] Python packages
echo ========================================
"%PY_EXE%" -m pip install --upgrade pip
set "REQ_FILE=%ROOT_DIR%\requirements_build.txt"
if not exist "%REQ_FILE%" set "REQ_FILE=%ROOT_DIR%\requirements.txt"
echo Using "%REQ_FILE%"
"%PY_EXE%" -m pip install -r "%REQ_FILE%"
if errorlevel 1 echo [WARNING] some packages failed to install. Continuing...
"%PY_EXE%" -m pip install pybind11 numpy
if errorlevel 1 ( set "FAIL_MSG=pip install of pybind11/numpy failed." & goto error )

set "PYBIND11_DIR="
"%PY_EXE%" -m pybind11 --cmakedir > "%TMP_OUT%" 2>nul
set /p PYBIND11_DIR=<"%TMP_OUT%"
del "%TMP_OUT%" >nul 2>&1
if not defined PYBIND11_DIR ( set "FAIL_MSG=Cannot get pybind11 CMake dir." & goto error )
echo pybind11_DIR = !PYBIND11_DIR!

REM ---------------------------------------------------------------------------
echo.
echo ========================================
echo [4/9] Visual Studio detection
echo ========================================
call :detect_vs
if errorlevel 1 ( set "FAIL_MSG=Visual Studio with C++ tools was not found." & goto error )
echo [Found] Visual Studio !VS_VER!
echo [Using] !VS_GENERATOR!

REM ПРИМЕЧАНИЕ: Intel OneAPI здесь НЕ активируется намеренно.
REM Сборка идёт без MPFR/MPI (GLOBALIZER_USE_MP=OFF, GLOBALIZER_USE_MPI=OFF),
REM поэтому OneAPI не требуется. Его setvars.bat ломает парсер cmd под
REM EnableDelayedExpansion (ошибка "Непредвиденное появление: ..").

REM ---------------------------------------------------------------------------
echo.
echo ========================================
echo [5/9] CMake selection
echo ========================================
call :select_cmake
if errorlevel 1 ( set "FAIL_MSG=No CMake supports generator !VS_GENERATOR!. Install a newer CMake or VS CMake component." & goto error )
echo [Using] "%CMAKE_EXE%"

if "%DO_CLEAN%"=="1" (
    echo /clean: removing CMake cache
    if exist "!BUILD_DIR!\CMakeCache.txt" del /q "!BUILD_DIR!\CMakeCache.txt"
    if exist "!BUILD_DIR!\CMakeFiles" rmdir /s /q "!BUILD_DIR!\CMakeFiles"
)

REM ---------------------------------------------------------------------------
echo.
echo ========================================
echo [6/9] CMake configuration (ALL projects)
echo ========================================
"%CMAKE_EXE%" -S "%ROOT_DIR%" -B "%BUILD_DIR%" -G "%VS_GENERATOR%" -A x64 ^
  -DGLOBALIZER_USE_MP=OFF ^
  -DGLOBALIZER_USE_MPI=OFF ^
  -DGLOBALIZER_USE_CUDA=OFF ^
  -DGLOBALIZER_BUILD_PROBLEMS=ON ^
  -DGLOBALIZER_BUILD_GCGEN=ON ^
  -DGLOBALIZER_BUILD_TESTS=ON ^
  -DGLOBALIZER_BUILD_INTEGRATION_TESTS=ON ^
  -DGLOBALIZER_PYBIND_VERSION=ON ^
  -DGLOBALIZER_PYTHON=ON ^
  -DGLOBALIZER_MAX_DIMENSION=130 ^
  -DGLOBALIZER_MAX_Number_Of_Function=70 ^
  -Drastrigin_build=ON ^
  -DrastriginInt_build=ON ^
  -DX2_build=ON ^
  -Dpython_objective_build=ON ^
  -Dstronginc3_build=ON ^
  -DrastriginC1_build=ON ^
  -DiOptProblemSimple_build=ON ^
  -DPython_EXECUTABLE="%PY_EXE%" ^
  -DPython_ROOT_DIR="%ENV_DIR%" ^
  -DPython_FIND_STRATEGY=LOCATION ^
  -DPython_FIND_REGISTRY=NEVER ^
  -DPython_FIND_DEBUG=OFF ^
  -Dpybind11_DIR="%PYBIND11_DIR%" ^
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5
if errorlevel 1 ( set "FAIL_MSG=CMake configuration failed. Rerun with /clean if the generator changed." & goto error )

REM ---------------------------------------------------------------------------
echo.
echo ========================================
echo [7/9] Build ALL (%BUILD_CONFIG%)
echo ========================================
"%CMAKE_EXE%" --build "%BUILD_DIR%" --config %BUILD_CONFIG% --target ALL_BUILD --parallel
if errorlevel 1 ( set "FAIL_MSG=Build of the solution failed." & goto error )

REM Собираем Python-модуль отдельно (не всегда входит в ALL_BUILD).
"%CMAKE_EXE%" --build "%BUILD_DIR%" --config %BUILD_CONFIG% --target %PY_MODULE% --parallel >nul 2>&1

if "%DO_TEST%"=="0" goto open_vs

REM ---------------------------------------------------------------------------
echo.
echo ========================================
echo [8/9] Tests: unit first, then integration
echo ========================================

echo.
echo --- [8.1] UNIT tests (GLOBALIZER_tests) ---
set "UNIT_EXE="
for %%F in ("%BIN_DIR%\GLOBALIZER_tests.exe" "%BIN_DIR%\GLOBALIZER_tests_d.exe") do (
    if not defined UNIT_EXE if exist "%%~F" set "UNIT_EXE=%%~F"
)
if not defined UNIT_EXE (
    echo [WARNING] GLOBALIZER_tests.exe not found in _bin - skipping unit tests.
) else (
    pushd "%BIN_DIR%"
    "!UNIT_EXE!"
    set "UNIT_RC=!errorlevel!"
    popd
    if not "!UNIT_RC!"=="0" ( set "FAIL_MSG=Unit tests FAILED (code !UNIT_RC!)." & goto error )
    echo [OK] Unit tests passed.
)

echo.
echo --- [8.2] INTEGRATION tests (examples + cases) ---
REM Прогоняем через CTest. Если он недоступен - вызываем раннер напрямую.
set "PYTHONPATH=%BIN_DIR%;%ROOT_DIR%\PYGlobalizer;%ROOT_DIR%\third_party\Problems\Problems;%PYTHONPATH%"
pushd "%BUILD_DIR%"
ctest -C %BUILD_CONFIG% -E "^unit" --output-on-failure
set "CT_RC=!errorlevel!"
popd
if not "!CT_RC!"=="0" (
    echo [Info] ctest returned !CT_RC!; running the examples runner directly...
    "%PY_EXE%" "%ROOT_DIR%\integration_tests\run_all_examples.py" --bin-dir "%BIN_DIR%"
    set "RUN_RC=!errorlevel!"
    if not "!RUN_RC!"=="0" ( set "FAIL_MSG=Integration tests FAILED (code !RUN_RC!)." & goto error )
)
echo [OK] Integration tests finished.

REM ---------------------------------------------------------------------------
:open_vs
echo.
echo ========================================
echo [9/9] Visual Studio
echo ========================================
if "%DO_OPEN%"=="0" (
    echo /noopen: not opening Visual Studio.
    goto done
)
set "SLN="
for %%F in ("%BUILD_DIR%\*.slnx" "%BUILD_DIR%\*.sln") do if not defined SLN set "SLN=%%~fF"
if not defined SLN ( set "FAIL_MSG=Solution file not found in build_64." & goto error )
echo Opening !SLN!
start "" "!SLN!"

:done
echo.
echo ========================================
echo DONE
echo ========================================
echo Solution : %BUILD_DIR%
echo Binaries : %BIN_DIR%
echo Env      : %ENV_DIR%
echo Python   : conda activate "%ENV_DIR%"  ^&^&  python examples\Example_simple.py
cd /d "%START_DIR%"
endlocal
exit /b 0


REM ===========================================================================
REM  Вспомогательные подпрограммы
REM ===========================================================================

:find_conda
where conda >nul 2>&1 && exit /b 0
for %%P in ("%USERPROFILE%\miniconda3" "%USERPROFILE%\anaconda3" "%USERPROFILE%\miniforge3" "%LOCALAPPDATA%\miniconda3" "%ProgramData%\miniconda3" "%ProgramData%\anaconda3") do (
    if exist "%%~P\condabin\conda.bat" (
        set "PATH=%%~P\condabin;!PATH!"
        exit /b 0
    )
)
exit /b 1

:create_env
set "SRC=%~1"
set "CH="
if /i "%SRC%"=="default"     set "CH="
if /i "%SRC%"=="conda-forge" set "CH=-c conda-forge"
if /i "%SRC%"=="yandex"      set "CH=-c https://mirrors.yandex.ru/mirrors/anaconda/cloud/conda-forge/ -c https://mirrors.yandex.ru/mirrors/anaconda/pkgs/main/"
if /i "%SRC%"=="aliyun"      set "CH=-c https://mirrors.aliyun.com/anaconda/pkgs/main/ -c https://mirrors.aliyun.com/anaconda/pkgs/free/"
if /i "%SRC%"=="tsinghua"    set "CH=-c https://mirrors.tuna.tsinghua.edu.cn/anaconda/pkgs/main/ -c https://mirrors.tuna.tsinghua.edu.cn/anaconda/pkgs/free/"
echo   [Attempt] conda env from source: %SRC%
if exist "%ENV_DIR%" rmdir /s /q "%ENV_DIR%"
call conda create -y -p "%ENV_DIR%" %CH% python=%PY_VER% pip
if exist "%PY_EXE%" ( echo   [OK] created from %SRC% & exit /b 0 )
echo   [FAILED] %SRC%
exit /b 1

:detect_vs
set "VS_GENERATOR="
set "VS_MAJOR="
set "VS_VER="
set "VS_PATH="
if not exist "%VSWHERE%" exit /b 1
"%VSWHERE%" -latest -prerelease -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationVersion > "%TMP_OUT%" 2>nul
set /p VS_VER=<"%TMP_OUT%"
"%VSWHERE%" -latest -prerelease -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath > "%TMP_OUT%" 2>nul
set /p VS_PATH=<"%TMP_OUT%"
del "%TMP_OUT%" >nul 2>&1
if not defined VS_VER exit /b 1
for /f "tokens=1 delims=." %%V in ("%VS_VER%") do set "VS_MAJOR=%%V"
if "%VS_MAJOR%"=="18" set "VS_GENERATOR=Visual Studio 18 2026"
if "%VS_MAJOR%"=="17" set "VS_GENERATOR=Visual Studio 17 2022"
if "%VS_MAJOR%"=="16" set "VS_GENERATOR=Visual Studio 16 2019"
if defined VS_GENERATOR exit /b 0
exit /b 1

:select_cmake
set "CMAKE_EXE="
where cmake >nul 2>&1
if errorlevel 1 goto select_cmake_vs
call :check_cmake cmake
if errorlevel 1 goto select_cmake_vs
set "CMAKE_EXE=cmake"
exit /b 0
:select_cmake_vs
set "VS_CMAKE=%VS_PATH%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
if not exist "%VS_CMAKE%" exit /b 1
call :check_cmake "%VS_CMAKE%"
if errorlevel 1 exit /b 1
set "CMAKE_EXE=%VS_CMAKE%"
exit /b 0

:check_cmake
"%~1" --help > "%TMP_OUT%" 2>nul
findstr /c:"%VS_GENERATOR%" "%TMP_OUT%" >nul 2>&1
set "CM_RC=%errorlevel%"
del "%TMP_OUT%" >nul 2>&1
if "%CM_RC%"=="0" exit /b 0
exit /b 1

:error
echo.
echo ========================================
echo  BUILD / TEST FAILED
echo  !FAIL_MSG!
echo ========================================
echo.
echo Troubleshooting:
echo  1. Rerun with /clean if the CMake generator changed.
echo  2. Run from "x64 Native Tools Command Prompt" or Anaconda Prompt.
echo  3. Check internet/firewall (conda + pip need network).
cd /d "%START_DIR%"
pause
endlocal
exit /b 1

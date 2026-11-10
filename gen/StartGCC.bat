@echo off
REM ============================================================================
REM  StartGCC.bat - аналог StartVS.bat для сборки компилятором GCC (MinGW-w64,
REM  например w64devkit) без Visual Studio.
REM
REM  Отличия от StartVS.bat:
REM   * генератор Ninja + gcc/g++ вместо "Visual Studio 17 2022";
REM   * вместо Intel oneAPI MPI используется MS-MPI (SDK, если установлен,
REM     иначе пакет msmpi из базового conda); если MPI не найден - stub-MPI;
REM   * вместо открытия .sln проект сразу собирается (cmake --build).
REM
REM  Каталог сборки: build_gcc_64, результат: _bin (как и для VS).
REM
REM  Путь к GCC: переменная окружения GCC_DIR (папка с gcc.exe).
REM  По умолчанию: <папка над репозиторием>\w64devkit\bin.
REM  Тип сборки: первый аргумент (Release по умолчанию, или Debug).
REM ============================================================================
setlocal enabledelayedexpansion
chcp 65001 >nul

set "START_DIR=%cd%"
for %%I in ("%~dp0..") do set "ROOT_DIR=%%~fI"
set "BUILD_DIR=%ROOT_DIR%\build_gcc_64"

set "BUILD_TYPE=%~1"
if "%BUILD_TYPE%"=="" set "BUILD_TYPE=Release"

if "%GCC_DIR%"=="" (
    for %%I in ("%ROOT_DIR%\..\w64devkit\bin") do set "GCC_DIR=%%~fI"
)

cd /d "%ROOT_DIR%"

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd /d "%BUILD_DIR%"

git submodule init
git submodule update

call conda init

echo.
echo ========================================
echo [1/6] Creating a Conda Environment...
echo ========================================

if exist "%BUILD_DIR%\Globalizer_env\python.exe" (
    echo [OK] Conda environment already exists: %BUILD_DIR%\Globalizer_env
    goto env_ready
)

set "SOURCES[0]=default"
set "SOURCES[1]=yandex"
set "SOURCES[2]=aliyun"
set "SOURCES[3]=tsinghua"
set "SUCCESS=0"

for /L %%I in (0,1,3) do (
    if !SUCCESS! equ 0 (
        set "CURRENT_SOURCE=!SOURCES[%%I]!"
        call :create_env !CURRENT_SOURCE!
        if !ERRORLEVEL! equ 0 (
            set "SUCCESS=1"
            echo [OK] Conda environment created successfully from !CURRENT_SOURCE!
        )
    )
)

if !SUCCESS! equ 0 (
    echo.
    echo [ERROR] Failed to create conda environment from all sources!
    echo Please check your internet connection or try manual installation.
    goto error
)

:env_ready
echo.
echo ========================================
echo [2/6] Activating Conda Environment...
echo ========================================
REM Базовый каталог conda - до активации (нужен для поиска MS-MPI).
set "CONDA_BASE="
for /f "delims=" %%B in ('conda info --base 2^>nul') do set "CONDA_BASE=%%B"

REM conda.exe не умеет activate без "conda init" в текущей оболочке, поэтому
REM вызываем condabin\conda.bat напрямую.
set "CONDA_CMD=conda"
if defined CONDA_BASE if exist "%CONDA_BASE%\condabin\conda.bat" set "CONDA_CMD=%CONDA_BASE%\condabin\conda.bat"
call "%CONDA_CMD%" activate "%BUILD_DIR%\Globalizer_env"
if %errorlevel% neq 0 (
    echo [ERROR] Failed to activate conda environment!
    goto error
)

echo.
echo ========================================
echo [3/6] Installing Python packages...
echo ========================================
REM requirements.txt - Globalizer libraries; iOptProblemSimple - the problem built below (-DiOptProblemSimple_build=ON).
call "%BUILD_DIR%\Globalizer_env\python.exe" -m pip install -r "%ROOT_DIR%\requirements.txt" -r "%ROOT_DIR%\third_party\Problems\Problems\iOptProblemSimple\requirements.txt"
if %errorlevel% neq 0 (
    echo [WARNING] Some packages may have failed to install. Continuing...
)

echo.
echo ========================================
echo [4/6] Setting up GCC toolchain and MPI...
echo ========================================
if not exist "%GCC_DIR%\g++.exe" (
    echo [ERROR] g++.exe not found in "%GCC_DIR%".
    echo Set GCC_DIR to the folder containing gcc.exe / g++.exe / ninja.exe.
    goto error
)
set "PATH=%GCC_DIR%;%PATH%"
echo   GCC: %GCC_DIR%
for /f "delims=" %%V in ('g++ -dumpfullversion') do echo   GCC version: %%V

set "GENERATOR=Ninja"
where ninja >nul 2>&1
if %errorlevel% neq 0 (
    echo [WARNING] ninja not found, using "MinGW Makefiles".
    set "GENERATOR=MinGW Makefiles"
)

REM --- MS-MPI ---------------------------------------------------------------
REM Если установлен MS-MPI SDK, его переменные MSMPI_INC / MSMPI_LIB64 уже
REM заданы. Иначе берём MS-MPI из базового conda (пакет msmpi). Копируем
REM ТОЛЬКО файлы MPI в отдельную папку: в Anaconda\Library\include лежат
REM также DLL-версии gmp.h / mpfr.h, которые перекрыли бы заголовки из
REM third_party\mpfr_gmp_mpir и сломали бы линковку.
set "USE_MPI=ON"
set "MSMPI_DLL_DIR="
if defined MSMPI_INC if defined MSMPI_LIB64 (
    echo   MPI: MS-MPI SDK from environment: !MSMPI_INC!
    goto mpi_ready
)
set "CONDA_MPI=%CONDA_BASE%\Library"
if defined CONDA_BASE if exist "%CONDA_MPI%\include\mpi.h" if exist "%CONDA_MPI%\lib\msmpi.lib" (
    set "LOCAL_MPI=%BUILD_DIR%\msmpi"
    if not exist "!LOCAL_MPI!\include" mkdir "!LOCAL_MPI!\include"
    if not exist "!LOCAL_MPI!\lib" mkdir "!LOCAL_MPI!\lib"
    for %%F in (mpi.h mpif.h mpifptr.h mpio.h mpiwarning.h) do (
        if exist "%CONDA_MPI%\include\%%F" copy /y "%CONDA_MPI%\include\%%F" "!LOCAL_MPI!\include\" >nul
    )
    copy /y "%CONDA_MPI%\lib\msmpi.lib" "!LOCAL_MPI!\lib\" >nul
    set "MSMPI_INC=!LOCAL_MPI!\include"
    set "MSMPI_LIB64=!LOCAL_MPI!\lib"
    set "MSMPI_LIB32=!LOCAL_MPI!\lib"
    set "MSMPI_DLL_DIR=%CONDA_MPI%\bin"
    echo   MPI: MS-MPI from conda: !CONDA_MPI!
    goto mpi_ready
)
echo [WARNING] MS-MPI not found ^(install MS-MPI SDK or "conda install msmpi"^).
echo           Building with stub MPI ^(GLOBALIZER_USE_MPI=OFF^).
set "USE_MPI=OFF"
:mpi_ready

echo.
echo ========================================
echo [5/6] CMake Configuration...
echo ========================================
call cmake -S "%ROOT_DIR%" -B "%BUILD_DIR%" -G "%GENERATOR%" ^
  -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
  -DCMAKE_C_COMPILER=gcc ^
  -DCMAKE_CXX_COMPILER=g++ ^
  -DGLOBALIZER_BUILD_PROBLEMS=ON ^
  -DGLOBALIZER_USE_MP=ON ^
  -DGLOBALIZER_BUILD_GCGEN=ON ^
  -DGLOBALIZER_MAX_DIMENSION=130 ^
  -DGLOBALIZER_MAX_Number_Of_Function=70 ^
  -DGLOBALIZER_BUILD_TESTS=ON ^
  -DGLOBALIZER_USE_MPI=%USE_MPI% ^
  -DGLOBALIZER_MPI=ms ^
  -DGLOBALIZER_PYTHON=ON ^
  -DPython_FIND_DEBUG=OFF ^
  -Drastrigin_build=ON ^
  -DrastriginInt_build=ON ^
  -DX2_build=ON ^
  -Dpython_objective_build=ON ^
  -Dstronginc3_build=ON ^
  -DrastriginC1_build=ON ^
  -DiOptProblemSimple_build=ON ^
  -DPython_EXECUTABLE="%BUILD_DIR%\Globalizer_env\python.exe" ^
  -DPython_ROOT_DIR="%BUILD_DIR%\Globalizer_env" ^
  -DPython_FIND_STRATEGY=LOCATION

if %errorlevel% neq 0 goto error

echo.
echo ========================================
echo [6/6] Building (%BUILD_TYPE%)...
echo ========================================
call cmake --build "%BUILD_DIR%" --parallel %NUMBER_OF_PROCESSORS%
if %errorlevel% neq 0 goto error

REM Рантайм MS-MPI рядом с исполняемыми файлами (не весь Anaconda\Library\bin
REM в PATH - там много посторонних DLL).
if defined MSMPI_DLL_DIR if exist "%MSMPI_DLL_DIR%\msmpi.dll" (
    copy /y "%MSMPI_DLL_DIR%\msmpi.dll" "%ROOT_DIR%\_bin\" >nul
)

echo.
echo ========================================
echo  BUILD SUCCEEDED
echo  Binaries: %ROOT_DIR%\_bin
echo  Примеры используют python312.dll - перед запуском добавьте в PATH:
echo    set "PATH=%BUILD_DIR%\Globalizer_env;%%PATH%%"
echo ========================================
cd /d "%START_DIR%"
exit /b 0

REM ========================================
REM FUNCTION: create_env
REM ========================================
:create_env
setlocal enabledelayedexpansion
set "SOURCE=%~1"

echo.
echo [Attempt] Creating environment from source: !SOURCE!

if "!SOURCE!"=="default" (
    echo   Using: Default conda channels
    call conda create -p "%BUILD_DIR%\Globalizer_env" python=3.12 -y
) else if "!SOURCE!"=="yandex" (
    echo   Using: Yandex mirrors
    call conda create -p "%BUILD_DIR%\Globalizer_env" ^
      -c https://mirrors.yandex.ru/mirrors/anaconda/cloud/conda-forge/ ^
      -c https://mirrors.yandex.ru/mirrors/anaconda/pkgs/main/ ^
      python=3.12 -y
) else if "!SOURCE!"=="aliyun" (
    echo   Using: Aliyun mirrors
    call conda create -p "%BUILD_DIR%\Globalizer_env" ^
      -c https://mirrors.aliyun.com/anaconda/pkgs/main/ ^
      -c https://mirrors.aliyun.com/anaconda/pkgs/free/ ^
      python=3.12 -y
) else if "!SOURCE!"=="tsinghua" (
    echo   Using: Tsinghua mirrors
    call conda create -p "%BUILD_DIR%\Globalizer_env" ^
      -c https://mirrors.tuna.tsinghua.edu.cn/anaconda/pkgs/main/ ^
      -c https://mirrors.tuna.tsinghua.edu.cn/anaconda/pkgs/free/ ^
      python=3.12 -y
)

if !errorlevel! equ 0 (
    echo   [OK] Environment created successfully
    endlocal & exit /b 0
) else (
    echo   [FAILED] Could not create environment from !SOURCE!
    endlocal & exit /b 1
)

REM ========================================
REM ERROR HANDLER
REM ========================================
:error
echo.
echo ========================================
echo  BUILD FAILED (errorlevel %errorlevel%)
echo  Смотрите сообщения выше.
echo ========================================
echo.
echo Troubleshooting:
echo  1. Check GCC_DIR (folder with gcc.exe, g++.exe, ninja.exe)
echo  2. Check your internet connection (conda / pip)
echo  3. For MPI install MS-MPI SDK or run: conda install msmpi
echo  4. Delete build_gcc_64\CMakeCache.txt and retry
echo.
cd /d "%START_DIR%"
pause
exit /b 1

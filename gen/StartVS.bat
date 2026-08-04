@echo off
setlocal enabledelayedexpansion

set START_DIR=%cd%
set ROOT_DIR=%~dp0\..

cd /d "%ROOT_DIR%"

if not exist "build_64" mkdir build_64
cd build_64

git submodule init
git submodule update

call conda init

echo.
echo ========================================
echo [1/6] Creating a Conda Environment...
echo ========================================

REM Список источников для попытки
set "SOURCES[0]=default"
set "SOURCES[1]=yandex"
set "SOURCES[2]=aliyun"
set "SOURCES[3]=tsinghua"

set "SOURCE_COUNT=4"
set "SUCCESS=0"

REM Попытка для каждого источника
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

echo.
echo ========================================
echo [2/6] Activating Conda Environment...
echo ========================================
call conda activate "%ROOT_DIR%\build_64\Globalizer_env"
if %errorlevel% neq 0 (
    echo [ERROR] Failed to activate conda environment!
    goto error
)

echo.
echo ========================================
echo [3/6] Installing Python packages...
echo ========================================
call pip install -r ..\requirements.txt
if %errorlevel% neq 0 (
    echo [WARNING] Some packages may have failed to install. Continuing...
)

echo.
echo ========================================
echo [4/6] Starting Intel OneAPI...
echo ========================================
call "C:\Program Files (x86)\Intel\oneAPI\setvars.bat" intel64 vs2022
if %errorlevel% neq 0 (
    echo [WARNING] Intel OneAPI initialization had issues. Continuing...
)

echo.
echo ========================================
echo [5/6] CMake Configuration...
echo ========================================
call cmake -G "Visual Studio 17 2022" -A x64 ^
  -DGLOBALIZER_BUILD_PROBLEMS=ON ^
  -DGLOBALIZER_USE_MP=ON ^
  -DGLOBALIZER_BUILD_GCGEN=ON ^
  -DGLOBALIZER_MAX_DIMENSION=130 ^
  -DGLOBALIZER_MAX_Number_Of_Function=70 ^
  -DGLOBALIZER_BUILD_TESTS=ON ^
  -DGLOBALIZER_USE_MPI=ON ^
  -DGLOBALIZER_MPI=intel ^
  -DGLOBALIZER_PYTHON=ON ^
  -DPython_FIND_DEBUG=OFF ^
  -Drastrigin_build=ON ^
  -DrastriginInt_build=ON ^
  -DX2_build=ON ^
  -Dpython_objective_build=ON ^
  -Dstronginc3_build=ON ^
  -DrastriginC1_build=ON ^
  -DiOptProblemSimple_build=ON ^
  -DPython_EXECUTABLE="%ROOT_DIR%\build_64\Globalizer_env\python.exe" ^
  -DPython_ROOT_DIR="%ROOT_DIR%\build_64\Globalizer_env" ^
  -DPython_FIND_STRATEGY=LOCATION ^
  ..

if %errorlevel% neq 0 goto error

echo.
echo ========================================
echo [6/6] Opening Visual Studio...
echo ========================================
if exist "globalizer.sln" (
    start "" "globalizer.sln"
) else if exist "globalizer.slnx" (
    start "" "globalizer.slnx"
) else (
    echo [ERROR] globalizer solution file not found!
    goto error
)

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
    call conda create -p "%ROOT_DIR%\build_64\Globalizer_env" python=3.12 -y
) else if "!SOURCE!"=="yandex" (
    echo   Using: Yandex mirrors
    call conda create -p "%ROOT_DIR%\build_64\Globalizer_env" ^
      -c https://mirrors.yandex.ru/mirrors/anaconda/cloud/conda-forge/ ^
      -c https://mirrors.yandex.ru/mirrors/anaconda/pkgs/main/ ^
      python=3.12 -y
) else if "!SOURCE!"=="aliyun" (
    echo   Using: Aliyun mirrors
    call conda create -p "%ROOT_DIR%\build_64\Globalizer_env" ^
      -c https://mirrors.aliyun.com/anaconda/pkgs/main/ ^
      -c https://mirrors.aliyun.com/anaconda/pkgs/free/ ^
      python=3.12 -y
) else if "!SOURCE!"=="tsinghua" (
    echo   Using: Tsinghua mirrors
    call conda create -p "%ROOT_DIR%\build_64\Globalizer_env" ^
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
echo  BUILD CONFIGURATION FAILED (errorlevel %errorlevel%)
echo  Смотрите сообщения выше.
echo ========================================
echo.
echo Troubleshooting:
echo  1. Check your internet connection
echo  2. Try running the script again (different mirror may work)
echo  3. Check firewall/proxy settings
echo  4. For offline installation, see README.md
echo.
cd /d "%START_DIR%"
pause
exit /b 1

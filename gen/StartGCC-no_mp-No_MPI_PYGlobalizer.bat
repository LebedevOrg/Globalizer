@echo off
REM ============================================================================
REM  StartGCC-no_mp-No_MPI_PYGlobalizer.bat
REM
REM  GCC (MinGW-w64, e.g. w64devkit) analogue of
REM  StartVS-no_mp-No_MPI_PYGlobalizer.bat: builds Globalizer as a Python
REM  extension module (PYDGlobalizer*.pyd) without Visual Studio.
REM    - no MPFR/GMP (GLOBALIZER_USE_MP=OFF)
REM    - no MPI      (GLOBALIZER_USE_MPI=OFF, mpi_stub is used)
REM    - pybind11 target PYDGlobalizer, Ninja + gcc/g++
REM
REM  Differences from the VS script:
REM    * generator Ninja (or "MinGW Makefiles") instead of Visual Studio;
REM    * the module is always built (there is no .sln to open).
REM
REM  Only requirements_build.txt is installed (what the build needs: pybind11,
REM  numpy). Examples that use plots or third-party problems need more, e.g.
REM  Example_simple.py: requirements.txt + iOptProblemSimple\requirements.txt.
REM
REM  Build dir: build_gcc_64_py, result: _bin\PYDGlobalizer.cp<ver>-win_amd64.pyd
REM  GCC: environment variable GCC_DIR (folder with gcc.exe),
REM       default <folder above the repository>\w64devkit\bin.
REM  Options: /clean - remove the CMake cache before configuring.
REM ============================================================================
setlocal EnableExtensions EnableDelayedExpansion

set "PY_VER=3.11"
set "PY_MODULE=PYDGlobalizer"
set "BUILD_TYPE=Release"

set "START_DIR=%cd%"
for %%I in ("%~dp0..") do set "ROOT_DIR=%%~fI"
set "BUILD_DIR=%ROOT_DIR%\build_gcc_64_py"
set "ENV_DIR=%BUILD_DIR%\Globalizer_env"
set "PY_EXE=%ENV_DIR%\python.exe"
set "TMP_OUT=%TEMP%\globalizer_setup_%RANDOM%.txt"

if "%GCC_DIR%"=="" (
    for %%I in ("%ROOT_DIR%\..\w64devkit\bin") do set "GCC_DIR=%%~fI"
)

set "DO_CLEAN=0"
for %%A in (%*) do (
    if /i "%%~A"=="/clean" set "DO_CLEAN=1"
)

echo.
echo Root dir : %ROOT_DIR%
echo Build dir: %BUILD_DIR%
echo Env dir  : %ENV_DIR%
echo GCC dir  : %GCC_DIR%

cd /d "%ROOT_DIR%"
if errorlevel 1 (
    set "FAIL_MSG=Cannot cd to root directory."
    goto error
)
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

echo.
echo ========================================
echo [1/7] Git submodules
echo ========================================
where git >nul 2>&1
if errorlevel 1 (
    echo [WARNING] git not found - skipping submodules.
) else (
    git -C "%ROOT_DIR%" submodule update --init --recursive
    if errorlevel 1 echo [WARNING] submodule update failed - third_party may be incomplete.
)

echo.
echo ========================================
echo [2/7] Conda environment (Python %PY_VER%)
echo ========================================
call :find_conda
if errorlevel 1 (
    set "FAIL_MSG=conda not found. Run from Anaconda Prompt or add condabin to PATH."
    goto error
)

if exist "%PY_EXE%" goto env_exists
for %%S in (conda-forge yandex tsinghua aliyun) do (
    if not exist "!PY_EXE!" call :create_env %%S
)
goto env_check
:env_exists
echo [OK] Environment already exists - reusing it.
:env_check
if not exist "%PY_EXE%" (
    set "FAIL_MSG=Failed to create conda environment from all channels."
    goto error
)

"%PY_EXE%" -c "import sys; v='%%d.%%d' %% sys.version_info[:2]; print('Python', sys.version.split()[0]); sys.exit(0 if v=='%PY_VER%' else 1)"
if errorlevel 1 (
    set "FAIL_MSG=Environment has a different Python than %PY_VER%."
    goto error
)

echo.
echo ========================================
echo [3/7] Python packages
echo ========================================
"%PY_EXE%" -m pip install --upgrade pip
REM Building the Python interface (PYDGlobalizer) needs only requirements_build.txt.
set "REQ_FILE=%ROOT_DIR%\requirements_build.txt"
echo Using "%REQ_FILE%"
"%PY_EXE%" -m pip install -r "%REQ_FILE%"
if errorlevel 1 (
    set "FAIL_MSG=pip install failed. pybind11 and numpy are required."
    goto error
)

set "PYBIND11_DIR="
"%PY_EXE%" -m pybind11 --cmakedir > "%TMP_OUT%" 2>nul
set /p PYBIND11_DIR=<"%TMP_OUT%"
del "%TMP_OUT%" >nul 2>&1
if not defined PYBIND11_DIR (
    set "FAIL_MSG=Cannot get pybind11 CMake dir."
    goto error
)
echo pybind11_DIR = !PYBIND11_DIR!

echo.
echo ========================================
echo [4/7] GCC toolchain
echo ========================================
if not exist "%GCC_DIR%\g++.exe" (
    set "FAIL_MSG=g++.exe not found in %GCC_DIR%. Set GCC_DIR to the folder with gcc.exe / g++.exe."
    goto error
)
set "PATH=%GCC_DIR%;%PATH%"
for /f "delims=" %%V in ('g++ -dumpfullversion') do echo [Found] GCC %%V
set "GENERATOR=Ninja"
where ninja >nul 2>&1
if errorlevel 1 (
    echo [WARNING] ninja not found, using "MinGW Makefiles".
    set "GENERATOR=MinGW Makefiles"
)
echo [Using] !GENERATOR!
where cmake >nul 2>&1
if errorlevel 1 (
    set "FAIL_MSG=cmake not found in PATH."
    goto error
)

echo.
echo ========================================
echo [5/7] CMake configuration
echo ========================================
if "%DO_CLEAN%"=="1" (
    echo /clean: removing CMake cache
    if exist "!BUILD_DIR!\CMakeCache.txt" del /q "!BUILD_DIR!\CMakeCache.txt"
    if exist "!BUILD_DIR!\CMakeFiles" rmdir /s /q "!BUILD_DIR!\CMakeFiles"
)

REM GLOBALIZER_PYBIND_VERSION is the CMake option.
REM PY_MODULE is the actual CMake target created by pybind11_add_module.
cmake -S "%ROOT_DIR%" -B "%BUILD_DIR%" -G "!GENERATOR!" ^
  -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
  -DCMAKE_C_COMPILER=gcc ^
  -DCMAKE_CXX_COMPILER=g++ ^
  -DGLOBALIZER_PYBIND_VERSION=ON ^
  -DGLOBALIZER_PYTHON=ON ^
  -DGLOBALIZER_USE_MP=OFF ^
  -DGLOBALIZER_USE_MPI=OFF ^
  -DGLOBALIZER_BUILD_PROBLEMS=ON ^
  -DGLOBALIZER_BUILD_GCGEN=ON ^
  -DGLOBALIZER_BUILD_TESTS=OFF ^
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
if errorlevel 1 (
    set "FAIL_MSG=CMake configuration failed. Rerun with /clean if the generator changed."
    goto error
)

echo.
echo ========================================
echo [6/7] Build %PY_MODULE% (%BUILD_TYPE%)
echo ========================================
cmake --build "%BUILD_DIR%" --target %PY_MODULE% --parallel %NUMBER_OF_PROCESSORS%
if errorlevel 1 (
    set "FAIL_MSG=Build of %PY_MODULE% failed."
    goto error
)

echo.
echo ========================================
echo [7/7] Import and collision test
echo ========================================
REM The module is linked statically with libstdc++/libgomp: only pythonXY.dll
REM is needed at run time, so PATH is not extended with GCC_DIR here.
set "PYTHONPATH=%ROOT_DIR%\_bin;%ROOT_DIR%\PYGlobalizer;%PYTHONPATH%"
"%PY_EXE%" -c "import os; import PYDGlobalizer as g; assert hasattr(g, 'solve'); assert hasattr(g, 'validate_problem'); path=os.path.normcase(os.path.abspath(g.__file__)); root=os.path.normcase(os.path.abspath(r'%ROOT_DIR%\_bin')); assert path.startswith(root + os.sep), 'Unexpected module path: ' + path; from PYProblem import PYProblem; print('[OK] PYDGlobalizer imported from', g.__file__); print('[OK] PYProblem imported from', PYProblem.__module__)"
if errorlevel 1 (
    set "FAIL_MSG=%PY_MODULE% was built but failed the import test. Check _bin and dependent DLLs."
    goto error
)

echo.
echo ========================================
echo DONE
echo ========================================
echo Run with:
echo   conda activate "%ENV_DIR%"
echo   cd /d "%ROOT_DIR%"
echo   python examples\example_scipy_interface.py
echo   python examples\example_optuna_interface.py
echo Example_simple.py / Example_SVC_2D.py additionally need:
echo   pip install -r requirements.txt -r third_party\Problems\Problems\iOptProblemSimple\requirements.txt
echo Python C++ module name: %PY_MODULE%
cd /d "%START_DIR%"
endlocal
exit /b 0

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
if /i "%SRC%"=="conda-forge" set "CH=-c conda-forge"
if /i "%SRC%"=="yandex" set "CH=-c https://mirrors.yandex.ru/mirrors/anaconda/cloud/conda-forge/"
if /i "%SRC%"=="tsinghua" set "CH=-c https://mirrors.tuna.tsinghua.edu.cn/anaconda/cloud/conda-forge/"
if /i "%SRC%"=="aliyun" set "CH=-c https://mirrors.aliyun.com/anaconda/cloud/conda-forge/"
if exist "%ENV_DIR%" rmdir /s /q "%ENV_DIR%"
call conda create -y -p "%ENV_DIR%" --override-channels %CH% python=%PY_VER% pip
if exist "%PY_EXE%" exit /b 0
exit /b 1

:error
echo.
echo ========================================
echo BUILD FAILED
echo !FAIL_MSG!
echo ========================================
cd /d "%START_DIR%"
pause
endlocal
exit /b 1

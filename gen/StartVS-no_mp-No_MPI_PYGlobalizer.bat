@echo off
REM ============================================================================
REM  StartVS-no_mp-No_MPI_PYGlobalizer.bat
REM
REM  Configures Globalizer as a Python extension module (PYDGlobalizer.pyd):
REM    - no MPFR/GMP (GLOBALIZER_USE_MP=OFF)
REM    - no MPI      (GLOBALIZER_USE_MPI=OFF, mpi_stub is used)
REM    - pybind11 target PYDGlobalizer
REM
REM  PYDGlobalizer is deliberately different from the PYGlobalizer/ helper
REM  directory. Therefore Python cannot confuse the binary extension with the
REM  helper modules (PYProblem.py, problem.py, trial.py).
REM ============================================================================
setlocal EnableExtensions EnableDelayedExpansion

set "PY_VER=3.11"
set "PY_MODULE=PYDGlobalizer"
set "BUILD_TESTS=OFF"
set "BUILD_CONFIG=Release"

set "START_DIR=%cd%"
for %%I in ("%~dp0..") do set "ROOT_DIR=%%~fI"
set "BUILD_DIR=%ROOT_DIR%\build_64_py"
set "ENV_DIR=%BUILD_DIR%\Globalizer_env"
set "PY_EXE=%ENV_DIR%\python.exe"
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
set "TMP_OUT=%TEMP%\globalizer_setup_%RANDOM%.txt"

set "DO_BUILD=0"
set "DO_CLEAN=0"
for %%A in (%*) do (
    if /i "%%~A"=="/build" set "DO_BUILD=1"
    if /i "%%~A"=="/clean" set "DO_CLEAN=1"
)

echo.
echo Root dir : %ROOT_DIR%
echo Build dir: %BUILD_DIR%
echo Env dir  : %ENV_DIR%

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

call conda activate "%ENV_DIR%"
if errorlevel 1 (
    set "FAIL_MSG=conda activate failed."
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
set "REQ_FILE=%ROOT_DIR%\requirements_build.txt"
if not exist "%REQ_FILE%" set "REQ_FILE=%ROOT_DIR%\requirements.txt"
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
echo [4/7] Visual Studio detection
echo ========================================
call :detect_vs
if errorlevel 1 (
    set "FAIL_MSG=Visual Studio with C++ tools was not found."
    goto error
)
echo [Found] Visual Studio !VS_VER!
echo [Using] !VS_GENERATOR!

echo.
echo ========================================
echo [5/7] CMake configuration
echo ========================================
call :select_cmake
if errorlevel 1 (
    set "FAIL_MSG=No CMake supports generator !VS_GENERATOR!."
    goto error
)

if "%DO_CLEAN%"=="1" (
    echo /clean: removing CMake cache
    if exist "!BUILD_DIR!\CMakeCache.txt" del /q "!BUILD_DIR!\CMakeCache.txt"
    if exist "!BUILD_DIR!\CMakeFiles" rmdir /s /q "!BUILD_DIR!\CMakeFiles"
)

REM GLOBALIZER_PYBIND_VERSION is the CMake option.
REM PY_MODULE is the actual CMake target created by pybind11_add_module.
"%CMAKE_EXE%" -S "%ROOT_DIR%" -B "%BUILD_DIR%" -G "%VS_GENERATOR%" -A x64 ^
  -DGLOBALIZER_PYBIND_VERSION=ON ^
  -DGLOBALIZER_PYTHON=ON ^
  -DGLOBALIZER_USE_MP=OFF ^
  -DGLOBALIZER_USE_MPI=OFF ^
  -DGLOBALIZER_BUILD_PROBLEMS=ON ^
  -DGLOBALIZER_BUILD_GCGEN=ON ^
  -DGLOBALIZER_BUILD_TESTS=%BUILD_TESTS% ^
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

if "%DO_BUILD%"=="0" goto open_vs

echo.
echo ========================================
echo [6/7] Build %PY_MODULE% (%BUILD_CONFIG%)
echo ========================================
"%CMAKE_EXE%" --build "%BUILD_DIR%" --config %BUILD_CONFIG% --target %PY_MODULE% --parallel
if errorlevel 1 (
    set "FAIL_MSG=Build of %PY_MODULE% failed."
    goto error
)

echo.
echo ========================================
echo [7/7] Import and collision test
echo ========================================
set "PYTHONPATH=%ROOT_DIR%\_bin;%ROOT_DIR%\PYGlobalizer;%PYTHONPATH%"
"%PY_EXE%" -c "import os; import PYDGlobalizer as g; assert hasattr(g, 'solve'); assert hasattr(g, 'validate_problem'); path=os.path.normcase(os.path.abspath(g.__file__)); root=os.path.normcase(os.path.abspath(r'%ROOT_DIR%\_bin')); assert path.startswith(root + os.sep), 'Unexpected module path: ' + path; from PYProblem import PYProblem; print('[OK] PYDGlobalizer imported from', g.__file__); print('[OK] PYProblem imported from', PYProblem.__module__)"
if errorlevel 1 (
    set "FAIL_MSG=%PY_MODULE% was built but failed the import test. Check _bin and dependent DLLs."
    goto error
)
goto done

:open_vs
echo.
echo ========================================
echo [6/7] Opening Visual Studio
echo ========================================
set "SLN="
for %%F in ("%BUILD_DIR%\*.slnx" "%BUILD_DIR%\*.sln") do if not defined SLN set "SLN=%%~fF"
if not defined SLN (
    set "FAIL_MSG=Solution file not found in build_64_py."
    goto error
)
echo !SLN!
echo Build target "%PY_MODULE%" in configuration %BUILD_CONFIG%.
start "" "!SLN!"

:done
echo.
echo ========================================
echo DONE
echo ========================================
echo Run with:
echo   conda activate "%ENV_DIR%"
echo   cd /d "%ROOT_DIR%"
echo   python examples\Example_simple.py
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
echo BUILD CONFIGURATION FAILED
echo !FAIL_MSG!
echo ========================================
cd /d "%START_DIR%"
pause
endlocal
exit /b 1

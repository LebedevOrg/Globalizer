@echo off
setlocal

set START_DIR=%cd%
set ROOT_DIR=%~dp0\..


cd /d "%ROOT_DIR%"

if not exist "build_64" mkdir build_64
cd build_64

git submodule init
git submodule update

call conda init

echo [1/5] Creating a Conda Environment...
::call conda create -p "%ROOT_DIR%\build_64\Globalizer_env" python=3.12 -y
call conda create -p "%ROOT_DIR%\build_64\Globalizer_env" -c conda-forge/label/debug python=3.12 -y

echo [2/5] activate a Conda Environment...
call conda activate "%ROOT_DIR%\build_64\Globalizer_env"

echo [3/5] Installing the library...
call pip install -r ..\third_party\Problems\Problems\iOptProblem\requirements.txt
call python "%ROOT_DIR%\third_party\Problems\Problems\iOptProblem\Tests\data\loader.py"
call python "%ROOT_DIR%\third_party\Problems\Problems\iOptProblem\SVC_3D_Transformator.py"

echo [4/5] Start Intell OneAPI

call "C:\Program Files (x86)\Intel\oneAPI\setvars.bat" intel64 vs2022

echo [5/5] CMake Configuration...
call cmake -DGLOBALIZER_BUILD_PROBLEMS=ON  -DGLOBALIZER_BUILD_GCGEN=ON -DBUILD_ALL_TASK=ON -DGLOBALIZER_MAX_DIMENSION=130 -DGLOBALIZER_MAX_Number_Of_Function=70 -DGLOBALIZER_BUILD_TESTS=ON -DGLOBALIZER_USE_MPI=ON -DGLOBALIZER_MPI=intel -DGLOBALIZER_PYTHON=ON -DPython_FIND_DEBUG=OFF ..

if %errorlevel% neq 0 goto error

echo [6/6] Opening Visual Studio...
if exist "globalizer.sln" (call "globalizer.sln") else if exist "globalizer.slnx" (call "globalizer.slnx") else echo Error: globalizer not found!


cd /d "%START_DIR%"
echo.

exit /b 0


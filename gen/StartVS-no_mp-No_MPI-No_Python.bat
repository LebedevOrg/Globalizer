@echo off
setlocal

set START_DIR=%cd%
set ROOT_DIR=%~dp0\..


cd /d "%ROOT_DIR%"

if not exist "build_64_nmmp" mkdir build_64_nmmp
cd build_64_nmmp

git submodule init
git submodule update


echo [1/2] CMake Configuration...
call cmake -DGLOBALIZER_BUILD_PROBLEMS=ON -DGLOBALIZER_MAX_DIMENSION=130 -DGLOBALIZER_MAX_Number_Of_Function=70 -DGLOBALIZER_PYTHON=OFF -DGLOBALIZER_BUILD_GCGEN=ON -Drastrigin_build=ON -DrastriginInt_build=ON -DX2_build=ON -Dstronginc3_build=ON -DrastriginC1_build=ON -DGLOBALIZER_BENCHMARKS_PYTHON=OFF ..

if %errorlevel% neq 0 goto error

echo [2/2] Opening Visual Studio...
if exist "globalizer.sln" (call "globalizer.sln") else if exist "globalizer.slnx" (call "globalizer.slnx") else echo Error: globalizer not found!

cd /d "%START_DIR%"
echo.

exit /b 0


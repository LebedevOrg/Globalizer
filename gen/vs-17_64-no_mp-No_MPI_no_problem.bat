set START_DIR=%cd%
set ROOT_DIR=%~dp0\..

git submodule init
git submodule update

cd %ROOT_DIR%
if not exist build_64_NoMPI_NoProblem mkdir build_64_NoMPI_NoProblem
cd build_64_NoMPI_NoProblem
cmake -G "Visual Studio 17 2022" -DGLOBALIZER_USE_MPI=OFF -DGLOBALIZER_USE_MP=OFF -DGLOBALIZER_USE_CUDA=OFF ..

globalizer.sln

cd %START_DIR%

pause

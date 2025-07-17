set START_DIR=%cd%
set ROOT_DIR=%~dp0\..
cd %ROOT_DIR%
if not exist build_64_NoMPI mkdir build_64_NoMPI
cd build_64_NoMPI
cmake -G "Visual Studio 17 2022" -DEXAMIN_USE_MPI=OFF -DEXAMIN_USE_MP=OFF -DEXAMIN_USE_CUDA=OFF -DEXAMIN_BUILD_TESTS=OFF ..

globalizer.sln

cd %START_DIR%

pause

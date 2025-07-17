set START_DIR=%cd%
set ROOT_DIR=%~dp0\..
cd %ROOT_DIR%
if not exist build_64_NoMPI mkdir build_64_NoMPI
cd build_64_NoMPI
cmake -G "Visual Studio 17 2022" -DEXAMIN_USE_MPI=OFF -DEXAMIN_USE_MP=ON -DEXAMIN_USE_CUDA=OFF -DEXAMIN_BUILD_TESTS=ON -DEXAMIN_BUILD_PLOTTER=OFF -DEXAMIN_USE_NLOPT=OFF -DEXAMIN_USE_TOROPOV=OFF -Dgrishagin_build=ON -DGCGen_build=ON  -Dgkls_build=ON -Drastrigin_build=ON -DX2_build=ON -DBanana_build=ON ..

globalizer.sln

cd %START_DIR%

pause

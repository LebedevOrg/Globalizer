set START_DIR=%cd%
set ROOT_DIR=%~dp0\..

git submodule init
git submodule update

cd %ROOT_DIR%
if not exist build_64 mkdir build_64
cd build_64
cmake -G "Visual Studio 17 2022" -DGLOBALIZER_USE_MPI=ON -DGLOBALIZER_MPI=intel -DGLOBALIZER_USE_MP=OFF -DGLOBALIZER_USE_CUDA=OFF -DGLOBALIZER_BUILD_TESTS=ON -DGLOBALIZER_BUILD_PROBLEMS=ON -DGLOBALIZER_BUILD_GCGEN=ON -Drastrigin_build=ON -DrastriginInt_build=ON -DX2_build=ON -Dpython_objective_build=ON -Dstronginc3_build=ON -DrastriginC1_build=ON -DiOptProblemSimple_build=ON ..


cd %START_DIR%

pause

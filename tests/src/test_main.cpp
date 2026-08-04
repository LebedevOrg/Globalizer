#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <gtest/gtest.h>
#include <mpi.h>

#include "Parameters.h"
#include "Extended.h"

// Единая точка инициализации глобального состояния для всех тестов.
class GlobalizerTestEnvironment : public ::testing::Environment
{
public:
  void SetUp() override
  {
    static char arg0[] = "tests";
    char* argv[] = { arg0, nullptr };
    int argc = 1;
    parameters.Init(argc, argv, false);   // ЕДИНСТВЕННЫЙ вызов Init на процесс
    Extended::SetTypeID(etDouble);         // фиксируем тип Extended глобально
  }
};

int main(int argc, char** argv)
{
  MPI_Init(&argc, &argv);
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::AddGlobalTestEnvironment(new GlobalizerTestEnvironment());
  return RUN_ALL_TESTS();
}
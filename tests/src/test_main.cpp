#define _CRT_SECURE_NO_WARNINGS

#include <gtest/gtest.h>
#include <mpi.h>
#include <string>

int main(int argc, char **argv)
{
  MPI_Init(&argc, &argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
  //MPI_Finalize(); this line is unreachable
}

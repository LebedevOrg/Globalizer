#include "mpi.h"

int MPI_Init(int *pargc, char ***pargv)
{
  return MPI_SUCCESS;
}

int MPI_Comm_rank( MPI_Comm comm, int *rank)
{
  *rank = 0;
  return MPI_SUCCESS;
}

int MPI_Abort(MPI_Comm, int)
{
  return MPI_SUCCESS;
}

int MPI_Initialized(int *flag)
{
  *flag = 1;
  return MPI_SUCCESS;
}

int MPI_Comm_size(MPI_Comm, int *size)
{
  *size = 1;
  return MPI_SUCCESS;
}

int MPI_Finalize(void)
{
  return MPI_SUCCESS;
}

int MPI_Send(void*, int, MPI_Datatype, int, int, MPI_Comm)
{
  return MPI_SUCCESS;
}
int MPI_Recv(void*, int, MPI_Datatype, int, int, MPI_Comm, MPI_Status *)
{
  return MPI_SUCCESS;
}

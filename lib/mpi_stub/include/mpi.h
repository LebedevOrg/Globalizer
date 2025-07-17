#ifndef STUB_MPI_H
#define STUB_MPI_H

#define MPI_SUCCESS          0      /* Successful return code */

typedef int MPI_Comm;
#define MPI_COMM_WORLD 91

typedef int MPI_Datatype;
#define MPI_CHAR           ((MPI_Datatype)1)
#define MPI_UNSIGNED_CHAR  ((MPI_Datatype)2)
#define MPI_BYTE           ((MPI_Datatype)3)
#define MPI_SHORT          ((MPI_Datatype)4)
#define MPI_UNSIGNED_SHORT ((MPI_Datatype)5)
#define MPI_INT            ((MPI_Datatype)6)
#define MPI_UNSIGNED       ((MPI_Datatype)7)
#define MPI_LONG           ((MPI_Datatype)8)
#define MPI_UNSIGNED_LONG  ((MPI_Datatype)9)
#define MPI_FLOAT          ((MPI_Datatype)10)
#define MPI_DOUBLE         ((MPI_Datatype)11)
#define MPI_LONG_DOUBLE    ((MPI_Datatype)12)
#define MPI_LONG_LONG_INT  ((MPI_Datatype)13)

#define MPI_PROC_NULL   (-1)
#define MPI_ANY_SOURCE 	(-2)
#define MPI_ROOT        (-3)
#define MPI_ANY_TAG     (-1)

/* The order of these elements must match that in mpif.h, mpi_f08_types.f90,
   and mpi_c_interface_types.f90 */
typedef struct MPI_Status {
    int count_lo;
    int count_hi_and_cancelled;
    int MPI_SOURCE;
    int MPI_TAG;
    int MPI_ERROR;
} MPI_Status;

int MPI_Init(int *, char ***);
int MPI_Finalize(void);
int MPI_Initialized(int *);
int MPI_Abort(MPI_Comm, int);

int MPI_Comm_size(MPI_Comm, int *);
int MPI_Comm_rank(MPI_Comm, int *);

int MPI_Send(void*, int, MPI_Datatype, int, int, MPI_Comm);
int MPI_Recv(void*, int, MPI_Datatype, int, int, MPI_Comm, MPI_Status *);

#endif

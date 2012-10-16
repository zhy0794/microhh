#ifndef PARALLEL
#include <cstdio>
#include <sys/time.h>
#include <mpi.h>
#include "grid.h"
#include "defines.h"
#include "mpiinterface.h"

cmpi::cmpi()
{
  initialized = false;
  allocated   = false;
}

cmpi::~cmpi()
{
  if(allocated)
    delete[] reqs;

  if(mpiid == 0) std::printf("Finished run on %d processes\n", nprocs);

  if(initialized)
    MPI_Finalize();
}

int cmpi::readinifile(cinput *inputin)
{
  int n = 0;

  n += inputin->getItem(&npx, "grid", "npx", 1);
  n += inputin->getItem(&npy, "grid", "npy", 1);

  if(n > 0)
    return 1;
  
  return 0;
}

int cmpi::startup()
{
  int n;

  // initialize the MPI
  n = MPI_Init(NULL, NULL);
  if(checkerror(n))
    return 1;

  initialized = true;

  // set the rank of the only process to 0
  mpiid = 0;

  // set the number of processes to 1
  nprocs = 1;

  if(mpiid == 0) std::printf("Starting run on %d processes\n", nprocs);

  return 0;
}

int cmpi::init()
{
  int n;

  if(nprocs != npx*npy)
  {
    if(mpiid == 0) std::printf("ERROR nprocs = %d does not equal npx*npy = %d*%d\n", nprocs, npx, npy);
    return 1;
  }

  int dims    [2] = {npy, npx};
  int periodic[2] = {true, true};

  // define the dimensions of the 2-D grid layout
  n = MPI_Dims_create(nprocs, 2, dims);
  if(checkerror(n))
    return 1;

  // create a 2-D grid communicator that is optimized for grid to grid transfer
  n = MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periodic, true , &commxy);
  if(checkerror(n))
    return 1;
  n = MPI_Comm_rank(commxy, &mpiid);
  if(checkerror(n))
    return 1;

  // retrieve the x- and y-coordinates in the 2-D grid for each process
  int mpicoords[2];
  n = MPI_Cart_coords(commxy, mpiid, 2, mpicoords);
  if(checkerror(n))
    return 1;

  // set the coordinates to 0
  mpicoordx = 0;
  mpicoordy = 0;

  int dimx[2] = {false, true };
  int dimy[2] = {true , false};

  n = MPI_Cart_sub(commxy, dimx, &commx);
  if(checkerror(n))
    return 1;
  n = MPI_Cart_sub(commxy, dimy, &commy);
  if(checkerror(n))
    return 1;

  // find out who are the neighbors of this process to facilitate the communication routines
  n = MPI_Cart_shift(commxy, 1, 1, &nwest , &neast );
  if(checkerror(n))
    return 1;
  n = MPI_Cart_shift(commxy, 0, 1, &nsouth, &nnorth);
  if(checkerror(n))
    return 1;

  // create the requests arrays for the nonblocking sends
  int npmax;
  npmax = std::max(npx, npy);

  // have at least as many communicators as prognostic variables
  npmax = std::max(npmax, 8*4);
  reqs  = new MPI_Request[npmax*2];
  reqsn = 0;

  allocated = true;

  return 0;
}

double cmpi::gettime()
{
  timeval timestruct;
  gettimeofday(&timestruct, NULL);
  double time;
  time = (double)timestruct.tv_sec + (double)timestruct.tv_usec*1.e-6;
  return time;
}

int cmpi::checkerror(int n)
{
  char errbuffer[MPI_MAX_ERROR_STRING];
  int errlen;

  if(n != MPI_SUCCESS)
  {
    MPI_Error_string(n, errbuffer, &errlen);
    std::printf("ERROR MPI %s\n", errbuffer);
    return 1;
  }

  return 0;
}

int cmpi::waitall()
{
  // wait for MPI processes and reset the number of pending requests
  // MPI_Waitall(reqsn, reqs, MPI_STATUSES_IGNORE);
  // reqsn = 0;

  return 0;
}

// all broadcasts return directly, because there is nothing to broadcast
int cmpi::broadcast(char *data, int datasize)
{
  return 0;
}

// overloaded broadcast functions
int cmpi::broadcast(int *data, int datasize)
{
  return 0;
}

int cmpi::broadcast(unsigned long *data, int datasize)
{
  return 0;
}

int cmpi::broadcast(double *data, int datasize)
{
  return 0;
}
#endif

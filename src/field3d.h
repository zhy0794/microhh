#ifndef FIELD3D
#define FIELD3D

#include "grid.h"

class cfield3d
{
  public:
    // functions
    cfield3d(cgrid *, double *, char *);
    ~cfield3d();
    int boundary_bottop(int);
    int boundary_cyclic();
    int dump(int);

    // variables
    double *data;
    char *name;

  private:
    cgrid *grid;
};
#endif


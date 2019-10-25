/* ----------------------------------------------------------------------
   LAMMPS - Large-scale Atomic/Molecular Massively Parallel Simulator
   http://lammps.sandia.gov, Sandia National Laboratories
   Steve Plimpton, sjplimp@sandia.gov

   Copyright (2003) Sandia Corporation.  Under the terms of Contract
   DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains
   certain rights in this software.  This software is distributed under
   the GNU General Public License.

   See the README file in the top-level LAMMPS directory.
------------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
   This file serves as an interface to the AENET package.

   Copyright (C) 2012-2019 Nongnuch Artrith and Alexander Urban
------------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
   Contributing author: Michael S. Chen, Markland Group, Stanford University
------------------------------------------------------------------------- */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "pair_aenet.h"
#include "atom.h"
#include "comm.h"
#include "force.h"
#include "neighbor.h"
#include "neigh_list.h"
#include "neigh_request.h"
#include "update.h"
#include "integrate.h"
#include "respa.h"
#include "math_const.h"
#include "memory.h"
#include "error.h"

extern "C"
{
#include "aenet.h"
}
  
using namespace LAMMPS_NS;
using namespace MathConst;

/* ---------------------------------------------------------------------- */

PairAenet::PairAenet(LAMMPS *lmp) : Pair(lmp)
{
  
}

/* ---------------------------------------------------------------------- */

PairAenet::~PairAenet()
{
  aenet_final(&stat);
  if (stat != 0) {
    sprintf(error_buffer, "aenet error code: %d", stat);      
    error->all(FLERR,error_buffer);
  }

  if (allocated) {
    memory->destroy(setflag);
    memory->destroy(cutsq);

    memory->destroy(atom_types);
    memory->destroy(pot_files);
  }
}

/* ---------------------------------------------------------------------- */

void PairAenet::compute(int eflag, int vflag)
{
  int i,ii,inum,itype;
  int j,jj,jnum;
  int *ilist,*numneigh,**firstneigh;

  if (eflag || vflag) ev_setup(eflag,vflag);
  else evflag = vflag_fdotr = 0;

  double **x = atom->x;
  double **f = atom->f;
  int *type = atom->type;
  int nlocal = atom->nlocal;

  inum = list->inum;
  ilist = list->ilist;
  numneigh = list->numneigh;
  firstneigh = list->firstneigh;

  // loop over neighbors of my atoms
  for (ii = 0; ii < inum; ii++) {
    double E_i = 0.0;
    i = ilist[ii];
    itype = type[i];
    double icoo[3] = { x[i][0], x[i][1], x[i][2] };
      
    jnum = numneigh[i];
    int jtype[jnum];
    int jlist[jnum];
    double jcoo[jnum][3];
    for (jj = 0; jj < jnum; jj++) {
      j = firstneigh[i][jj];
      j &= NEIGHMASK;
      jlist[jj] = j + 1;
      jtype[jj] = type[j];
      jcoo[jj][0] = x[j][0];
      jcoo[jj][1] = x[j][1];
      jcoo[jj][2] = x[j][2];
    }

    aenet_atomic_energy_and_forces(icoo, itype, i+1,
				   jnum, (double*)jcoo, jtype,
				   jlist, atom->natoms, &E_i,
				   (double*)&(f[0][0]), &stat);
    
    if (stat != 0) {
      sprintf(error_buffer, "aenet error code: %d", stat);      
      error->all(FLERR,error_buffer);
    }

    if (evflag) ev_tally(0,0,nlocal, 1,
			 E_i,0.0,0.0,0.0,0.0,0.0);
    
  }
  
  if (vflag_fdotr) virial_fdotr_compute();
  
}


/* ----------------------------------------------------------------------
   allocate all arrays
------------------------------------------------------------------------- */

void PairAenet::allocate()
{
  allocated = 1;
  int n = atom->ntypes;

  memory->create(setflag,n+1,n+1,"pair:setflag");
  for (int i = 1; i <= n; i++)
    for (int j = i; j <= n; j++)
      setflag[i][j] = 0;

  memory->create(cutsq,n+1,n+1,"pair:cutsq");

}

/* ----------------------------------------------------------------------
   global settings
------------------------------------------------------------------------- */

void PairAenet::settings(int narg, char **arg)
{
  if (narg != atom->ntypes) error->all(FLERR,"# of pair_style inputs != atoms->ntypes");

  memory->create(atom_types, (atom->ntypes)+1, 4, "pair:atom_types");
  memory->create(pot_files, (atom->ntypes)+1, 64, "pair:pot_files");

  for (int i = 0; i < narg; i++) {
    sprintf(pot_files[i], "%s", arg[i]);

    char *pch = strtok(arg[i], ".");
    sprintf(atom_types[i], "%s", pch);
  }

}

/* ----------------------------------------------------------------------
   set coeffs for one or more type pairs
------------------------------------------------------------------------- */

void PairAenet::coeff(int narg, char **arg)
{
  if (narg != 2)
    error->all(FLERR,"Incorrect args for pair coefficients");
  if (!allocated) allocate();

  int ilo,ihi,jlo,jhi;
  force->bounds(FLERR, arg[0],atom->ntypes,ilo,ihi);
  force->bounds(FLERR, arg[1],atom->ntypes,jlo,jhi);
  
  int count = 0;
  for (int i = ilo; i <= ihi; i++) {
    for (int j = MAX(jlo,i); j <= jhi; j++) {
      setflag[i][j] = 1;
      count++;
    }
  }

  if (count == 0) error->all(FLERR,"Incorrect args for pair coefficients");

}

/* ----------------------------------------------------------------------
   init specific to this pair style
------------------------------------------------------------------------- */

void PairAenet::init_style()
{
  int irequest = neighbor->request(this,instance_me);
  neighbor->requests[irequest]->half = 0;
  neighbor->requests[irequest]->full = 1;
  
  if (!initialized) {
    aenet_init(atom->ntypes, atom_types, &stat);
    initialized = true;
  }
  if (stat != 0) {
      sprintf(error_buffer, "aenet error code: %d", stat);      
      error->all(FLERR,error_buffer);
  }
  if (!aenet_all_loaded()) {    
    for (int i = 0; i < atom->ntypes; i++) {
      aenet_load_potential(i+1, pot_files[i], &stat);
    }

    if (stat != 0) {
      sprintf(error_buffer, "aenet error code: %d", stat);      
      error->all(FLERR,error_buffer);
    }
  }

  cut_global = aenet_Rc_max;
  
}

/* ----------------------------------------------------------------------
   init for one type pair i,j and corresponding j,i
------------------------------------------------------------------------- */

double PairAenet::init_one(int i, int j)
{
  return cut_global;
}

/* ----------------------------------------------------------------------
   proc 0 writes to restart file
------------------------------------------------------------------------- */

void PairAenet::write_restart(FILE *fp)
{
  write_restart_settings(fp);

  int i,j;
  for (i = 1; i <= atom->ntypes; i++) {
    for (j = i; j <= atom->ntypes; j++) {
      fwrite(&setflag[i][j],sizeof(int),1,fp);
      if (setflag[i][j]) {}
    }
  }
}

/* ----------------------------------------------------------------------
   proc 0 reads from restart file, bcasts
------------------------------------------------------------------------- */

void PairAenet::read_restart(FILE *fp)
{
  read_restart_settings(fp);
  allocate();

  int i,j;
  int me = comm->me;
  for (i = 1; i <= atom->ntypes; i++) {
    for (j = i; j <= atom->ntypes; j++) {
      if (me == 0) fread(&setflag[i][j],sizeof(int),1,fp);
      MPI_Bcast(&setflag[i][j],1,MPI_INT,0,world);
      if (setflag[i][j]) {}
    }
  }
}

/* ----------------------------------------------------------------------
   proc 0 writes to restart file
------------------------------------------------------------------------- */

void PairAenet::write_restart_settings(FILE *fp)
{
  fwrite(&offset_flag,sizeof(int),1,fp);
  fwrite(&mix_flag,sizeof(int),1,fp);
  fwrite(&tail_flag,sizeof(int),1,fp);
}

/* ----------------------------------------------------------------------
   proc 0 reads from restart file, bcasts
------------------------------------------------------------------------- */

void PairAenet::read_restart_settings(FILE *fp)
{
  int me = comm->me;
  if (me == 0) {
    fread(&offset_flag,sizeof(int),1,fp);
    fread(&mix_flag,sizeof(int),1,fp);
    fread(&tail_flag,sizeof(int),1,fp);
  }
  MPI_Bcast(&offset_flag,1,MPI_INT,0,world);
  MPI_Bcast(&mix_flag,1,MPI_INT,0,world);
  MPI_Bcast(&tail_flag,1,MPI_INT,0,world);
}

/* ---------------------------------------------------------------------- */

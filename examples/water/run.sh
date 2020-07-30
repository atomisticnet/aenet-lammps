#!/bin/bash

# Specify location where LAMMPS (compiled with aenet) is installed
LAMMPSROOT=

# Add any additional setup as required. Assumes compiled LAMMPS executable is lmp_mpi
export LD_LIBRARY_PATH=${LAMMPSROOT}/lib/aenet/lib${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}
exe=${LAMMPSROOT}/src/lmp_mpi

${exe} -in md.lmp -log log.lammps


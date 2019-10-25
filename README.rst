=====================
ænet-LAMMPS Interface
=====================

This directory provides all files needed to link the LAMMPS_ molecular
dynamics code with ``aenetLib``.

The present interface code has been tested with LAMMPS version August 7, 2019.

.. _LAMMPS: https://lammps.sandia.gov/

Prerequisites
-------------

To be linked with Tinker, ænet has to be compiled as library.  Build the library by running

.. code-block:: sh

  make -f ./makefiles/Makefile.gfortran_openblas_serial lib

in the ænet ``src`` directory.

Building LAMMPS with ænet support
---------------------------------

1. Copy the USER-AENET folder to $LAMMPSSRC/src/
2. Create the folder $LAMMPSSRC/lib/aenet, $LAMMPSSRC/lib/aenet/include, $LAMMPSSRC/lib/aenet/lib and make sure that
  - aenet.h is in $LAMMPSSRC/lib/aenet/include/
  - the necessary library files (i.e. libaenet.a, libaenet.so, liblbfgb.a, liblbfgsb.so) are in $LAMMPSSRC/lib/aenet/lib
3. In $LAMMPSSRC/src/, run 'make yes-user-aenet' to enable this package
  - NOTE: this assumes that aenetLib was compiled using Makefile.gfortran_openblas_serial. If this is not the case, then please make modifications to the Install.sh filefound in this folder as needed.
4. Compile LAMMPS as usual (e.g. 'make mpi')


Running LAMMPS simulations using ænet potentials
------------------------------------------------

A partial input example for water:

.. code-block:: highlight

  units electron
  mass 1 15.999491
  mass 2  1.007825
  pair_style aenet O.25t-25t.ann H.25t-25t.ann
  pair_coeff * *

The user must specify the aenet fits such that the first fit corresponds to element 1 and so on.

The selected LAMMPS units should match the units of the training set data that was fitted by aenet (i.e. if using 'units electron', the aenet training set must have been specified in terms of Bohrs and Hartrees)

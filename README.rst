=====================
ænet-LAMMPS Interface
=====================

This directory provides all files needed to link the LAMMPS_ molecular
dynamics code with ``aenetLib``.

The present interface code has been tested with LAMMPS version August 7, 2019.

.. _LAMMPS: https://lammps.sandia.gov/

The interface relies on a serial build of the aenet library for predicting energies and forces and leaves LAMMPS to handle parallelization. Hence the usual MPI and OpenMP implementations of LAMMPS are compatible with this interface.

Prerequisites
-------------

A serial aenet library is required and can be compiled by calling one of the following calls:

.. code-block:: sh
  
  make -f ./makefiles/Makefile.gfortran_serial lib
  make -f ./makefiles/Makefile.gfortran_openblas_serial lib
  make -f ./makefiles/Makefile.ifort_serial lib

in the ænet ``src`` directory.

Building LAMMPS with ænet support
---------------------------------
After compiling the aenet library files and downloading the aenet-LAMMPS interface from this repository:

1. Copy the USER-AENET folder to $LAMMPSSRC/src/ where $LAMMPSSRC is the path to your LAMMPS codebase
2. Replace $LAMMPSSRC/src/Makefile with ‘Makefile-aenetlammps‘
3. In $LAMMPSSRC/src/USER-AENET/ you will find multiple ‘Install.sh-*‘ files. Select the file analogous to which makefile you used to compile the aenet library and copy it into a new file in the same directory called ‘Install.sh‘. In other words, if you compiled the aenet library using ‘Makefile.gfortran_serial‘ then you should select ‘Install.sh-gfortran_serial‘.
  - Note that if you are using the intel compilers, you may have to recompile the aenet library using ‘Makefile.ifort_serial‘ but editing the makefile so as to remove the ‘-check-bounds‘ flag.
4. Create the folders $LAMMPSSRC/lib/aenet/src and $LAMMPSSRC/lib/aenet/lib and make sure to copy or link over:
  - aenet.h to $LAMMPSSRC/lib/aenet/include/
  - library files (i.e. libaenet.a, libaenet.so, liblbfg.a, liblbfg.so) to $LAMMPSSRC/lib/aenet/lib/
5. In $LAMMPSSRC/src/, run 'make yes-user-aenet' to enable the interface package. Be sure to enable any other packages you want to use as well
6. Compile LAMMPS as usual (e.g. go to $LAMMPSSRC and call ‘make mpi’)


Running LAMMPS simulations using ænet potentials
------------------------------------------------

The aenet library files, as well as any other dependencies, need to be properly loaded (i.e. on the $LD_LIBRARY_PATH). The LAMMPS input script also needs to be configured so as to use the ‘aenet’ pair style and to specify which neural network parameter files to use. A partial example for water is provided below:

.. code-block:: highlight

  units metal
  mass 1  1.007825
  mass 2 15.999491
  pair_style aenet O.25t-25t.ann H.25t-25t.ann
  pair_coeff * *

The user must specify the aenet fits such that the first fit corresponds to element 1 and so on.

The parameter files (*.ann files) need to be enumerated in a specific order. In the example above element 1 is designated to be oxygen and element 2 is hydrogen. Consequently, the aenet parameter file for oxygen (i.e. O.25t-25t.ann) needs to be specified first and then followed by the parameter file for hydrogen.

The selected LAMMPS units should match the units of the training data that was fitted by aenet. In the example above, ‘metal’ units are used meaning that the aenet potentials were fitted to training data for which the energies were reported in electron-volts and the positions in Angstroms. If instead the training data used Hartrees and Bohrs, then the corresponding LAMMPS units should be ‘electron’.


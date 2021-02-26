=====================
ænet-LAMMPS Interface
=====================

If you make use of the aenet-LAMMPS interface, please cite the following reference:

M.S. Chen, T. Morawietz, T.E. Markland, N. Artrith,
AENET-LAMMPS and AENET-TINKER: Interfaces for Accurate and Efficient Molecular Dynamics Simulations with Machine Learning Potentials, in preparation (2020).

The database can be obtained from the Materials Cloud repository, DOI: https://doi.org/10.24435/materialscloud:dx-ct

ænet-LAMMPS
-------------
This directory provides all files needed to link the LAMMPS_ molecular
dynamics code with ``aenetLib``.

The present interface code has been tested with the LAMMPS version released on February 4, 2020 (Download_).

.. _LAMMPS: https://lammps.sandia.gov/
.. _Download: https://lammps.sandia.gov/tars/lammps-4Feb20.tar.gz 

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

1. Copy the USER-AENET folder to $LAMMPSROOT/src/ where $LAMMPSROOT is the path to your LAMMPS codebase

2. Replace $LAMMPSROOT/src/Makefile with ‘Makefile-aenetlammps‘

3. In $LAMMPSROOT/src/USER-AENET/ you will find multiple ‘Install.sh-*‘ files. Select the file analogous to which makefile you used to compile the aenet library and copy it into a new file in the same directory called ‘Install.sh‘. In other words, if you compiled the aenet library using ‘Makefile.gfortran_serial‘ then you should select ‘Install.sh-gfortran_serial‘.
  - **Note:** if you are using the Intel compilers, you may have to recompile the aenet library using 
              ‘Makefile.ifort_serial‘ but editing the makefile so as to remove the ‘-check-bounds‘ flag.
  - **Note:** if you are using the Intel compilers, you might have to add the `ifcore` library with to the `PKG_LIB` 
              line in the file `$LAMMPSROOT/src/Makefile.package`.  At the end of the line, add `-lifcore`.
  - **Note:** it might be desirable to link the ænet libraries statically, so that no environment paths need
              to be set.  To link statically, change `-laenet -llbfgsb` to 
              `../../lib/aenet/lib/libaenet.a ../../lib/aenet/lib/liblbfgsb.a` in the file `Makefile.package`
  
4. Create the folders $LAMMPSROOT/lib/aenet/include and $LAMMPSROOT/lib/aenet/lib and make sure to copy or link over:
  - `aenet.h` to `$LAMMPSROOT/lib/aenet/include/`
  - library files (i.e. `libaenet.a`, `libaenet.so`, `liblbfgsb.a`, `liblbfgsb.so`) to `$LAMMPSROOT/lib/aenet/lib/`
  - **Note:** in `aenet.h`, change `_Bool` to `bool` to make the header file compatible with C++

5. In $LAMMPSROOT/src/, run 'make yes-user-aenet' to enable the interface package. Be sure to enable any other packages you want to use as well

6. Compile LAMMPS as usual (e.g. go to $LAMMPSROOT/src and call ‘make mpi’)
  - **Note:** when compilation with the Intel compilers is desired, use `make intel_cpu_openmpi` or similar.
              (For a list of available targets, run `make` without any option.)


Running LAMMPS simulations using ænet potentials
------------------------------------------------

The aenet library files, as well as any other dependencies, need to be properly loaded (i.e. on the $LD_LIBRARY_PATH). The LAMMPS input script also needs to be configured so as to use the ‘aenet’ pair style and to specify which neural network parameter files to use. A partial example for water is provided below:

.. code-block:: highlight

  units metal
  mass 1  1.007825
  mass 2 15.999491
  pair_style aenet H.ann O.ann
  pair_coeff * *

The user must specify the aenet fits such that the first fit corresponds to element 1 and so on.

The parameter files (*.ann files) need to be enumerated in a specific order. In the example above element 1 is designated to be oxygen and element 2 is hydrogen. Consequently, the aenet parameter file for oxygen (i.e. O.ann) needs to be specified first and then followed by the parameter file for hydrogen.

The selected LAMMPS units should match the units of the training data that was fitted by aenet. In the example above, ‘metal’ units are used meaning that the aenet potentials were fitted to training data for which the energies were reported in electron-volts and the positions in Angstroms. If instead the training data used Hartrees and Bohrs, then the corresponding LAMMPS units should be ‘electron’.


### Input Files for Molecular Dynamics (MD) Simulation using the aenet-LAMMPS Interface

* `H.ann`, `O.ann`, `train.out`

   ANN potential files and associated training output. Note that the fits are those of epoch 3649.

* `01_Start/firstframe.start`

   Initial frame for starting MD simulation. 64 water molecules.

* `md.lmp`

   LAMMPS input file with parameters for the MD simulation. Configured to output
   trajectory, velocities, forces, and energies to files found in 02_Traj/ and
   to run a simulation at 300K for 100 steps (200fs).

* `02_Traj/`

  Contains outputs for a reference run using these same ANN potential files.

* `run.sh`

  Barebones bash run script.

### Input Files for Molecular Dynamics (MD) Simulation using the aenet-LAMMPS Interface

* `H.25t-25t.ann-04128`, `O.25t-25t.ann-04128`, `train.out`

   ANN potential files and associated training output.

* `01_Start/firstframe.start`

   Initial frame for starting MD simulation. 64 water molecules.

* `md.lmp`

   LAMMPS input file with parameters for the MD simulation. Configured to output
   trajectory, velocities, forces, and energies to files found in 02_Traj/ and
   to run a simulation at 300K for 100ps.

* `02_Traj/`

  Contains outputs for a reference run using these same ANN potential files.

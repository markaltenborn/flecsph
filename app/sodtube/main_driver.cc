/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

 /*~--------------------------------------------------------------------------~*
 * 
 * /@@@@@@@@  @@           @@@@@@   @@@@@@@@ @@@@@@@  @@      @@
 * /@@/////  /@@          @@////@@ @@////// /@@////@@/@@     /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@   /@@/@@     /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@@@@@@ /@@@@@@@@@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@////  /@@//////@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@      /@@     /@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@      /@@     /@@
 * //       ///  //////   //////  ////////  //       //      //  
 *
 *~--------------------------------------------------------------------------~*/

/**
 * @file main_driver.cc
 * @author Julien Loiseau
 * @date April 2017
 * @brief Specialization and Main driver used in FleCSI. 
 * The Specialization Driver is normally used to register data and the main 
 * code is in the Driver.  
 */

#include <iostream>
#include <numeric> // For accumulate
#include <iostream>

#include <mpi.h>
#include <legion.h>
#include <omp.h>

#include "flecsi/execution/execution.h"
#include "flecsi/data/data_client.h"
#include "flecsi/data/data.h"

#include <bodies_system.h>

#include "eos_analytics.h"
//#include "physics.h"

#define INTERNAL_ENERGY

namespace flecsi{
namespace execution{

void
mpi_init_task(int startiteration){
  // TODO find a way to use the file name from the specialiszation_driver
  
  int rank;
  int size;
  MPI_Comm_size(MPI_COMM_WORLD,&size);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  
  int totaliters = 50;
  int iteroutput = 1;
  double totaltime = 0.0;
  double maxtime = 10.0;
  int iter = startiteration; 

  const char * inputFile = "hdf5_sodtube.h5part";
  const char * outputFile = "output_sodtube.h5part"; 
  // Remove output file 
  remove(outputFile); 

  body_system<double,gdimension> bs;
  bs.read_bodies("hdf5_sodtube.h5part",startiteration);
  // Init if default values are not ok
  physics::dt = 0.025;// bs.get_attribute<double>("hdf5_sodtube.h5part","timestep");
  physics::alpha = 1; 
  physics::beta = 2; 
  physics::do_boundaries = true;
  physics::stop_boundaries = true;
  physics::gamma = 1.4;//5./3.;
  physics::nparticles = bs.getNParticles();

  if(rank==0){
    std::cout<<"timestep="<<physics::dt<<std::endl;
  }
  double h = bs.getSmoothinglength();
  physics::epsilon = 0.1*h;

  // Set the boundaries to be at 10% of the total range
  auto range_boundaries = bs.getRange(); 
  double distance = fabs(range_boundaries[1][0]-range_boundaries[0][0]);
  physics::min_boundary = {range_boundaries[0][0]+distance*0.1+2*h};
  physics::max_boundary = {range_boundaries[1][0]-distance*0.1-2*h};

  if(rank==0){
    std::cout<<"Boundaries="<<physics::min_boundary<<";"<<
      physics::max_boundary<<std::endl;
  }

  bs.update_iteration();

  if(rank==0)
    std::cout<<"compute_density_pressure_soundspeed"<<std::flush; 
    bs.apply_square(physics::compute_density_pressure_soundspeed_sodtube);
  if(rank==0)
    std::cout<<".done"<<std::endl;
  if(rank==0)
    std::cout<<"Init Internal energy and velocity for verlet"<<std::flush; 
    bs.apply_all(physics::init_energy_velocity);
  if(rank==0)
    std::cout<<".done"<<std::endl;

#ifdef OUTPUT
  bs.write_bodies("output_sodtube",iter);
  //io::outputDataHDF5(rbodies,"output_sodtube.h5part",0);
  //tcolorer.mpi_output_txt(rbodies,iter,"output_sodtube"); 
#endif

  ++iter; 
  do
  { 
    MPI_Barrier(MPI_COMM_WORLD);
    if(rank==0)
      std::cout<<std::endl<<"#### Iteration "<<iter<<std::endl;
    MPI_Barrier(MPI_COMM_WORLD);

    // Compute and prepare the tree for this iteration 
    // - Compute the Max smoothing length 
    // - Compute the range of the system using the smoothinglength
    // - Cmopute the keys 
    // - Distributed qsort and sharing 
    // - Generate and feed the tree
    // - Exchange branches for smoothing length 
    // - Compute and exchange ghosts in real smoothing length 
    bs.update_iteration();

    // Reset Acceleration
    bs.apply_all(
      [] (body_holder* srch)
      {
        body* source = srch->getBody();
        source->setAcceleration(point_t{});
      });
   
    if(rank==0)
      std::cout<<"Hydro + Internalenergy"<<std::flush; 
    bs.apply_square(physics::compute_hydroacc_internalenergy);
    if(rank==0)
      std::cout<<".done"<<std::endl;
 
    // new dt
    //if(rank==0)
    //  std::cout<<"Timestep"<<std::flush; 
    //bs.apply_square(physics::compute_dt);
    //if(rank==0){
    //  std::cout<<".done"<<std::endl; 
    //  std::cout<<"dt="<<physics::dt<<std::endl;
    //}

    if(rank==0)
      std::cout<<"verlet"<<std::flush; 
      bs.apply_all(physics::verlet_integration);
    if(rank==0)
      std::cout<<".done"<<std::endl;
    

    if(rank==0)
      std::cout<<"compute_density_pressure_soundspeed"<<std::flush; 
    bs.apply_square(physics::compute_density_pressure_soundspeed_sodtube);
    if(rank==0)
      std::cout<<".done"<<std::endl;

#ifdef OUTPUT
    if(iter % iteroutput == 0){ 
      bs.write_bodies("output_sodtube",iter/iteroutput);
    }
#endif
    ++iter;

    physics::totaltime+=physics::dt;
    if(rank==0){
      std::cout<<"totaltime="<<physics::totaltime<< std::endl;
    }
    
  }while(iter<totaliters);
}

flecsi_register_mpi_task(mpi_init_task);

void 
specialization_tlt_init(int argc, char * argv[]){
  
  // Default start at iteration 0
  int startiteration = 0;
  if(argc == 2){
    startiteration = atoi(argv[1]);
  }

  std::cout << "In user specialization_driver" << std::endl;
  /*const char * filename = argv[1];*/
  /*std::string  filename(argv[1]);
  std::cout<<filename<<std::endl;*/
  flecsi_execute_mpi_task(mpi_init_task,startiteration); 
} // specialization driver

void 
driver(int argc,  char * argv[]){
  std::cout << "In user driver" << std::endl;
} // driver


} // namespace
} // namespace



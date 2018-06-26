/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#include <iostream>
#include <algorithm>
#include <cassert>

#include "hdf5ParticleIO.h"
#include "kernel.h"

const double ldistance = 0.001;  // Distance between the particles 
const double localgamma = 5./3.;
const double rho_in = 1.0;
const double pressure_in = 1.0e-7;
const double u_in = pressure_in/(rho_in*(localgamma - 1.0));
const double u_blast = 1.0;
const double smoothing_length = 5.*ldistance;
const char* fileprefix = "hdf5_sedov";


int main(int argc, char * argv[]){

  int64_t sparticles = 101;
  if(argc!=2){
    printf("./sedov_generator [square nParticles]\n");
    fprintf(stderr,"Generating default number of particles=%ld*%ld=%ld",
        sparticles,sparticles,sparticles*sparticles);
  }else{
    sparticles = atoll(argv[1]);
  }

  int rank, size; 
  int provided; 
  MPI_Init_thread(&argc,&argv,MPI_THREAD_MULTIPLE,&provided);
  assert(provided>=MPI_THREAD_MULTIPLE); 
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  MPI_Comm_size(MPI_COMM_WORLD,&size);

  int64_t nparticles = sparticles*sparticles;

  printf("Generating %ld particles by %ldx%ld\n",
        nparticles,sparticles,sparticles);

  // Start on  0 0

  double maxxposition = (sparticles)*ldistance;
  double maxyposition = (sparticles)*ldistance;

  double x_c = (sparticles-1)*ldistance/2.0;
  double y_c = (sparticles-1)*ldistance/2.0;

  double mass = rho_in * maxxposition * maxyposition/nparticles;  

  // Position
  double* x = new double[nparticles]();
  double* y = new double[nparticles]();
  double* z = new double[nparticles]();
  // Velocity
  double* vx = new double[nparticles]();
  double* vy = new double[nparticles]();
  double* vz = new double[nparticles]();
  // Acceleration
  double* ax = new double[nparticles]();
  double* ay = new double[nparticles]();
  double* az = new double[nparticles]();
  // Smoothing length 
  double* h = new double[nparticles]();
  // Density 
  double* rho = new double[nparticles]();
  // Internal Energy 
  double* u = new double[nparticles]();
  // Pressure
  double* P = new double[nparticles]();
  // Mass
  double* m = new double[nparticles]();
  // Id
  int64_t* id = new int64_t[nparticles]();
  // Timestep 
  double* dt = new double[nparticles]();
  
  // Id of my first particle 
  int64_t posid = 0;

  // Header data 
  // the number of particles = nparticles 
  // The value for constant timestep 
  double timestep = 1.e-3;
  int dimension = 2;
  
  double xposition = 0;
  double yposition = 0;

  int64_t particles_blast = 0;
  int64_t tparticles = 0;

  for (int64_t part=0; part<nparticles; ++part) {
    tparticles++;
    x[part] = xposition;
    y[part] = yposition;

    if (sqrt((x[part]-x_c)*(x[part]-x_c) + (y[part]-y_c)*(y[part]-y_c)) < 2.0*ldistance) {
      particles_blast++;
    }
 
    xposition+= ldistance;
    if(xposition > maxxposition){
      xposition = 0.;
      yposition+=ldistance;
    }
  }

 
  for(int64_t part=0; part<nparticles; ++part){    
    //tparticles++;
    //x[part] = xposition;
    //y[part] = yposition;
         
    P[part]   = pressure_in;
    rho[part] = rho_in; 
    m[part]   = mass;
    u[part]   = u_in;
    h[part]   = smoothing_length;
    id[part]  = posid++;

    if (sqrt((x[part]-x_c)*(x[part]-x_c) + (y[part]-y_c)*(y[part]-y_c)) < 2.0*ldistance) {
      u[part] = u_blast/particles_blast;
      P[part] = u[part]*rho[part]*(localgamma - 1.0);
    }

    //xposition+= ldistance; 
    //if(xposition > maxxposition){
    //  xposition = 0.;
    //  yposition+=ldistance;
    //}
  }

  std::cout<<"Real number of particles: "<<tparticles<<std::endl;

  char filename[128];
  //sprintf(filename,"%s_%d.h5part",fileprefix,nparticles);
  sprintf(filename,"%s.h5part",fileprefix);
  // Remove the previous file 
  remove(filename); 


  Flecsi_Sim_IO::HDF5ParticleIO testDataSet; 
  testDataSet.createDataset(filename,MPI_COMM_WORLD);

  // add the global attributes
  testDataSet.writeDatasetAttribute("nparticles","int64_t",tparticles);
  testDataSet.writeDatasetAttribute("timestep","double",timestep);
  testDataSet.writeDatasetAttribute("dimension","int32_t",dimension);
  testDataSet.writeDatasetAttribute("use_fixed_timestep","int32_t",1);

  //const char * simName = "sodtube_1D";
  //testDataSet.writeDatasetAttributeArray("name","string",simName);
  testDataSet.closeFile();

  testDataSet.openFile(MPI_COMM_WORLD);
  testDataSet.setTimeStep(0);

  Flecsi_Sim_IO::Variable _d1,_d2,_d3;

  _d1.createVariable("x",Flecsi_Sim_IO::point,"double",tparticles,x);
  _d2.createVariable("y",Flecsi_Sim_IO::point,"double",tparticles,y);
  _d3.createVariable("z",Flecsi_Sim_IO::point,"double",tparticles,z);

  testDataSet.vars.push_back(_d1);
  testDataSet.vars.push_back(_d2);
  testDataSet.vars.push_back(_d3);

  testDataSet.writeVariables();

  _d1.createVariable("vx",Flecsi_Sim_IO::point,"double",tparticles,vx);
  _d2.createVariable("vy",Flecsi_Sim_IO::point,"double",tparticles,vy);
  _d3.createVariable("vz",Flecsi_Sim_IO::point,"double",tparticles,vz);

  testDataSet.vars.push_back(_d1);
  testDataSet.vars.push_back(_d2);
  testDataSet.vars.push_back(_d3);

  testDataSet.writeVariables();

  _d1.createVariable("ax",Flecsi_Sim_IO::point,"double",tparticles,ax);
  _d2.createVariable("ay",Flecsi_Sim_IO::point,"double",tparticles,ay);
  _d3.createVariable("az",Flecsi_Sim_IO::point,"double",tparticles,az);

  testDataSet.vars.push_back(_d1);
  testDataSet.vars.push_back(_d2);
  testDataSet.vars.push_back(_d3);

  testDataSet.writeVariables();


  _d1.createVariable("h",Flecsi_Sim_IO::point,"double",tparticles,h);
  _d2.createVariable("rho",Flecsi_Sim_IO::point,"double",tparticles,rho);
  _d3.createVariable("u",Flecsi_Sim_IO::point,"double",tparticles,u);
  
  testDataSet.vars.push_back(_d1);
  testDataSet.vars.push_back(_d2);
  testDataSet.vars.push_back(_d3);

  testDataSet.writeVariables();

  _d1.createVariable("P",Flecsi_Sim_IO::point,"double",tparticles,P);
  _d2.createVariable("m",Flecsi_Sim_IO::point,"double",tparticles,m);
  _d3.createVariable("id",Flecsi_Sim_IO::point,"int64_t",tparticles,id);
  
  testDataSet.vars.push_back(_d1);
  testDataSet.vars.push_back(_d2);
  testDataSet.vars.push_back(_d3);

  testDataSet.writeVariables();

  testDataSet.closeFile(); 
  
  delete[] x;
  delete[] y;
  delete[] z;
  delete[] vx;
  delete[] vy;
  delete[] vz;
  delete[] ax;
  delete[] ay;
  delete[] az;
  delete[] h;
  delete[] rho;
  delete[] u;
  delete[] P;
  delete[] m;
  delete[] id;
  delete[] dt;
 
  MPI_Finalize();
  return 0;
}

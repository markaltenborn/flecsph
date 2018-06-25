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
 * @file body.h
 * @author Julien Loiseau
 * @date April 2017
 * @brief Representation of a body, a particle for our SPH implementation 
 */

#ifndef body_h
#define body_h

#define OUTPUT

#warning "CHANGE TO FLECSI ONE"
#include "tree_topology.h"
#include "flecsi/geometry/point.h"
#include "flecsi/geometry/space_vector.h"

#include "user.h"

#define INTERNAL_ENERGY

class body{

  static const size_t dimension = gdimension;
  using element_t = type_t; 
  using point_t = flecsi::point<element_t, dimension>;
  
public:
  
  body(const point_t& position, 
      const point_t& velocity, 
      const point_t& velocityhalf, 
      const point_t& acceleration,
      const double density, 
      const double pressure, 
      const double entropy, 
      const double mass,
      const double smoothinglength
  ):  position_(position), 
      velocity_(velocity),
      velocityhalf_(velocityhalf),
      acceleration_(acceleration),
      density_(density),
      pressure_(pressure),
      //entropy_(entropy),
      mass_(mass),
      smoothinglength_(smoothinglength),
      soundspeed_(0.0),
      internalenergy_(0.0)
      //gravforce_(point_t{}),
      //hydroforce_(point_t{})
  {};

  body()
  {};

  const point_t& coordinates() const{return position_;}
  const point_t& getPosition() const{return position_;}
  double getMass() const{return mass_;}
  double getSmoothinglength() const{return smoothinglength_;}
  double getPressure() const{return pressure_;}
  double getSoundspeed() const{return soundspeed_;}
  //double getEntropy() const{return entropy_;}
  double getDensity() const{return density_;}
  point_t getVelocity() const{return velocity_;}
  point_t getVelocityTmp() const{return velocityTmp_;}
  point_t getVelocityNM1() const{ return velocityNM1_;}
  point_t getVelocityCor() const{return velocityCor_;}
  //point_t getHydroForce() const{return hydroforce_;}
  //point_t getGravForce() const{return gravforce_;}
  point_t getVelocityhalf() const{return velocityhalf_;}
  point_t getAcceleration() const{return acceleration_;}
  double getInternalenergy() const{return internalenergy_;}
  double getInternalenergyTmp() const{return internalenergyTmp_;} 
  double getInternalenergyNM1() const{return internalenergyNM1_;} 
  point_t getLinMomentum() const { 
    point_t res = {};
    for(int i = 0 ; i < dimension; ++i){
      res[i] = velocity_[i] * mass_;
    }
    return res;
  };
  //double getDudt(){return dudt_;};
  int64_t getId(){return id_;};
  double getDt(){return dt_;};
  double getDudt(){return dudt_;};
  int getType(){return type_;}; 

  bool is_wall(){return type_ == 1;};

  void setPosition(point_t position){position_ = position;}
  void setAcceleration(point_t acceleration){acceleration_ = acceleration;}
  void setVelocity(point_t velocity){velocity_ = velocity;}
  void setVelocityhalf(point_t velocityhalf){velocityhalf_ = velocityhalf;}
  //void setGravForce(point_t gravforce){gravforce_ = gravforce;}
  //void setHydroForce(point_t hydroforce){hydroforce_ = hydroforce;}
  void setSoundspeed(double soundspeed){soundspeed_ = soundspeed;}
  void setPressure(double pressure){pressure_ = pressure;}
  void setDensity(double density){density_ = density;}
  void setMass(double mass){mass_ = mass;};
  //void setLinMomentum(point_t lin_momentum){lin_momentum_ = lin_momentum;}
  void setInternalenergy(double internalenergy)
    {internalenergy_=internalenergy;};
  void setInternalenergyTmp(double internalenergyTmp)
    {internalenergyTmp_=internalenergyTmp;};
  void setInternalenergyNM1(double internalenergyNM1)
    {internalenergyNM1_=internalenergyNM1;};
  void setSmoothinglength(double smoothinglength)
    {smoothinglength_=smoothinglength;};
 // void setDudt(double dudt){dudt_ = dudt;};
  void setDt(double dt){dt_ = dt;};
  void setId(int64_t id){id_ = id;};
  void setDudt(double dudt){dudt_ = dudt;};
  void setType(int type){type_ = type;}; 
  void setVelocityNM1(point_t velocityNM1){velocityNM1_ = velocityNM1;};
  void setVelocityTmp(point_t velocityTmp){velocityTmp_ = velocityTmp;};
  void setVelocityCor(point_t velocityCor){velocityCor_ = velocityCor;};

  friend std::ostream& operator<<(std::ostream& os, const body& b){
    // TODO change regarding to dimension 
    os << std::setprecision(10); 
    os << "Particle: Pos: " <<b.position_ << " rho: " << b.density_; 
    os << " h: " << b.smoothinglength_;
    os << " P: " << b.pressure_;
    os << " v: " << b.velocity_ ;//<< " VelH: " << b.velocityhalf_;
    os << " m: " << b.mass_;
    os << " u: " << b.internalenergy_;
    os << " cs: " << b.soundspeed_;
    //os << " Force: hyd: " << b.hydroforce_;
    //os << " grav: " << b.gravforce_;
    os << " a: " << b.acceleration_;
    os << " id: " << b.id_; 
    return os;
  }      

private:
  point_t position_;
  point_t velocity_;
  point_t velocityhalf_;
  point_t velocityNM1_; 
  point_t velocityTmp_;
  point_t velocityCor_;
  point_t acceleration_;
  double density_;
  double pressure_; 
  double entropy_;
  double mass_;
  double smoothinglength_; 
  double soundspeed_;
  double internalenergy_;
  double internalenergyNM1_;
  double internalenergyTmp_;
  //point_t lin_momentum_; //TODO : Need to check
  //double dudt_;
  //point_t gravforce_;
  //point_t hydroforce_;
  double dt_;
  int64_t id_;
  double dudt_;
  int type_; 
}; // class body 
  
#endif // body_h


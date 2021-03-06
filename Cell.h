#ifndef __CELL_H__
#define __CELL_H__

#include "ckmulticast.h"
#include <string>

//fluid equation of state
//inline double Eos(double rho, double type) {
inline double Eos(double rho) 
{
  int gama = 7;
  double B = 100 * RHO0 * maxVel * maxVel / gama; 
  return B * (pow(rho / RHO0, gama) - 1) + BASEPRES;
}

//data message to be sent to computes
struct ParticleDataMsg : public CkMcastBaseMsg, public CMessage_ParticleDataMsg {
  Particle * part; //list of atoms

  int lengthAll;  //length of list
  int x, y, z;    // (x,y,z) coordinate of cell sending this message

  ParticleDataMsg(const int x_, const int y_, const int z_, const int numPos)
    : x(x_), y(y_), z(z_), lengthAll(numPos) { }

  void pup(PUP::er &p){
    CMessage_ParticleDataMsg::pup(p);
    p | lengthAll;
    p | x; p | y; p | z;
    if (p.isUnpacking()){
      part = new Particle[lengthAll];
    } 
    PUParray(p, part, lengthAll);
  } 
};

//chare used to represent a cell
class Cell : public CBase_Cell {
private:
  Cell_SDAG_CODE;

  // list of atoms
  std::vector<Particle> particles;
  std::vector<Particle> particles2;

  // my compute locations
  std::vector<CkArrayIndex6D> computesList;
  // to count the number of steps, and decide when to stop
  int stepCount;
  // number of atoms in my cell
  int myNumParts;
  // number of interacting neighbors
  int inbrs;
  double stepTime;
  double periodTime;
  double startTime;
  int updateCount;
  // store kinetic energy - initial and final
  double energy[2];
  int numReadyCheckpoint;
  void migrateToCell(Particle p, int &px, int &py, int &pz);
  // updates properties after receiving forces from computes
  void updatePropertiesSPH(vec4 *dVel_dRho, int iteration);

  // limit velcities to an upper limit
  void limitVelocity(Particle &p);
  // particles going out of right enters from left
  Particle& wrapAround(Particle &p);
  // handle to section proxy of computes
  CProxySection_Compute mCastSecProxy;

public:
  Cell();
  Cell(CkMigrateMessage *msg);
  ~Cell();
  void createComputes();  //add my computes
  void createSection();   //created multicast section of computes
  void migrateParticles(int step);
  void sendPositions(int iteration);
  void writeCell(int stepCount);
  void writeTimings(double periodTime, double currSimTime, int currStep);
  void startCheckpoint(int);
  void pup(PUP::er &p);
};

#endif

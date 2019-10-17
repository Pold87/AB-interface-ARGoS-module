#ifndef EPUCK_ENVIRONMENT_CLASSIFICATION_H
#define EPUCK_ENVIRONMENT_CLASSIFICATION_H
#include <argos3/core/control_interface/ci_controller.h>
#include <argos3/core/utility/configuration/argos_configuration.h>
#include <argos3/plugins/robots/e-puck/control_interface/ci_epuck_ground_sensor.h>
#include <argos3/plugins/robots/e-puck/control_interface/ci_epuck_proximity_sensor.h>
#include <argos3/plugins/robots/e-puck/control_interface/ci_epuck_range_and_bearing_actuator.h>
#include <argos3/plugins/robots/e-puck/control_interface/ci_epuck_range_and_bearing_sensor.h>
#include <argos3/plugins/robots/e-puck/control_interface/ci_epuck_wheels_actuator.h>
#include <argos3/plugins/robots/generic/control_interface/ci_differential_steering_actuator.h>
#include <argos3/plugins/robots/generic/control_interface/ci_leds_actuator.h>
#include <argos3/core/utility/math/rng.h>
#include <argos3/core/utility/math/vector2.h>
#include <ctime>
#include <fstream>
#include <iostream>
#include <set>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "generic_interface.h" /* Use geth from C++ */

#define N_COL 3

using namespace argos;

class EPuck_Environment_Classification : public CCI_Controller {

public:
  struct CollectedData {
    CColor readColor;
    UInt32 count;
    CollectedData();
  };

  struct Opinion {
    UInt32 actualOpinion;
    UInt32 countedCellOfActualOpinion;
    Real quality;
    CColor actualOpCol;
    Opinion();
  };

  // Random walk
  struct Movement {
    SInt32 walkTime;        // Movement time counter;
    UInt32 actualDirection; // 0, straight; 1, turn CW; 2, turn CCW (TOCHECK: if
                            // 1 is counterclockwise or vice versa; fix comment)
    Movement();
  };

  struct SimulationState {
    UInt32 decisionRule;
    Real percentRed, percentBlue;
    Real g;
    Real sigma;
    bool exitFlag;
    bool profiling;
    std::string radix;
    std::string baseDir; /* Basedir of the controller folder */
    std::string datadirBase;
    int basePort;
    UInt32 numPackSaved;
    UInt32 status;
    UInt32 LAMBDA, turn;
    bool useClassicalApproach;
    int maxFlooding;
    bool determineConsensus;
    std::string contractAddress;
    std::string contractABI;
    std::string containerNameBase;
    int explorationTime;
    void Init(TConfigurationNode &t_node);
  };

  struct SStateData {

    SInt32 explorDurationTime;
    SInt32 remainingExplorationTime;
    SInt32 diffusingDurationTime;
    SInt32 remainingDiffusingTime;

    enum EState {
      STATE_EXPLORING,
      STATE_DIFFUSING,
    } State;
  };

  EPuck_Environment_Classification();

  virtual ~EPuck_Environment_Classification() {}

  virtual void Init(TConfigurationNode &t_node);
  virtual void ControlStep();
  virtual void RandomWalk();
  virtual void ObstacleAvoidance();
  virtual void Reset(){};
  void Explore();
  void Diffusing();
  void Listening();
  void ConnectAndListen();
  void Move();
  void TurnLeds();
  void prepare();
  Real ExponentialFormula(Real mean) {

    CRange<Real> cRange(0.0, 1.0);
    return -log(m_pcRNG->Uniform(cRange)) * mean;
  }

  virtual void Destroy() {}

  inline CollectedData &GetColData() { return collectedData; }
  inline SStateData &GetStateData() { return m_sStateData; }
  inline SimulationState &GetSimulationState() { return simulationParams; }
  inline Movement &GetMovement() { return movement; }
  inline Opinion &GetOpinion() { return opinion; }
  inline std::string &GetAddress() { return address; }
  inline std::string &GetMinerAddress() { return minerAddress; }
  inline bool isMining() { return mining; }

  inline bool IsExploring() const {
    return m_sStateData.State == SStateData::STATE_EXPLORING;
  }
  
  inline bool IsDiffusing() const {
    return m_sStateData.State == SStateData::STATE_DIFFUSING;
  }

  inline std::string getEnode() { return enode; }
  inline int getByzantineStyle() { return byzantineStyle; }
  inline bool getConsensusReached() { return consensusReached; }
  inline void setByzantineStyle(int style) { byzantineStyle = ByzantineStyle(style); }
  inline GethInterface getGethInterface() { return *gethInterface; }
  void UpdateNeighbors(std::set<int> newNeighbors);

  enum ByzantineStyle { 
     No = 0,
     AlwaysZero = 1, AlwaysOne = 2, RandomZeroOne = 3, UniformRandom = 4, GaussianNoise = 5,
     SybilAlwaysZero = 11, SybilAlwaysOne = 12, SybilRandomZeroOne = 13,
     SybilUniformRandom = 14, SybilGaussianNoise = 15,
     Jamming = 20
  };

private:
  int robotId;
  int scMean;
  GethInterface *gethInterface;
  void WaitForDecision();
  CCI_EPuckWheelsActuator *m_pcWheels;
  Real m_fWheelVelocity;
  CCI_EPuckRangeAndBearingActuator *m_pcRABA;
  CCI_EPuckRangeAndBearingSensor *m_pcRABS;
  CDegrees m_cAlpha;                        // OBST. AVOID.
  Real m_fDelta;                            // OBST. AVOID.
  CCI_EPuckProximitySensor *m_pcProximity;  // OBST. AVOID.
  CRange<CRadians> m_cGoStraightAngleRange; // OBST. AVOID.
  CCI_LEDsActuator *m_pcLEDs;
  CRandom::CRNG *m_pcRNG;

  std::ofstream epuckFile;
  SStateData m_sStateData;
  SimulationState simulationParams;
  std::ofstream numberReceived;
  CollectedData collectedData;
  Opinion opinion;
  Movement movement;
  int initializationValues[N_COL];
  std::string address;
  std::string minerAddress;
  std::set<int> neighbors;
  std::string enode;
  std::ofstream votesFile;
  std::string blockchainPath;
  bool beginning;
  int nodeInt;
  std::map<int, int> robotIdToNode;
  bool mining;
  bool threadCurrentlyRunning;
  int eventTrials;
  ByzantineStyle byzantineStyle;
  bool consensusReached;
  bool receivedDecision;
  CColor red, blue, green;
};

#endif

#ifndef EPUCK_ENVIRONMENT_CLASSIFICATION_H
#define EPUCK_ENVIRONMENT_CLASSIFICATION_H
#include <argos3/core/control_interface/ci_controller.h>
#include <argos3/core/utility/configuration/argos_configuration.h>
#include <argos3/plugins/robots/e-puck/control_interface/ci_epuck_range_and_bearing_actuator.h>
#include <argos3/plugins/robots/e-puck/control_interface/ci_epuck_range_and_bearing_sensor.h>
#include <argos3/plugins/robots/e-puck/control_interface/ci_epuck_proximity_sensor.h>
#include <argos3/plugins/robots/generic/control_interface/ci_differential_steering_actuator.h>
#include <argos3/plugins/robots/generic/control_interface/ci_leds_actuator.h>
#include <argos3/plugins/robots/e-puck/control_interface/ci_epuck_ground_sensor.h>
#include <argos3/plugins/robots/e-puck/control_interface/ci_epuck_wheels_actuator.h>

#include <argos3/core/utility/math/rng.h>
#include <argos3/core/utility/math/vector2.h>
#include <vector>
#include <set>
#include <string>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <iostream>

#include "geth_static.h" /* Use geth from C++ */

#define N_COL  3

/*
 * All the ARGoS stuff in the 'argos' namespace.
 * With this statement, you save typing argos:: every time.
 */
using namespace argos;

class EPuck_Environment_Classification : public CCI_Controller {
  
public:
	 struct CollectedData {
	     CColor readColor;
	     UInt32 count;
	     CollectedData();
	  };

	  struct informationCollected{
	     UInt8 receivedOpinion;
	     Real receivedQuality;
	     UInt8 senderID;
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
	    SInt32 walkTime;         // Movement time counter;
	    UInt32 actualDirection;  // 0, straight; 1, turn CW; 2, turn CCW (TOCHECK: if 1 is counterclockwise or vice versa; fix comment)
	    Movement();
	  };

	  struct SimulationState {
	    UInt32 decision_rule;
	    Real percentRed, percentBlue;
	    Real g;
	    Real sigma;
	    bool exitFlag;
	    bool profiling;
	    bool useMultipleNodes;
	    bool useBackgroundGethCalls;
	    std::string radix;
	    std::string baseDir; /* Basedir of the controller folder */
	    std::string interfacePath;
	    std::string mappingPath;
	    std::string regenerateFile;
	    //	    std::string mappingByzantinePath;
	    std::string blockchainPath;
	    std::string datadirBase;
	    int basePort;
	    //	    int numByzantine;
	    UInt32 numPackSaved;
	    UInt32 status;
	    UInt32 LAMDA, turn;
	    bool useClassicalApproach;
	    UInt32 numRobots; /* total amount of robots in the experiment */
	     void Init(TConfigurationNode& t_node);
	  };

	  struct SStateData {

		  /* For variables for the time of the states because two of them are decreased to count the time
		   * steps spent in that state and the other two are used to keep track of the duration times, for
		   * record the statistics.
		   */
	     SInt32 explorDurationTime;
	     SInt32 remainingExplorationTime;
	     SInt32 diffusingDurationTime;
	     SInt32 remainingDiffusingTime;
	     /* Following variables are not used anymore, but could be usefull to have them */
//	     int exportTime[N_COL];
//	     int numberOfExplorations[N_COL];
//	     int numberOfDiffusions[N_COL];

	     enum EState {
	         STATE_EXPLORING,
	         STATE_DIFFUSING,
	      } State;};
public:
   /* Class constructor. */
   EPuck_Environment_Classification();
 
   /* Class destructor. */
   virtual ~EPuck_Environment_Classification() {}

   virtual void Init(TConfigurationNode& t_node);
   virtual void ControlStep();
   virtual void RandomWalk();
   virtual void Reset() {};
   void fromLoopFunctionResPrepare();
   void fromLoopFunctionResStart();
   void killGethAndRemoveFolders(std::string bcPath, std::string regenFile);
   void Explore();
   void Diffusing();   
   void Listening();
   void ConnectAndListen();
   void DiffuseInformation();
   void DecisionRule(UInt32 decision_rule);
   void Move();
   void TurnLeds();
   Real ExponentialFormula(Real mean){

	   CRange<Real> cRange(0.0,1.0);
	   return -log(m_pcRNG->Uniform(cRange)) * mean;
   }

   /* Unused (ma carlo l'aveva messa senza usarla quindi boh)*/
   virtual void Destroy() {}

   inline CollectedData& GetColData() {
      return collectedData;
   }
   inline SStateData& GetStateData() {
      return m_sStateData;
   }
   inline SimulationState& GetSimulationState(){
      return simulationParams;
   }
   inline std::vector<informationCollected>& GetReceivedOpinions() {
      return receivedOpinions;
   }
   inline Movement & GetMovement() {
      return movement;
   }
   inline Opinion & GetOpinion() {
      return opinion;
   }

   inline std::string & GetAddress() {
      return address;
   }

   inline std::string & GetMinerAddress() {
      return minerAddress;
   }

   inline bool isMining() {
     return mining;
   }
   
   inline bool IsExploring() const {
      return m_sStateData.State == SStateData::STATE_EXPLORING;
   }
   inline bool IsDiffusing() const {
      return m_sStateData.State == SStateData::STATE_DIFFUSING;
   }

   inline std::string getEnode()  {
     return enode;
   }

   inline int getNodeInt() {
     return nodeInt;
   }

   inline void setContractAddress(std::string contractAddr) {
     contractAddress = contractAddr;
   }

   inline int getByzantineStyle() {
     return byzantineStyle;
   }

   inline void setByzantineStyle(int style) {
     byzantineStyle = style;
   }
   
   void UpdateNeighbors(std::set<int> newNeighbors);

private:

   void InitGeth(int robotId);
   void WaitForDecision();
   void readNodeMapping();
   //   void readByzantineMapping();
   void DistributeID();
   
   CCI_EPuckWheelsActuator* m_pcWheels;
   Real m_fWheelVelocity;
   CCI_EPuckRangeAndBearingActuator*  m_pcRABA;
   CCI_EPuckRangeAndBearingSensor* m_pcRABS;
   CDegrees m_cAlpha;                         // OBST. AVOID.
   Real m_fDelta;                             // OBST. AVOID.
   CCI_EPuckProximitySensor* m_pcProximity;   // OBST. AVOID.
   CRange<CRadians> m_cGoStraightAngleRange;  // OBST. AVOID.
   CCI_LEDsActuator* m_pcLEDs;
   CRandom::CRNG* m_pcRNG;

   /* Files */
   std::ofstream epuckFile;
   /* Data structures for collect opinions in diffusing state */
   std::vector<informationCollected> receivedOpinions;  // Set of information collected in every diffusing states
   informationCollected IC;
   /* All others used variables */
   SStateData m_sStateData;
   SimulationState simulationParams;
    std::ofstream numberReceived;		// Flag: just robot "ff0" (always present) writes his qualities after each exp. state
   CollectedData collectedData;
   Opinion opinion;
   Movement movement;
   int initializationValues[N_COL];
   std::string address;
   std::string minerAddress;
   std::string contractAddress;
   std::string rawTx;
   std::set<int> neighbors;
   std::string enode;

   std::string blockchainPath;
   blockWithHash bwh;
   bool beginning;
   int nodeInt;
   std::map<int, int> robotIdToNode;  
   bool mining;
   int byzantineStyle;
   bool threadCurrentlyRunning;
   int eventTrials;
   bool receivedDecision; // Indicates if the robots already received a new opinion from the smart contract (for multi threading)
   CColor red, blue, green;                    // Add here eventual additional color AGGIUNGERECOLORI
//   int totalCounted, countedOfThisOpinion[N_COL];  USED JUST FOR STATISTICS, no more used
};

#endif

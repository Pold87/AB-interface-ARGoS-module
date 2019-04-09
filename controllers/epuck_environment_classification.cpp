#include "epuck_environment_classification.h"
#include "helpers.h"

#define ALPHA_CHANNEL 0
#define COLOR_STRENGTH 255
#define N_COL 3

#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>
#include <thread>
#include <unistd.h>

using namespace std;

map<int, string> enodes;

EPuck_Environment_Classification::EPuck_Environment_Classification()
    : m_pcWheels(NULL), m_fWheelVelocity(10.0f), m_pcRABA(NULL), m_pcRABS(NULL),
      m_cAlpha(10.0f), m_fDelta(0.5f), m_pcProximity(NULL),
      m_cGoStraightAngleRange(-ToRadians(m_cAlpha), ToRadians(m_cAlpha)) {}

EPuck_Environment_Classification::CollectedData::CollectedData() : count(0) {}

EPuck_Environment_Classification::Opinion::Opinion()
    : countedCellOfActualOpinion(0) {}

EPuck_Environment_Classification::Movement::Movement()
    : walkTime(3), actualDirection(0) {}

void EPuck_Environment_Classification::SimulationState::Init(
    TConfigurationNode &t_node) {

  try {
    // Getting sigma, G value and the decision rule to follow
    GetNodeAttribute(t_node, "g", g);
    GetNodeAttribute(t_node, "sigma", sigma);
    GetNodeAttribute(t_node, "lambda", LAMBDA);
    GetNodeAttribute(t_node, "turn", turn);
    GetNodeAttribute(t_node, "decision_rule", decision_rule);
    GetNodeAttribute(t_node, "exitFlag", exitFlag);
    GetNodeAttribute(t_node, "percent_white", percentRed);
    GetNodeAttribute(t_node, "percent_black", percentBlue);
    GetNodeAttribute(t_node, "num_pack_saved", numPackSaved);
    GetNodeAttribute(t_node, "base_dir", baseDir);
    GetNodeAttribute(t_node, "use_classical_approach", useClassicalApproach);
    GetNodeAttribute(t_node, "profiling", profiling);
    GetNodeAttribute(t_node, "max_flooding", maxFlooding);
    GetNodeAttribute(t_node, "determine_consensus", determineConsensus);
    GetNodeAttribute(t_node, "contract_address", contractAddress);
    GetNodeAttribute(t_node, "contract_abi", contractABI);
    GetNodeAttribute(t_node, "container_name_base", containerNameBase);
  } catch (CARGoSException &ex) {
    THROW_ARGOSEXCEPTION_NESTED(
        "Error initializing controller state parameters.", ex);
  }
}

void EPuck_Environment_Classification::Init(TConfigurationNode &t_node) {

  robotId = Id2Int(GetId()) + 1; // + 1 since docker containers start with 1
  eventTrials = 0;
  receivedDecision = true;
  threadCurrentlyRunning = false;
  consensusReached = false;

  // Initialize the actuators (and sensors) and the initial velocity as straight
  // walking
  m_pcWheels = GetActuator<CCI_EPuckWheelsActuator>("epuck_wheels");
  m_pcProximity = GetSensor<CCI_EPuckProximitySensor>("epuck_proximity");
  m_pcLEDs = GetActuator<CCI_LEDsActuator>("leds");
  m_pcRABA =
      GetActuator<CCI_EPuckRangeAndBearingActuator>("epuck_range_and_bearing");
  m_pcRABS =
      GetSensor<CCI_EPuckRangeAndBearingSensor>("epuck_range_and_bearing");
  m_pcRNG = CRandom::CreateRNG("argos");
  m_cGoStraightAngleRange.Set(-ToRadians(m_cAlpha), ToRadians(m_cAlpha));
  GetNodeAttributeOrDefault(t_node, "velocity", m_fWheelVelocity,
                            m_fWheelVelocity);
  simulationParams.Init(GetNode(t_node, "simulation_parameters"));
  simulationParams.g = simulationParams.g * 10;
  simulationParams.sigma = simulationParams.sigma * 10;

  // Determine container name (base + robot ID)
  ostringstream containerNameStream;
  containerNameStream << simulationParams.containerNameBase << robotId;

  gethInterface = new GethInterface(robotId,
				    simulationParams.contractABI,
				    simulationParams.contractAddress,
				    containerNameStream.str(),
				    simulationParams.containerNameBase,
				    "localhost",
				    "/root/templates/");

  enodes[robotId] = gethInterface->getEnode();
  gethInterface->startMining();

  // Colours read from robots could be changed and added here!
  red.Set(COLOR_STRENGTH, 0, 0, ALPHA_CHANNEL); 
  green.Set(0, COLOR_STRENGTH, 0, ALPHA_CHANNEL);
  blue.Set(0, 0, COLOR_STRENGTH, ALPHA_CHANNEL);

  // Assign the initial state of the robots: all in exploration state
  m_sStateData.State = SStateData::STATE_EXPLORING;

  std::string m_strOutput;
  m_strOutput = GetId();

  // Initial quality: has to be estimated in the first exploration state
  opinion.quality = 0;

  if (simulationParams.percentRed < simulationParams.percentBlue)
    simulationParams.percentRed = simulationParams.percentBlue;
  simulationParams.percentRed = simulationParams.percentRed / 100;
}

void EPuck_Environment_Classification::prepare() {

  opinion.countedCellOfActualOpinion = 0;
  opinion.quality = 0;
  collectedData.count = 0;

  CCI_EPuckRangeAndBearingActuator::TData toSend;
  toSend[0] = Id2Int(GetId()) + 1;
  toSend[1] = 0;
  toSend[2] = 0;
  toSend[3] = 0;
  m_pcRABA->SetData(toSend);
  m_pcRABS->ClearPackets();
  TurnLeds();

  // Assign the initial state of the robots: all in exploration state
  m_sStateData.State = SStateData::STATE_EXPLORING;

  // Assign the exploration time
  m_sStateData.remainingExplorationTime = 30;
  m_sStateData.explorDurationTime = m_sStateData.remainingExplorationTime;
  
}

void EPuck_Environment_Classification::ControlStep() {

  ConnectAndListen();
  TurnLeds();
  Move();

  switch (m_sStateData.State) {

  case SStateData::STATE_EXPLORING: {
    Explore();
    break;
  }

  case SStateData::STATE_DIFFUSING: {
    Diffusing();
    break;
  }
  }

  RandomWalk();
}

void EPuck_Environment_Classification::Explore() {

  /* 20 is for jamming */
  if (byzantineStyle == 20) {
    CCI_EPuckRangeAndBearingActuator::TData toSend;
    toSend[0] = robotId;
    toSend[1] = 0;
    toSend[2] = 0;
    toSend[3] = 1;
    m_pcRABA->SetData(toSend);
  }

  /* remainingExplorationTime it's the variable decremented each control step.
   * This variable represents the time that a robot must still spend in
   * exploration state. If this variable it's greater than zero, then it must be
   * decremented and the robot should do exploration's stuffs (Update counters
   * figuring out in which cell he is. It's done in loop function */
  if (m_sStateData.remainingExplorationTime > 0) {
    m_sStateData.remainingExplorationTime--;
  }

  /* If its time to change state, then the robot has to reset his own variables:
   * - Assign a new random exponential time: remainingExplorationTime and
   * explorDurationTime (used to keep trace of the exploration times, just for
   * statistic aims);
   * - Calculate the quality of the opinion, basing on the sensed datas (Number
   * of counted cells of actual opinion / Number of total counted cells);
   * - Reset counting variables (countedCellOfActualOpinion and count [total
   * number of cells counted]);
   * - Change state: Exploration->Diffusing;
   * - Generate a new Diffusing time (same as exploring, but used for Diffusing
   * state and calculated with different params for the random variable;
   */
  else {

    /* Calculate opinion ratio of white cells to total cells) */
    opinion.quality = (Real)((Real)(opinion.countedCellOfActualOpinion) /
                             (Real)(collectedData.count));

    /* If this robot is a Byzantine robot, it always uses quality estimate 1.0
     */
    // TODO: Use variables instead of numbers
    if (byzantineStyle == 1 || byzantineStyle == 11) {
      opinion.quality = 0.0;

      /* If this robot is a Byzantine robot, its quality estimate is
         drawn from a value between 0.0 and 1.0 */
    } else if (byzantineStyle == 2 || byzantineStyle == 12) {
      opinion.quality = 1.0;

    } else if (byzantineStyle == 3 || byzantineStyle == 13) {

      CRange<Real> zeroOne(0.0, 1.0);
      Real p = m_pcRNG->Uniform(zeroOne);
      if (p > 0.5) {
        opinion.quality = 0.0;
      } else {
        opinion.quality = 1.0;
      }

    } else if (byzantineStyle == 4 || byzantineStyle == 14) {
      opinion.quality = m_pcRNG->Uniform(CRange<Real>(0.0, 1.0));

    } else if (byzantineStyle == 5 || byzantineStyle == 15) {
      Real p = m_pcRNG->Gaussian(0.05, 0.0);
      opinion.quality += p;

      /* Constrain it between 0.0 and 1.0 */
      if (opinion.quality > 1.0)
        opinion.quality = 1.0;

      if (opinion.quality < 0.0)
        opinion.quality = 0.0;
    }

    opinion.countedCellOfActualOpinion = 0;
    collectedData.count = 0;
    m_sStateData.State = SStateData::STATE_DIFFUSING;

    int opinionInt = (int)(
        opinion.quality *
        10000000); // Convert opinion quality to a value between 0 and 10000000

    int argsEmpty[0] = {};

    long long wei = 5000000000000000000;

    /* Submit a vote via the new interface*/
    // TODO: attach wei again just for DEBUGGING!
    int arg = opinionInt;

    cout << " Voting !!" << endl;
    
    gethInterface->scInterface("vote", arg, 0);
    //gethInterface->scInterfaceCall0("getMean", 0);
    //gethInterface->scInterfaceCall0("localCount", 0);

    /* Ask for payout via the new interface*/

    CRange<Real> zeroOne(0.0, 1.0);
    Real p = m_pcRNG->Uniform(zeroOne);
    if (p < 0.1) {
      gethInterface->scInterface("askForPayout", 0);
    }

    if (byzantineStyle > 10 && byzantineStyle < 20) {
      for (int i = 0; i < simulationParams.maxFlooding - 1; i++) {
	gethInterface->scInterface("vote", arg, wei);
      }
    }

    // Assigning a new exploration and time, for the next exploration state
    m_sStateData.remainingExplorationTime = 30;
    m_sStateData.explorDurationTime = m_sStateData.remainingExplorationTime;
  }
}

void EPuck_Environment_Classification::Diffusing() {

  if (simulationParams.determineConsensus) {

    // Query if a consensus has been reached 
    if (!threadCurrentlyRunning) {
      threadCurrentlyRunning = true;
      thread t1(&EPuck_Environment_Classification::WaitForDecision, this);
      t1.detach();
    }
  }

  // Change to EXPLORING state and choose another opinion with decision rules
  m_sStateData.State = SStateData::STATE_EXPLORING;
}

// Implement the movement leaded by the random walk (see loop_function)
void EPuck_Environment_Classification::Move() {
  if (movement.actualDirection == 0) // Go straight
    m_pcWheels->SetLinearVelocity(m_fWheelVelocity, m_fWheelVelocity);
  else if (movement.actualDirection == 1) // Turn right
    m_pcWheels->SetLinearVelocity(m_fWheelVelocity, -m_fWheelVelocity);
  else if (movement.actualDirection == 2) // Turn left
    m_pcWheels->SetLinearVelocity(-m_fWheelVelocity, m_fWheelVelocity);
}

void EPuck_Environment_Classification::ObstacleAvoidance() {

  // Get readings from proximity sensor and sum them together
  const CCI_EPuckProximitySensor::TReadings &tProxReads =
      m_pcProximity->GetReadings();
  CVector2 cAccumulator;
  for (size_t i = 0; i < tProxReads.size(); ++i) {
    cAccumulator += CVector2(tProxReads[i].Value, tProxReads[i].Angle);
  }
  if (tProxReads.size() > 0)
    cAccumulator /= tProxReads.size();
  // If the angle of the vector is not small enough or the closest obstacle is
  // not far enough curve a little
  CRadians cAngle = cAccumulator.Angle();
  if (!(m_cGoStraightAngleRange.WithinMinBoundIncludedMaxBoundIncluded(
            cAngle) &&
        cAccumulator.Length() < m_fDelta)) {
    // Turn, depending on the sign of the angle
    if (cAngle.GetValue() > 0.0f) {
      m_pcWheels->SetLinearVelocity(m_fWheelVelocity, 0.0f);
    } else {
      m_pcWheels->SetLinearVelocity(0.0f, m_fWheelVelocity);
    }
  }
}

void EPuck_Environment_Classification::RandomWalk() {

  // walkTime represents the number of clock cycles (random number) of walk in a
  // random direction
  if (movement.walkTime == 0) // Is the walkTime in that direction finished? ->
  {                           // -> YES: change direction//

    if (movement.actualDirection ==
        0) // If robot was going straight then turn standing in ->
           // -> a position for an uniformly distribuited time //
    {
      CRange<Real> zeroOne(0.0, 1.0);
      Real p = m_pcRNG->Uniform(zeroOne);
      p = p * simulationParams.turn;
      Real dir = m_pcRNG->Uniform(CRange<Real>(-1.0, 1.0));
      if (dir > 0)
        movement.actualDirection = 1;
      else
        movement.actualDirection = 2;
      movement.walkTime = Floor(p);
    }

    else // The robot was turning, time to go straight for ->
         // -> an exponential period of time //
    {
      movement.walkTime =
          Ceil(m_pcRNG->Exponential((Real)simulationParams.LAMBDA) *
               4); // Exponential random generator. *50 is a scale factor for
                   // the time
      movement.actualDirection = 0;
    }
  } else { // NO: The period of time is not finished, decrement the ->
    // -> walkTime and keep the direction //
    movement.walkTime--;
  }
}

// Wait until a transaction is mined and the corresponding event is created
void EPuck_Environment_Classification::WaitForDecision() {

  string eventResult;

  cout << "Robot id is " << robotId << endl;
  // TODO: Get eventResult here
  // eventResult = eventInterfaceConsensus(robotId, interface, contractAddress,
  // nodeInt, simulationParams.blockchainPath);

  threadCurrentlyRunning = false;
}

void EPuck_Environment_Classification::ConnectAndListen() {

  set<int> currentNeighbors;

  const CCI_EPuckRangeAndBearingSensor::TPackets &tPackets =
      m_pcRABS->GetPackets();

  bool containedJammer = false;
  for (size_t i = 0; i < tPackets.size(); ++i) {
    currentNeighbors.insert(tPackets[i]->Data[0]);
    /* Check if there's a jammer */
    if (tPackets[i]->Data[3] == 1)
      containedJammer = true;
  }

  if (containedJammer)
    currentNeighbors.clear();

  UpdateNeighbors(currentNeighbors);
  m_pcRABS->ClearPackets();
}

// 0 = NOTHING (smart contract calls can return 0 as default value, therefore, 0
// should never be used);
// 1 = WHITE;
// 2 = BLACK;
// 3 = GREEN;
void EPuck_Environment_Classification::TurnLeds() {

  switch (opinion.actualOpinion) {

  case 1: {
    opinion.actualOpCol = CColor::WHITE;
    m_pcLEDs->SetAllColors(CColor::WHITE);
    break;
  }
  case 2: {
    opinion.actualOpCol = CColor::BLACK;
    m_pcLEDs->SetAllColors(CColor::BLACK);
    break;
  }
  case 3: {
    opinion.actualOpCol = CColor::GREEN;
    m_pcLEDs->SetAllColors(CColor::GREEN);
    break;
  }
  }
}

// Connect and disconnect Ethereum processes to/from each other
void EPuck_Environment_Classification::UpdateNeighbors(set<int> newNeighbors) {

  set<int> neighborsToAdd;
  set<int> neighborsToRemove;

  // Old neighbors minus new neighbors = neighbors that should be removed
  std::set_difference(
      neighbors.begin(), neighbors.end(), newNeighbors.begin(),
      newNeighbors.end(),
      std::inserter(neighborsToRemove, neighborsToRemove.end()));

  // New neighbors minus old neighbors = neighbors that should be added
  std::set_difference(newNeighbors.begin(), newNeighbors.end(),
                      neighbors.begin(), neighbors.end(),
                      std::inserter(neighborsToAdd, neighborsToAdd.end()));

  std::set<int>::iterator it;
  for (it = neighbors.begin(); it != neighbors.end(); ++it) {
    int i = *it;
  }

  for (it = newNeighbors.begin(); it != newNeighbors.end(); ++it) {
    int i = *it;
  }

  for (it = neighborsToRemove.begin(); it != neighborsToRemove.end(); ++it) {
    int i = *it;
    gethInterface->removePeer(enodes[i]);
  }

  for (it = neighborsToAdd.begin(); it != neighborsToAdd.end(); ++it) {
    int i = *it;
    gethInterface->addPeer(enodes[i]);
  }

  // Update neighbor array
  set<int> neighborsTmp(newNeighbors);
  neighbors = neighborsTmp;
}

REGISTER_CONTROLLER(EPuck_Environment_Classification,
                    "epuck_environment_classification_controller")

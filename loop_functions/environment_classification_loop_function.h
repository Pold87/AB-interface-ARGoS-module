#ifndef ENVIRONMENT_CLASSIFICATION_LOOP_FUNCTION_H
#define ENVIRONMENT_CLASSIFICATION_LOOP_FUNCTION_H
#define NUM_OF_CELLS     100
#define N_COL		 3
#include <argos3/core/simulator/loop_functions.h>
#include <argos3/core/simulator/entity/floor_entity.h>
#include <epuck_environment_classification.h>
#include <argos3/core/simulator/simulator.h>
#include <argos3/plugins/robots/e-puck/simulator/epuck_entity.h>
#include <argos3/core/utility/configuration/argos_configuration.h>
#include <string>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <stddef.h>


using namespace argos;

class CEnvironmentClassificationLoopFunctions : public CLoopFunctions {

	public:
  CEnvironmentClassificationLoopFunctions();
  virtual ~CEnvironmentClassificationLoopFunctions(){}

  virtual void Init(TConfigurationNode& t_node);
  virtual CColor GetFloorColor(const CVector2& c_pos_on_floor);

		inline UInt32 GetGrid(UInt32 i) {
			return grid[i];
		}

		virtual void Reset();
		virtual void InitRobots();
		virtual bool IsExperimentFinished();
		virtual void PreStep();
		virtual void UpdateStatistics(EPuck_Environment_Classification::Opinion& opinion, EPuck_Environment_Classification::SStateData& sStateData, bool isByzantine);
		virtual void Destroy();
		virtual void RandomWalk(EPuck_Environment_Classification::Movement& movement);
		virtual void UpdateCount(EPuck_Environment_Classification::CollectedData& CollectedData,UInt32 cell, CVector2 cPos, EPuck_Environment_Classification::Opinion& opinion, EPuck_Environment_Classification::SStateData& sStateData, std::string& id, EPuck_Environment_Classification::SimulationState& simulationParam);
		virtual void MoveRobotsAwayFromArena(UInt32 opinionsToAssign[]);
		virtual void AssignNewStateAndPosition();
private:
  virtual void fillSettings(TConfigurationNode& tEnvironment);
  virtual void getAndWriteStats();
  
  GethInterface *gethInterface;
  std::string containerName;	
  std::string contractAddress;
  std::string contractABI;  
  std::string containerNameBase;
  std::string enodes;
  
  /* Variable of the environment, help variables and experiment finished signal */
  CRange<Real> zeroOne;
  CRange<Real> bigRange;
  CRange<Real> arenaSizeRange;
  CRandom::CRNG* m_pcRNG;
  bool m_bExperimentFinished;
  CFloorEntity* m_pcFloor;
  UInt32 colorOfCell[N_COL], grid[NUM_OF_CELLS]; // Cells colors and grid with all the arenas cells

  int totalCells;
  Real arenaSize;
  double cellDimension;

  
  /* Defined colours (they are defined because like that there is the possibility to change
   * in every way. By the way, argos colors are usable
   */
  CColor red;
  CColor blue;
  CColor green;
  
  /* File's flags */
  std::ofstream everyTicksFile;		// Flag: write or not the situation(number of robots with opinions)every TOT ticks, true->write
  std::ofstream runsFile;			// Flag: write the result of every run, true->write
  std::ofstream globalStatFile;		// Flag: write global stats: time of exploring, diffusings,counted cells... true->write
  std::ofstream everyQualityFile;		// Flag: every robot writes his quality and his opinion, after each exploration state
  std::ofstream oneRobotFile;			// Flag: just robot "ff0" (always present) writes his qualities after each exp. state
  std::ofstream blockChainFile;			// Flag: just robot "ff0" (always present) writes his qualities after each exp. state
  std::ofstream timeFile; // Save the consensus time in seconds
  std::ofstream timeFileEnd; // Save the consensus time in seconds

  /* Flags to decide if save or not the files */
  bool everyTicksFileFlag;
  bool runsFileFlag;
  bool qualityFileFlag;
  bool globalStatFileFlag;
  bool oneRobotFileFlag;
  bool blockChainFileFlag;
  
  /* Counters for the number of robots in each state for every colour */
  UInt32 robotsInExplorationCounter[N_COL];
  UInt32 robotsInDiffusionCounter[N_COL];
  
  /* Byzantine counters for the number of robots in each state for every colour */	
  UInt32 byzantineRobotsInExplorationCounter[N_COL];
  UInt32 byzantineRobotsInDiffusionCounter[N_COL];
  
  /* When consensusReached is equal to the number of the colors the swarm reached a consensus */
  UInt32 consensusReached;
  
  /* When true the experiment is not run. It could be true because of the incorrect number of robots
   * with an initial opinions (r_0+b_0 != number_of_robots) or because the percentages of colors of the
   * cells is incorrect.
   */
  bool incorrectParameters;
  
  /* If an error somewhere occured */
  bool errorOccurred;
  
  /* True  -> Use percentage to assign the colors of the floor
   * False -> Use absolute numbers */
  bool using_percentage;
  
  /* True  -> Exit for consensous reached
   * False -> Exit for number of qualities counted
   */
  bool exitFlag;
  
  /* Percentage of coloured cells */
  Real percentageOfColors[N_COL];
  
  /* Variables used for catching statistics (not used anymore) */
  Real totalCountedCells, countedOpinions[N_COL], quality[N_COL], totalExploringTime, totalDiffusingTime[N_COL];
  UInt32 numberOfExplorations[N_COL],numberOfDiffusions[N_COL];
  
  /* Parameters passed from configuration file */
  std::string passedRadix;
  UInt32 timeStep; // save statistics in everyTicksFile every <timeStep> ticks
  UInt32 LAMBDA,turn; // Parameters for the randomWalk: Lambda is the exponential mean and turn is the uniform parameter
  UInt32 number_of_runs;
  UInt32 written_qualities;    // Number of qualities already written in the files
  UInt32 number_of_qualities;  // Total number of qualities that have to be written
  UInt32 max_time;
  
  /* Number of robots with i-th initial opinion */
  UInt32 initialOpinions[N_COL];
  
  UInt32 n_robots;  /* Swarm Size */
  UInt32 g;
  UInt32 sigma;
  UInt32 decisionRule;
  //  UInt32 ticksPerSecond;
  std::string baseDirLoop;
  std::string dataDir;
  std::string datadirBase;
  int numByzantine;
  int byzantineSwarmStyle;
  bool useClassicalApproach;
  int lengthOfRuns;
  bool subswarmConsensus;
  int colorMixing;
  bool determineConsensus;
};

#endif

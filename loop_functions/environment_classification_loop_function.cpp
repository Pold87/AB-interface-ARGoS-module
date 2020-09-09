#include <iostream>
#include <unistd.h>
#include "environment_classification_loop_function.h"
#include <argos3/core/utility/math/rng.h>
#include "generic_interface.h" /* Use geth from C++ */

#include <time.h>

#define N_COL 3

#define DEBUGLOOP true

/****************************************/
/****************************************/
CEnvironmentClassificationLoopFunctions::CEnvironmentClassificationLoopFunctions() :

  zeroOne(0.0f,1.0f),
  bigRange(0.0f,30000.0f),
  m_bExperimentFinished(false),
  m_pcFloor(NULL)
{
}

using namespace std;

static const int maxTime = 200; /* Maximum amount of time per robot to wait until
				   they received their ether */
static const int maxContractAddressTrials = 50; /* Repeats getting the contract address procedure (sometimes the first result is a TypeError */
static const int trialsMiningNotWorking = 40; /* If after x trials the number of white votes is still zero  */



CColor CEnvironmentClassificationLoopFunctions::GetFloorColor(const CVector2& c_pos_on_floor){
  UInt32 x;
  UInt32 y;
  UInt32 i;
  
  if ((c_pos_on_floor.GetX() > arenaSize) || (c_pos_on_floor.GetY() > arenaSize))
    return CColor::YELLOW;
  
    x = (UInt32)(((Real)c_pos_on_floor.GetX())/(Real)cellDimension);
    y = (UInt32)(((Real)c_pos_on_floor.GetY())/(Real)cellDimension);

    UInt32 numCellsPerRow = (UInt32) ((Real) arenaSize) / ((Real) cellDimension);
    
    i=(UInt32) (y * numCellsPerRow + x);
    
    switch ( grid[i])
      {
      case 0:
	return CColor::RED;
      case 1:
	return CColor::WHITE;
      case 2:
	return CColor::BLACK;
      }
    return CColor::YELLOW;
  }



void CEnvironmentClassificationLoopFunctions::fillSettings(TConfigurationNode& tEnvironment)
{
  try
  {
    /* Retrieving information about arena */
    GetNodeAttribute(tEnvironment, "number_of_red_cells", colorOfCell[0]);
    GetNodeAttribute(tEnvironment, "number_of_white_cells", colorOfCell[1]);
    GetNodeAttribute(tEnvironment, "number_of_black_cells",colorOfCell[2]);


    GetNodeAttribute(tEnvironment, "arena_size", arenaSize);
    GetNodeAttribute(tEnvironment, "cell_dimension", cellDimension);

    
    GetNodeAttribute(tEnvironment, "percent_red", percentageOfColors[0]);
    GetNodeAttribute(tEnvironment, "percent_white", percentageOfColors[1]);
    GetNodeAttribute(tEnvironment, "percent_black", percentageOfColors[2]);
    GetNodeAttribute(tEnvironment, "using_percentage", using_percentage);
    GetNodeAttribute(tEnvironment, "exit", exitFlag);

    /* Retrieving information about initial state of robots */
    GetNodeAttribute(tEnvironment, "r_0", initialOpinions[0]);
    GetNodeAttribute(tEnvironment, "w_0", initialOpinions[1]);
    GetNodeAttribute(tEnvironment, "b_0", initialOpinions[2]);
    GetNodeAttribute(tEnvironment, "number_of_robots", n_robots);
    GetNodeAttribute(tEnvironment, "number_of_qualities", number_of_qualities);

    /* Retrieving information about simulation parameters */
    GetNodeAttribute(tEnvironment, "g", g);
    GetNodeAttribute(tEnvironment, "sigma", sigma);
    GetNodeAttribute(tEnvironment, "lambda", LAMBDA);
    GetNodeAttribute(tEnvironment, "turn", turn);
    GetNodeAttribute(tEnvironment, "decision_rule", decisionRule);
    GetNodeAttribute(tEnvironment, "number_of_runs", number_of_runs);
    //    GetNodeAttribute(tEnvironment, "ticks_per_seconds", ticksPerSecond);

    /* Retrieving information about how to catch and where to save statistics */
    GetNodeAttribute(tEnvironment, "save_every_ticks", timeStep);
    GetNodeAttribute(tEnvironment, "save_every_ticks_flag", everyTicksFileFlag);
    GetNodeAttribute(tEnvironment, "save_every_run_flag", runsFileFlag);
    GetNodeAttribute(tEnvironment, "save_every_quality_flag", qualityFileFlag);
    GetNodeAttribute(tEnvironment, "save_every_robot_flag", oneRobotFileFlag);
    GetNodeAttribute(tEnvironment, "save_global_stat_flag", globalStatFileFlag);
    GetNodeAttribute(tEnvironment, "save_blockchain_flag", blockChainFileFlag);
    GetNodeAttribute(tEnvironment, "radix", passedRadix);
    GetNodeAttribute(tEnvironment, "base_dir_loop", baseDirLoop);
    GetNodeAttribute(tEnvironment, "data_dir", dataDir);
    GetNodeAttribute(tEnvironment, "num_byzantine", numByzantine);
    GetNodeAttribute(tEnvironment, "byzantine_swarm_style", byzantineSwarmStyle);
    GetNodeAttribute(tEnvironment, "use_classical_approach", useClassicalApproach);
    GetNodeAttribute(tEnvironment, "subswarm_consensus", subswarmConsensus);
    GetNodeAttribute(tEnvironment, "length_of_runs", lengthOfRuns);
    GetNodeAttribute(tEnvironment, "color_mixing", colorMixing);
    GetNodeAttribute(tEnvironment, "determine_consensus", determineConsensus);
    GetNodeAttribute(tEnvironment, "contract_address", contractAddress);
    GetNodeAttribute(tEnvironment, "contract_abi", contractABI);
    GetNodeAttribute(tEnvironment, "container_name_base", containerNameBase);
  }
  catch(CARGoSException& ex)
  {
    THROW_ARGOSEXCEPTION_NESTED("Error parsing loop functions!", ex);
  }
}

void CEnvironmentClassificationLoopFunctions::InitRobots()
{

  errorOccurred = false;

  /* Resetting number of opinions that have to be written */
  written_qualities = 0;

  m_pcRNG->SetSeed((int)m_pcRNG->Uniform(bigRange));
  m_pcRNG->Reset();

  GetSpace().SetSimulationClock(0);
  consensusReached = N_COL;

  for(size_t i = 0; i<N_COL; i++)
  {
    robotsInExplorationCounter[i] = 0;
    robotsInDiffusionCounter[i] = 0;
  }

  if (colorMixing == 1) {

    int temp1;
    /* Mix the colours in the vector of cells to avoid the problem of eventual correlations*/
    for (int k = 0; k < 8; k++)
    {
      for (int i = totalCells - 1; i >= 0; --i)
      {
        int j = ((int)m_pcRNG->Uniform(bigRange)%(i+1));
        temp1 = grid[i];
        grid[i] = grid[j];
        grid[j] = temp1;
      }
    }
  }


  /* Helper array, used to store and shuffle the initial opinions of the robots */
  UInt32 opinionsToAssign[n_robots];

  /* Build a vector containing the initial opinions */
  for (int i=0; i<initialOpinions[0]; i++)
    opinionsToAssign[i] = 0;

  for (int i=initialOpinions[0] ; i<initialOpinions[0]+initialOpinions[1]; i++)
    opinionsToAssign[i] = 1;

  for (int i=initialOpinions[0]+initialOpinions[1]; i<n_robots; i++)
    opinionsToAssign[i] = 2;

  /* Vector shuffling, for randomize the opinions among the robots */
  for (int i = n_robots-1; i >= 0; --i)
  {
    int j = ((int)m_pcRNG->Uniform(bigRange) % (i+1));
    int temp = opinionsToAssign[i];
    opinionsToAssign[i] = opinionsToAssign[j];
    opinionsToAssign[j] = temp;
  }

  int remainingByzantine = numByzantine;

  /* Variable i is used to check the vector with the mixed opinion to assign a new opinion to every robots*/
  int i = 0;
  CSpace::TMapPerType& m_cEpuck = GetSpace().GetEntitiesByType("epuck");
  for(CSpace::TMapPerType::iterator it = m_cEpuck.begin(); it != m_cEpuck.end(); ++it)
  {
    /* Get handle to e-puck entity and controller */
    CEPuckEntity& cEpuck = *any_cast<CEPuckEntity*>(it->second);
    EPuck_Environment_Classification& cController =  dynamic_cast<EPuck_Environment_Classification&>(cEpuck.GetControllableEntity().GetController());
    EPuck_Environment_Classification::Opinion& opinion = cController.GetOpinion();
    EPuck_Environment_Classification::CollectedData& collectedData = cController.GetColData();

    /* Resetting initial state of the robots: exploring for everyone */
    cController.GetStateData().State = EPuck_Environment_Classification::SStateData::STATE_EXPLORING;

    /* Assign a random actual opinion using the shuffled vector */
    opinion.actualOpinion = opinionsToAssign[i];
    i++;


    if (remainingByzantine > 0)
    {
      cController.setByzantineStyle(byzantineSwarmStyle);
      remainingByzantine--;
    }
    else
    {
      cController.setByzantineStyle(0);
    }

    opinion.countedCellOfActualOpinion = 0;
    collectedData.count = 0;
    if(opinion.actualOpinion == 1)
      opinion.actualOpCol = CColor::WHITE;
    if(opinion.actualOpinion == 2)
      opinion.actualOpCol = CColor::BLACK;
    /* Setting robots initial states: exploring state */

    cController.prepare();
  }

  AssignNewStateAndPosition();
}

void CEnvironmentClassificationLoopFunctions::getAndWriteStats() {

      // Select a random robot
    CRange<UInt32> rangeNumRobots(0, n_robots);
    UInt32 selRobot = m_pcRNG->Uniform(rangeNumRobots);
    UInt32 s = 0;

    cout << "Random number is " << selRobot << endl;
    
    string mean, localCount, voteCount, weight, blockchainSize, blockNumber;
    int totalSubmittedVotes = 0;

    CSpace::TMapPerType& m_cEpuck = GetSpace().GetEntitiesByType("epuck");
    for(CSpace::TMapPerType::iterator it = m_cEpuck.begin(); it != m_cEpuck.end(); ++it) {

      /* Get handle to e-puck entity and controller */
      CEPuckEntity& cEpuck = *any_cast<CEPuckEntity*>(it->second);

      EPuck_Environment_Classification& cController =  dynamic_cast<EPuck_Environment_Classification&>(cEpuck.GetControllableEntity().GetController());

      totalSubmittedVotes += cController.getMySubmittedVotes();

      if (s == selRobot) {

	cout << "Selected robot " << s << endl;
      

      mean = cController.getGethInterface().scReturn0("getMean", 0);
      voteCount = cController.getGethInterface().scReturn0("voteCount", 0);    
      localCount = cController.getGethInterface().scReturn0("weightCount", 0);    
      blockNumber = cController.getGethInterface().scReturn0("getBlockNumber", 0);  
      blockchainSize = cController.getGethInterface().getBlockChainSize();
      
      cout << "Getting weight" << endl;
      weight = cController.getGethInterface().scReturn0("getWeight", 0);
      }
      s++;      
    }

    if (blockChainFile.is_open()) {
//      blockChainFile << GetSpace().GetSimulationClock() << "\t" << mean << "\t" << localCount << "\t" << voteCount << "\t" << totalSubmittedVotes << "\t" << blockNumber << "\t" << blockchainSize << endl;
      blockChainFile << GetSpace().GetSimulationClock() << "\t" << mean << "\t" << localCount << "\t" << voteCount << "\t" << totalSubmittedVotes << "\t" << blockNumber << "\t" << "NA" << endl;
    }
    
    if (everyTicksFile.is_open()) {
      everyTicksFile << (GetSpace().GetSimulationClock()) << "\t";
      everyTicksFile << number_of_runs << "\t";

      for ( UInt32 c = 0; c < N_COL; c++ )
      {
        everyTicksFile << robotsInExplorationCounter[c] << "\t\t" << robotsInDiffusionCounter[c]  << "\t\t" << blockchainSize;
      }
      everyTicksFile << std::endl;
    }
}

void CEnvironmentClassificationLoopFunctions::Init(TConfigurationNode& t_node) {

  TConfigurationNode& tEnvironment = GetNode(t_node, "cells");
  fillSettings(tEnvironment);

  totalCells = (arenaSize / cellDimension) * (arenaSize / cellDimension);
  
  time_t ti;
  time(&ti);
  std::string m_strOutput;
  std::stringstream nRunsStream;
  nRunsStream << number_of_runs;
  std::string nRuns = nRunsStream.str();
  m_strOutput = dataDir + passedRadix +"-timestart.RUN" + nRuns;
  incorrectParameters = false;
  m_pcRNG = CRandom::CreateRNG("argos");

  /* Setting variables according with the parameters of the configuration file (XML) */

  /* Translating percentages in numbers */
  if(using_percentage)
  {
    for( int c = 0; c < N_COL; c++ )
    {
      colorOfCell[c] = (int)(percentageOfColors[c]*((Real)totalCells/100.0));
      cout << "Color: " << colorOfCell[c] << endl;
    }
  }

  /*
   * Check: number of robots = sum of the initial opinions
   *        number of colors cells sums up to the total number of cells
   */
  if((n_robots == initialOpinions[0]+initialOpinions[1]+initialOpinions[2])&&(colorOfCell[0]+colorOfCell[1]+colorOfCell[2] == totalCells))
  {
    consensusReached = N_COL;
    /* Multiply sigma and G per 10, so that they will be transformed into ticks (were inserted as  seconds) */
    sigma = sigma * 10;
    g = g * 10;
    max_time = max_time * 10;

    int k = 0;


      int myrow;
      // Create regular grid
      if (colorMixing == 3) {
	while ( k <  totalCells) {

	  myrow = k / 20;

	  int everyXTile = 20;
	  
	  // numStripes calculates the number of stripes but does not say anything about the placement
	int numStripes = (percentageOfColors[2] * (colorOfCell[1] + colorOfCell[2])) / 2000;

	
	vector<int> specialRows; 
	
	switch (numStripes) {
	case 0:
	case 20:
	  break;
	case 2:
	case 18:
	  specialRows.push_back(9);
	  specialRows.push_back(11);	  
	  break;

	case 4:
	case 16:
	  specialRows.push_back(7);
	  specialRows.push_back(9);	  
	  specialRows.push_back(11);
	  specialRows.push_back(13);	  
	  break;

	case 6:
	case 14:
	  specialRows.push_back(5);
	  specialRows.push_back(7);
	  specialRows.push_back(9);	  
	  specialRows.push_back(11);
	  specialRows.push_back(13);	  
	  specialRows.push_back(15);	  
	  break;


	case 8:
	case 12:
	  specialRows.push_back(3);
	  specialRows.push_back(5);
	  specialRows.push_back(7);
	  specialRows.push_back(9);	  
	  specialRows.push_back(11);
	  specialRows.push_back(13);	  
	  specialRows.push_back(15);
	  specialRows.push_back(17);	  

	  break;

	case 10:
	  specialRows.push_back(1);
	  specialRows.push_back(3);
	  specialRows.push_back(5);
	  specialRows.push_back(7);
	  specialRows.push_back(9);	  
	  specialRows.push_back(11);
	  specialRows.push_back(13);	  
	  specialRows.push_back(15);
	  specialRows.push_back(17);
	  specialRows.push_back(19);	  
	  
	  break;
	  
	}

	int stripesColor, normalColor;
	if (numStripes > 10) {
	  stripesColor = 1;
	  normalColor = 2;
	} else {
	  stripesColor = 2;
	  normalColor = 1;
	}
	
	if (find(specialRows.begin(), specialRows.end(), myrow)!=specialRows.end()) {
	    grid[k] = stripesColor;
	} else {
	  grid[k] = normalColor;
	}
	
	
	// if ((myrow % everyXRow) == 0)  {
	//   grid[k] = 2;
	// } else {
	//   grid[k] = 1;
	// }
	
	k++;
	}	

      } if (colorMixing == 4) {

	int acc = 0;
	
	int col1 = 1;
	int col2 = 2;

	int goal = 0;
	int myrow;
	
	for ( int i = 0; i < N_COL; i++ ) {
	  for( int j = 0; j < colorOfCell[i] ; j++,k++ ) {
	    

	    myrow = k / 20;

	    goal = myrow % 10;

	    int percent = (int) percentageOfColors[2];
	    
	    switch (percent) {

	    case 0:
	      grid[k] = 1;
	      break;
	    case 10:
	    if (k % 10 == goal) {
	      grid[k] = 2;
	    } else {
	      grid[k] = 1;
	    }	      
	      break;
	    case 20:
	    if (k % 10 == goal || k % 10 == (goal + 5) % 10) {
	      grid[k] = 2;
	    } else {
	      grid[k] = 1;
	    }	      	      
	      break;
	    case 30:
	    if (k % 10 == goal || k % 10 == (goal + 3) % 10 || k % 10 == (goal + 6) % 10) {
	      grid[k] = 2;
	    } else {
	      grid[k] = 1;
	    }	      	      	      
	      break;
	    case 40:
	    if (k % 10 == goal || k % 10 == (goal + 3) % 10 || k % 10 == (goal + 6) % 10 | k % 10 == (goal + 8) % 10) {
	      grid[k] = 2;
	    } else {
	      grid[k] = 1;
	    }	      	      	      	      
	      break;
	    case 50:
	    if (k % 10 == goal || k % 10 == (goal + 2) % 10 || k % 10 == (goal + 4) % 10 || k % 10 == (goal + 6) % 10 || k % 10 == (goal + 8) % 10) {
	      grid[k] = 2;
	    } else {
	      grid[k] = 1;
	    }	      
	      break;
	    case 60:
	    if (k % 10 == goal || k % 10 == (goal + 3) % 10 || k % 10 == (goal + 6) % 10 | k % 10 == (goal + 8) % 10) {
	      grid[k] = 1;
	    } else {
	      grid[k] = 2;
	    }	      	      	      	      	      
	      break;
	    case 70:
	    if (k % 10 == goal || k % 10 == (goal + 3) % 10 || k % 10 == (goal + 6) % 10) {
	      grid[k] = 1;
	    } else {
	      grid[k] = 2;
	    }	      	      	      
	      break;
	    case 80:
	    if (k % 10 == goal || k % 10 == (goal + 5) % 10) {
	      grid[k] = 1;
	    } else {
	      grid[k] = 2;
	    }	      	      	      
	      break;
	    case 90:
	    if (k % 10 == goal) {
	      grid[k] = 1;
	    } else {
	      grid[k] = 2;
	    }	      	      
	      break;
	    case 100:
	      grid[k] = 2;
	      break;
	    }
	    
	  }
	}
		
	
      } else {

    
    /* Generate random color for each cell according with the choosen probabilities*/
    for ( int i = 0; i < N_COL; i++ )
      for( int j = 0; j < colorOfCell[i] ; j++,k++ )
        grid[k] = i;

      }
    
    UInt32 i=0;

    m_pcRNG->SetSeed(std::clock());
    m_pcRNG->Reset();

    i=0;
    /* Setting variables for statistics */
    totalCountedCells  = 0;
    totalExploringTime = 0;
    for( int c = 0; c < N_COL; c++ )
    {
      /* Each position of the vectors corresponds to a different colour (Eg: Posiz 0 = RED, 1 = GREEN, 2 = BLUE */
      countedOpinions[c]      = 0;
      quality[c]              = 0;
      totalDiffusingTime[c]   = 0;
      numberOfExplorations[c] = 0;
    }

    /* Initialize the robots */
    InitRobots();
    i = 0;

    /*
     * File saving number of diffusing and exploring robots, for every opinion.
     * It saves situation every timeStep ticks (timeStep seconds)
     */
    if(everyTicksFileFlag)
    {
      std::stringstream ss;
      ss << number_of_runs;
      std::string nRuns = ss.str();
      m_strOutput = dataDir + passedRadix +".RUN"+nRuns;
      everyTicksFile.open(m_strOutput.c_str(), std::ios_base::trunc | std::ios_base::out);
      everyTicksFile << "clock\trun\texploringWhite\tdiffusingWhite\texploringGreen\tdiffusingGreen\texploringBlack\tdiffusingBlack\tbyzantineExploringWhite\tbyzantineDiffusingWhite\tbyzantineExploringGreen\tbyzantineDiffusingGreen\tbyzantineExploringBlack\tbyzantineDiffusingBlack\t" << std::endl;
    }

    /* Blockchain Statistics */
    if(blockChainFileFlag) {
      
      std::stringstream ss;
      ss << number_of_runs;
      std::string nRuns = ss.str();
      
      m_strOutput = dataDir + passedRadix +"-blockchain.RUN" + nRuns;
      blockChainFile.open(m_strOutput.c_str(), std::ios_base::trunc | std::ios_base::out);
      blockChainFile << "clock\tmean\tcount\tvoteCount\ttotalSubmittedVotes\tblockNumber\tblockchain_size_KB\tblockchain_path" << std::endl;
    }
    
    /*
     * File saving the the exit time and the number of robots (per opinion) after every run has been executed
     */
    if(runsFileFlag)
    {
      m_strOutput = dataDir + passedRadix+".RUNS";
      runsFile.open(m_strOutput.c_str(), std::ios_base::out | std::ios_base::app);
      runsFile << "Runs\t\tExitTime\tWhites\t\tGreens\t\tBlacks" << std::endl;
    }

    /*
     * File saving the quality and the opinion of every robots after every changing from exploration to diffusing state
     * (the quality and the   actualOpinion are the definitive ones)
     */
    if(qualityFileFlag)
    {
      m_strOutput = dataDir + passedRadix + ".qualitiesFile";
      everyQualityFile.open(m_strOutput.c_str(), std::ios_base::trunc | std::ios_base::out);
      everyQualityFile << "Q\tOP" << std::endl;
    }

    /* File saving all the statistics (times, counted cells and qualities) after the whole experiment is finished */
    if(globalStatFileFlag)
    {
      m_strOutput = dataDir + passedRadix + ".globalStatistics";
      globalStatFile.open(m_strOutput.c_str(), std::ios_base::trunc | std::ios_base::out);
      globalStatFile << "TEX\tTC\tTDR\tQR\tCR\tTDG\tQG\tCG\tTDB\tQB\tCB\t" << std::endl;
    }
    if(oneRobotFileFlag)
    {
      m_strOutput = dataDir + passedRadix + ".oneRobotFile";
      oneRobotFile.open(m_strOutput.c_str(), std::ios_base::trunc | std::ios_base::out);
    }

    /* Incorrect parameters (Number of robots or initial colors) -> Terminate the execution without write files */
  }
  else
  {
    incorrectParameters = true;
    IsExperimentFinished();
  }

  // Kill the bootstrap Ethereum node
  system("docker service rm ethereum_bootstrap");
}

void CEnvironmentClassificationLoopFunctions::Reset()
{

}

/* Move every robot away from the arena*/
void CEnvironmentClassificationLoopFunctions::MoveRobotsAwayFromArena(UInt32 opinionsToAssign[])
{

  /* t is the index for the opinionToAssign vector */
  int t=0;
  /* Move every robot away from the arena (cNewPosition = CVector3(150.0f, 15.0f, 0.1f): a casual
     position) and assign a new initial opinion (opinionToAssign[t]) */
  CSpace::TMapPerType& m_cEpuck = GetSpace().GetEntitiesByType("epuck");
  for(CSpace::TMapPerType::iterator it = m_cEpuck.begin(); it != m_cEpuck.end(); ++it)
  {
    CEPuckEntity& cEpuck = *any_cast<CEPuckEntity*>(it->second);
    EPuck_Environment_Classification& cController =  dynamic_cast<EPuck_Environment_Classification&>(cEpuck.GetControllableEntity().GetController());
    CQuaternion cNewOrientation = cEpuck. GetEmbodiedEntity().GetOriginAnchor().Orientation;
    CVector3 cNewPosition = CVector3(1.0f, 1.0f, 0.1f);
    cEpuck.GetEmbodiedEntity().MoveTo(cNewPosition, cNewOrientation, false);
    EPuck_Environment_Classification::Opinion& opinion = cController.GetOpinion();
    EPuck_Environment_Classification::CollectedData& collectedData = cController.GetColData();

    /* Assign a random actual opinion using the shuffled vector */
  }
}

/* Assign a new state and a new position to the robots */
void CEnvironmentClassificationLoopFunctions::AssignNewStateAndPosition()
{

  CSpace::TMapPerType& m_cEpuck = GetSpace().GetEntitiesByType("epuck");
  for(CSpace::TMapPerType::iterator it = m_cEpuck.begin(); it != m_cEpuck.end(); ++it)
  {
    CEPuckEntity& cEpuck = *any_cast<CEPuckEntity*>(it->second);
    EPuck_Environment_Classification& cController =  dynamic_cast<EPuck_Environment_Classification&>(cEpuck.GetControllableEntity().GetController());
    // CQuaternion cNewOrientation = cEpuck. GetEmbodiedEntity().GetOriginAnchor().Orientation;

    // /* Generating Uniformly distribuited x and y coordinates for the new position of the robot */
    // Real xp = m_pcRNG->Uniform(arenaSizeRangeX);
    // Real yp = m_pcRNG->Uniform(arenaSizeRangeY);
    // CVector3 cNewPosition = CVector3(xp,yp,0.1f);

    // UInt32 i, unMaxRepositioningTrials = 100;

    // for(i = 0; i < unMaxRepositioningTrials; i++)
    // {
    //   if(cEpuck.GetEmbodiedEntity().MoveTo(cNewPosition, cNewOrientation, false)) break;

    //   xp = m_pcRNG->Uniform(arenaSizeRangeX);
    //   yp = m_pcRNG->Uniform(arenaSizeRangeY);

    //   cNewPosition.SetX(xp);
    //   cNewPosition.SetY(yp);
    // }

    /* Resetting initial state of the robots: exploring for everyone */
    cController.GetStateData().State = EPuck_Environment_Classification::SStateData::STATE_EXPLORING;
  }
}

/************************************************ IS EXPERIMENT FINISHED ***************************************/
/***************************************************************************************************************/
bool CEnvironmentClassificationLoopFunctions::IsExperimentFinished()
{

  /* If parameters are uncorrect then finish the experiment (Eg: number of robots vs sum of the initial opinion,
   * or the colours of the cells mismatching */
  if( incorrectParameters )
  {
    cout << "incorrectParameters was true" << endl;
    Reset();
    return true;
  }

  if( errorOccurred )
  {
    cout << "errorOccured was true" << endl;
    Reset();
    return true;
  }

  /* Vary termination condition: [GetSpace().GetSimulationClock() >= max_time] [consensusReached != N_COL]
   * [NumberOfQualities == WrittenQualities ] */
  CSpace::TMapPerType& m_cEpuck = GetSpace().GetEntitiesByType("epuck");
  CSpace::TMapPerType::iterator it = m_cEpuck.begin();
  if(exitFlag)
  {
    if( consensusReached == 100 || GetSpace().GetSimulationClock() == lengthOfRuns)
    {
      number_of_runs--;

      getAndWriteStats();
      
      /* RUNSFILE: Write statistics of the last run */
      if (runsFile.is_open())
      {
        runsFile << number_of_runs+1 <<"\t\t"
                 << (GetSpace().GetSimulationClock()-1) <<"\t\t";

        for ( UInt32 c = 0; c < N_COL; c++ )
          runsFile << robotsInDiffusionCounter[c] + robotsInExplorationCounter[c] <<"\t\t";
        runsFile<<std::endl;
      }

      if (number_of_runs<=0)
      {

        /* Close runsFile*/
        if(runsFile.is_open()) {
          runsFile.close();
        }

        if (everyQualityFile.is_open()) {
          everyQualityFile.close();
        }

	if (blockChainFile.is_open()) {
	  blockChainFile.close();
	}

	
        /* Calculate statistics */
        UInt32 totalNumberOfExplorations = 0;
        for ( UInt32 c = 0; c < N_COL; c++)
        {
          totalNumberOfExplorations += numberOfExplorations[c];
          totalDiffusingTime[c] = totalDiffusingTime[c]/numberOfDiffusions[c];
          quality[c] = (Real)((quality[c])/((Real)numberOfExplorations[c]));
          countedOpinions[c] = (Real)((Real)(countedOpinions[c])/(Real)(numberOfExplorations[c]));
        }
        totalExploringTime = (Real)((Real)totalExploringTime/(Real)totalNumberOfExplorations);
        totalCountedCells=(Real)((Real)totalCountedCells/(Real)totalNumberOfExplorations);


        /* globalStatFile: write the general statistics, such as counted cells,
           times of diffusing and exploring */
        if (globalStatFile.is_open())
        {
          globalStatFile<< std::setprecision(3) << totalExploringTime << "\t"
                        << std::setprecision(3) << totalCountedCells << "\t";
          for ( UInt32 c = 0; c < N_COL; c++)
          {
            globalStatFile<< std::setprecision(3) << totalDiffusingTime[c] << "\t"
                          << std::setprecision(3) << quality[c] << "\t"
                          << std::setprecision(3) << countedOpinions[c] << "\t";
          }
          globalStatFile.close();
        }

        /* Set experimentFinished variable to true -> the experiment will terminate */
        std::cout << "Consensus is reached and the experiment is FINISHED" << std::endl;
        m_bExperimentFinished = true;

      }
      else
      {
        written_qualities = 0;
        Reset();
      }
    }
    return m_bExperimentFinished;
  }
  else
  {

    if( written_qualities == number_of_qualities)  //consensusReached != N_COL){  written_qualities == number_of_qualities
    {
      number_of_runs--;

      /* RUNSFILE: Write statistics of the last run */
      if (runsFile.is_open())
      {
        runsFile << number_of_runs+1 <<"\t\t"
                 << (GetSpace().GetSimulationClock()-1) <<"\t\t";

        for ( UInt32 c = 0; c < N_COL; c++ )
          runsFile << robotsInDiffusionCounter[c] + robotsInExplorationCounter[c] <<"\t\t";
        runsFile<<std::endl;
      }


      time_t ti_end;
      time(&ti_end);

      std::string m_strOutput;
      std::stringstream nRunsStream;
      nRunsStream << number_of_runs;
      std::string nRuns = nRunsStream.str();

      if (number_of_runs<=0)
      {

        /* Close runsFile*/
        if(runsFile.is_open())
        {
          runsFile.close();
        }
        /* Calcolate statistics */
        UInt32 totalNumberOfExplorations = 0;
        for ( UInt32 c = 0; c < N_COL; c++)
        {
          totalNumberOfExplorations += numberOfExplorations[c];
          totalDiffusingTime[c] = (Real)((Real)totalDiffusingTime[c]/(Real)numberOfDiffusions[c]);
          quality[c] = (Real)((quality[c])/((Real)numberOfExplorations[c]));
          countedOpinions[c] = (Real)((Real)(countedOpinions[c])/(Real)(numberOfExplorations[c]));
        }
        totalExploringTime = (Real)((Real)totalExploringTime/(Real)totalNumberOfExplorations);
        totalCountedCells=(Real)((Real)totalCountedCells/(Real)totalNumberOfExplorations);

        /* Close qualities file */
        if (everyQualityFile.is_open())
        {
          everyQualityFile.close();
        }


        /* globalStatFile: write the general statistics, such as counted cells,
           times of diffusing and exploring */
        if (globalStatFile.is_open())
        {
          globalStatFile<< std::setprecision(3) << totalExploringTime << "\t"
                        << std::setprecision(3) << totalCountedCells << "\t";
          for ( UInt32 c = 0; c < N_COL; c++)
          {
            globalStatFile<< std::setprecision(3) << totalDiffusingTime[c] << "\t"
                          << std::setprecision(3) << quality[c] << "\t"
                          << std::setprecision(3) << countedOpinions[c] << "\t";
          }
          globalStatFile.close();
        }

        /* Set experimentFinished variable to true -> the experiment will terminate */
        m_bExperimentFinished = true;
      }
      else
      {
        written_qualities = 0;
        Reset();
      }
    }
    return m_bExperimentFinished;
  }
}

/************************************************* DESTROY *****************************************************/
/***************************************************************************************************************/
void CEnvironmentClassificationLoopFunctions::Destroy()
{

}

/************************************************ PRESTEP ******************************************************/
/***************************************************************************************************************/

void CEnvironmentClassificationLoopFunctions::PreStep()
{

  /* Reset counters: these array are counting the number of robots in each state. Every position corresponds to a color:
     robotsInExplorationCounter[0] -> number of robots exploring with opinion red
     robotsInExplorationCounter[1] -> number of robots exploring with opinion green
     ... */

  
  for ( UInt32 c=0; c<N_COL; c++ )
  {
    robotsInExplorationCounter[c] = 0;
    robotsInDiffusionCounter[c] = 0;
  }
  CSpace::TMapPerType& m_cEpuck = GetSpace().GetEntitiesByType("epuck");
  for(CSpace::TMapPerType::iterator it = m_cEpuck.begin(); it != m_cEpuck.end(); ++it)
  {
    /* Get handle to e-puck entity and controller */
    CEPuckEntity& cEpuck = *any_cast<CEPuckEntity*>(it->second);

    EPuck_Environment_Classification& cController =  dynamic_cast<EPuck_Environment_Classification&>(cEpuck.GetControllableEntity().GetController());

    Real x = cEpuck. GetEmbodiedEntity().GetOriginAnchor().Position.GetX(); // X coordinate of the robot
    Real y = cEpuck. GetEmbodiedEntity().GetOriginAnchor().Position.GetY(); // Y coordinate of the robot

    CVector2 cPos;
    cPos.Set(x,y);						// Vector position of the robot
    /* Figure out in which cell (EG: which is the index of the array grid) the robot is */
    UInt32 cell = (UInt32) ((y+0.009)*10000)/(Real) (cellDimension * 100000);
    cell = (UInt32) 40*cell + ((x+0.009)*10000)/(Real)(cellDimension * 100000);

    /* Get parameters of the robot: color, state, opinion and movement datas*/
    EPuck_Environment_Classification::CollectedData& collectedData = cController.GetColData();
    EPuck_Environment_Classification::SStateData& sStateData = cController.GetStateData();
    EPuck_Environment_Classification::Movement& movement = cController.GetMovement();
    EPuck_Environment_Classification::Opinion& opinion = cController.GetOpinion();
    std::string id = cController.GetId();
    EPuck_Environment_Classification::SimulationState& simulationParam = cController.GetSimulationState();

    /* Update statistics about the robot opinions*/

    /* TODO: remove third argument */
    UpdateStatistics(opinion, sStateData, false);
    UpdateCount(collectedData, cell, cPos, opinion, sStateData, id, simulationParam);
    RandomWalk(movement);
  }

  /* Check if a consensus has been reached (i.e., SE is below threshold) */


  if (determineConsensus)
  {

    bool totalConsensusReached = true;
    for(CSpace::TMapPerType::iterator it = m_cEpuck.begin(); it != m_cEpuck.end(); ++it)
    {
      /* Get handle to e-puck entity and controller */
      CEPuckEntity& cEpuck = *any_cast<CEPuckEntity*>(it->second);

      EPuck_Environment_Classification& cController =  dynamic_cast<EPuck_Environment_Classification&>(cEpuck.GetControllableEntity().GetController());
      totalConsensusReached = totalConsensusReached && cController.getConsensusReached();
    }

    if (totalConsensusReached)
    {
      consensusReached = 100;
    }
  }

  /* EVERYTICKSFILE: Write this statistics only if the file is open and it's the right timeStep (multiple of timeStep) */
  if ( ! (GetSpace().GetSimulationClock() % timeStep) ) {
    cout << "Time step is " << GetSpace().GetSimulationClock() << endl;
    getAndWriteStats();
  }
}

void CEnvironmentClassificationLoopFunctions::UpdateStatistics(EPuck_Environment_Classification::Opinion& opinion,
    EPuck_Environment_Classification::SStateData& sStateData, bool isByzantine)
{

  /* Increment counters of the opinions and states of the robots */

  if (sStateData.State == EPuck_Environment_Classification::SStateData::STATE_EXPLORING)
    robotsInExplorationCounter[opinion.actualOpinion]++;
  if (sStateData.State == EPuck_Environment_Classification::SStateData::STATE_DIFFUSING)
    robotsInDiffusionCounter[opinion.actualOpinion]++;
}

/* Update count of the total number of cells and of the cells according with the opinion*/
void CEnvironmentClassificationLoopFunctions::UpdateCount(EPuck_Environment_Classification::CollectedData& collectedData, UInt32 cell, CVector2 cPos, EPuck_Environment_Classification::Opinion& opinion, EPuck_Environment_Classification::SStateData& sStateData, std::string& id, EPuck_Environment_Classification::SimulationState& simulationParam)
{

  collectedData.readColor = GetFloorColor(cPos);

  if(collectedData.readColor == opinion.actualOpCol) 	       // If is as my opinion //
    opinion.countedCellOfActualOpinion++;                  // Increment opinion counter)//

  collectedData.count++;

  // However increment the count of cells passed thorugh //

  /* Collecting datas for statistics: times and qualities *///
  if(sStateData.remainingExplorationTime == 1)
  {

    oneRobotFile << std::setprecision(3) << opinion.quality << " ";
    written_qualities++;
  }
}
/* Implement random walk */
void CEnvironmentClassificationLoopFunctions::RandomWalk(EPuck_Environment_Classification::Movement& movement)
{

  /* walkTime represents the number of clock cycles (random number) of walk in a random direction*/
  if ( movement.walkTime == 0 )                            // Is the walkTime in that direction finished? ->
  {
    // -> YES: change direction//

    if ( movement.actualDirection == 0 )                  // If robot was going straight then turn standing in ->
      // -> a position for an uniformly distribuited time //
    {
      Real p = m_pcRNG->Uniform(zeroOne);
      p = p*turn - (turn/2);
      if ( p > 0 )
        movement.actualDirection = 1;
      else
        movement.actualDirection = 2;
      movement.walkTime = (UInt32) abs(p);
    }

    else 						// The robot was turning, time to go straight for ->
      // -> an exponential period of time //
    {
      movement.walkTime = (m_pcRNG->Exponential((Real) LAMBDA )) * 4; // Exponential random generator. *50 is a scale factor for the time
      movement.actualDirection = 0;
    }
  }
  else 							// NO: The period of time is not finished, decrement the ->
    // -> walkTime and keep the direction //
    movement.walkTime--;
}

REGISTER_LOOP_FUNCTIONS(CEnvironmentClassificationLoopFunctions, "environment_classification_loop_functions")

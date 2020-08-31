#!/bin/sh

USERNAME=`whoami`
mailto='volker.strobel87@gmail.com'
DOCKERBASE='/home/volker/Documents/mygithub-software/ARGoS-Blockchain-interface/'
TEMPLATE='experiments/epuck_EC_locale_template.argos'
CONTRACT="${DOCKERBASE}/geth/shared/Estimation.sol"
SCTEMPLATE="${DOCKERBASE}/geth/shared/Estimation.sol_template"
OUTFILE="experiments/epuck.argos"
SCOUTFILE="${DOCKERBASE}/geth/shared/Estimation.sol"
BASEDIR="$PWD/controllers/epuck_environment_classification/"
BLOCKCHAINPATH="$HOME/eth_data_para/data" # always without '/' at the end!!

DECISIONRULE=$1
NUMROBOTS=(4)
REPETITIONS=30
TAUS=(1000000)
LENGTHOFRUNS=(1000)

MIXINGS=1
VISUALIZATION=visualization #visualization or none

ARENASIZEDIM="2.0"
CELLDIMENSION="0.1"
# Cell dimension should be ARENASIZE / 20 for 400 tiles
# The cell dimension can be changed, this will result in more or less tiles
# HOWEVER!!: Then you also have to change header file value for TOTAL_CELLS
ARENASIZEPLUSLARGE=`echo $ARENASIZEDIM + 0.1 | bc`
ARENASIZEPLUSSMALL=`echo $ARENASIZEDIM + 0.0075 | bc`
ARENASIZEHALF=`echo $ARENASIZEDIM / 2 | bc`
ARENASIZEMINUS=`echo $ARENASIZEDIM - 0.1 | bc`


MININGDIFF=1000000
USEMULTIPLENODES=true
USEBACKGROUNDGETHCALLS=true
MAPPINGPATH="$HOME/Documents/blockchain-journal-bc/experiments/config.txt"
CHANGEDIFFIULTY=""
NUMRUNS=1
THREADS=1
NOW=`date +"%d-%m-%Y"`
# The miner node is the first of the used nodes
USECLASSICALAPPROACH=false
DISTRIBUTEETHER="false"
CONTAINERNAMEBASE="ethereum_eth."
CONTRACTADDRESS="${DOCKERBASE}/geth/deployed_contract/contractAddress.txt"
CONTRACTABI="${DOCKERBASE}/geth/deployed_contract/contractABI.abi"
MAXFLOODING=20
REALTIME="true"

# 1: Always send 0.0 as value
# 2: Always send 1.0 as value
# 3: Send 0.0 with probability 0.5, send 1.0 else
# 4: Send a random number between 0.0 and 1.0
# 5: Send the true value but apply Gaussian noise to the sensor readings
# 11: Perform a Sybil and flooding attack, always send 0.0 as value
# 12: Perform a Sybil and flooding attack, always send 1.0 as value
# 13: Perform a Sybil and flooding attack, send 0.0 with probabiity 0.5, send 1.0 else
# 14: Perform a Sybil and flooding attack, send a random number between 0.0 and 1.0
# 15: Perform a Sybil and flooding attack, send the true value but with some Gaussian noise
# 20: Perform a jamming attack



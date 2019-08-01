# Usage: bash <starter>.sh <decisionrule>
USERNAME=`whoami`
mailto='volker.strobel87@gmail.com'
DOCKERBASE='/home/vstrobel/Documents/docker-geth-network/'
TEMPLATE='experiments/epuck_EC_locale_template.argos'
OUTFILE="experiments/epuck.argos"
BASEDIR="$PWD/controllers/epuck_environment_classification/"
BLOCKCHAINPATH="$HOME/eth_data_para/data" # always without '/' at the end!!
NUMROBOTS=(20)
THRESHOLDS=(80000) 
REPETITIONS=10
DECISIONRULE=$1
MININGDIFF=1000000
PERCENT_BLACKS=(10 20 30 40 50 60 70 80 90)
USEMULTIPLENODES=true
USEBACKGROUNDGETHCALLS=true
MAPPINGPATH="$HOME/Documents/blockchain-journal-bc/experiments/config.txt"
CHANGEDIFFIULTY=""
NUMRUNS=1
THREADS=1
NOW=`date +"%d-%m-%Y"`
# The miner node is the first of the used nodes
USECLASSICALAPPROACH=false
DETERMINECONSENSUS="false"
DISTRIBUTEETHER="false"
CONTAINERNAMEBASE="ethereum_eth."
CONTRACTADDRESS="${DOCKERBASE}/geth/deployed_contract/contractAddress.txt"
CONTRACTABI="${DOCKERBASE}/geth/deployed_contract/contractABI.abi"
NUMBYZANTINE=(0)

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

BYZANTINESWARMSTYLES=( 0 )
MIXINGS=("false")  # mix or tiles or just have a binary field
MAXFLOODING=20
SUBSWARMCONSENSUS=false # Determines if all N robots have to agree or
		       # only the beneficial subswarm.

REALTIME="true"

 # Iterate over experimental settings and start experiments
 
 for i in `seq 1 $REPETITIONS`; do

     for y in "${NUMBYZANTINE[@]}"; do

	 for THRESHOLD in "${THRESHOLDS[@]}"; do

	     for MIXING in "${MIXINGS[@]}"; do

		 for BYZANTINESWARMSTYLE in "${BYZANTINESWARMSTYLES[@]}"; do
		     DATADIRBASE="data/experiment-payable-_mixing${MIXING}_byzstyle${BYZANTINESWARMSTYLE}-node-${NOW}/"		     

	     DATADIR="${DATADIRBASE}${THRESHOLD}/"
	     mkdir -p $DATADIR
	     
 
	     for k in "${NUMROBOTS[@]}"; do

	     R0=$k
	     B0=0

	 for p in "${PERCENT_BLACKS[@]}"; do

	PERCENT_BLACK=$p
	PERCENT_WHITE=$(expr 100 - $PERCENT_BLACK)
	
	RADIX=$(printf 'num%d_black%d_byz%d_run%d' $k $PERCENT_BLACK $y $i)
	
	# Create template
	sed -e "s|BASEDIR|$BASEDIR|g"\
	    -e "s|CONTRACTADDRESS|$CONTRACTADDRESS|g"\
	    -e "s|CONTRACTABI|$CONTRACTABI|g"\
	    -e "s|NUMRUNS|$NUMRUNS|g"\
	    -e "s|DATADIR|$DATADIR|g"\
	    -e "s|RADIX|$RADIX|g"\
	    -e "s|NUMROBOTS|$k|g"\
	    -e "s|R0|$R0|g"\
	    -e "s|B0|$B0|g"\
	    -e "s|PERCENT_BLACK|$PERCENT_BLACK|g"\
	    -e "s|PERCENT_WHITE|$PERCENT_WHITE|g"\
	    -e "s|DECISIONRULE|$DECISIONRULE|g"\
	    -e "s|USEMULTIPLENODES|$USEMULTIPLENODES|g"\
	    -e "s|MININGDIFF|$MININGDIFF|g"\
	    -e "s|BLOCKCHAINPATH|$BLOCKCHAINPATH|g"\
	    -e "s|THREADS|$THREADS|g"\
	    -e "s|USECLASSICALAPPROACH|$USECLASSICALAPPROACH|g"\
	    -e "s|NUMBYZANTINE|$y|g"\
	    -e "s|BYZANTINESWARMSTYLE|$BYZANTINESWARMSTYLE|g"\
	    -e "s|SUBSWARMCONSENSUS|$SUBSWARMCONSENSUS|g"\
	    -e "s|REGENERATEFILE|$REGENERATEFILE|g"\
	    -e "s|REALTIME|$REALTIME|g"\
	    -e "s|FLOODINGATTACK|$FLOODINGATTACK|g"\
	    -e "s|MAXFLOODING|$MAXFLOODING|g"\
	    -e "s|MIXING|$MIXING|g"\
	    -e "s|DETERMINECONSENSUS|$DETERMINECONSENSUS|g"\
	    -e "s|DISTRIBUTEETHER|$DISTRIBUTEETHER|g"\
	    -e "s|CONTAINERNAMEBASE|$CONTAINERNAMEBASE|g"\
	    $TEMPLATE > $OUTFILE
	
	bash /home/vstrobel/Documents/docker-geth-network/local_scripts/stop_network.sh $k
        sleep 5
	# Restart network
	bash /home/vstrobel/Documents/docker-geth-network/local_scripts/start_network.sh $k
	# Start experiment
	argos3 -c $OUTFILE
	
	bash /home/vstrobel/Documents/docker-geth-network/local_scripts/stop_network.sh $k

	 done
	     done
	 
		 done

	     done

	 done
    
     done
done

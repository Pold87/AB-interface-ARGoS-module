# Usage: bash start_xyz.sh <node1> <node2> <decision_rule>
USERNAME=`whoami`
mailto='volker.strobel87@gmail.com'
TEMPLATE='experiments/epuck_EC_locale_template.argos'
OUTFILE="experiments/epuck$1.argos"
SCTEMPLATE='contracts/smart_contract_template.sol'
SCOUT='contracts/smart_contract_threshold.sol'
BASEDIR="$HOME/Documents/blockchain-journal-bc/controllers/epuck_environment_classification/"
BLOCKCHAINPATH="$HOME/eth_data_para$1/data" # always without '/' at the end!!
MINERID=$(expr 120 + $1)
echo "MINERID is ${MINERID}"
NUMROBOTS=(20)
THRESHOLDS=(80000) 
REPETITIONS=10
DECISIONRULE=$3
PERCENT_BLACKS=(40)
MININGDIFF=1000000
USEMULTIPLENODES=true
USEBACKGROUNDGETHCALLS=true
MAPPINGPATH="$HOME/Documents/blockchain-journal-bc/experiments/config$1.txt"
CHANGEDIFFIULTY=""
NUMRUNS=1
THREADS=20
NOW=`date +"%d-%m-%Y"`
USEDNODES=($1 $2)
echo "USEDNODES is ${USEDNODES}"
BASEPORT=$((33000 + $1 * 200))
echo "BASEPORT is ${BASEPORT}"
REGENERATEFILE="$(pwd)/regenerate${USEDNODES[0]}.sh"
# The miner node is the first of the used nodes
MINERNODE=${USEDNODES[0]}
USECLASSICALAPPROACH=false
DETERMINECONSENSUS="false"
DISTRIBUTEETHER="false"
NUMBYZANTINE=(0 1 2 3 4 5 6 7 8 9)

# 1: Always send 0.0 as value
# 2: Always send 1.0 as value
# 3: Send 0.0 with probabiity 0.5, send 1.0 else
# 4: Send a random number between 0.0 and 1.0
# 5: Send the true value but apply Gaussian noise to the sensor readings
# 11: Perform a Sybil and flooding attack, always send 0.0 as value
# 12: Perform a Sybil and flooding attack, always send 1.0 as value
# 13: Perform a Sybil and flooding attack, send 0.0 with probabiity 0.5, send 1.0 else
# 14: Perform a Sybil and flooding attack, send a random number between 0.0 and 1.0
# 15: Perform a Sybil and flooding attack, send the true value but with some Gaussian noise
# 20: Perform a jamming attack

BYZANTINESWARMSTYLES=( 1 )
MIXINGS=("true")  # mix or tiles or just have a binary field
MAXFLOODING=20
SUBSWARMCONSENSUS=false # Determines if all N robots have to agree or
		       # only the beneficial subswarm.

if [ "$USECLASSICALAPPROACH" == "true" ]; then
    REALTIME="false"
else
    REALTIME="true"
fi

 # Rebuild geth with another value in checkDifficulty
 if [ $CHANGEDIFFIULTY ]; then
     ./create_geths.sh $MININGDIFF
 fi
 
 # Iterate over experimental settings and start experiments
 
 for i in `seq 1 $REPETITIONS`; do

     for y in "${NUMBYZANTINE[@]}"; do

	 for THRESHOLD in "${THRESHOLDS[@]}"; do

	     for MIXING in "${MIXINGS[@]}"; do

		 for BYZANTINESWARMSTYLE in "${BYZANTINESWARMSTYLES[@]}"; do
		     DATADIRBASE="data/experiment1_decision3_mixing${MIXING}_byzstyle${BYZANTINESWARMSTYLE}-node$1-${NOW}/"		     

	     DATADIR="${DATADIRBASE}${THRESHOLD}/"
	     mkdir -p $DATADIR
	     
 
	     for k in "${NUMROBOTS[@]}"; do

	     R0=$k
	     B0=0

	 for p in "${PERCENT_BLACKS[@]}"; do

	PERCENT_BLACK=$p
	PERCENT_WHITE=$(expr 100 - $PERCENT_BLACK)
	
	if [ $USECLASSICALAPPROACH == "false" ]; then

	    echo "Blockchain version!"
	    
	    GENERATEDAG=`cat regeneratedag.txt`
	    if [ $GENERATEDAG ]; then
		#if [ "$i" -gt 0 ]; then
		rm ~/.ethash/*
		ssh ${USERNAME}@c3-0 "geth makedag 0 ~/.ethash"
		echo "" > regeneratedag.txt
		#fi
	    fi
	    
	    # Create the mapping file
	    python experiments/create_node_mapping_call.py $MAPPINGPATH $NUMROBOTS ${USEDNODES[0]} ${USEDNODES[1]}

	    # Create template for:
	    # Create directories for collecting data and the geth processes
	    # Create file for killing the blockchain proceeses on these nodes
	    sed -e "s|NODEA|${USEDNODES[0]}|g" -e "s|NODEB|${USEDNODES[1]}|g" -e "s|BLOCKCHAINPATH|$BLOCKCHAINPATH|g" -e "s|PWD|$(pwd)|g" regenerate_template.sh > $REGENERATEFILE
	    # and apply the regeneration file for the blockchain folders
	    bash $REGENERATEFILE
	    
	fi
	
	RADIX=$(printf 'num%d_black%d_byz%d_run%d' $k $PERCENT_BLACK $y $i)

	# Create and compile smart contract
	sed -e "s|THRESHOLD|$THRESHOLD|g" $SCTEMPLATE > $SCOUT
	solc --overwrite --abi --bin -o . $SCOUT
	cp Estimation.bin "${BASEDIR}/data$1.txt"
	cp Estimation.abi "${BASEDIR}/interface$1.txt"	      
	
	# Create template
	sed -e "s|BASEDIR|$BASEDIR|g" -e "s|NUMRUNS|$NUMRUNS|g" -e "s|DATADIR|$DATADIR|g" -e "s|RADIX|$RADIX|g" -e "s|NUMROBOTS|$k|g" -e "s|R0|$R0|g" -e "s|B0|$B0|g" -e "s|PERCENT_BLACK|$PERCENT_BLACK|g" -e "s|PERCENT_WHITE|$PERCENT_WHITE|g" -e "s|DECISIONRULE|$DECISIONRULE|g" -e "s|USEMULTIPLENODES|$USEMULTIPLENODES|g" -e "s|MININGDIFF|$MININGDIFF|g" -e "s|MINERNODE|$MINERNODE|g" -e "s|MINERID|$MINERID|g" -e "s|BASEPORT|$BASEPORT|g" -e "s|USEBACKGROUNDGETHCALLS|$USEBACKGROUNDGETHCALLS|g" -e "s|BLOCKCHAINPATH|$BLOCKCHAINPATH|g" -e "s|MAPPINGPATH|$MAPPINGPATH|g" -e "s|THREADS|$THREADS|g" -e "s|USECLASSICALAPPROACH|$USECLASSICALAPPROACH|g" -e "s|NUMBYZANTINE|$y|g" -e "s|BYZANTINESWARMSTYLE|$BYZANTINESWARMSTYLE|g" -e "s|SUBSWARMCONSENSUS|$SUBSWARMCONSENSUS|g" -e "s|REGENERATEFILE|$REGENERATEFILE|g" -e "s|REALTIME|$REALTIME|g" -e "s|FLOODINGATTACK|$FLOODINGATTACK|g" -e "s|MAXFLOODING|$MAXFLOODING|g" -e "s|MIXING|$MIXING|g" -e "s|DETERMINECONSENSUS|$DETERMINECONSENSUS|g" -e "s|DISTRIBUTEETHER|$DISTRIBUTEETHER|g" $TEMPLATE > $OUTFILE
	
	# Start experiment
	argos3 -c $OUTFILE
	
	if [ USECLASSICALAPPROACH == "false" ]; then
	    
	    # Clean up
	    bash "${BLOCKCHAINPATH}/bckillerccall"
	    bash "killblockchainalldirect $1 $2"
	    #mkdir -p "${DATADIR}${p}-${i}"
	    #mv "${BLOCKCHAINPATH}"* "${DATADIR}${p}-${i}"
	    rm -rf "${BLOCKCHAINPATH}"*
	    rm $REGENERATEFILE
	    
	fi
	
	 done
	     done
	 
		 done

	     done

	 done
    
     done


sendmail $mailto < finished.txt
     
done

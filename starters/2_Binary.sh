#!/bin/sh

. ./global_config.sh

EXPERIMENT="2_Binary"
PERCENT_BLACKS=(10 20 30 40 50 60 70 80 90)
DETERMINECONSENSUS="false"
NUMBYZANTINE=(0)
BYZANTINESWARMSTYLES=( 0 )
MIXINGS=2 # mix or tiles or just have a binary field
SUBSWARMCONSENSUS=false # Determines if all N robots have to agree or
		       # only the beneficial subswarm.
. ./run_experiment.sh

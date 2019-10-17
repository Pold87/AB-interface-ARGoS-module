#!/bin/sh

. ./global_config.sh

EXPERIMENT="5_Byzantine"
PERCENT_BLACKS=(25)
DETERMINECONSENSUS="false"
NUMBYZANTINE=(0 1 2 3 4 5 6 7)
BYZANTINESWARMSTYLES=( 1 )
SUBSWARMCONSENSUS=false # Determines if all N robots have to agree or
		       # only the beneficial subswarm.

. ./run_experiment.sh

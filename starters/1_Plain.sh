#!/bin/sh

. ./global_config.sh

EXPERIMENT="1_Plain"
PERCENT_BLACKS=(0 10 20 30 40 50 60 70 80 90 100)
DETERMINECONSENSUS="false"
NUMBYZANTINE=(0)
BYZANTINESWARMSTYLES=( 0 )
MAXFLOODING=20
LENGTHOFRUNS=1000
SUBSWARMCONSENSUS=false # Determines if all N robots have to agree or
		       # only the beneficial subswarm.

. ./run_experiment.sh

# ARGoS-Blockchain interface (ARGoS module)

This module allows for running the ARGoS robot simulations described
in the article [Blockchain Technology Secures Robot Swarms: A
Comparison of Consensus Protocols and Their Resilience to Byzantine
Robots](https://www.frontiersin.org/articles/10.3389/frobt.2020.00054/full)
by [Strobel, V.](http://iridia.ulb.ac.be/~vstrobel/), [Castello
Ferrer, E.](http://www.eduardocastello.com/), and [Dorigo,
M.](http://iridia.ulb.ac.be/~mdorigo/HomePageDorigo/). For the
ARGoS-Blockchain interface, the interaction with the Ethereum nodes is
done via C++, using the code in the following repository:
https://github.com/Pold87/ARGoS-Blockchain-interface/

## Overview of the framework
![Overview](img/interface.png?raw=true "Overview")

## Setup

```
mkdir build
cd build
cmake ..
make
```

## Run

Use one of the starter scripts in the folder `starters`. For example:

```
bash starters/1_Plain.sh 3
```

## Folder structure

* `/home/vstrobel/Documents/docker-geth-network/geth/shared/` contains
the smart contracts,
* `starters` contains the starter scripts; they
source the files `general_config.sh` and `run_experiment.sh`

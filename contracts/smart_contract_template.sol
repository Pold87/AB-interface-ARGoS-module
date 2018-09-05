pragma solidity ^0.4.0;
contract Estimation {

int public mean;
uint public F = 1;
int public count = 0;
int public threshold = 140000;
int public m2;
int public numCurrentVotes = 0;
uint savedMoney = 0;
uint public latest_payout = 0;
int W_n;

struct votingInformation {
    address robot;
    int quality;
    int diff;
    uint blockNumber;
    uint weight;
    uint money;
  }

votingInformation[] allVotes;
votingInformation[] ripedVotes;
votingInformation[] ripedVotesGreater;
votingInformation[] ripedVotesSmaller;
votingInformation[] youngVotes;
votingInformation[] newVotes;
mapping(address => int) public payoutForAddress;

event consensusReached(uint c);

function abs(int x) internal pure returns (int y) {
    if (x < 0) {
      return -x;
    } else {
      return x;
    }
  }

function getBalance() public constant returns (uint) {
    return address(this).balance;
}

function getBlockNumber() public constant returns (uint) {
    return block.number;
}

 function sqrt(int x) internal pure returns (int y) {
   int z = (x + 1) / 2;
   y = x;
   while (z < y) {
     y = z;
     z = (x / z + z) / 2;
   }
 }

function askForPayout() public {

  if (latest_payout > (block.number - 2)) {
    revert();
  }

    uint totalPayout = 0;
    totalPayout += savedMoney;
    savedMoney = 0;

    /* Find out which votes are old enough */
    for (uint a = 0; a < allVotes.length; a++) {
        if (allVotes[a].blockNumber < block.number - 2) {

	    if (allVotes[a].quality > mean) {
	      allVotes[a].diff = abs(mean - allVotes[a].quality);
	      ripedVotesGreater.push(allVotes[a]);
	    } else {
	      allVotes[a].diff = abs(mean - allVotes[a].quality);
	      ripedVotesSmaller.push(allVotes[a]);
	    }

        totalPayout += allVotes[a].money;
    } else {
            youngVotes.push(allVotes[a]);
        }
    }

    uint j;
    /* Sort greater than (in ascending order) */
    for (uint i = 0; i < ripedVotesGreater.length; i++){
        votingInformation memory vi = ripedVotesGreater[i];
        j = i;

        while (j > 0 && ripedVotesGreater[j-1].quality > vi.quality) {
            ripedVotesGreater[j] = ripedVotesGreater[j - 1];
            j -= 1;
        }
        ripedVotesGreater[j] = vi;
    }


    /* Sort smaller than (in descending order) */
    for (uint d = 0; d < ripedVotesSmaller.length; d++){
        votingInformation memory vi2 = ripedVotesSmaller[d];
        vi2.quality = abs(mean - vi2.quality);
        j = d;

        while (j > 0 && ripedVotesSmaller[j - 1].quality < vi2.quality) {
            ripedVotesSmaller[j] = ripedVotesSmaller[j - 1];
            j -= 1;
        }
        ripedVotesSmaller[j] = vi2;
    }


    if (ripedVotesGreater.length < F) {
      delete ripedVotesGreater;
    } else {
      ripedVotesGreater.length = ripedVotesGreater.length - F;
    }

    if (ripedVotesSmaller.length < F) {
      delete ripedVotesSmaller;
    } else {
      ripedVotesSmaller.length = ripedVotesSmaller.length - F;
    }


  uint payoutPerRobot;
  if (totalPayout == 0) {
    payoutPerRobot = 0;
  } else if (ripedVotesGreater.length + ripedVotesSmaller.length == 0){
    payoutPerRobot = 0;
    savedMoney = totalPayout;
  } else if (totalPayout == 1) {
    payoutPerRobot = totalPayout;
  } else {
    payoutPerRobot = totalPayout / (ripedVotesGreater.length + ripedVotesSmaller.length);
  }


  if (totalPayout > 0) {

    for (uint z = 0; z < ripedVotesGreater.length; z++) {
      ripedVotesGreater[z].robot.send(payoutPerRobot);
      int deltaGreater = ripedVotesGreater[z].quality - mean;
      count = count + 1;
      mean += mean + (deltaGreater / count);
    }

    for (uint r = 0; r < ripedVotesSmaller.length; r++) {
      ripedVotesSmaller[r].robot.send(payoutPerRobot);
      int deltaSmaller = ripedVotesSmaller[r].quality - mean;
      count = count + 1;
      mean += mean + (deltaSmaller / count);
    }


    }
   delete ripedVotes;
   delete ripedVotesGreater;
   delete ripedVotesSmaller;
   delete allVotes;

   for (uint h = 0; h < youngVotes.length; h++) {
     allVotes.push(youngVotes[h]);
   }

   delete youngVotes;

   latest_payout = block.number;

 }


function getSenderBalance() public constant returns (uint) {
    return msg.sender.balance;
}

 function vote(int x_n) public payable {

    if (msg.value < 2 ether)
        revert();

    votingInformation memory vi = votingInformation(msg.sender, x_n, 0, block.number, msg.sender.balance, msg.value);

    allVotes.push(vi);
 }

 function getMean() public constant returns (int) {
   return mean;
 }

 function getCount() public constant returns (int) {
   return count;
 }

 function calcSE() public constant returns (int) {
  int myvar = m2 / (count - 1);
  int acc = myvar / count;
  int se = sqrt(acc);

  return se;
 }

 function checkStop() public constant returns (int) {

   if (count < 2) {
     return 2;
   }

   int myvar = m2 / (count - 1);
   int acc = myvar / count;
   int se = sqrt(acc);

   if (se < threshold) {
     return 1;
   } else {
     return 2;
   }
 }
}

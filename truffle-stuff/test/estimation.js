var Estimation = artifacts.require("./Estimation.sol");
var Papa = require('papaparse');
var path = process.cwd;
var fs = require("fs");
var file = fs.readFileSync("/Users/volkerstrobel/Documents/mygithub-software/blockchain-journal-bc/truffle-stuff/test/mylongfile.csv", { encoding: 'binary' });

var tth = require('truffle-test-helpers');

/**
 *  Sends a request to the RPC provider to mine a single block
 *  synchronously
 */

function getRandomInt(min, max) {
    return Math.floor(Math.random() * (max - min + 1)) + min;
}


var numByzantine = 4;

contract('Estimation', function(accounts) {
    
    it("Load stuff", async function() {
	
	return Estimation.deployed().then(async function(instance) {

	    estimation = instance;
	    
	    var currentBlock = 0;

	    var endresults;
	    Papa.parse(file, {
		complete: function(results){
		    endresults = results.data
		}
	    });

	    /* Donate money to helper */
	    for (i = 0; i < 20; i++) {
		await web3.eth.sendTransaction({from: accounts[i], to: accounts[20], value: 90000000000000000000});
	    }	    

	return endresults;	    
	    	    	
	}).then(async function (res) {

	    
	    for (i = 0; i < res.length; i++) {

		if (i % 5 == 0) {
		    var winner = getRandomInt(0, 19);
		    await web3.eth.sendTransaction({from: accounts[20], to: accounts[winner], value: 5000000000000000000});
		}

		console.log("winner is", winner);

		var robot = parseInt(res[i][0]);
		var opinion = parseInt(res[i][1]);

		if (robot < numByzantine) {
		    opinion = 0;
		}
		
		
	    var block = parseInt(res[i][2]);


//		if (block > currentBlock) {
		//		await advanceBlock();
		//before(async () => {
		    // Advance to the next block to correctly read time in the solidity "now" function interpreted by testrpc
		//});
		currentBlock = block;
//		}

		console.log("robot", robot, web3.fromWei(web3.eth.getBalance(accounts[robot])).toNumber());
		if (web3.eth.getBalance(accounts[robot])> 4000000000000000000) {
		      try{
			  await estimation.vote(opinion, {from: accounts[robot], value: 4000000000000000000, gas: 10000000});
		      }
		    catch(e){
			console.log("Strange stuff is happening here");
		    }
		}  else {
		    console.log("I'm too poor");
		}

		try{
		    await estimation.askForPayout({gas: 10000000});
		}
		catch(e){
		    //console.log("latest payout too soon");
		}
		   		
		let u = await estimation.getMean();
		console.log(u.toNumber() / 10000000);
		console.log("");
	    }

	    let c = await estimation.getMean();
	    console.log(c / 10000000)
	    return estimation.getMean();
	    
	}).then(function (mean) {
	    console.log(mean.toNumber())});
    });
    });

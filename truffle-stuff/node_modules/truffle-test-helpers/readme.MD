# Truffle test helpers

Inspired by [Zeppelin solidity](https://github.com/OpenZeppelin/zeppelin-solidity/) this package provides a number of test helpers to used in truffle javascript tests.

## Installation
```
npm install truffle-test-helpers
```

## Usage

```js
import {
  advanceBlock,
  advanceToBlock,
  increaseTime,
  increaseTimeTo,
  duration,
  revert,
  latestTime
} from 'truffle-test-helpers';
```

- `duration` useful to create durations for contracts that require time to be provided as one of their function arguments. Usage:

```js
const time1 = duration.seconds(1);
const time2 = duration.minutes(1);
const time3 = duration.hours(1);
const time4 = duration.days(1);
const time5 = duration.weeks(1);
const time6 = duration.years(1); 
```

- `latestTime` gets the current time of blockchain.

- `advanceBlock` is mostly used in a generic `before` each contract:

```js
contract('MyContract', function() {
  before(async function() {
    // Advance to the next block to correctly read time in the solidity "now" function interpreted by ganache
    await advanceBlock();
  });
});
```

- `increaseTimeTo` advances to specified block in time. Example:
                   
```js
contract('MyContract', function() {
 before(async function() {
   // Advance to the next block to correctly read time in the solidity "now" function interpreted by ganache
   await advanceBlock();
 });
 
 beforeEach(function() {
   this.startTime = latestTime();
   this.endTime = this.startTime + duration.minutes(2);
 });
 
 it('some test', async function() {
   await advanceToBlock(this.endTime);
 });
});
```

- `increaseTime` advances on some duration. Used under the hood by `increaseTimeTo`.

- `advanceBlock` step to next block by mining.

- `advanceToBlock` advances to specified block in time. Uses `advanceBlock` under the hood. 

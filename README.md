# EOS fake.token dApp
The purpose of this dApp is to create fake tokens (forged tokens) that imitate already existing valid tokens. This fake tokens can be then tested against your dApps and smart contracts to check the resilience of your contract against invalid token deposits. 

### Why is important to test contract against fake token deposits?
EOS smart contract cdt is at a current stage a very flexible framework and gives the developer a lot of freedom to composing a smart contract. Unfortunately, this is also a drawback because one has to think off all the cases where something could go wrong.  
This is especially the case with handling deposits to the smart contract. It has already happened in the past that developers have forgotten to check which token contract deposited tokens. One such incident happened also to one of the largest betting dApps [EOSBet](https://www.reddit.com/r/eos/comments/9fxyd4/eosbet_transfer_hack_statement/).

## How to use
Before you can use fake tokens you have to create and issue them. This is a normal procedure as you would do when issuing real token by calling action `create` first and then call action `issue`. This will create new tokens and send them to the issuing account.  
Then you have the option to distribute these tokens via *faucet*.  

## Faucet
To use faucet for distributing your fake tokens you first have to open it. This is done by calling an action `openfaucet`. After that, you will have to deposit some tokens to it. To refill faucet you just deposit tokens to this dApp account. dApp will then payout to each claimer on a daily basis an amount you specified.

### How to claim fake EOS token from faucet:
`cleos push action fake.token gettoken '["<claiming_account>", "4,EOS"]' -p <claiming_account>`

You should receive 10000.0000 EOS.

## Test net
This dApp is already set up on the `fake.token` account and is available on [Jungle](https://jungle.bloks.io/account/fake.token) and [Kylin](https://kylin.bloks.io/account/fake.token) test net.

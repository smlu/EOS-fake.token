# EOS fake.token dApp
The purpose of this dApp is to create fake tokens (forged tokens) that imitate already existing tokens. This fake tokens can  then be used to test the resilience of your dApps and smart contracts against invalid token deposits.

### Why is important to test contract against fake token deposits?
EOS smart contract cdt is at a current stage a very flexible framework and gives the developer a lot of freedom to composing a smart contract. Unfortunately, this is also a drawback because one has to think off all the cases where something could go wrong.
This is especially the case with handling deposits to the smart contract. It has already happened in the past that developers have forgotten to check which token contract deposited tokens. One such incident happened also to one of the largest betting dApps [EOSBet](https://www.reddit.com/r/eos/comments/9fxyd4/eosbet_transfer_hack_statement/).

## How to use
Before you can use fake token for testing you first have to create and issue it. This is the normal procedure as you would do when issuing real token by calling first the action `create` and then call the action `issue`. This will create new fake tokens and send them to the issuing account.
Afterward you have an option to distribute the newly created tokens via *faucet*.

## Faucet
To use faucet for distributing your fake tokens you first have to open faucet for specific token. This is done by calling the action `openfaucet`. After opening token's faucet you have to deposit some tokens to it. To refill a faucet, you just need to deposit your tokens to this dApp. The dApp will then payout to each claimer an amount you specified on a daily basis.

### How to claim fake EOS token from faucet:
`cleos push action fake.token gettoken '["<claiming_account>", "4,EOS"]' -p <claiming_account>`

You should receive fake 10000.0000 EOS.

## Test net
This test dApp is available on [Jungle](https://jungle.bloks.io/account/fakefaketokn) and [Kylin](https://kylin.bloks.io/account/fake.token) test nets.

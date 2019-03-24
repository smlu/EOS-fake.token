#pragma once

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/multi_index.hpp>
#include <eosio/time.hpp>

#include <string>

namespace fake {
    using namespace eosio;

    class [[eosio::contract("fake.token")]] token : public contract {
        public:
            using contract::contract;

            [[eosio::action]]
            void create(name issuer, asset maximum_supply);

            [[eosio::action]]
            void issue(name to, asset quantity, std::string memo);

            [[eosio::action]]
            void retire(asset quantity, std::string memo);

            [[eosio::action]]
            void transfer(name from, name to, asset quantity, std::string memo);

            [[eosio::action]]
            void open(name owner, const symbol& symbol, name ram_payer);

            [[eosio::action]]
            void close(name owner, const symbol& symbol);

            [[eosio::action]]
            void gettoken(name account, const symbol& symbol);

            [[eosio::action]]
            void setfaucetpay(name owner, asset payout);

            [[eosio::action]]
            void openfaucet(name owner, asset payout);

            [[eosio::action]]
            void closefaucet(name owner, const symbol& symbol);

            static asset get_supply(name token_contract_account, symbol_code sym_code)
            {
                stats statstable(token_contract_account, sym_code.raw());
                const auto& st = statstable.get(sym_code.raw());
                return st.supply;
            }

            static asset get_balance(name token_contract_account, name owner, symbol_code sym_code)
            {
                accounts accountstable(token_contract_account, owner.value);
                const auto& ac = accountstable.get(sym_code.raw());
                return ac.balance;
            }

        private:
            struct [[eosio::table]] account {
                asset balance;
                uint64_t primary_key() const { return balance.symbol.code().raw(); }
            };

            struct [[eosio::table]] currency_stats {
                asset supply;
                asset max_supply;
                name issuer;

                uint64_t primary_key() const { return supply.symbol.code().raw(); }
            };

            struct [[eosio::table("faucets")]] faucet {
                name owner;
                asset supply;
                asset payout;

                uint64_t primary_key() const { return supply.symbol.code().raw(); }
            };

            struct [[eosio::table("faucet.users")]] faucet_user {
                symbol sym;
                time_point_sec last_claim;

                uint64_t primary_key() const { return sym.code().raw(); }
                EOSLIB_SERIALIZE(faucet_user, (sym)(last_claim))
            };

            typedef eosio::multi_index<"accounts"_n, account> accounts;
            typedef eosio::multi_index<"stat"_n, currency_stats> stats;
            typedef eosio::multi_index<"faucets"_n, faucet> faucets;
            typedef eosio::multi_index<"faucet.users"_n, faucet_user> fusers;

            void sub_balance(name owner, asset value);
            void add_balance(name owner, asset value, name ram_payer);
    };

} /// namespace fake
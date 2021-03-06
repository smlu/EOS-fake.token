#include "fake.token.hpp"
#include <eosio/system.hpp>

using namespace eosio;
using namespace fake;

void token::create(name issuer, asset  maximum_supply)
{
    require_auth(issuer);

    auto sym = maximum_supply.symbol;
    check(sym.is_valid(), "invalid symbol name");
    check(maximum_supply.is_valid(), "invalid supply");
    check(maximum_supply.amount > 0, "max-supply must be positive");

    stats statstable(_self, sym.code().raw());
    auto existing = statstable.find(sym.code().raw());
    check(existing == statstable.end(), "token with symbol already exists");

    statstable.emplace(issuer, [&](auto& s) {
        s.supply.symbol = maximum_supply.symbol;
        s.max_supply    = maximum_supply;
        s.issuer        = issuer;
    });
}


void token::issue(name to, asset quantity, std::string memo)
{
    auto sym = quantity.symbol;
    check(sym.is_valid(), "invalid symbol name");
    check(memo.size() <= 256, "memo has more than 256 bytes" );

    stats statstable(_self, sym.code().raw());
    auto existing = statstable.find( sym.code().raw() );
    check(existing != statstable.end(), "token with symbol does not exist, create token before issue");
    const auto& st = *existing;

    require_auth(st.issuer );
    check(quantity.is_valid(), "invalid quantity");
    check(quantity.amount > 0, "must issue positive quantity");

    check(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
    check(quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

    statstable.modify(st, same_payer, [&](auto& s) {
        s.supply += quantity;
    });

    add_balance(st.issuer, quantity, st.issuer);
    if(to != st.issuer) {
        SEND_INLINE_ACTION(*this, transfer, { st.issuer, "active"_n },
            { st.issuer, to, quantity, memo }
        );
    }
}

void token::retire(asset quantity, std::string memo)
{
    auto sym = quantity.symbol;
    check(sym.is_valid(), "invalid symbol name");
    check(memo.size() <= 256, "memo has more than 256 bytes");

    stats statstable(_self, sym.code().raw());
    auto existing = statstable.find( sym.code().raw() );
    check(existing != statstable.end(), "token with symbol does not exist");
    const auto& st = *existing;

    require_auth(st.issuer );
    check(quantity.is_valid(), "invalid quantity");
    check(quantity.amount > 0, "must retire positive quantity");

    check(quantity.symbol == st.supply.symbol, "symbol precision mismatch");

    statstable.modify(st, same_payer, [&](auto& s) {
        s.supply -= quantity;
    });

    sub_balance( st.issuer, quantity );
}

void token::transfer(name from, name to, asset quantity, std::string memo)
{
    check(from != to, "cannot transfer to self");
    require_auth(from);
    check(is_account(to), "to account does not exist");
    auto sym = quantity.symbol.code();
    stats statstable(_self, sym.raw());
    const auto& st = statstable.get( sym.raw());

    require_recipient(from);
    require_recipient(to);

    check(quantity.is_valid(), "invalid quantity");
    check(quantity.amount > 0, "must transfer positive quantity");
    check(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
    check(memo.size() <= 256, "memo has more than 256 bytes");

    auto payer = has_auth(to) ? to : from;
    sub_balance(from, quantity);
    add_balance(to, quantity, payer);

    // if(from == _self)
    // {
    //     auto sym_code = quantity.symbol.code().raw();
    //     faucets faucets(_self, sym_code);
    //     auto fit = faucets.find(sym_code);
    //     if(fit != faucets.end()) {
    //         require_auth({ from, "eosio.code"_n });
    //     }
    // }
    // else
    if(_first_receiver == _self && to == _self)
    {
        auto sym_code = quantity.symbol.code().raw();
        faucets faucets(_self, sym_code);
        auto fit = faucets.find(sym_code);
        if(fit != faucets.end())
        {
            faucets.modify(fit, same_payer, [&](auto& f){
                f.supply += quantity;
            });
        }
    }
}

void token::sub_balance(name owner, asset value) {
    accounts from_acnts(_self, owner.value);

    const auto& from = from_acnts.get(value.symbol.code().raw(), "no balance object found");
    check(from.balance.amount >= value.amount, "overdrawn balance");

    from_acnts.modify(from, owner, [&](auto& a) {
        a.balance -= value;
    });
}

void token::add_balance(name owner, asset value, name ram_payer)
{
    accounts to_acnts(_self, owner.value);
    auto to = to_acnts.find(value.symbol.code().raw());
    if(to == to_acnts.end()) {
        to_acnts.emplace(ram_payer, [&]( auto& a){
            a.balance = value;
        });
    }
    else {
        to_acnts.modify(to, same_payer, [&]( auto& a) {
            a.balance += value;
        });
    }
}

void token::open(name owner, const symbol& symbol, name ram_payer)
{
    require_auth(ram_payer);
    auto sym_code_raw = symbol.code().raw();

    stats statstable(_self, sym_code_raw);
    const auto& st = statstable.get(sym_code_raw, "symbol does not exist");
    check(st.supply.symbol == symbol, "symbol precision mismatch");

    accounts acnts(_self, owner.value);
    auto it = acnts.find(sym_code_raw);
    if(it == acnts.end()) {
        acnts.emplace( ram_payer, [&](auto& a){
            a.balance = asset{0, symbol};
        });
    }
}

void token::close(name owner, const symbol& symbol)
{
    require_auth(owner);
    auto sym_code_raw = symbol.code().raw();

    stats statstable(_self, sym_code_raw);
    const auto& st = statstable.get(sym_code_raw, "symbol does not exist");
    check(st.supply.symbol == symbol, "symbol precision mismatch");

    accounts acnts(_self, owner.value);
    auto it = acnts.find(sym_code_raw);
    check(it != acnts.end(), "Balance row already deleted or never existed. Action won't have any effect.");
    check(it->balance.amount == 0, "Cannot close because the balance is not zero.");
    acnts.erase(it);
}

void token::gettoken(name account, const symbol& sym)
{
    require_auth(account);

    auto sym_code = sym.code().raw();
    faucets faucets(_self, sym_code);

    const auto& faucet = faucets.get(sym_code, "faucet doesn't exists");
    check((faucet.supply - faucet.payout).amount >= 0, "dry faucet");


    fusers usr(_self, account.value);
    auto fit = usr.find(sym_code);
    if(fit == usr.end())
    {
        usr.emplace(account, [&](auto& f){
            f.sym = sym;
        });
        fit = usr.require_find(sym_code);
    }

    auto time_now = time_point_sec(current_time_point());
    check(time_now - fit->last_claim >= days(1), "reached 24 hours max faucet payout");
    usr.modify(fit, account, [&](auto& f){
        f.last_claim = time_now;
    });

    faucets.modify(faucet, account, [](auto& f){
        f.supply -= f.payout;
    });

    SEND_INLINE_ACTION(*this, transfer, { _self, "active"_n },
        { _self, account, faucet.payout, "faucet" }
    );
}

void token::setfaucetpay(name owner, asset payout)
{
    require_auth(owner);

    auto sym_code = payout.symbol.code().raw();
    faucets faucets(_self, sym_code);
    auto fit = faucets.find(sym_code);

    check(fit != faucets.end(), "faucet doesn't exists");
    check(fit->owner == owner, "unauthorized");

    faucets.modify(fit, owner, [&](auto& f){
        f.payout = payout;
    });
}

void token::openfaucet(name owner, asset payout)
{
    require_auth(owner);

    auto sym_code = payout.symbol.code().raw();
    faucets faucets(_self, sym_code);
    auto fit = faucets.find(sym_code);
    check(fit == faucets.end(), "faucet already opened");

    faucets.emplace(owner, [&](auto& f) {
        f.owner  = owner;
        f.supply = asset(0, payout.symbol);
        f.payout = payout;
    });
}

void token::closefaucet(name owner, const symbol& sym)
{
    require_auth(owner);

    auto sym_code = sym.code().raw();
    faucets faucets(_self, sym_code);

    auto fit = faucets.find(sym_code);
    if(fit == faucets.end())
    {
        fusers usr(_self, owner.value);
        const auto& f = usr.get(sym.code().raw(), "faucet doesn't exists");
        usr.erase(f);
        return;
    }

    check(fit->owner == owner, "unauthorized");
    if(fit->supply.amount > 0) {
        SEND_INLINE_ACTION(*this, transfer, { _self, "active"_n },
            { _self, fit->owner, fit->supply, "faucet closed" }
        );
    }

    faucets.erase(fit);
}
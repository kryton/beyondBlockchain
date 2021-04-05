#pragma once

#include <eosio/eosio.hpp>
#include <eosio/print.hpp>
#include <functional> //for std::hash
#include <string>
#include <stdint.h>
#include <eosio/asset.hpp>
#include <eosio/transaction.hpp>
#include <stdlib.h>
#include <set>
using namespace eosio;

#define MAINTENANCE 0
#define MY_CONTRACT_BETNOW betnow
#define MY_CONTRACT_BETNOW_APEX "betnow"
#define MY_CONTRACT_BETCONTEST betcontest
#define MY_CONTRACT_BETCONTEST_APEX "betcontest"
#define MY_CONTRACT_BETBANK betbank
#define MY_CONTRACT_BETBANK_APEX "betbank"

uint64_t minimumTransaction = 100;
std::string transferMemo = "bypass";
symbol ASSET = symbol("EOS", 4);
time_point nowTime = current_time_point();
float kfactor = .3;
float contestFactor = .5;
uint64_t betLeaderBonus = 3;
static const char *charmap = "0123456789";
name newdex = name("newposincome");
name eosioStake = name("eosio.stake");
name eosioContract = name("eosio");
name eosioRam = name("eosio.ram");

/*****************TABLES******************/
struct [[eosio::table, eosio::contract(MY_CONTRACT_BETNOW_APEX)]] betstable
{
   name account;
   uint128_t superkey;
   asset amount;
   asset amountW; //amount with bonus
   uint64_t forecast;
   uint64_t secondaryKey;
   uint128_t primary_key() const { return superkey; }
   uint64_t get_secondaryKey() const { return secondaryKey; }
   uint64_t get_user() const { return account.value; }
};

struct [[eosio::table, eosio::contract(MY_CONTRACT_BETNOW_APEX)]] dapptable
{
   uint64_t key = 1;
   uint64_t users = 0;
   uint64_t events = 0;
   asset volume = asset((uint64_t)0, ASSET);
   uint64_t primary_key() const { return key; }
};

//forecasts = string containing forecasts separeted by ":"
struct [[eosio::table, eosio::contract(MY_CONTRACT_APEX)]] eventstable
{
   uint64_t event;
   time_point betStartTime;
   time_point betEndTime;
   std::string forecasts; //1:2:..:n
   std::string eventDescription;
   std::string eventCategory;
   uint64_t contestKey;
   bool close;
   uint64_t winnerF = 9999; //forecast=9999;
   float reward = -9999;    //reward=-9999;
   float bonus = 0;
   float winningFee = 0.05;
   uint64_t betCounterEvent = 0;
   uint64_t primary_key() const { return event; }
   uint64_t get_contestKey() const { return contestKey; }
};

struct [[eosio::table, eosio::contract(MY_CONTRACT_APEX)]] betsumtable
{
   uint64_t betkey;
   std::string result;
   asset balanceW = asset((uint64_t)0, ASSET);
   asset balanceL = asset((uint64_t)0, ASSET);
   uint64_t primary_key() const { return betkey; }
};

struct [[eosio::table, eosio::contract(MY_CONTRACT_BETNOW_APEX)]] userstable
{
   name user;
   uint64_t userid;
   asset totalWin = asset((uint64_t)0, ASSET);
   uint64_t betNowPoints = 0;
   std::string userBets = "";
   uint64_t primary_key() const { return userid; }
   uint64_t get_betNowPoints() const { return betNowPoints; }
};

struct [[eosio::table, eosio::contract(MY_CONTRACT_APEX)]] contesttable
{
   uint64_t contestKey;
   std::string contestDescription;
   asset balance = asset((uint64_t)0, ASSET);
   std::string contestWeights;
   bool close;
   uint64_t primary_key() const { return contestKey; }
};

struct [[eosio::table, eosio::contract(MY_CONTRACT_BETCONTEST_APEX)]] ranking
{
   name user;
   uint64_t betPoints = 0;
   uint128_t primary_key() const { return user.value; }
   uint64_t get_betPoints() const { return betPoints; }
};

/*****************FUNCTIONS******************/
bool checkSymbol(asset quantity)
{
   if (quantity.symbol != ASSET)
      return false;
   return true;
}

bool checkIncomingTX(name to, name self)
{
   if (to != self)
      return false;
   return true;
}

bool isNumber(const std::string &s)
{
   return !s.empty() && std::find_if(s.begin(),
                                     s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
}

// Concatenation of ids

static uint128_t combine_ids_128_64(const uint64_t &x, const uint64_t &y)
{
   return (uint128_t{x} << 64) | y;
}
uint128_t combine_ids_128_64(const uint64_t &x, const uint32_t &y)
{
   return (uint128_t{x} << 64) | (uint64_t)y;
}
uint128_t combine_ids_64(const uint32_t &x, const uint32_t &y)
{
   return (uint64_t{x} << 32) | y;
}
// Example of secondary index
// uint128_t id_forecast_key() const { return (uint128_t)event << 64 | forecast; }

std::string uint128ToString(const uint128_t &value)
{
   std::string result;
   result.reserve(40); // max. 40 digits possible ( uint64_t has 20)
   uint128_t helper = value;
   do
   {
      result += charmap[helper % 10];
      helper /= 10;
   } while (helper);
   std::reverse(result.begin(), result.end());
   return result;
}

// Function for splitting string by delimiter, return vector of string
std::vector<std::string> splitString(std::string text, char delimiter)
{
   std::vector<std::string> array;
   size_t pos = 0;
   std::string elem;
   while ((pos = text.find(delimiter)) != std::string::npos)
   {
      elem = text.substr(0, pos);
      array.push_back(elem);
      text.erase(0, pos + 1); //1 = delimiter.length()
   }
   array.push_back(text);
   return array;
}

#define MY_CONTRACT_CLASS betcontest
#define MY_CONTRACT betcontest
#define MY_CONTRACT_APEX "betcontest"
#include "../include/common.hpp"
using namespace eosio;

class [[eosio::contract(MY_CONTRACT_APEX)]] MY_CONTRACT_CLASS : public eosio::contract

{
public:
   using contract::contract;

   [[eosio::action]] void userupdate(name account, uint64_t contestKey, int64_t toBeAdded)
   {
      require_auth(name(MY_CONTRACT_BETNOW_APEX));
      ranking_index rankingTable(get_self(), contestKey);
      auto iteratorRankingTable = rankingTable.find(account.value);
      if (iteratorRankingTable != rankingTable.end())
      {
         rankingTable.modify(iteratorRankingTable, get_self(), [&](auto &row) {
            if ((toBeAdded < 0) && (row.betPoints < abs(toBeAdded)))
            {
               row.betPoints = 0;
            }
            else
            {
               row.betPoints += toBeAdded;
            }
         });
      }
      else if (iteratorRankingTable == rankingTable.end() && toBeAdded > 0)
      {
         rankingTable.emplace(get_self(), [&](auto &row) {
            row.user = account;
            row.betPoints = toBeAdded;
         });
      }
   }

   [[eosio::action]] void betleaderpts(uint64_t event)
   {
      require_auth(name(MY_CONTRACT_BETNOW_APEX));
      uint64_t totalAmountEvent = getEventTot(event);
      eventsTable_index eventstable(name(MY_CONTRACT_BETNOW_APEX), (name(MY_CONTRACT_BETNOW_APEX)).value);
      auto iteratorEventsTable = eventstable.find(event);
      betsTable_index betsTable(name(MY_CONTRACT_BETNOW_APEX), event);
      auto betsTable_index = betsTable.template get_index<"bysecondkey"_n>();
      uint64_t upperBound = (iteratorEventsTable->winnerF + 1) * 100000000000000;
      auto iteratorBetsTable = betsTable_index.upper_bound(upperBound);
      if (iteratorBetsTable != betsTable_index.begin())
      {
         iteratorBetsTable--;
      };
      auto tempIterator = iteratorBetsTable;
      uint64_t equalValues = 0;
      float points = iteratorEventsTable->reward * (1 + kfactor * (float(iteratorBetsTable->amount.amount) / (float(totalAmountEvent) / iteratorEventsTable->betCounterEvent)));
      while (iteratorBetsTable->amount.amount == tempIterator->amount.amount && tempIterator != betsTable_index.begin())
      {
         equalValues++;
         tempIterator--;
      }
      uint64_t toBeAdded = (uint64_t)(points * 1000 * betLeaderBonus / equalValues);
      for (int j = 0; j < equalValues; j++)
      {
         ranking_index rankingTable(get_self(), iteratorEventsTable->contestKey);
         auto iteratorRankingTable = rankingTable.find(iteratorBetsTable->account.value);
         if (iteratorRankingTable != rankingTable.end())
         {
            rankingTable.modify(iteratorRankingTable, get_self(), [&](auto &row) {
               row.betPoints += toBeAdded;
            });
         }
         else
         {
            rankingTable.emplace(get_self(), [&](auto &row) {
               row.user = iteratorBetsTable->account;
               row.betPoints = toBeAdded;
            });
         }
         std::string message = "Added " + uint128ToString(toBeAdded) + " betnow bonus points to balance!";
         sendSummary(iteratorBetsTable->account, "BETLEADER", message);
         iteratorBetsTable--;
      }
   }

   [[eosio::action]] void contestadd(uint64_t contestKey, std::string contestDescription, std::string contestWeights)
   {
      eosio::check(has_auth(name(MY_CONTRACT_APEX)) || has_auth(name(MY_CONTRACT_BETNOW_APEX)), "ERROR: Missing required authority!");
      eosio::check(contestKey > 0, "ERROR: ContestKey cannot be 0!");
      contestWeights.erase(std::remove(contestWeights.begin(), contestWeights.end(), ' '), contestWeights.end());
      std::vector<uint64_t> weightElements = parseContestWeights(contestWeights);
      eosio::check(weightElements.size() > 2, "ERROR: More than three weights required!");
      float sumOfWeights = totalWeight(weightElements);
      std::string sumOfWeightString = "ERROR: Weights not summing up to 100%! total is " + std::to_string(sumOfWeights);
      eosio::check(sumOfWeights == 100, sumOfWeightString);
      contestTable_index contesttable(get_self(), _first_receiver.value);
      auto iteratorContestsTable = contesttable.find(contestKey);
      if (iteratorContestsTable == contesttable.end())
      {
         contesttable.emplace(get_self(), [&](auto &row) {
            row.contestKey = contestKey;
            row.contestDescription = contestDescription;
            row.contestWeights = contestWeights;
         });
         sendSummary(_self, "SUCCESS", "Added contest to contestTable!");
      }
      else
      {
         sendSummary(_self, "ERROR", "Contest already registred!");
      };
   }

   [[eosio::action]] void contestclose(uint64_t contestKey, bool close)
   {
      eosio::check(has_auth(name(MY_CONTRACT_BETCONTEST_APEX)) || has_auth(name(MY_CONTRACT_BETNOW_APEX)), "ERROR: Missing required authority!");
      eosio::check(contestCloseStatusIsDifferent(contestKey, close), "ERROR: Contest close bool is already as wanted!");
      contestTable_index contesttable(get_self(), _first_receiver.value);
      auto iteratorContestsTable = contesttable.find(contestKey);
      if (iteratorContestsTable != contesttable.end())
      {
         eventsTable_index eventsTable(name(MY_CONTRACT_BETNOW_APEX), (name(MY_CONTRACT_BETNOW_APEX)).value);
         auto iteratorContestsTable = contesttable.find(contestKey);
         auto eventsTable_index = eventsTable.template get_index<name("bycontestkey")>();
         auto iteratorEventsTable = eventsTable_index.upper_bound(contestKey);
         if (iteratorEventsTable != eventsTable_index.begin())
            iteratorEventsTable--;
         //sendSummary(_self, "INFO",  iteratorEventsTable->eventDescription);
         //sendSummary(_self, "INFO", uint128ToString(contestKey) );
         if (iteratorEventsTable == eventsTable_index.end() || iteratorEventsTable->contestKey != contestKey)
         {
            contesttable.modify(iteratorContestsTable, get_self(), [&](auto &row) {
               row.close = close;
            });
         }
         else
         {
            sendSummary(_self, "ERROR", "Events still presents in eventstable with this contestKey!");
         }
      }
      else
      {
         sendSummary(_self, "ERROR", "Contest not found!");
      };
   }

   [[eosio::action]] void contestpay(uint64_t contestKey, uint64_t usersToBePaid)
   {
      eosio::check(has_auth(name(MY_CONTRACT_BETCONTEST_APEX)) || has_auth(name(MY_CONTRACT_BETNOW_APEX)), "ERROR: Missing required authority!");
      eosio::check(contestCanBePaid(contestKey), "ERROR: Contest either does not exist or is not closed!");
      eosio::check(usersToBePaid > 0, "ERROR: usersToBePaid must be > 0");
      contestTable_index contesttable(get_self(), _first_receiver.value);
      auto iteratorContestsTable = contesttable.find(contestKey);
      uint64_t total = 0;
      ranking_index rankingTable(get_self(), contestKey);
      auto ranking_index = rankingTable.template get_index<"bybetpoints"_n>();
      auto iteratorRankingTable = ranking_index.begin();
      auto lastToBePaid = ranking_index.end();
      auto lowerBound = ranking_index.begin();
      std::vector<uint64_t> contestWeights = parseContestWeights(iteratorContestsTable->contestWeights);
      if (std::distance(ranking_index.begin(), ranking_index.end()) > contestWeights.size())
      {
         std::advance(lastToBePaid, -contestWeights.size());
         lowerBound = lastToBePaid;
         while (lastToBePaid->betPoints == lowerBound->betPoints && lowerBound != ranking_index.begin())
         {
            lowerBound--;
         }
         if (lowerBound != ranking_index.begin())
         {
            lowerBound++;
         }
      }
      uint64_t i = 0;
      while (i < usersToBePaid && iteratorRankingTable != ranking_index.end())
      {
         uint64_t loserEntries = std::distance(iteratorRankingTable, lowerBound);
         if (loserEntries == 0)
         {
            while (i < usersToBePaid && iteratorRankingTable != ranking_index.end())
            {
               uint64_t remainingWinnerEntries = std::distance(iteratorRankingTable, ranking_index.end());
               asset Reward(0, ASSET);
               auto tempIterator = iteratorRankingTable;
               tempIterator++; //evaluate next element
               uint64_t equalValues = 1;
               float sumOfWeights = 0;
               if (remainingWinnerEntries <= contestWeights.size())
               {
                  sumOfWeights = contestWeights[remainingWinnerEntries - 1];
               }
               while (iteratorRankingTable->betPoints == tempIterator->betPoints && tempIterator != ranking_index.end())
               {
                  if (remainingWinnerEntries - equalValues <= contestWeights.size())
                  {
                     sumOfWeights += contestWeights[remainingWinnerEntries - equalValues - 1];
                  }
                  equalValues++;
                  tempIterator++;
               }
               for (int j = 0; j < equalValues; j++)
               {
                  Reward.amount = (iteratorContestsTable->balance.amount * (sumOfWeights / equalValues));
                  Reward.amount = (Reward.amount / 1000) * 10;
                  std::string memo = "Thank you for partecipating in our contest, your ranking position was " + std::to_string(remainingWinnerEntries - equalValues + 1) + "! You have won " + std::to_string((float)(iteratorContestsTable->balance.amount) / 10000) + " EOS!";
                  total += Reward.amount;
                  if (Reward.amount > 0)
                  {
                     sendEOS(iteratorRankingTable->user, Reward, memo);
                  }
                  iteratorRankingTable = ranking_index.erase(iteratorRankingTable);
                  i++;
               }
            }
         }
         else
         {
            iteratorRankingTable = ranking_index.erase(iteratorRankingTable);
            i++;
         }
      }
      if (ranking_index.begin() == ranking_index.end())
      {
         sendSummary(_self, "INFO", "no more entries left for this contest!");
         eraseContest(contestKey);
      }
      print("|total|");
      print(std::to_string(total));
   }

   [[eosio::action]] void contestrname(uint64_t contestKey, std::string contestDescription)
   {
      eosio::check(has_auth(name(MY_CONTRACT_BETCONTEST_APEX)) || has_auth(name(MY_CONTRACT_BETNOW_APEX)), "ERROR: Missing required authority!");
      contestTable_index contesttable(get_self(), _first_receiver.value);
      auto iteratorContestsTable = contesttable.find(contestKey);
      if (iteratorContestsTable != contesttable.end())
      {
         eventsTable_index eventsTable(name(MY_CONTRACT_BETNOW_APEX), (name(MY_CONTRACT_BETNOW_APEX)).value);
         auto iteratorContestsTable = contesttable.find(contestKey);
         contesttable.modify(iteratorContestsTable, get_self(), [&](auto &row) {
            row.contestDescription = contestDescription;
         });
      }
      else
      {
         sendSummary(_self, "ERROR", "Contest not found!");
      };
   }

   [[eosio::action]] void userdelete(name user)
   {
      require_auth(name(MY_CONTRACT_BETNOW_APEX));
      contestTable_index contesttable(get_self(), _first_receiver.value);
      for (auto iteratorContestsTable = contesttable.begin(); iteratorContestsTable != contesttable.end();)
      {
         ranking_index rankingTable(get_self(), iteratorContestsTable->contestKey);
         auto iteratorRankingTable = rankingTable.find(user.value);
         if (iteratorRankingTable != rankingTable.end())
         {
            rankingTable.erase(iteratorRankingTable);
         }
         iteratorContestsTable++;
      }
      sendSummary(user, "SUCCESS:", "Deleted records from rankingTable!");
   }

   [[eosio::action]] void notify(name account, std::string msg)
   {
      eosio::check(has_auth(name(MY_CONTRACT_BETCONTEST_APEX)) || has_auth(name(MY_CONTRACT_BETNOW_APEX)), "ERROR: Missing required authority!");
      require_recipient(account);
   }

   [[eosio::action]] void deletetables()
   {
      require_auth(_self);
      contestTable_index contesttable(get_self(), get_self().value);
      auto iteratorContestsTable = contesttable.begin();
      while (iteratorContestsTable != contesttable.end())
      {
         ranking_index ranking(get_self(), iteratorContestsTable->contestKey);
         auto iteratorRankingTable = ranking.begin();
         while (iteratorRankingTable != ranking.end())
         {
            iteratorRankingTable = ranking.erase(iteratorRankingTable);
         }
         iteratorContestsTable = contesttable.erase(iteratorContestsTable);
      }
   }

   void updateblnce(name from, name to, asset quantity, std::string memo)
   {
      require_auth(from);
      if (from != name(MY_CONTRACT_BETNOW_APEX))
      {
         return;
      }
      if (!checkSymbol(quantity) || !checkIncomingTX(to, _self))
      {
         return;
      }
      std::vector<std::string> memoElements = parseMemoContests(memo);
      if (memoElements.size() == 1)
      {
         return;
      }
      uint64_t contestKey = std::stoull(memoElements[0]);
      std::string contestDescription = memoElements[1];
      if (!checkContest(contestKey, contestDescription))
      {
         return;
      }
      else
      {
         contestTable_index contesttable(get_self(), to.value);
         auto iteratorContestsTable = contesttable.find(contestKey);
         contesttable.modify(iteratorContestsTable, to, [&](auto &row) {
            row.balance += quantity;
         });
         sendSummary(to, "SUCCESS", "Updated contest balance!");
      }
   }

private:
   typedef eosio::multi_index<"contests"_n, contesttable> contestTable_index;

   typedef eosio::multi_index<"ranking"_n, ranking,
                              indexed_by<"bybetpoints"_n, const_mem_fun<ranking, uint64_t, &ranking::get_betPoints>>>
       ranking_index;

   typedef eosio::multi_index<"bets"_n, betstable,
                              indexed_by<"byuser"_n, const_mem_fun<betstable, uint64_t, &betstable::get_user>>,
                              indexed_by<"bysecondkey"_n, const_mem_fun<betstable, uint64_t, &betstable::get_secondaryKey>>>
       betsTable_index;

   typedef eosio::multi_index<"events"_n, eventstable,
                              indexed_by<"bycontestkey"_n, const_mem_fun<eventstable, uint64_t, &eventstable::get_contestKey>>>
       eventsTable_index;

   typedef eosio::multi_index<"totalbalance"_n, betsumtable> betSumTable_index;

   void sendSummary(name account, std::string status, std::string message)
   {
      std::string completeMessage;
      if (status == "ERROR:")
      {
         completeMessage = "Hi, " + name{account}.to_string() + "! ERROR: " + message;
      }
      else if (status == "INFO")
      {
         completeMessage = "Hi, " + name{account}.to_string() + "! INFO: " + message;
      }
      else
      {
         completeMessage = "Hi, " + name{account}.to_string() + "! " + message;
      }
      action(
          permission_level{get_self(), "active"_n},
          get_self(),
          "notify"_n,
          std::make_tuple(account, completeMessage))
          .send();
   };

   void eraseContest(uint64_t contestKey)
   {
      eosio::check(has_auth(name(MY_CONTRACT_BETCONTEST_APEX)) || has_auth(name(MY_CONTRACT_BETNOW_APEX)), "ERROR: Missing required authority!");
      contestTable_index contesttable(get_self(), get_self().value);
      auto iteratorContestsTable = contesttable.find(contestKey);
      if (iteratorContestsTable != contesttable.end() && iteratorContestsTable->close == true)
      {
         contesttable.erase(iteratorContestsTable);
         sendSummary(_self, "SUCCESS", "Erased contest from contestTable");
      }
      else
      {
         sendSummary(_self, "ERROR:", "contest not found!");
      }
   }

   std::vector<std::string> parseMemoContests(std::string memo)
   {
      std::string tempMemo = memo;
      std::vector<std::string> memoElements = splitString(memo, '|');
      if (memoElements.size() == 2)
      {
         return memoElements;
      }
      memoElements.resize(1, 0);
      return memoElements;
   }

   std::vector<uint64_t> parseContestWeights(std::string memo)
   {
      std::string tempMemo = memo;
      tempMemo.erase(std::remove(tempMemo.begin(), tempMemo.end(), '|'), tempMemo.end());
      std::vector<std::string> memoElements = splitString(memo, '|');
      std::vector<uint64_t> floatElements;
      if (isNumber(tempMemo))
      {
         if (memoElements.size() > 2)
         {
            int i = 0;
            while (i < memoElements.size())
            {
               floatElements.push_back((std::stoi(memoElements[i])));
               i++;
            }
            return floatElements;
         }
      }
      floatElements.resize(1, 0);
      return floatElements;
   }

   uint64_t getEventTot(uint64_t event)
   {
      eventsTable_index eventstable(name(MY_CONTRACT_BETNOW_APEX), (name(MY_CONTRACT_BETNOW_APEX)).value);
      betSumTable_index totalbalance(name(MY_CONTRACT_BETNOW_APEX), (name(MY_CONTRACT_BETNOW_APEX)).value);
      auto iteratorEventsTable = eventstable.find(event);
      uint64_t betCounterEvent = iteratorEventsTable->betCounterEvent;
      std::vector<std::string> ArrayForecastsEvent = splitString(iteratorEventsTable->forecasts, ':');
      uint64_t totalAmountEvent = 0;
      for (auto const &forecast : ArrayForecastsEvent)
      {
         uint64_t betKey = event * (uint64_t)1000000 + std::stoull(forecast);
         auto iteratorBetSumTable = totalbalance.find(betKey);
         totalAmountEvent += iteratorBetSumTable->balanceL.amount; //actual value
      }
      return totalAmountEvent;
   }

   float totalWeight(std::vector<uint64_t> weightElements)
   {
      int i = 0;
      uint64_t total = 0;
      while (i < weightElements.size())
      {
         total += weightElements[i];
         i++;
      }
      return total;
   }

   bool checkContest(uint64_t contestKey, std::string contestDescription)
   {
      contestTable_index contesttable(get_self(), get_self().value);
      auto iteratorContestsTable = contesttable.find(contestKey);
      if ((iteratorContestsTable == contesttable.end()) || (iteratorContestsTable->contestDescription != contestDescription))
         return false;
      return true;
   }

   bool contestCanBePaid(uint64_t contestKey)
   {
      contestTable_index contesttable(get_self(), get_self().value);
      auto iteratorContestsTable = contesttable.find(contestKey);
      if ((iteratorContestsTable == contesttable.end()) || (iteratorContestsTable->close == false))
         return false;
      return true;
   }

   bool contestCloseStatusIsDifferent(uint64_t contestKey, bool close)
   {
      contestTable_index contesttable(get_self(), get_self().value);
      auto iteratorContestsTable = contesttable.find(contestKey);
      if (iteratorContestsTable != contesttable.end() && iteratorContestsTable->close == close)
         return false;
      return true;
   }

   void sendEOS(name to, asset quantity, std::string memo)
   {
      name from = name(MY_CONTRACT_BETCONTEST_APEX);
      action sendEOSToken = action(
          permission_level{get_self(), "active"_n},
          "eosio.token"_n,
          "transfer"_n,
          std::make_tuple(from, to, quantity, memo));
      sendEOSToken.send();
   }

   static uint128_t combine_ids_128_64_64(const uint64_t &x, const uint64_t &y)
   {
      return (uint128_t{x} << 64) | y;
   }
};

extern "C" void apply(uint64_t receiver, uint64_t code, uint64_t action)
{
   if (MAINTENANCE == 1)
   {
      eosio::check(has_auth(name(MY_CONTRACT_BETCONTEST_APEX)) || has_auth(name(MY_CONTRACT_BETNOW_APEX)), "ERROR: We are working for you!");
   }
   if (action == name("transfer").value && code == name("eosio.token").value)
   {
      execute_action<MY_CONTRACT_CLASS>(eosio::name(receiver), eosio::name(code),
                                        &MY_CONTRACT_CLASS::updateblnce);
   }
   else if (code == receiver)
   {
      switch (action)
      {
         EOSIO_DISPATCH_HELPER(MY_CONTRACT_CLASS,
                               (notify)(userdelete)(betleaderpts)(contestpay)(contestrname)(contestadd)(contestclose)(userupdate));
      }
   }
}

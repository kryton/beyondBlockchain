#define MY_CONTRACT_CLASS betnow
#define MY_CONTRACT betnow
#define MY_CONTRACT_APEX "betnow"
#include "../include/common.hpp"
using namespace eosio;

class [[eosio::contract(MY_CONTRACT_APEX)]] MY_CONTRACT_CLASS : public eosio::contract

{

public:
   using contract::contract;

   /***********************ACTIONS*********************************/

   //! User action for signup to betnow.io
   /*!
         require_auth(account).
         This action use the following table: usersTable.
         \param account User sender of the action.
         \return nothing, send message
         \sa checkUserRegistration()
      */
   [[eosio::action]] //signup to betnow.io
   void
   signup(name account)
   {
      eosio::check(has_auth(name(MY_CONTRACT_BETCONTEST_APEX)) || has_auth(account), "ERROR: Missing required authority!");
      eosio::check(!checkUserRegistration(account), "ERROR: user is already registred!");
      usersTable_index usersTable(get_self(), get_first_receiver().value);
      usersTable.emplace(_first_receiver, [&](auto &row) {
         row.user = account;
         row.userid = account.value;
      });
      sendSummary(account, "SUCCESS:", " Welcome to betnow.io!");
      updateDappUserCounter('+');
   }

   //! User action for unsubscribe to betnow.io
   /*!
         require_auth(account).
         This action uses the following table: usersTable.
         \param account User sender of the action.
         \return nothing, send message
         \sa checkUserRegistration() and userDeleteFromRankingTable()
      */
   [[eosio::action]] //unsubscribe to betnow.io
   void
   userdelete(name account)
   {
      require_auth(account);
      eosio::check(checkUserRegistration(account), "ERROR: user is not registred!");
      if (!checkUserRegistration(account))
      {
         sendSummary(account, "ERROR:", " User not registered!");
         return;
      }
      if (!checkUserIsBetting(account))
      {
         usersTable_index usersTable(get_self(), get_self().value);
         usersTable.erase(usersTable.find(account.value));
         userDeleteFromRankingTable(account);
         sendSummary(account, "SUCCESS:", "Deleted User from betnow.io!");
         updateDappUserCounter('-');
      }
      else
      {
         sendSummary(account, "ERROR:", " User still has open bets!");
      }
   }

   //! Action that close or open event.
   /*!
         require_auth(_self).
         This action uses the following table: eventstable.
         \param event event that has to be close.
         \param close boolean for close or open event.
         \return nothing
      */
   [[eosio::action]] //does not check time
   void
   eventclose(uint64_t event, bool close)
   {
      eosio::check(has_auth(name(MY_CONTRACT_BETCONTEST_APEX)) || has_auth(name(MY_CONTRACT_BETNOW_APEX)), "ERROR: Missing required authority!");
      eosio::check(eventCloseStatusIsDifferent(event, close), "ERROR: Event close bool is already as wanted!");
      eventsTable_index eventstable(get_self(), _first_receiver.value);
      auto iteratorEventsTable = eventstable.find(event);
      if (iteratorEventsTable != eventstable.end())
      {
         eventstable.modify(iteratorEventsTable, _self, [&](auto &row) { row.close = close; });
         if (close)
            sendSummary(_self, "SUCCESS:", " Closed event!");
         else
            sendSummary(_self, "SUCCESS:", " Opened event!");
      }
      else
      {
         sendSummary(_self, "ERROR", " event not found!");
      }
   }

   //! Refunds all bets for a specific event.
   /*!
         require_auth(_self).
         This action uses the following table: eventstable and betsTable.
         \param event event.
         \param usersToBeRefunded Number of user.
         \return nothing
         \sa sendEOS() and cleanupevent()
      */
   [[eosio::action]] void eventrefund(uint64_t event, uint64_t usersToBeRefunded)
   {
      require_auth(_self);
      eosio::check(event, "ERROR: Must be set an event");
      eosio::check(usersToBeRefunded > 0, "ERROR: Must be usersToBeRefunded > 0");
      eventsTable_index eventstable(get_self(), _first_receiver.value);
      betsTable_index betsTable(get_self(), event);
      auto iteratorEventsTable = eventstable.find(event);
      if (iteratorEventsTable == eventstable.end())
      {
         sendSummary(_self, "ERROR", " event not found!");
         return;
      }
      if (iteratorEventsTable->close == true && iteratorEventsTable->reward < 0)
      {
         auto iteratorBetsTable = betsTable.begin();
         uint64_t i = 0;
         while (i < usersToBeRefunded && iteratorBetsTable != betsTable.end())
         {
            asset toSend = iteratorBetsTable->amount;
            std::string memo = "This is a refund!";
            sendEOS(iteratorBetsTable->account, toSend, memo);
            iteratorBetsTable = betsTable.erase(iteratorBetsTable);
            i++;
         }
         if (iteratorBetsTable == betsTable.end())
         {
            eosio::print("No more bets left with this event!", _first_receiver);
            cleanupevent(event);
         }
      }
      else
      {
         sendSummary(_self, "ERROR", " event is open!");
         return;
      }
   }

   [[eosio::action]] void deletetables()
   {
      require_auth(_self);
      eventsTable_index eventstable(get_self(), _first_receiver.value);
      auto iteratorEventsTable = eventstable.begin();
      while (iteratorEventsTable != eventstable.end())
      {
         betsTable_index betsTable(get_self(), iteratorEventsTable->event);
         auto iteratorBetsTable = betsTable.begin();
         while (iteratorBetsTable != betsTable.end())
         {
            iteratorBetsTable = betsTable.erase(iteratorBetsTable);
         }
         iteratorEventsTable = eventstable.erase(iteratorEventsTable);
      }
      betSumTable_index totalbalance(get_self(), _first_receiver.value);
      auto iteratorBetSumTable = totalbalance.begin();
      while (iteratorBetSumTable != totalbalance.end())
      {
         iteratorBetSumTable = totalbalance.erase(iteratorBetSumTable);
      }
      usersTable_index usersTable(get_self(), _first_receiver.value);
      auto iteratorUsersTable = usersTable.begin();
      while (iteratorUsersTable != usersTable.end())
      {
         iteratorUsersTable = usersTable.erase(iteratorUsersTable);
      }
      dappTable_index dappTable(get_self(), _first_receiver.value);
      auto iteratorDappTable = dappTable.begin();
      while (iteratorDappTable != dappTable.end())
      {
         iteratorDappTable = dappTable.erase(iteratorDappTable);
      }
      sendSummary(_self, "SUCCESS:", "Deleted tables! ");
   }

   //! Create event.
   /*!
         require_auth(_self).
         This action uses the following table: eventstable.
         \param event event that has to be create.
         \param eventCategory
         \param eventDescription
         \param contestKey
         \param contestDescription
         \param forecasts "forecast1:forecast2:forecast3:..."
         \param results "result1:result2:result3:..."
         \param betStart
         \param betEnd
         \param bonus
         \param winningFee "Bookmaker fee"
         \return nothing
         \sa checkContest() and registerBetsOnBetSumTable()
      */
   [[eosio::action]] void eventcreate(uint64_t event, std::string eventCategory, std::string eventDescription, uint64_t contestKey, std::string contestDescription, std::string forecasts, std::string results, uint64_t betStart, uint64_t betEnd, float bonus, float winningFee)
   {
      require_auth(_self);
      eosio::check(0 <= bonus && bonus <= 1, "ERROR: Must be 0 <= bonus <= 1");
      eosio::check(0 <= winningFee && winningFee <= 1, "ERROR: Must be 0 <= winningFee <= 1");
      time_point betStartTime = time_point{microseconds{static_cast<int64_t>(betStart)}};
      time_point betEndTime = time_point{microseconds{static_cast<int64_t>(betEnd)}};
      eventsTable_index eventstable(get_self(), _first_receiver.value);
      auto iteratorEventsTable = eventstable.find(event);
      if (!checkContest(contestKey, contestDescription))
      {
         sendSummary(_self, "ERROR", "Contest not found or wrong description!");
         return;
      }
      if (nowTime > betStartTime || betStartTime >= betEndTime || nowTime > betEndTime)
      {
         sendSummary(_self, "ERROR", " Check betStartTime and betEndTime!");
         return;
      }
      if (iteratorEventsTable != eventstable.end())
      {
         sendSummary(_self, "ERROR", " Event already created!");
         return;
      }
      if (!parseDescriptionForCreateEvent(eventDescription))
      {
         sendSummary(_self, "ERROR", "Wrong description, needs to specify EventType:Event !");
         return;
      }
      if (!parseMemoForecastResult(forecasts, results))
      {
         sendSummary(_self, "ERROR", " Numbers of forecasts has to be equal to results!");
         return;
      }
      else
      {
         if (!forecasts.empty())
         {
            eventstable.emplace(_self, [&](auto &row) {
               row.event = event;
               row.forecasts = forecasts;
               row.betStartTime = betStartTime;
               row.betEndTime = betEndTime;
               row.bonus = ((int)(1000 * bonus)) / 1000.0;
               row.eventDescription = eventDescription;
               row.eventCategory = eventCategory;
               row.contestKey = contestKey;
            });
            registerBetsOnBetSumTable(event, forecasts, results);
            updateDappEventCounter();
            sendSummary(_self, "SUCCESS:", " Successfully recorded event!");
         }
         else
         {
            sendSummary(_self, "ERROR", " Empty forecasts");
         }
      }
   }

   //! Calculate reward of the event
   /*!
         This action uses the following table: eventstable.
         \param event event that has to be done.
         \param winner
         \param usersPerTX Number of user to iterate
         \return nothing
         \sa getReward() and eventpay()
      */
   [[eosio::action]] void rewardpull(uint64_t event, uint64_t winner, uint64_t usersPerTX)
   {
      eosio::check(has_auth(name(MY_CONTRACT_BETCONTEST_APEX)) || has_auth(name(MY_CONTRACT_BETNOW_APEX)), "ERROR: Missing required authority!");
      eosio::check(winner >= 0, "ERROR: Must be winner >= 0");
      eosio::check(usersPerTX >= 0, "ERROR: Must be usersPerTX >= 0");
      bool winnerCheck = false;
      eventsTable_index eventstable(get_self(), _first_receiver.value);
      auto iteratorEventsTable = eventstable.find(event);
      std::vector<std::string> arrayForecastsEvent;
      arrayForecastsEvent = splitString(iteratorEventsTable->forecasts, ':');
      std::set<uint64_t> setOfForecst;
      float Reward;
      asset winnersbalance = asset((uint64_t)0, ASSET);
      asset losersbalance = asset((uint64_t)0, ASSET);
      if (iteratorEventsTable != eventstable.end() && (iteratorEventsTable->betEndTime < nowTime) && (iteratorEventsTable->reward == -9999))
      {
         for (auto const &forecast : arrayForecastsEvent)
         {
            if (std::stoull(forecast) == winner)
            {
               winnerCheck = true;
            }
         }
         eosio::check(winnerCheck, "ERROR: Winner doesn't exist");
         Reward = getReward(event, winner, winnersbalance, losersbalance, iteratorEventsTable->winningFee, arrayForecastsEvent); //.5 % fee
         eventstable.modify(iteratorEventsTable, _self, [&](auto &row) {
               row.close = true;
               row.event = event;
               row.winnerF = winner;
               row.reward = Reward; });
         sendBonusPointsToBetLeader(event);
         sendSummary(_first_receiver, "SUCCESS", " Successfully added rewards to eventsTable");
      }
      else if (iteratorEventsTable == eventstable.end() || (iteratorEventsTable->betEndTime > nowTime))
      {
         sendSummary(_first_receiver, "ERROR", "event not found or ongoing");
         return;
      }
      else if (iteratorEventsTable->reward >= 0)
      {
         sendSummary(_first_receiver, "", " Reward already calculated for this event!");
         return;
      }
   }

   //! Pay winner user of the event
   /*!
         require_auth(_self).
         This action uses the following tables: eventstable and betsTable.
         \param event event that has to be done.
         \param usersToBePaid Number of user to iterate
         \return nothing
         \sa sendEOS(), updateTotalWin() and userUpdateScore()
      */
   [[eosio::action]] void eventpay(uint64_t event, uint64_t usersToBePaid)
   {
      require_auth(_self);
      eventsTable_index eventstable(get_self(), _first_receiver.value);
      betsTable_index betsTable(get_self(), event);
      auto iteratorEventsTable = eventstable.find(event);
      uint64_t i = 0;
      asset toSend = asset((uint64_t)0, ASSET);
      int64_t toBeAdded;
      if (iteratorEventsTable != eventstable.end() && iteratorEventsTable->close == true && iteratorEventsTable->reward >= 0)
      {
         uint64_t totalAmountEvent = getEventTot(event);
         uint64_t numberOfForecasts = splitString(iteratorEventsTable->forecasts, ':').size();
         auto iteratorBetsTable = betsTable.begin();
         while (i < usersToBePaid && iteratorBetsTable != betsTable.end())
         {
            uint64_t amount = iteratorBetsTable->amountW.amount;
            float points = iteratorEventsTable->reward * (1 + kfactor * (float(amount) / (float(totalAmountEvent) / iteratorEventsTable->betCounterEvent)));
            if (iteratorBetsTable->forecast == iteratorEventsTable->winnerF)
            { //user has won
               toSend.amount = iteratorBetsTable->amountW.amount * (iteratorEventsTable->reward) + iteratorBetsTable->amount.amount;
               std::string memo = "Thank you for betting with us! You have won " + std::to_string((float)(iteratorBetsTable->amountW.amount * (iteratorEventsTable->reward) / 10000)) + " EOS!";
               toBeAdded = (uint64_t)(points * 1000);
               if (toSend.amount > 0)
               {
                  sendEOS(iteratorBetsTable->account, toSend, memo);
               }
               updateTotalWin(iteratorBetsTable->account, toSend);
               userUpdateScore(iteratorBetsTable->account, iteratorEventsTable->contestKey, toBeAdded);
               userUpdateBetPoints(toBeAdded, iteratorBetsTable->account);
            }
            else
            { //user has lost
               toBeAdded = -(uint64_t)((points / (2 * (numberOfForecasts - 1)) * 1000));
               userUpdateScore(iteratorBetsTable->account, iteratorEventsTable->contestKey, toBeAdded);
            }
            userUpdateBets('-', iteratorBetsTable->account, event);
            iteratorBetsTable = betsTable.erase(iteratorBetsTable);
            i++;
         }
         if (betsTable.begin() == betsTable.end())
         {
            cleanupevent(event);
            sendSummary(_self, "INFO", "No more bets left with this event!");
         }
      }
      else
      {
         sendSummary(_self, "ERROR", "Event is not recorded or is not closed");
      }
   }

   //! User action for claiming reward.
   /*!
         require_auth(account).
         Check if the user is the register to BetNow.io, then check if the event is not empty or close.
         Search inside every forecast of the current event for the user with his superkey. In this way the score is update in either cases, winning and loser.
         This action uses the following tables: eventstable and betsTable.
         \param account 
         \param event event for the reward.
         \return nothing
         \sa checkUserRegistration(), sendEOS(), userUpdateScore() and forceEraseBet()
      */
   [[eosio::action]] void rewardclaim(name account, uint64_t event)
   {
      require_auth(account);
      std::string errorMemo;
      eosio::check(checkUserRegistration(account), "ERROR: user is not registred!");
      eventsTable_index eventstable(get_self(), _first_receiver.value);
      betsTable_index betsTable(get_self(), event);
      auto iteratorEventsTable = eventstable.find(event);
      std::vector<std::string> ArrayForecastsEvent;
      asset toSend = asset((uint64_t)0, ASSET);
      bool flagExists = false;
      int64_t toBeAdded;
      if (iteratorEventsTable != eventstable.end() && iteratorEventsTable->close == true && iteratorEventsTable->reward >= 0)
      {
         ArrayForecastsEvent = splitString(iteratorEventsTable->forecasts, ':');
         uint64_t totalAmountEvent = getEventTot(event);
         uint64_t numberOfForecasts = splitString(iteratorEventsTable->forecasts, ':').size();
         for (auto const &forecast : ArrayForecastsEvent)
         {
            uint128_t superkey = generateSuperKey(account, event, std::stoull(forecast));
            auto iteratorBetsTable = betsTable.find(superkey);
            if (iteratorBetsTable != betsTable.end())
            {
               uint64_t amount = iteratorBetsTable->amountW.amount;
               float points = iteratorEventsTable->reward * (1 + kfactor * (float(amount) / (float(totalAmountEvent) / iteratorEventsTable->betCounterEvent)));
               flagExists = true;
               if (iteratorBetsTable->forecast == iteratorEventsTable->winnerF && iteratorEventsTable->reward >= 0)
               { //user has won
                  toSend.amount = iteratorBetsTable->amountW.amount * (iteratorEventsTable->reward) + iteratorBetsTable->amount.amount;
                  std::string memo = "Thank you for betting with us! You have won " + std::to_string((float)(iteratorBetsTable->amountW.amount * (iteratorEventsTable->reward) / 10000)) + " EOS!";
                  if (toSend.amount > 0)
                  {
                     sendEOS(account, toSend, memo);
                  }
                  toBeAdded = (uint64_t)(points * 1000);
                  userUpdateScore(iteratorBetsTable->account, iteratorEventsTable->contestKey, toBeAdded);
               }
               else
               {
                  toBeAdded = -(uint64_t)((points / (2 * (numberOfForecasts - 1)) * 1000));
                  userUpdateScore(iteratorBetsTable->account, iteratorEventsTable->contestKey, toBeAdded);
               }
               forceEraseBet(account, event, iteratorBetsTable->forecast);
            }
         }
         if (flagExists == false)
         {
            errorMemo = "No bets found! ";
            sendSummary(account, "ERROR:", errorMemo);
         }
         userUpdateBets('-', account, event);
      }
      else
      {
         sendSummary(account, "ERROR", "Event is not recorded or is not closed");
      }
   }

   //! Migrate Event adding bonus to users of the old event
   /*!
         require_auth(_self).
         Check if the event is not empty or close.
         
         This action uses the following tables: eventstable and betsTable.
         \param fromEvent 
         \param toEvent
         \param usersToMigrate
         \param extraBonus float((uint64_t)((1 + extraBonus)) * 1000)) / 1000
         \return nothing
         \sa checkEventsForMigrate(), updateEventCounter(), updateBalance() and cleanupevent()
      */
   [[eosio::action]] void eventmigrate(uint64_t fromEvent, uint64_t toEvent, uint64_t usersToMigrate, float extraBonus)
   {
      eosio::check(has_auth(name(MY_CONTRACT_BETCONTEST_APEX)) || has_auth(name(MY_CONTRACT_BETNOW_APEX)), "ERROR: Missing required authority!");
      eosio::check(fromEvent != toEvent, "ERROR: fromEvent == toEvent!");
      eosio::check(usersToMigrate > 0, "ERROR: usersToMigrate must be > 0!");
      betsTable_index betsTableFrom(get_self(), fromEvent);
      betsTable_index betsTableTo(get_self(), toEvent);
      bool flagCycle = true;
      uint64_t i = 0;

      if (checkEventsForMigrate(fromEvent, toEvent) == true)
      {
         auto iteratorBetsTableFrom = betsTableFrom.begin();
         while (i < usersToMigrate && iteratorBetsTableFrom != betsTableFrom.end())
         {
            float bonus;
            uint128_t superkeyNew = generateSuperKey(iteratorBetsTableFrom->account, toEvent, iteratorBetsTableFrom->forecast);
            uint128_t superkeyOld = generateSuperKey(iteratorBetsTableFrom->account, fromEvent, iteratorBetsTableFrom->forecast);
            uint64_t newBetkey = toEvent * (uint64_t)1000000 + iteratorBetsTableFrom->forecast;
            bonus = 1 + extraBonus;
            bonus = float((uint64_t)(bonus * 1000)) / 1000; //remove decimals after E-3
            asset amountW = asset(iteratorBetsTableFrom->amountW.amount * bonus, ASSET);
            betsTableTo.emplace(_first_receiver, [&](auto &row) {
               row.account = iteratorBetsTableFrom->account;
               row.superkey = superkeyNew;
               row.secondaryKey = (iteratorBetsTableFrom->forecast * 100000000000000) + (iteratorBetsTableFrom->amount.amount / 100);
               row.amount = iteratorBetsTableFrom->amount; // amount without bonus
               row.amountW = amountW;                      // amount with bonus
               row.forecast = iteratorBetsTableFrom->forecast;
            });
            updateEventCounter('+', toEvent);
            uint64_t newBetKey = toEvent * (uint64_t)1000000 + iteratorBetsTableFrom->forecast;
            sendSummary(_first_receiver, "SUCCESS:", "Migrated bet to new event! ");
            updateBalance(newBetkey, iteratorBetsTableFrom->amount, amountW);
            iteratorBetsTableFrom = betsTableFrom.erase(iteratorBetsTableFrom);
            i++;
         }
         if (iteratorBetsTableFrom == betsTableFrom.end())
         {
            eventsTable_index eventstable(get_self(), _first_receiver.value);
            eosio::print("No more bets left with this event!", _first_receiver);
            cleanupevent(fromEvent);
         }
      }
   }

   //! Notify
   [[eosio::action]] void notify(name account, std::string msg)
   {
      eosio::check(has_auth(name(MY_CONTRACT_BETCONTEST_APEX)) || has_auth(name(MY_CONTRACT_BETNOW_APEX)), "ERROR: Missing required authority!");
      require_recipient(account);
   }

   //! Core function
   /*!
         require_auth(from).
         
         \param from name argument.
         \param to name argument.
         \param quantity asset argument.
         \param memo string argument made like event|forecasts|percents.
         \return The test results
         \sa checkSymbol(), checkIncomingTX(), checkDonation(), sendRefund(), checkUserRegistration(), splitString(), checkMemoForecasts() and updateBalance()
      */
   //memo: "event|forecasts|percents" "placebet:10|1:2:3|80:15:5"
   void placebet(name from, name to, asset quantity, std::string memo)
   {
      require_auth(from);
      name contract = to;
      name account = from;
      std::string errorMemo = "";
      if (!checkSymbol(quantity) || !checkIncomingTX(to, _self) || memo == transferMemo || from == newdex || from == eosioStake || from == eosioContract || from == eosioRam)
      {
         return;
      }
      if (checkDonation(memo) || (quantity.amount <= minimumTransaction))
      {
         sendSummary(from, "INFO:", "Thank you for your donation!");
         return;
      }
      std::vector<std::string> memoElements = parseMemoBets(memo);
      if (memoElements.size() == 1)
      {
         errorMemo = "Wrong MEMO, commission was taken ! ";
         sendRefund(quantity, 0.0100, from);
         sendSummary(from, "ERROR:", errorMemo);
         return;
      }
      if (!checkUserRegistration(from))
      {
         errorMemo = "User not registred ! ";
         sendRefund(quantity, 0.0100, from);
         sendSummary(from, "ERROR:", errorMemo);
         return;
      }

      bool flagEvent = false;
      uint64_t event = std::stoull(memoElements[0]);
      eventsTable_index eventstable(get_self(), contract.value);
      auto iteratorEventsTable = eventstable.find(event);
      if (iteratorEventsTable == eventstable.end())
      {
         sendSummary(account, "ERROR", " Event not found");
      }
      else
      {
         if (iteratorEventsTable->close == false && iteratorEventsTable->betStartTime < nowTime && nowTime < iteratorEventsTable->betEndTime)
         {
            flagEvent = true;
            std::vector<std::string> ArrayForecastsUser, ArrayForecastsEvent, ArrayForecastsAmount;
            ArrayForecastsUser = splitString(memoElements[1], ':');
            ArrayForecastsAmount = splitString(memoElements[2], ':');
            ArrayForecastsEvent = splitString(iteratorEventsTable->forecasts, ':');
            //Check if every forecast has its own bet amount
            if (!checkMemoForecasts(account, ArrayForecastsUser, ArrayForecastsEvent, ArrayForecastsAmount))
            {
               sendRefund(quantity, 0.005, from);
               return;
            }
            //checks that sum of bets < amount sent
            uint64_t amount = quantity.amount;
            uint64_t totalForecastsAmount = 0;
            uint64_t temp = 0;
            for (auto const &forecastAmount : ArrayForecastsAmount)
            {
               temp = std::stoull(forecastAmount);
               totalForecastsAmount = totalForecastsAmount + temp;
            }
            if (totalForecastsAmount > amount)
            {
               sendSummary(account, "ERROR", "Amount is greater than sum of bets!");
               sendRefund(quantity, 0.005, from);
               return;
            }
            //Check if forecast selected by user exists in this specific event
            bool flagForecast = false;
            int index = 0;
            asset refund(0, ASSET);
            for (auto const &forecastUser : ArrayForecastsUser)
            { //Range-based for loop
               flagForecast = false;
               for (auto const &forecastEvents : ArrayForecastsEvent)
               {
                  if (forecastUser == forecastEvents)
                  { //Found
                     flagForecast = true;

                     uint64_t forecast = std::stoull(forecastUser);
                     uint128_t superkey = generateSuperKey(account, event, forecast);

                     betsTable_index betsTable(get_self(), event);
                     auto iteratorBetsTable = betsTable.find(superkey);
                     uint64_t betKey = event * (uint64_t)1000000 + forecast;
                     if (iteratorBetsTable == betsTable.end())
                     {
                        asset bet = asset(std::stoull(ArrayForecastsAmount[index]), ASSET);
                        asset betW = addBonus(bet, event);
                        betsTable.emplace(get_self(), [&](auto &row) {
                           row.account = account;
                           row.superkey = superkey;
                           row.secondaryKey = (forecast * 100000000000000) + (bet.amount / 100);
                           row.amount = bet;   // amount without bonus
                           row.amountW = betW; // amount with bonus
                           row.forecast = forecast;
                        });
                        sendSummary(account, "SUCCESS:", " Added record to betsTable");
                        asset amount(std::stoull(ArrayForecastsAmount[index]), ASSET);
                        updateBalance(betKey, bet, betW);
                        updateEventCounter('+', event);
                        userUpdateBets('+', account, event);
                     }
                     else
                     {
                        std::string changes;
                        asset bet = asset(std::stoull(ArrayForecastsAmount[index]), ASSET);
                        asset betW = addBonus(bet, event);
                        betsTable.modify(iteratorBetsTable, contract, [&](auto &row) {
                           row.amount += bet;                      //amount
                           row.amountW += betW;                    //amount
                           row.secondaryKey += (bet.amount / 100); //amount
                        });
                        sendSummary(account, "SUCCESS:", " Updated record to betsTable");
                        asset amount(std::stoull(ArrayForecastsAmount[index]), ASSET);
                        updateBalance(betKey, bet, betW);
                     }
                     break;
                  }
               }
               if (flagForecast == false)
               {
                  refund.amount += std::stoull(ArrayForecastsAmount[index]);
               }

               index++;
            }
            if (refund.amount != 0)
            {
               sendSummary(account, "ERROR", "One or more forecast are inexistent for this event");
               sendRefund(refund, 0.01, from);
            }
         }
         else
         {
            sendSummary(account, "ERROR", " event is closed!");
         };
      }
      if (flagEvent == false)
      {
         sendRefund(quantity, 0.01, from);
      }
   }

private:
   typedef eosio::multi_index<"bets"_n, betstable,
                              indexed_by<"byuser"_n, const_mem_fun<betstable, uint64_t, &betstable::get_user>>,
                              indexed_by<"bysecondkey"_n, const_mem_fun<betstable, uint64_t, &betstable::get_secondaryKey>>>
       betsTable_index;

   typedef eosio::multi_index<"events"_n, eventstable,
                              indexed_by<"bycontestkey"_n, const_mem_fun<eventstable, uint64_t, &eventstable::get_contestKey>>>
       eventsTable_index;

   typedef eosio::multi_index<"totalbalance"_n, betsumtable> betSumTable_index;

   typedef eosio::multi_index<"users"_n, userstable,
                              indexed_by<"bybetpoints"_n, const_mem_fun<userstable, uint64_t, &userstable::get_betNowPoints>>>
       usersTable_index;

   typedef eosio::multi_index<"dappstats"_n, dapptable> dappTable_index;

   typedef eosio::multi_index<"contests"_n, contesttable> contestTable_index;

   /***********************FUNCTIONS*********************************/

   void eraseEvent(uint64_t event, uint64_t forecast)
   {
      eosio::check(has_auth(name(MY_CONTRACT_BETCONTEST_APEX)) || has_auth(name(MY_CONTRACT_BETNOW_APEX)), "ERROR: Missing required authority!");
      eventsTable_index eventstable(get_self(), _first_receiver.value);
      auto iteratorEventsTable = eventstable.find(event);
      if (iteratorEventsTable != eventstable.end())
      {
         if (iteratorEventsTable->winnerF == forecast)
         {
            eventstable.erase(iteratorEventsTable);
            sendSummary(_self, "SUCCESS", "Erased record from eventsTable");
         }
         else
         {
            sendSummary(_self, "ERROR:", "Wrong forecast for this event! Remember default is 9999");
         }
      }
      else
      {
         sendSummary(_self, "ERROR:", "event not found!");
      }
   }

   //! Delete event
   /*!
         Check if the event is empty and close.
         This action uses the following tables: eventstable and betsTable.
         \param event event that has to be clean.
         \return nothing
         \sa eraseCounter() and eraseEvent()
      */
   void cleanupevent(uint64_t event)
   {
      eosio::check(has_auth(name(MY_CONTRACT_BETCONTEST_APEX)) || has_auth(name(MY_CONTRACT_BETNOW_APEX)), "ERROR: Missing required authority!");
      eventsTable_index eventstable(name(MY_CONTRACT_BETNOW_APEX), (name(MY_CONTRACT_BETNOW_APEX)).value);
      betsTable_index betsTable(name(MY_CONTRACT_BETNOW_APEX), event);
      auto iteratorEventsTable = eventstable.find(event);
      if (iteratorEventsTable == eventstable.end() || iteratorEventsTable->close == false || betsTable.begin() != betsTable.end())
      {
         sendSummary(_self, "ERROR", "Event not found, ongoing or still has bets");
      }
      else
      {
         std::vector<std::string> ArrayForecastsEvent;
         ArrayForecastsEvent = splitString(iteratorEventsTable->forecasts, ':');
         eraseCounter(event, ArrayForecastsEvent);
         eraseEvent(event, iteratorEventsTable->winnerF);
         return;
      }
   }

   //! Erase Counter
   /*!
         This action uses the following table: betsSumTable.
         \param event event that has to be erase.
         \param ArrayForecastsEvent String made by all the forecast for the event
         \return nothing
      */
   void eraseCounter(uint64_t event, std::vector<std::string> ArrayForecastsEvent)
   {
      eosio::check(has_auth(name(MY_CONTRACT_BETCONTEST_APEX)) || has_auth(name(MY_CONTRACT_BETNOW_APEX)), "ERROR: Missing required authority!");
      betSumTable_index totalbalance(get_self(), _first_receiver.value);
      for (auto const &forecast : ArrayForecastsEvent)
      {
         uint64_t betKey = event * (uint64_t)1000000 + std::stoull(forecast);
         auto iteratorBetSumTable = totalbalance.find(betKey);
         if (iteratorBetSumTable == totalbalance.end())
         {
            sendSummary(_self, "ERROR:", "Betkey not found!");
         }
         else
         {
            totalbalance.erase(iteratorBetSumTable);
            sendSummary(_self, "SUCCESS:", "Erased record from sumdb");
         }
      }
   }

   //! Get reward for the event
   /*!
         Generate the reward of the event based on winnersbalance and losersbalance
         This action uses the following tables: eventstable and betsSumTable.
         \param event
         \param winner
         \param winnersbalance
         \param losersbalance
         \param fee 
         \param  ArrayForecastsEvent
         \return Reward
         \sa splitString(), toBeAddedToContest(), profit() and sendEOS()
      */
   float getReward(uint64_t event, uint64_t winner, asset & winnersbalance, asset & losersbalance, float fee, std::vector<std::string> arrayForecastsEvent)
   {
      require_auth(_self);
      betSumTable_index totalbalance(get_self(), _first_receiver.value);
      eventsTable_index eventstable(get_self(), _first_receiver.value);
      auto iteratorEventsTable = eventstable.find(event);
      float Reward;
      winnersbalance = asset((uint64_t)0, ASSET);
      losersbalance = asset((uint64_t)0, ASSET);
      for (auto const &forecast : arrayForecastsEvent)
      {
         uint64_t betKey = event * (uint64_t)1000000 + std::stoull(forecast);
         auto itr = totalbalance.find(betKey);
         if (std::stoull(forecast) == winner)
         {
            winnersbalance += itr->balanceW;
         }
         else
         {
            losersbalance += itr->balanceL;
         }
      }
      if (winnersbalance.amount == 0)
      {
         sendSummary(_first_receiver, "ERROR:", "No winners for this bet");
         Reward = 0;
      }
      else
      {
         contestTable_index contesttable(name(MY_CONTRACT_BETCONTEST_APEX), (name(MY_CONTRACT_BETCONTEST_APEX)).value);
         auto iteratorContestTable = contesttable.find(iteratorEventsTable->contestKey);
         if (fee > 0)
         {
            asset toBeAddedToContest(0, ASSET);
            asset profit(0, ASSET);
            updateDappVolume(winnersbalance);
            toBeAddedToContest.amount = losersbalance.amount * fee * contestFactor;
            profit.amount = losersbalance.amount * fee * (1 - contestFactor);
            if (profit.amount > 0)
            {
               std::string memo = std::to_string(iteratorEventsTable->contestKey) + "|" + iteratorContestTable->contestDescription;
               sendEOS(name(MY_CONTRACT_BETCONTEST_APEX), toBeAddedToContest, memo);
               sendEOS(name(MY_CONTRACT_BETBANK_APEX), profit, "profit");
            }
         }
         Reward = (losersbalance.amount * (1 - fee)) / winnersbalance.amount;
         std::string message = "Event= " + std::to_string(event) + "; Winner= " + std::to_string(winnersbalance.amount) + "; Loser= " + std::to_string(losersbalance.amount) + "; Reward= " + std::to_string(Reward) + "/EOS ";
         sendSummary(_first_receiver, "SUCCESS", message);
      }
      return Reward;
   }

   //! Calculate bonus to single bet
   /*!
         Generate bonus for winner bet calculated by the event reward
         This action uses the following table: eventstable
         \param amount
         \param event
         \return amount
      */
   asset addBonus(asset amount, uint64_t event)
   {
      float bonus;
      eventsTable_index eventstable(_self, _self.value);
      auto iteratorEventsTable = eventstable.find(event);
      microseconds timeLeft = iteratorEventsTable->betEndTime - nowTime;
      microseconds betLenght = iteratorEventsTable->betEndTime - iteratorEventsTable->betStartTime;
      bonus = 1 + iteratorEventsTable->bonus * (float)(timeLeft.count()) / betLenght.count();
      bonus = float((uint64_t)(bonus * 1000)) / 1000; //remove decimals after E-3
      amount.amount = amount.amount * bonus;
      return amount;
   }

   //! updateEventCounter
   /*!
         This action uses the following table: eventstable
         \param increase
         \param event
         \return nothing
      */
   void updateEventCounter(char increase, uint64_t event)
   {
      std::string errorMemo = "";
      eventsTable_index eventstable(_self, _self.value);
      auto iteratorEventsTable = eventstable.find(event);
      if (increase == '+')
      {
         eventstable.modify(iteratorEventsTable, name(MY_CONTRACT_BETNOW_APEX), [&](auto &row) { row.betCounterEvent += 1; });
      }
      else if (increase == '-')
      {
         eventstable.modify(iteratorEventsTable, name(MY_CONTRACT_BETNOW_APEX), [&](auto &row) { row.betCounterEvent -= 1; });
      }
      else
      {
         errorMemo = "wrong char, expected + or -!";
         sendSummary(get_self(), "ERROR:", errorMemo);
      }
      return;
   }

   //! userUpdateBets
   /*!
         This action uses the following table: usersTable
         \param increase
         \param user
         \param event
         \return nothing
      */
   void userUpdateBets(char increase, name user, uint64_t event)
   {
      std::string errorMemo = "";
      std::string newUserBetsString;
      std::string strEvent = std::to_string(event);
      std::vector<std::string>::iterator iteratorVector;
      bool modifyTable = false;
      usersTable_index usersTable(get_self(), get_self().value);
      auto iteratorUsersTable = usersTable.find(user.value);
      std::vector<std::string> userBetsVector = splitString(iteratorUsersTable->userBets, ':');
      iteratorVector = std::find(userBetsVector.begin(), userBetsVector.end(), strEvent);
      if (increase == '+' && iteratorVector == userBetsVector.end())
      {                                      //User's first bet on this event
         userBetsVector.push_back(strEvent); //Adding event to array
         modifyTable = true;
      }
      else if (increase == '-' && iteratorVector != userBetsVector.end())
      {                                        //User already has a bet on this event
         userBetsVector.erase(iteratorVector); //Removing event in array
         modifyTable = true;
      }
      else if (increase != '-' && increase != '+')
      {
         errorMemo = "wrong char, expected + or -!";
         sendSummary(get_self(), "ERROR:", errorMemo);
      }
      else
      {
         errorMemo = "no need to update userEvents field!";
         sendSummary(get_self(), "ERROR:", errorMemo);
      }
      if (modifyTable)
      {
         for (auto const &s : userBetsVector)
         {
            newUserBetsString += s + ':';
         };
         if (newUserBetsString.size() > 0)
            newUserBetsString.resize(newUserBetsString.size() - 1);
         usersTable.modify(iteratorUsersTable, _self, [&](auto &row) { row.userBets = newUserBetsString; });
      }
      return;
   }

   void updateDappUserCounter(char increase)
   {
      std::string errorMemo = "";
      dappTable_index dappTable(_self, _self.value);
      auto iteratorDappTable = dappTable.find(1);
      if (iteratorDappTable == dappTable.end())
      {
         dappTable.emplace(name(MY_CONTRACT_BETNOW_APEX), [&](auto &row) {
            row.key = 1;
            row.users = 1;
         });
      }
      else
      {
         if (increase == '+')
         {
            dappTable.modify(iteratorDappTable, name(MY_CONTRACT_BETNOW_APEX), [&](auto &row) { row.users += 1; });
         }
         else if (increase == '-')
         {
            dappTable.modify(iteratorDappTable, name(MY_CONTRACT_BETNOW_APEX), [&](auto &row) { row.users -= 1; });
         }
         else
         {
            errorMemo = "wrong char, expected + or -!";
            sendSummary(get_self(), "ERROR:", errorMemo);
         }
      }
      return;
   }

   void updateDappEventCounter()
   {
      std::string errorMemo = "";
      dappTable_index dappTable(_self, _self.value);
      auto iteratorDappTable = dappTable.find(1);
      if (iteratorDappTable == dappTable.end())
      {
         dappTable.emplace(name(MY_CONTRACT_BETNOW_APEX), [&](auto &row) {
            row.key = 1;
            row.events = 1;
         });
      }
      else
      {
         dappTable.modify(iteratorDappTable, name(MY_CONTRACT_BETNOW_APEX), [&](auto &row) { row.events += 1; });
      }
      return;
   }

   void userUpdateBetPoints(int64_t betpoints, name user)
   {
      std::string errorMemo = "";
      usersTable_index usersTable(name(MY_CONTRACT_BETNOW_APEX), (name(MY_CONTRACT_BETNOW_APEX)).value);
      auto iteratorUsersTable = usersTable.find(user.value);
      if (iteratorUsersTable == usersTable.end())
      {
         errorMemo = "User is not longer registred!";
         sendSummary(name(MY_CONTRACT_BETCONTEST_APEX), "ERROR:", errorMemo);
      }
      else
      {
         usersTable.modify(iteratorUsersTable, name(MY_CONTRACT_BETNOW_APEX), [&](auto &row) {
            if ((betpoints < 0) && (row.betNowPoints < abs(betpoints)))
            {
               row.betNowPoints = 0;
            }
            else
            {
               row.betNowPoints += betpoints;
            }
         });
      }
      return;
   }

   void updateDappVolume(asset volume)
   {
      std::string errorMemo = "";
      dappTable_index dappTable(_self, _self.value);
      auto iteratorDappTable = dappTable.find(1);
      if (iteratorDappTable == dappTable.end())
      {
         dappTable.emplace(name(MY_CONTRACT_BETNOW_APEX), [&](auto &row) {
            row.key = 1;
            row.volume = volume;
         });
      }
      else
      {
         dappTable.modify(iteratorDappTable, name(MY_CONTRACT_BETNOW_APEX), [&](auto &row) { row.volume += volume; });
      }
      return;
   }

   //! Checks event for migrate
   /*!
         Checks if From and To event are ok for migrate.
         Checks if the events exists, are close or open or the to event is going to start or they have not the same forecasts
         This action use the following table: eventstable
         \param fromEvent
         \param toEvent
         \return flagError
      */
   bool checkEventsForMigrate(uint64_t fromEvent, uint64_t toEvent)
   {
      eventsTable_index eventstable(get_self(), _first_receiver.value);
      auto iteratorEventsTable1 = eventstable.find(fromEvent);
      auto iteratorEventsTable2 = eventstable.find(toEvent);
      std::string errorMemo = "";
      if (iteratorEventsTable1 == eventstable.end())
      {
         errorMemo += "fromEvent not found! ";
      }
      else
      {
         if (iteratorEventsTable1->reward > 0)
         {
            errorMemo += "fromEvent reward already calculated! ";
         }
         else
         {
            if (iteratorEventsTable1->close == false)
            {
               errorMemo += "fromEvent still open! ";
            }
         }
      }
      if (iteratorEventsTable2 == eventstable.end())
      {
         errorMemo += "toEvent not found! ";
      }
      else
      {
         if (iteratorEventsTable2 != eventstable.end() && iteratorEventsTable2->betStartTime.elapsed.count() - nowTime.elapsed.count() < 3600000000)
         {
            errorMemo += "toEvent has started/will start in less than 1 hour!";
         }
      }
      bool flagError = (errorMemo == "") ? true : false;
      if ((flagError == true) && (iteratorEventsTable1->forecasts != iteratorEventsTable2->forecasts))
      {
         errorMemo += "Events have different forecasts! ";
         flagError = false;
      }
      if (flagError == false)
      {
         sendSummary(_self, "ERROR:", errorMemo);
      }
      return flagError;
   }

   //! Check if the transaction is a donation
   /*!
         Check the memo, if the memo dosen't start with "placebet:" is a donation
         \param memo
         \return bool
      */
   bool checkDonation(std::string & memo)
   {
      if (memo.substr(0, 9) == "placebet:")
      {
         memo.replace(memo.find("placebet:"), 9, "");
         return false;
      }
      eosio::print(memo.substr(0, 8));
      return true;
   }

   //! Checks if the user is registered
   /*!
         Checks the user existance inside usersTable
         This action use the following table: usersTable
         \param user
         \return bool
      */
   bool checkUserRegistration(name user)
   {
      usersTable_index usersTable(get_self(), get_self().value);
      auto iteratorUsersTable = usersTable.find(user.value);
      if (iteratorUsersTable == usersTable.end())
         return false;
      return true;
   }

   bool checkUserIsBetting(name user)
   {
      usersTable_index usersTable(get_self(), get_self().value);
      auto iteratorUsersTable = usersTable.find(user.value);
      if (iteratorUsersTable->userBets == "")
         return false;
      return true;
   }

   bool checkContest(uint64_t contestKey, std::string contestDescription)
   {
      contestTable_index contesttable(name(MY_CONTRACT_BETCONTEST_APEX), (name(MY_CONTRACT_BETCONTEST_APEX)).value);
      auto iteratorContestTable = contesttable.find(contestKey);
      if ((iteratorContestTable == contesttable.end()) || (iteratorContestTable->contestDescription != contestDescription))
         return false;
      return true;
   }

   bool eventCloseStatusIsDifferent(uint64_t event, bool close)
   {
      eventsTable_index eventstable(name(MY_CONTRACT_BETNOW_APEX), (name(MY_CONTRACT_BETNOW_APEX)).value);
      auto iteratorEventsTable = eventstable.find(event);
      if (iteratorEventsTable != eventstable.end() && iteratorEventsTable->close == close)
         return false;
      return true;
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

   bool checkMemoForecasts(name account, std::vector<std::string> ArrayForecastsUser, std::vector<std::string> ArrayForecastsEvent, std::vector<std::string> ArrayForecastsAmount)
   {
      if (ArrayForecastsUser.size() != ArrayForecastsAmount.size())
      {
         sendSummary(account, "ERROR", "Wrong numbers of forecasts and relative percents");
         return false;
      }
      if (ArrayForecastsUser.size() > ArrayForecastsEvent.size())
      {
         sendSummary(account, "ERROR", "MEMO has more forecasts than this event has!");
         return false;
      }
      return true;
   }

   void forceEraseBet(name account, uint64_t event, uint64_t forecast)
   {
      //require_auth(account);
      eosio::check(has_auth(account) || has_auth(get_self()), "ERROR: Missing required authority!");
      uint128_t superkey = generateSuperKey(account, event, forecast);
      betsTable_index betsTable(get_self(), event);
      auto iteratorBetsTable = betsTable.find(superkey);
      if (iteratorBetsTable == betsTable.end())
      {
         sendSummary(account, "ERROR:", "Bet not found!");
         return;
      }
      else
      {
         betsTable.erase(iteratorBetsTable);
         //sendSummary(account, "SUCCESS:", "Erased record from betsTable!");
      }
   }

   void updateBalance(uint64_t betKey, asset value, asset valueW)
   {
      betSumTable_index totalbalance(name(MY_CONTRACT_BETNOW_APEX), _self.value);
      auto iteratorBetSumTable = totalbalance.find(betKey);
      if (iteratorBetSumTable != totalbalance.end())
      {
         totalbalance.modify(iteratorBetSumTable, name(MY_CONTRACT_BETNOW_APEX), [&](auto &row) {
            row.balanceL += value;
            row.balanceW += valueW;
         });
      }
      else
      {
         sendSummary(_self, "ERROR:", "Betkey not found!");
      };
   }

   void updateTotalWin(name user, asset amount)
   {
      usersTable_index usersTable(get_self(), get_self().value);
      auto iteratorUsersTable = usersTable.find(user.value);
      usersTable.modify(iteratorUsersTable, _self, [&](auto &row) { row.totalWin += amount; });
   }

   void registerBetsOnBetSumTable(uint64_t event, std::string forecasts, std::string results)
   {
      require_auth(_self);
      std::vector<std::string> ArrayForecasts = splitString(forecasts, ':');
      std::vector<std::string> ArrayResults = splitString(results, ':');
      std::string errorMemo = "";
      bool flagError = true;
      if (ArrayForecasts.size() < 2)
      {
         errorMemo += "more than 1 forecast needed! ";
         flagError = false;
      }
      if (ArrayResults.size() < 2)
      {
         errorMemo += "more than 1 result needed! ";
         flagError = false;
      }
      if (ArrayResults.size() != ArrayResults.size())
      {
         errorMemo += "forecasts size != reuslts size! ";
         flagError = false;
      }
      if (flagError == false)
      {
         sendSummary(_self, "ERROR:", errorMemo);
         return;
      }
      else
      {
         betSumTable_index totalbalance(get_self(), _first_receiver.value);
         uint8_t i = 0;
         for (auto const &forecast : ArrayForecasts)
         {
            uint64_t betKey = event * (uint64_t)1000000 + std::stoull(forecast);
            auto iteratorBetSumTable = totalbalance.find(betKey);
            if (iteratorBetSumTable == totalbalance.end())
            {
               totalbalance.emplace(name(MY_CONTRACT_BETNOW_APEX), [&](auto &row) {
                  row.betkey = betKey;
                  row.result = ArrayResults[i];
               });
            }
            else
            {
               sendSummary(_self, "ERROR:", "Betkey already registred!"); // if ArrayForecasts = {1,2,2,3} prints this ERROR message and skips one of the "2"
            };
            i += 1;
         }
      }
   }

   uint128_t generateSuperKey(name account, uint64_t event, uint64_t forecast)
   {
      return (uint128_t)1000000000000000000 * account.value + event * (uint128_t)1000000 + forecast;
   }

   void sendEOS(name to, asset quantity, std::string memo)
   {
      name from = name(MY_CONTRACT_BETNOW_APEX);
      action counter = action(
          permission_level{get_self(), "active"_n},
          "eosio.token"_n,
          "transfer"_n,
          std::make_tuple(from, to, quantity, memo));
      counter.send();
   }

   void sendRefund(asset amount, float fee, name to)
   {
      float gross = amount.amount;
      float net = gross * (1 - fee);
      uint64_t rounded = ((uint64_t)net / 1000) * 1000;
      uint64_t total_commission = (uint64_t)gross - rounded;
      amount.amount = amount.amount - total_commission;
      if (amount.amount != 0)
      {
         //eosio::print("amount to send= ", amount.amount);
         sendEOS(to, amount, "This is a refund, commission was held!");
      }
   }

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

   void sendBonusPointsToBetLeader(uint64_t event)
   {
      action betleaderpts = action(
          permission_level{get_self(), "active"_n},
          name(MY_CONTRACT_BETCONTEST_APEX),
          "betleaderpts"_n,
          std::make_tuple(event));
      betleaderpts.send();
   }

   void userDeleteFromRankingTable(name account)
   {
      action userdelete = action(
          permission_level{get_self(), "active"_n},
          name(MY_CONTRACT_BETCONTEST_APEX),
          "userdelete"_n,
          std::make_tuple(account));
      userdelete.send();
   }

   void userUpdateScore(name account, uint64_t contestKey, int64_t toBeAdded)
   {
      action userupdate = action(
          permission_level{get_self(), "active"_n},
          name(MY_CONTRACT_BETCONTEST_APEX),
          "userupdate"_n,
          std::make_tuple(account, contestKey, toBeAdded));
      userupdate.send();
   }

   void updateContestBalance(uint64_t contestKey, asset toBeAdded)
   {
      action updateblnc = action(
          permission_level{get_self(), "active"_n},
          name(MY_CONTRACT_BETCONTEST_APEX),
          "updateblnc"_n,
          std::make_tuple(contestKey, toBeAdded));
      updateblnc.send();
   }

   std::vector<std::string> parseMemoBets(std::string memo)
   {
      memo.erase(std::remove(memo.begin(), memo.end(), ' '), memo.end());
      memo.erase(std::remove(memo.begin(), memo.end(), '.'), memo.end());
      std::string tempMemo = memo;
      tempMemo.erase(std::remove(tempMemo.begin(), tempMemo.end(), ':'), tempMemo.end());
      tempMemo.erase(std::remove(tempMemo.begin(), tempMemo.end(), '|'), tempMemo.end());
      std::vector<std::string> memoElements = splitString(memo, '|');
      if (isNumber(tempMemo))
      {
         //It has to be made of three elements, if more or less it's wrong formatted
         if (memoElements.size() == 3)
         {
            return memoElements;
         }
      }
      memoElements.resize(1, 0);
      return memoElements;
   }

   bool parseMemoForecastResult(std::string forecasts, std::string results)
   {
      std::vector<std::string> forecastsElements = splitString(forecasts, ':');
      std::vector<std::string> resultsElements = splitString(results, ':');
      if (forecastsElements.size() == resultsElements.size())
      {
         return true;
      }
      return false;
   }

   bool parseDescriptionForCreateEvent(std::string description)
   {
      std::vector<std::string> elements = splitString(description, ':');
      if (elements.size() == 2)
      {
         return true;
      }
      return false;
   }

   //not usefull
   uint64_t hash_64_fnv1a(name user)
   {
      std::string data = name{user}.to_string();
      uint64_t hash = 0xcbf29ce484222325;
      uint64_t prime = 0x100000001b3;

      for (int i = 0; i < 12; ++i)
      {
         uint64_t value = data[i];
         hash = hash ^ value;
         hash *= prime;
      }
      return hash;
   }
};

extern "C" void apply(uint64_t receiver, uint64_t code, uint64_t action)
{
   if (MAINTENANCE == 1)
   { //! Maintenance variable
      eosio::check(has_auth(name(MY_CONTRACT_BETNOW_APEX)) || has_auth(name(MY_CONTRACT_BETCONTEST_APEX)), "ERROR: We are working for you!");
   }
   if (action == name("transfer").value && code == name("eosio.token").value)
   {
      execute_action<MY_CONTRACT_CLASS>(eosio::name(receiver), eosio::name(code),
                                        &MY_CONTRACT_CLASS::placebet);
   }
   else if (code == receiver)
   {
      switch (action)
      {
         EOSIO_DISPATCH_HELPER(MY_CONTRACT_CLASS,
                               (notify)(signup)(userdelete) /*(beterase)*/ (rewardpull)(eventclose)(eventcreate)(eventpay)(rewardclaim)(eventrefund)(eventmigrate));
      }
   }
}

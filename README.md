# Betnow Beyond Blockchain
File and instruction for EOSIO Hackathon: Beyond Blockchain

## Folder content
* Betnow contract: .abi and .wasm file compiled for betnow account
* Betcontest contract: .abi and .wasm file compiled for betcontest account
* Website: code for the website

## Betnow contract

betstable: contains all the information related to a sigle bet

    uint128_t superkey*             ->  concatenated (account name + event + forecast)
    name account                    ->  better's account name
    asset amount                    ->  amount without bonus
    asset amountW                   ->  amount with bonus
    uint64_t event                  ->  eventID
    uint64_t forecast               ->  bet's forecast

    index:  betsTable_index

    byforecast
    byevent
    byuser
    byidforecast

eventstable: contains all the information related to a sigle event

    uint64_t event*                 ->  eventID
    time_point betStartTime         ->  event's starting time at GTM
    time_point betEndTime           ->  event's starting time at GTM
    std::string forecasts           ->  eventID possible forecasts separated by ":" (1:2:..:n)
    std::string eventDescription    ->  event description "L.A. Lakers - Miami Heats"
    uint64_t contestKey             ->  contestKey for betcontest contract
    bool close                      ->  if true event is closed
    uint64_t winnerF                ->  winnfer forecast, by default it is set to 9999
    float reward                    ->  rewards per EOS, by default it is set to -9999
    float bonus                     ->  max bonus amount (at time=betStartTime) expressed in %
    float exitFee                   ->  min exit fee (at time=betStartTime) expressed in %
    uint64_t betCounter             ->  counts all bets fot this event
    name betLeader                  ->  user with the highest bet
    asset maxBet                    ->  maxBet placed by betLeader

    index   eventsTable_index

betsumtable: contains the sum of all bets for a specific forecast

    uint64_t betkey*                ->  concatenated (event + forcast)
    asset balanceW                  ->  sum of balances with bonus
    asset balanceL                  ->  sum of balances without bonus
    std::string result              ->  description of result "L.A. Lakers"


    index:  betSumTable_index

userstable: contains all platform users

    uint128_t superkey*             ->  concatenated (account name + contestKey)
    name user                       ->  user
    uint_64t userid                 ->  platform user registration number
    uint64_t contestKey             ->  contest key
    uint64_t betpoints = 0;         ->  user Bet Points balance

    uint128_t primary_key() const { return superkey; }
    uint64_t get_contestKey() const { return contestKey; }
    uint64_t get_user() const { return user; }
    uint64_t get_betpoints() const { return score; }


    index:  usersTable_index

    bycontestkey
    bybetpoints
    byuser

ACTIONS inside contract

- placebet(name from, name to, asset quantity, std::string memo)
  cleos push action eosio.token transfer '["USER", "betnow", "1.0000 SYS", "placebet:EVENT|FORECAST_a:FORECAST_b|AMOUNT_a:AMOUNT_b"]' -p USER@active
  cleos push action eosio.token transfer '["bob", "betnow", "10.0000 SYS", "placebet:1|1:2|7.0000:3.0000"]' -p bob@active

Checks:

    -   Checks that symbol is global asset and that is an incoming TX
    -   If it is a donation or amount is greater than minimumTrasaction sends greetings message and exits;
    -   Checks that memo's format is correct, if not sends refund and error message;
    -   Checks if User is registred, if not sends refund and error message;


- beterase(name account, uint64_t event, uint64_t forecast)
  cleos push action betnow beterase '["USER", "event", "forecast"]' -p USER@active
  cleos push action betnow beterase '["bob", "1", "1"]' -p bob@active
- eventclose(uint64_t event, bool close)
  cleos push action betnow eventclose '["EVENT","TRUE"]' -p betnow@active
  cleos push action betnow eventclose '["1",true]' -p betnow@active
- eventcreate (uint64_t event, std::string eventCategory, std::string eventDescription, uint64_t contestKey, std::string contestDescription, std::string forecasts, std::string results, uint64_t betStart, uint64_t betEnd, float bonus, float exitFee, float winningFee)

  cleos push action betnow eventcreate '[0,"test event",1, "NBA", "0:1:2:3", "home:away:tie:none",1555783226972836,1555783346972836,.2,.5, 0.03]' -p betnow@active

- rewardpull(uint64_t event, uint64_t winner, float fee, uint64_t userspertxs, bool sequence)
  cleos push action betnow rewardpull '[EVENT,WINNERFORECAST,FEE,USERSPERTXS,SEQUENCE]' -p betnow@active
  cleos push action betnow rewardpull '[1,2,0.01,10,true]' -p betnow@active

- eventpay(uint64_t event, uint64_t usersToBePaid, bool sequence)
  cleos push action betnow eventpay '[EVENT,USERSTOBEPAID,SEQUENCE]' -p betnow@active
  cleos push action betnow eventpay '[1,10,true]' -p betnow@active

- rewardclaim(name user, uint64_t event)
  cleos push action betnow rewardclaim '[EVENT,USERSTOBEPAID]' -p betnow@active
  cleos push action betnow rewardclaim '["bob",1]' -p bob@active

- eventrefund(uint64_t event, uint64_t usersToBeRefunded)
  cleos push action betnow eventrefund '[EVENT,USERSTOBEREFUNDED]' -p betnow@active
  cleos push action betnow eventrefund '[1,10]' -p betnow@active

- eventmigrate(uint64_t fromEvent, uint64_t toEvent, uint64_t usersToMigrate, float extraBonus, bool sequence)
  cleos push action betnow eventmigrate '[FROMEVENT,TOEVENT,USERSTOMIGRATE,BONUS,SEQUENCE]' -p betnow@active
  cleos push action betnow eventmigrate '[1,10,10,0.2,true]' -p betnow@active
- signup(name account)
  cleos push action betnow signup '["USER"]' -p USER@active
  cleos push action betnow signup '["bob"]' -p bob@active

- userdelete(name account)
  cleos push action betnow userdelete '["USER"]' -p USER@active
  cleos push action betnow userdelete '["bob"]' -p bob@active

* notify(name account, string status, std::string msg)
  status = ERROR: || INFO || ""


## Betcontest contract

betstable: contains all the information related to a sigle bet 

    uint128_t superkey*             ->  concatenated (account name + event + forecast)
    name account                    ->  better's account name
    asset amount                    ->  amount without bonus
    asset amountW                   ->  amount with bonus
    uint64_t event                  ->  eventID
    uint64_t forecast               ->  bet's forecast

    index:  betsTable_index 

    byforecast
    byevent
    byuser
    byidforecast

eventstable: contains all the information related to a sigle event
   
    uint64_t event*                 ->  eventID
    time_point betStartTime         ->  event's starting time at GTM
    time_point betEndTime           ->  event's starting time at GTM
    std::string forecasts           ->  eventID possible forecasts separated by ":" (1:2:..:n)
    std::string eventDescription    ->  event description "L.A. Lakers - Miami Heats"
    uint64_t contestKey             ->  contestKey for betcontest contract
    bool close                      ->  if true event is closed
    uint64_t winnerF                ->  winnfer forecast, by default it is set to 9999 
    float reward                    ->  rewards per EOS, by default it is set to -9999 
    float bonus                     ->  max bonus amount (at time=betStartTime) expressed in %
    float exitFee                   ->  min exit fee (at time=betStartTime) expressed in %
    uint64_t betCounter             ->  counts all bets fot this event
    name betLeader                  ->  user with the highest bet
    asset maxBet                    ->  maxBet placed by betLeader

    index   eventsTable_index

betsumtable: contains the sum of all bets for a specific forecast

    uint64_t betkey*                ->  concatenated (event + forcast)
    asset balanceW                  ->  sum of balances with bonus
    asset balanceL                  ->  sum of balances without bonus
    std::string result              ->  description of result "L.A. Lakers" 


    index:  betSumTable_index;

userstable: contains all platform users

    name user                       ->  user
    uint_64t userid                 ->  platform user registration number
    index:  usersTable_index;



ACTIONS inside contract


- userupdate(name account, uint64_t contestKey, uint64_t event, uint64_t amount, bool winner)
    cleos push action betcontest userupdate '["USER", "CONTESTKY", "EVENT", "AMOUNT, "WINNER]' -p betnow@active
    cleos push action betcontest userupdate '["bob", 1, 1, 10000, true]' -p betnow@active

AUTH: "betnow"

Calculates the amount of points to be added for the user and updates rankingTable. If user pont balance is < 0, writes 0
        
- beterase(name account, uint64_t event, uint64_t forecast)
    cleos push action betnow beterase '["USER", "event", "forecast"]' -p USER@active
    cleos push action betnow beterase '["bob", "1", "1"]' -p bob@active
    

- notify(name account, string status, std::string msg)
    status = ERROR: || INFO || ""


## Website
File for deploy betnow website connected to jungle 3
If you login with Betnow account you could use admin page to manage all the events



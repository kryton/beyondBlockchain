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




FUNCTIONS 


- bool checkSymbol(asset quantity) 

returns false if SYMBOL is different from global ASSET

- bool checkIncomingTX(name to) 

Returns false if TX is not incoming (all outbound TXs)

- std::vector<std::string> splitString(std::string text, char delimiter)

Splits text string by char delimiter and returns vector


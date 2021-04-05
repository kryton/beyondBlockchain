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

    
FUNCTIONS inside contract

- uint64_t getEventTot(uint64_t event)

Returns the total amount bet on a specific event (summing up all the declared contest in eventsTable)

<h1 class="contract">notify</h1>

spec-version: 0.0.2
title: Notify
summary: This action will send a notification to the user account when another action succeeds or fails. This action is intended to be called automatically from our contracts only.
icon:

<h1 class="contract">updbetpoints</h1>

spec-version: 0.0.2
title: Update Bet Points
summary: This action will update the user Betpoints balance on the usersTable. This action is intended to be called automatically from our contracts only.
icon:

<h1 class="contract">signup</h1>

spec-version: 0.0.2
title: Sign Up
summary: This action will insert a new entry in the userstable allowing the user to interact with our platform. If your calling this action from CLI or manually, keep in mind that this action will return an ERROR notification if you are already on our usersTable. If you are calling this action from our platform then you should be elegible to signup and you will receive a welcome message. The data is stored in the multi index table. By signing this transaction the user consents to our
<a
href="https://www.betnow.io/terms-of-service" >
Terms of Service
</a> .The ram costs are paid by the smart contract.
icon:

<h1 class="contract">userdelete</h1>

spec-version: 0.0.2
title: Delete User
summary: This action will delete any user entry in the userstable. This action will prevent the user to further interact with our platform (untill a new signup action is called). If the user has open bets this action will return an ERROR notification otherwise a goodbye message. In order to this action to succeed make sure to not have any user record in the betsTable. By signing this transaction you will lose all your Betpoints.
icon:

<h1 class="contract">deletetables</h1>

spec-version: 0.0.2
title: Bet Info
summary: This action will delete all tables. Only for dev.
icon:

<h1 class="contract">beterase</h1>

spec-version: 0.0.2
title: Erase Bet
summary: This action will delete the requested user's bet from the betsTable. This action will return an ERROR notification if the event is already closed or not found in the eventTable. This action can be called only by the user. The user will receive a partial refund based on the current exitFee. The rest of the amount will stay part of the prize. To learn more about our platform, visit our
<a
href="https://www.betnow.io/FAQ" >
FAQ.
</a>
icon:

<h1 class="contract">rewardpull</h1>

spec-version: 0.0.2
title: Pull Reward
summary: This action will calculate the event's reward and replace the default value on the eventsTable allowing the users to claim their rewards. This action can be called from Betnow.io only.
icon:

<h1 class="contract">eventclose</h1>

spec-version: 0.0.2
title: Close Event
summary: This action will modify the event's close status on the eventsTable allowing or preventing users to place bets. This action can be called from Betnow.io only.
icon:

<h1 class="contract">eventcreate</h1>

spec-version: 0.0.2
title: Create Event
summary: This action will create a new event record on the eventsTable. This action can be called from Betnow.io only.
icon:

<h1 class="contract">eventpay</h1>

spec-version: 0.0.2
title: Pay Event
summary: This action will send to the winning users their rewads and delete the user's records from the event's betsTable. This action can be either called from Betnow.io or from one of our contracts.
icon:

<h1 class="contract">rewardclaim</h1>

spec-version: 0.0.2
title: Claim Reward
summary: This action will iterate the user's bets on a specific event and send the related prize (if any). This action will also delete the user's records from the event's betsTable. If your calling this action from CLI or manually, keep in mind that this action will return an ERROR notification if the user is not elegible to claim the reward. If you are calling this action from our platform then you should be elegible. This action can be called only by the user. To learn more, visit our
<a
href="https://www.betnow.io/FAQ" >
FAQ.
</a>
icon:

<h1 class="contract">eventrefund</h1>

spec-version: 0.0.2
title: Refund Event
summary: This action will refund all users for a specif event. This action can be either called from Betnow.io or from one of our contracts in case of need (e.g. an event gets cancelled).
icon:

<h1 class="contract">eventmigrate</h1>

spec-version: 0.0.2
title: Migrate Event
summary: This action will migrate all users for a specif event to a new event. This action can be either called from Betnow.io or from one of our contracts in case of need (e.g. an event gets postponed).
icon:

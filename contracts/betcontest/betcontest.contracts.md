<h1 class="contract">notify</h1>
---
spec-version: 0.0.2
title: Notify
summary: This action will send a notification to the user account when another action succeeds or fails. This action is intended to be called automatically from our contracts only.
icon: 

<h1 class="contract">userdelete</h1>
---
spec-version: 0.0.2
title: Delete User
summary: This action will delete any user entry from this contract. This action is intended to only be called automatically from our main contract after the user's has perfomed the homonymous action on it.

<h1 class="contract">deletetables</h1>
---
spec-version: 0.0.2
title: Bet Info
summary: This action will delete all tables. Only for dev.
icon: 

<h1 class="contract">betleaderpts</h1>
---
spec-version: 0.0.2
title: Bet Leader Points
summary: This action will add the extra Betpoints to the bet leader balance for a specif contest. This action is intended to be called automatically from our contracts only.
To learn more about our platform, visit our 
<a
href="https://www.betnow.io/FAQ" >
FAQ.
</a>
icon: 

<h1 class="contract">contestpay</h1>
---
spec-version: 0.0.2
title: Pay Contest
summary: This action will pay the prizes to the top users for a specific contest. This action can be called from Betnow.io only.
In order to receive the reward you must be in the top 10, to learn more on contests, visit our 
<a
href="https://www.betnow.io/FAQ" >
FAQ.
</a>
icon: 

<h1 class="contract">contestadd</h1>
---
spec-version: 0.0.2
title: Add Contest
summary: This action will create a new contest record on the contestsTable. This action can be called from Betnow.io only.
icon: 

<h1 class="contract">contestclose</h1>
---
spec-version: 0.0.2
title: Close Contest
summary: This action will modify the contest's close status on the contestsTable. This action is intended to be called from Betnow.io only to close the contest and proceed with the contestpay action.
icon: 

<h1 class="contract">updateblnce</h1>
---
spec-version: 0.0.2
title: Update Balance
summary: This action will increase the contest balance by the amount sent by the main contract. This action is intended to be called from Betnow.io only.
icon: 

<h1 class="contract">userupdate</h1>
---
spec-version: 0.0.2
title: Update User
summary: This action will update the user's Betpoints on the rankingTable. This action is intended to be called automatically from our contracts only.
To learn more about our Betpoints, visit our 
<a
href="https://www.betnow.io/FAQ" >
FAQ.
</a>
icon: 

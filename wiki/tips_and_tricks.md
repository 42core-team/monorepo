---
title: "💡 Tips & Tricks"
permalink: "tips_and_tricks"
sidebarTitle: "💡 Tips & Tricks"
---

# 🏆 The holy rules of Core Game victory

1. **Play online as much as possible.** Fight against others often because losing means learning.

2. **Knowledge is power.** Nobody has ever won without learning the basics before rushing into battle.

3. **Keep it simple.** A simple, well-rounded bot will crush an overcomplicated mess. Build, test, and refine instead of overplanning and overscoping at the start.

4. **Eliminate avoidable action failures.** It will pay off in the long run.

5. **Use the `void *data` field.** You can save any data you want on every object, which lets you build more detailed strategies and coordinate your troops with a custom [state machine](https://www.reddit.com/r/learnprogramming/comments/1g5yxci/state_machines_for_a_beginner/). The library won't touch it. Just remember to free everything at the end. 💪

6. **Transfer gems between units.** Remember that [`core_action_transferGems`](reference/c/actions/core_action_transferGems) also works between units, as well as a way of dropping gems with no other unit around.

7. **Use the debug tools.** Add information to unit tooltips with the [debug functions](reference/c/debug/core_debug_addObjectInfo). Showing each unit's current job makes strategy bugs much easier to see in a replay.

8. **Learn getter filtering.** Read the [getter filtering guide](documentation/getter_filtering), this will be very useful to get exactly the objects you want.

9. **Have fun!** Don't take things too seriously, and be nice to each other. Embrace the good vibes of friendly competition. ⚡️

10. **Join the Discord!** - You can ask questions there. Even if you don't have a question right now, checking out what others are up to can be helpful intel.

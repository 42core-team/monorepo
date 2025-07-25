# Debug Visualizer

Three parts of the ts code:

1. time manager
    - Playback options. Play, Play Reverse, Pause, Next Action, Prev. Action, Next Tick, Prev. Tick, Skip to start, Skip to end, speed up, speed down.
    - Returns tick info whenever displayer wants it.
    - If possible, options to switch between action-focused playback (each action takes same amount of time) and tick-focused playback (each tick takes same amount of time, however many actions happen in a given tick)
2. replay loader
    - Replay loader abstracts away the state json diff logic, and returns the full state json at any tick required.
3. displayer
    - Displayer calls on the time manager at any point it can render a frame to get the current tick info. To achieve this, all animations are mathematical functions somehow displacing their position or rotation, e.g. sine applied to units position for a dagger stab.

### Tick info:

Defines with the context of the replay loader exactly what should currently be rendered, up to the position of an object while in-animation

```
interface tickData {
  gameTick: number; // int e.g. 42
  animTick: number; // float, e.g. 42.394 -> 42 defines action, 0.394 defines animation progress
}
```

-   How does the replay diff logic work? -> https://github.com/42core-team/even_COREnier/blob/main/src/game/ReplayEncoder.cpp
-   How does the state format look like? -> https://github.com/42core-team/even_COREnier/blob/main/src/game/StateEncoder.cpp

## Rendered Actions:

-   move
-   attack
-   build
-   transfer money
-   create
-   rotate (e.g. for a unit to look in another direction. must also be tracked)
-   die / get destroyed

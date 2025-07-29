# Debug Visualizer

-   `npm run serve` -> Run visualizer under `localhost:8080`
-   `npm run dev` -> Run dev mode, no need to manually recompile the ts
-   `npm run build` -> Build ts files into servable js files

## Docker

This project includes Docker support for containerized deployment.

### Local Development with Docker

```bash
# Build the Docker image
docker build -t debug-visualizer .

# Run the container
docker run -p 8080:80 debug-visualizer

# Access the application at http://localhost:8080
```

### Multi-Architecture Support

The Docker image is built for both AMD64 and ARM64 architectures:

- **AMD64**: For x86_64 systems (most desktop/laptop computers)
- **ARM64**: For ARM-based systems (Apple Silicon Macs, ARM servers)

### CI/CD Pipeline

The GitHub Actions workflow automatically:

- **Builds and pushes images** for `main` and `dev` branches
- **Tests builds** for pull requests (no push)
- **Tags images** with:
  - Branch name (e.g., `main`, `dev`)
  - Short SHA (e.g., `main-abc1234`)
  - `latest` tag for the default branch

#### Image Registry

Images are pushed to GitHub Container Registry (ghcr.io) with the format:
```
ghcr.io/{username}/{repository}:{tag}
```

#### Matrix Build Strategy

The workflow uses matrix builds to build for both architectures:
- AMD64 builds run on `ubuntu-latest` runners
- ARM64 builds run on `ubuntu-latest-arm64` runners

This ensures optimal performance and compatibility for each architecture.

## Dev Notes

Three parts of the ts code:

1. time manager
    - Playback options. Play, Play Reverse, Pause, Next Action, Prev. Action, Next Tick, Prev. Tick, Skip to start, Skip to end, speed up, speed down.
    - Returns tick info whenever displayer wants it.
    - If possible, options to switch between action-focused playback (each action takes same amount of time, all actions are animated in sequence) and tick-focused playback (each tick takes same amount of time, all actions are animated at the same time)
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
  tickfocused: boolean; // tick-focused or action-focused playback
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

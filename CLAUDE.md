# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Development Commands

- `npm run dev` - Start development mode with TypeScript watch compilation and local server
- `npm run serve` - Build TypeScript and serve on localhost:8080 
- `npm run build` - Compile TypeScript files to JavaScript

## Code Architecture

This is a game replay visualizer that renders animated playback of game states. The codebase follows a modular TypeScript architecture with three core components:

### Core Architecture

1. **Time Manager** (`src/ts/time_manager/timeManager.ts`)
   - Controls playback state (play/pause/speed/navigation)
   - Provides tick-based timing with animation progress (tickProgress 0-1)
   - Handles keyboard shortcuts (spacebar=play/pause, arrows=navigation, r/s=start, e=end)
   - Returns `tickData` containing current tick and animation progress

2. **Replay Loader** (`src/ts/replay_loader/replayLoader.ts`)
   - Loads and processes JSON replay files with differential state encoding
   - Implements caching system for performance (default 25-tick intervals)
   - Handles live reload detection via ETag headers
   - Supports drag-and-drop file loading
   - Provides `getStateAt(tick)` for full game state reconstruction

3. **Renderer** (`src/ts/renderer/renderer.ts`)
   - SVG-based grid rendering with smooth animations
   - Interpolates object positions between ticks using sine wave progression
   - Handles object scaling for spawn/death animations
   - Interactive tooltips showing object details
   - Asset management for different unit types and teams

### Key Data Flow

- Main.ts orchestrates module initialization: ReplayLoader → TimeManager → Renderer
- Renderer calls `getCurrentTickData()` each frame to get current timing
- Uses timing data to fetch game state and interpolate animations
- All animations are mathematical functions (sine waves) for smooth transitions

### File Structure

- `src/ts/` - TypeScript source files
- `src/public/js/` - Compiled JavaScript output
- `src/public/assets/` - SVG assets for game objects organized by type and team
- `src/public/misc/replay_latest.json` - Default replay file location

### Configuration

- TypeScript compiles from `src/ts/` to `src/public/js/`
- Uses ESNext modules with strict mode enabled
- Biome for linting, Prettier for formatting (devDependencies)
- Supports query parameter `?replay=path` to load custom replay files

### Docker Support

- Multi-architecture builds (AMD64/ARM64) 
- GitHub Actions CI/CD with container registry publishing
- Nginx-based production deployment
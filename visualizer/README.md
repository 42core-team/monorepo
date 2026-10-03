[![visualizer.coregame.sh](https://status.coregame.sh/badge/web/status?labelColor=&color=&style=for-the-badge&label=visualizer.coregame.sh)](https://visualizer.coregame.sh)

# Debug Visualizer

-   `npm run preview` -> Run compiled visualizer
-   `npm run dev` -> Run dev mode, no need to manually recompile the ts
-   `npm run build` -> Build ts files into servable js files

## URL Parameters

You can control the visualizer via query parameters. Multiple values are supported where noted (use `,` to separate). Examples assume the app origin is `http://localhost:5173`.

| Param | Type / Values | Default | Effect |
|---|---|---:|---|
| `replays` | string array (path/URL) | `/replays/replay_latest.json` | List of replay files to load. |
| `live` | WebSocket URL | none | Streams the replay as it is played. An empty value uses `ws://<visualizer-host>:4445` (or `wss://` on HTTPS). This takes precedence over `replays`. |
| `speed` | number (`0.5` … `50`, step `0.5`) | none | Sets initial ticks/sec (also persisted to `tm.speed`). |
| `dynamicSpeed` | `"on"` or `"off"` | `"off"` | Sets initial ticks/sec so the replay takes roughly 30s-60s to wrap up. |
| `autoplay` | `"off"`, `"start"` or `"full"` | `"off"` | `"start"` automatically starts playback when a replay loaded, `"full"` additionally moves on to next replay file after one finished, `"off"` does neither. |
| `ui` | `"false"` or `"true"` | _shown_ | When `false`, hides all elements with class `.ui`. |
| `theme` | `"light"` or `"dark"` | system/light | Sets theme, persisted to `localStorage["ui.theme"]`. |
| `gridlines` | `"on"` \| `"off"` | `"on"` | Toggles grid rendering; persisted to `localStorage["ui.gridlines"]`. |
| `themeColor` | hex `RRGGBB` | `5a7cff` | Sets accent color; persisted to `localStorage["ui.themeColor"]` Don't include the # hashtag. |
| `bgColor` | hex `RRGGBB` | derived from theme | Overrides the app background color. Don't include the # hashtag. Useful for embedding in iframes to match the parent page background. |
| `suppress_version_warning` | `"true"` \| `"false"` | `"false"` | Stores preference in `localStorage["ui.suppressVersionWarning"]`. Hook into this in any version-check logic you add. |
| `mode` | `"QUEUE"` \| `"SWISS"` \| `"ELIMINATION"` | `"QUEUE"` | Fireworks intensity profile (read by `fireworksRenderer`). Used by website when running tournament games. |
| `round` | integer | none | Used with `mode=ELIMINATION` for fireworks strength. Used by website when running tournament games. |
| `maxRounds` | integer | none | Used with `mode=ELIMINATION` for fireworks strength. Used by website when running tournament games. |

#### Examples

- Single replay at higher speed, dark, no gridlines:

[http://localhost:4242/?replay=/replays/r1.json\&speed=12\&theme=dark\&gridlines=off](http://localhost:4242/?replay=/replays/r1.json&speed=12&theme=dark&gridlines=off)

- Autoplay three replays, custom theme color:

[http://localhost:4242/?autoplay=/a.json,/b.json,/c.json\&themeColor=%235A7CFF](http://localhost:4242/?autoplay=/a.json,/b.json,/c.json&themeColor=%235A7CFF)

Note: URL-encode `#` as `%23` in links.

## Live replays

Set the optional `liveReplayPort` in the server config, for example `4445`, then open the visualizer with `?live=ws://localhost:4445`. The server sends an initial snapshot containing every tick already played and streams subsequent replay-format tick diffs. The saved `replay_latest.json` format and normal file playback are unchanged.
- Headless wall display (no UI), suppress version warning:

[http://localhost:4242/?ui=false\&suppress\_version\_warning=true](http://localhost:4242/?ui=false&suppress_version_warning=true)

- Tournament finals fireworks:

[http://localhost:4242/?mode=ELIMINATION\&round=5\&maxRounds=5](http://localhost:4242/?mode=ELIMINATION&round=5&maxRounds=5)


## Docker

This project includes Docker support for containerized deployment.

### Local Development with Docker

```bash
# Build the Docker image
./scripts/prepare-version-context.sh
docker build -t debug-visualizer .

# Run the container
docker run -p 8080:80 --rm debug-visualizer

# Access the application at http://localhost:8080
```

Pass `--build-arg INCLUDE_DEV_ALIAS=true` to expose the current checkout at
`/dev/` as well as `/`. Stable release tags are grouped by their first three
version components and the newest fourth-component revision is embedded at a
route such as `/v0.1.2/`.

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

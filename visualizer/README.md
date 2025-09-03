[![visualizer.coregame.de](https://status.coregame.de/badge/web/status?labelColor=&color=&style=for-the-badge&label=visualizer.coregame.de)](https://visualizer.coregame.de)

# Debug Visualizer

-   `npm run preview` -> Run compiled visualizer
-   `npm run dev` -> Run dev mode, no need to manually recompile the ts
-   `npm run build` -> Build ts files into servable js files

## URL Parameters

You can control the visualizer via query parameters. Multiple values are supported where noted (use `,` to separate). Examples assume the app origin is `http://localhost:5173`.

| Param | Type / Values | Default | Effect |
|---|---|---:|---|
| `replay` | string (path/URL) | `/replays/replay_latest.json` | Replay file to load. |
| `speed` | number (`0.5` … `50`, step `0.5`) | none | Sets initial ticks/sec (also persisted to `tm.speed`). |
| `autoplay` | list of strings | none | Comma-separated list of replay paths to auto-play sequentially. |
| `ui` | `"false"` or `"true"` | _shown_ | When `false`, hides all elements with class `.ui`. |
| `theme` | `"light"` or `"dark"` | system/light | Sets theme, persisted to `localStorage["ui.theme"]`. |
| `gridlines` | `"on"` \| `"off"` | `"on"` | Toggles grid rendering; persisted to `localStorage["ui.gridlines"]`. |
| `themeColor` | hex `RRGGBB` | `5a7cff` | Sets accent color; persisted to `localStorage["ui.themeColor"]` Don't include the # hashtag. |
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
- Headless wall display (no UI), suppress version warning:

[http://localhost:4242/?ui=false\&suppress\_version\_warning=true](http://localhost:4242/?ui=false&suppress_version_warning=true)

- Tournament finals fireworks:

[http://localhost:4242/?mode=ELIMINATION\&round=5\&maxRounds=5](http://localhost:4242/?mode=ELIMINATION&round=5&maxRounds=5)


## Docker

This project includes Docker support for containerized deployment.

### Local Development with Docker

```bash
# Build the Docker image
docker build -t debug-visualizer .

# Run the container
docker run -p 8080:80 --rm debug-visualizer

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

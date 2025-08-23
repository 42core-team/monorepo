# Debug Visualizer

-   `npm run preview` -> Run compiled visualizer
-   `npm run dev` -> Run dev mode, no need to manually recompile the ts
-   `npm run build` -> Build ts files into servable js files

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

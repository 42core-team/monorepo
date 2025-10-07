This page is focused around making sure your bot can properly run on the website. Otherwise, it won't be able to compete in the tournament or win.

> Generally, you don't need to worry about going through this page, just test that your bot runs on the website once and you'll be fine. As long as you keep the folder name `my-core-bot` and not modify the `my-core-bot/Makefile`, your bot will run fine on the website. If your bot doesn't run on the website though, this is the page for you.

# What the Website Expects

- **Repository layout**: Your bot source must live at `my-core-bot/` at the repo root.
- **Build command**: The website runner executes `make` inside `my-core-bot/`.
- **Executable name**: The produced binary must be named `bot` (no extension).
- **Runtime command**: Your bot is started as `./bot <rndId>` by the job.

```text
/ (repo root)
└─ my-core-bot/
   ├─ Makefile  (must build an executable named "bot")
   ├─ src/
   └─ ...
```

# Do NOT modify these

- **Makefile interface**: Keep `make` building the default target to `bot`. Do not rename the binary or require custom targets like `make run`.
- **Working directory assumption**: The job will `cd /shared-data/repo/my-core-bot && make && ./bot <id>`.
  - Changing folder names, nesting deeper, or relocating sources will break website runs.

# Environment & Security Constraints

- **No outbound network**: Egress for the bot UID is blocked to anything but loopback. Assume no internet.
- **Non-root user**: The bot runs as an unprivileged user (UID 2000), non-root enforced.
- **Read-only root FS**: Container root filesystem is read-only. Your repo is mounted writable at `/shared-data/repo`.
- **Resource limits**: Typical per-bot limits around CPU `1`, memory `512Mi`, and a volume size limit of `250Mi` at `/shared-data`. Code should be efficient and avoid large transient allocations.

# Logging & Diagnostics

- **Stdout/stderr**: Use standard output for logs. They can be viewed on the website.

# Common Pitfalls (and fixes)

- **Binary not named `bot`**: Ensure your `Makefile` links to an output named `bot` in `my-core-bot/`.
- **Wrong paths**: Includes or assets should be relative to your repo and not depend on absolute local paths.
- **Writes to root**: Write temp files inside your repo (e.g., `./tmp/`) not to `/tmp` if your code assumes permissions; prefer within `/shared-data/repo/`.

# Local Parity Tips

- Test by running exactly what the website runs:

```bash
cd my-core-bot && make && ./bot 42
```
> The result should be your bot running trying to connect

# FAQ

- "Can I change the folder name or move sources?"
  - No. Keep `my-core-bot/` and produce `./bot` after `make`.
- "Can I use the network for fetching data?"
  - No. Put data into your repo if needed and keep sizes small.
- "Can I create files at runtime?"
  - Yes, inside the repo mount. Avoid writing outside `/shared-data/repo`.

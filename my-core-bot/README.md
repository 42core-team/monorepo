<p align="center">
  <img src="https://avatars.githubusercontent.com/u/147154505?s=200&v=4" alt="CORE Logo" width="150">
</p>

# 🌟 CORE REPO

## 🎉 Good Luck, Have Fun, and RTFM!1!!1 🚀

Welcome to the **CORE** project repository! We’re excited to have you on board for this coding adventure.

### 🚀 Quick Start Guide

1. Clone the repository and set up your dev container:
   ```bash
   git clone your-repo-url && cd my-core-bot && make devcontainer
   ```
2. Run `make` in the terminal to test.
3. Open [localhost](http://localhost) in your browser to see the gameplay.
4. Keep going writing your bot! Get started under `my-core-bot/src/main.c`!

### 📚 Useful Links
- **Official CORE Wiki**: [coregame.de/wiki](https://coregame.de/wiki)

### 🛠️ Spin Up Your Dev Container

Want to get hacking right away? Set up your dev environment in one simple command using [Devpod](https://devpod.sh/)! 🚀

```bash
make devcontainer
```

This command will:
1. Automatically download and install the **Devpod CLI** (if it’s not already there).
2. Ensure **Docker** is up and running (it will attempt to start Docker on 42 iMacs if it’s not started).
3. Set up the **Docker provider** for Devpod.
4. Launch your preferred IDE inside a fully configured **Dev Container**

> 💡 **Tip**: You can specify your favorite IDE by passing the `IDE` variable. For example:
> ```bash
> make devcontainer IDE=zed
> ```

📋 **Default IDE**: `vscode`
🧰 **Supported IDEs**: `vscode`, `openvscode`, `cursor`, `zed`, `codium`, `intellij`, `pycharm`, `phpstorm`,
`rider`, `fleet`, `goland`, `webstorm`, `rustrover`, `rubymine`, `clion`, `dataspell`, `jupyternotebook`,
`vscode-insiders`, `positron`, `rstudio`

#### 🛑 Stop the Dev Container
To stop the running Dev Container, use:
```bash
make stop-devcontainer
```
This will stop the container without removing it, allowing you to restart it later.

#### ❌ Remove the Dev Container
To completely remove the Dev Container, use:
```bash
make remove-devcontainer
```
This will delete the container and its associated resources.

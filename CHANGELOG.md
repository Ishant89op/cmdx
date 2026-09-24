# Changelog

## 1.1.0 (2026-09-24)

### Added
- Implemented command cache feature: commands that execute successfully now save their configuration to cache, which is automatically restored on next launch.
- Added `--clean` (`-c`) or `--no-cache` flag to start without loading cached config.
- Added Edit menu with options to reset to defaults and clear cache in GUI mode.
- Tracking checklist `commands/COMMANDS_CHECKLIST.md` added for AI agents.


## 1.0.2 (2026-09-06)

### Changed

- Simplified `--help` (`cmdx -h`) and `about` output with a concise definition of the tool's purpose ("Cmdx - Visual command builder and runner").
- Removed multiplatform claims and total command count references from the help message.

## 1.0.1 (2026-08-29)

### Fixed

- Source installation on Kali and other supported package-manager platforms now installs the required Qt6 and nlohmann-json development dependencies before configuring CMake.
- The installer now recognizes Kali and Parrot as Debian-family distributions for source builds.

### Added

- Expanded the offline command database to 147 complete command definitions.
- Added `commands/COMMANDS.txt` as the names-only command inventory.
- Added `AI_HANDOFF.md` to document command-definition research and continuation progress.

## 0.0.0 (2026-05-06)

Initial alpha release for installer and packaging validation.

### Features

- GUI command builder with Qt6 - configure flags visually and run
- Interactive terminal mode for headless environments
- Offline JSON database of 127 commands across Linux, macOS, and Windows
- Sudo checkbox in the GUI
- Subcommand support with per-subcommand options (e.g. git, docker, systemctl)
- Option dependencies, mutex groups, and required field validation
- Collapsible option groups with accordion UI
- File and directory picker widgets
- Command preview with live updates
- Execute constructed commands directly in the current terminal

### CLI

- `cmdx <command>` opens the GUI (default mode)
- `cmdx -t <command>` for interactive terminal mode
- `cmdx --list` / `-l` to list all available commands

### Packaging

- Native installers: `.deb`, `.rpm`, `.pkg`, `.exe`
- Docker image with multi-stage build
- Universal install script (`install.sh`)
- GitHub Actions CI for automated cross-platform builds and releases

### Command Database (127 tools)

- Linux core: cat, chmod, cp, df, du, find, grep, kill, ls, mkdir, mv, ps, rm, sed, tar, etc.
- Linux network: curl, nc, nmap, ping, rsync, scp, ssh, ss, wget, etc.
- Linux security: aircrack-ng, ffuf, gobuster, hashcat, hydra, john, nikto, sqlmap, etc.
- Linux package managers: apt, dnf, pacman, snap, flatpak
- macOS: brew, diskutil, defaults, launchctl, etc.
- Windows cmd: ipconfig, netstat, robocopy, tasklist, etc.
- Windows PowerShell: Get-Process, Get-Service, etc.
- Cross-platform: git, docker, kubectl, terraform, cargo, node, python, etc.

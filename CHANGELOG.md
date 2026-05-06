# Changelog

## 1.0.0 (2026-05-04)

Initial release.

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

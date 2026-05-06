# cmdx

Cmdx is an offline, native GUI for system commands. Browse commands, configure flags visually, and execute.

Uniqueness: Offline database of commands.

## Install

### One-liner (auto-detects your OS)

```bash
curl -fsSL https://raw.githubusercontent.com/Ishant89op/cmdx/main/install.sh | bash
```

### Docker

```bash
# Terminal mode
docker run --rm -it ghcr.io/ishant89op/cmdx -t nmap

# GUI mode (Linux with X11)
docker run --rm -it -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix ghcr.io/ishant89op/cmdx nmap

# List all commands
docker run --rm ghcr.io/ishant89op/cmdx --list
```

### Native Packages

| Platform | Install |
|---|---|
| Debian / Ubuntu | `sudo dpkg -i cmdx_1.0.0_amd64.deb` |
| Fedora / RHEL | `sudo rpm -i cmdx-1.0.0.x86_64.rpm` |
| Arch | `cmake -B build && cmake --build build && sudo cmake --install build` |
| macOS | Run the `.pkg` installer from Releases |
| Windows | Run the Windows installer `.exe` from Releases |

### From Source

```bash
cmake -B build -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build -j$(nproc)
sudo cmake --install build
```

## Usage

```bash
cmdx <command>                # GUI builder (default)
cmdx -g <command>             # GUI builder (explicit)
cmdx -t <command>             # Terminal mode
cmdx --list                   # List all commands
cmdx -l                       # List all commands (short)
cmdx                          # About
```

### Examples

```bash
cmdx nmap          # Build an nmap command visually
cmdx -t docker     # Build a docker command in the terminal
cmdx --list        # Show all 127+ supported commands
```

On Windows, prefer the installer `.exe` from Releases. The raw `cmdx.exe` binary is a build artifact and does not include the bundled Qt/MSVC runtime files needed for end-user installs.

## Building Packages

```bash
cd build
cpack -G DEB          # .deb
cpack -G RPM          # .rpm
cpack -G TGZ          # .tar.gz
cpack -G productbuild # .pkg (macOS)
cpack -G NSIS         # .exe (Windows)
```

## Adding Commands

Drop a JSON file into `commands/`. No rebuild needed.

## Requirements

- CMake 3.16+
- Qt6 Widgets
- nlohmann/json
- C++17 compiler

## License

MIT

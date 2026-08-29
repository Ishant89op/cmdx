#!/usr/bin/env bash
# cmdx installer - detects OS and installs the right package
# Usage: curl -fsSL https://raw.githubusercontent.com/Ishant89op/cmdx/main/install.sh | bash

set -e

VERSION="1.0.1"
REPO="Ishant89op/cmdx"
BASE_URL="https://github.com/$REPO/releases/download/v$VERSION"

echo ""
echo "  cmdx installer v$VERSION"
echo "  ────────────────────────"
echo ""

# Detect OS and package manager
detect_platform() {
    OS="$(uname -s)"
    ARCH="$(uname -m)"

    case "$OS" in
        Linux)
            if [ -f /etc/os-release ]; then
                . /etc/os-release
                DISTRO="$ID"
            else
                DISTRO="unknown"
            fi
            ;;
        Darwin)
            DISTRO="macos"
            ;;
        MINGW*|MSYS*|CYGWIN*)
            DISTRO="windows"
            ;;
        *)
            echo "  [x] Unsupported OS: $OS"
            exit 1
            ;;
    esac

    echo "  Detected: $DISTRO ($ARCH)"
}

install_deb() {
    local pkg="cmdx_${VERSION}_amd64.deb"
    echo "  Downloading $pkg..."
    curl -fsSL -o "/tmp/$pkg" "$BASE_URL/$pkg"
    echo "  Installing (requires sudo)..."
    sudo apt install -y "/tmp/$pkg"
    rm -f "/tmp/$pkg"
}

install_rpm() {
    local pkg="cmdx-${VERSION}.x86_64.rpm"
    echo "  Downloading $pkg..."
    curl -fsSL -o "/tmp/$pkg" "$BASE_URL/$pkg"
    echo "  Installing (requires sudo)..."
    if command -v dnf >/dev/null 2>&1; then
        sudo dnf install -y "/tmp/$pkg"
    elif command -v yum >/dev/null 2>&1; then
        sudo yum install -y "/tmp/$pkg"
    else
        sudo rpm -i "/tmp/$pkg"
    fi
    rm -f "/tmp/$pkg"
}

install_macos() {
    local pkg="Cmdx-Installer-${VERSION}-macos.pkg"
    echo "  Downloading $pkg..."
    curl -fsSL -o "/tmp/$pkg" "$BASE_URL/$pkg"
    echo "  Installing package..."
    sudo installer -pkg "/tmp/$pkg" -target /
    rm -f "/tmp/$pkg"
}

install_build_dependencies() {
    echo "  Installing build dependencies..."

    case "$DISTRO" in
        ubuntu|debian|linuxmint|pop|kali|parrot)
            if ! command -v apt-get >/dev/null 2>&1; then
                echo "  [x] apt-get not found; cannot install Qt6 and nlohmann-json."
                exit 1
            fi
            sudo apt-get update
            sudo apt-get install -y \
                build-essential cmake \
                qt6-base-dev \
                nlohmann-json3-dev
            ;;
        fedora|rhel|centos|rocky|alma)
            if command -v dnf >/dev/null 2>&1; then
                sudo dnf install -y gcc-c++ cmake make qt6-qtbase-devel json-devel
            elif command -v yum >/dev/null 2>&1; then
                sudo yum install -y gcc-c++ cmake make qt6-qtbase-devel json-devel
            else
                echo "  [x] dnf/yum not found; cannot install build dependencies."
                exit 1
            fi
            ;;
        arch|manjaro|endeavouros)
            if ! command -v pacman >/dev/null 2>&1; then
                echo "  [x] pacman not found; cannot install build dependencies."
                exit 1
            fi
            sudo pacman -Sy --needed --noconfirm base-devel cmake qt6-base nlohmann-json
            ;;
        macos)
            if ! command -v brew >/dev/null 2>&1; then
                echo "  [x] Homebrew not found; install Homebrew, Qt6, and nlohmann-json first."
                exit 1
            fi
            brew install cmake qt@6 nlohmann-json
            ;;
        *)
            echo "  [x] Unsupported package manager for '$DISTRO'."
            echo "      Install CMake, Qt6 Widgets development files, and nlohmann-json manually."
            exit 1
            ;;
    esac
}

install_from_source() {
    echo "  Building from source..."
    install_build_dependencies

    local tmpdir
    tmpdir="$(mktemp -d)"
    git clone --depth 1 "https://github.com/$REPO.git" "$tmpdir/cmdx"
    cd "$tmpdir/cmdx"
    cmake -B build -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release
    cmake --build build -j"$(nproc 2>/dev/null || sysctl -n hw.logicalcpu)"
    sudo cmake --install build
    rm -rf "$tmpdir"
}

detect_platform

case "$DISTRO" in
    ubuntu|debian|linuxmint|pop)
        install_deb
        ;;
    fedora|rhel|centos|rocky|alma)
        install_rpm
        ;;
    arch|manjaro|endeavouros)
        echo "  Installing from source (Arch)..."
        install_from_source
        ;;
    macos)
        install_macos
        ;;
    *)
        echo "  No prebuilt package for '$DISTRO'. Building from source..."
        install_from_source
        ;;
esac

echo ""
echo "  [ok] cmdx installed successfully!"
cmdx --version 2>/dev/null && echo "" || true
echo "  Try: cmdx nmap"
echo ""

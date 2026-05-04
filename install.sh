#!/usr/bin/env bash
# cmdx installer - detects OS and installs the right package
# Usage: curl -fsSL https://raw.githubusercontent.com/Ishant89op/cmdx/main/install.sh | bash

set -e

VERSION="1.0.0"
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
    sudo dpkg -i "/tmp/$pkg"
    rm -f "/tmp/$pkg"
}

install_rpm() {
    local pkg="cmdx-${VERSION}.x86_64.rpm"
    echo "  Downloading $pkg..."
    curl -fsSL -o "/tmp/$pkg" "$BASE_URL/$pkg"
    echo "  Installing (requires sudo)..."
    sudo rpm -i "/tmp/$pkg"
    rm -f "/tmp/$pkg"
}

install_macos() {
    local pkg="cmdx-${VERSION}-Darwin.tar.gz"
    echo "  Downloading $pkg..."
    curl -fsSL -o "/tmp/$pkg" "$BASE_URL/$pkg"
    echo "  Installing to /usr/local/bin..."
    sudo tar -xzf "/tmp/$pkg" -C /usr/local --strip-components=1
    rm -f "/tmp/$pkg"
}

install_from_source() {
    echo "  Building from source..."
    if ! command -v cmake &>/dev/null; then
        echo "  [x] cmake not found. Install cmake first."
        exit 1
    fi
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

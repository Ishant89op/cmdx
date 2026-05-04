#!/usr/bin/env bash
# Test cmdx build + install across Linux distros via Docker
# Usage: ./test/test_all.sh

set -e

cd "$(dirname "$0")/.."

DISTROS=("ubuntu" "debian" "fedora" "arch")
PASS=0
FAIL=0

for distro in "${DISTROS[@]}"; do
    echo ""
    echo "════════════════════════════════════════════════"
    echo "  Testing: $distro"
    echo "════════════════════════════════════════════════"

    if docker build -f "test/Dockerfile.$distro" -t "cmdx-test-$distro" . ; then
        echo "  [ok] $distro - PASSED"
        ((PASS++))
    else
        echo "  [x] $distro - FAILED"
        ((FAIL++))
    fi

    # Cleanup
    docker rmi "cmdx-test-$distro" >/dev/null 2>&1 || true
done

echo ""
echo "════════════════════════════════════════════════"
echo "  Results: $PASS passed, $FAIL failed"
echo "════════════════════════════════════════════════"

[ $FAIL -eq 0 ] && exit 0 || exit 1

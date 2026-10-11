#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
: "${VITASDK:?Set VITASDK to the VitaSDK directory}"
export PATH="$VITASDK/bin:$PATH"
iteration=$(sed -n 's/.*VERSION "\([0-9][0-9]\.[0-9][0-9]\)".*/\1/p' CMakeLists.txt)
test -n "$iteration"
build_dir="build-iterations/$iteration"
cmake -S . -B "$build_dir" -DCMAKE_BUILD_TYPE=Release
cmake --build "$build_dir" -j2

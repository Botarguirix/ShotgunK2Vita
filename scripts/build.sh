#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
: "${VITASDK:?Set VITASDK to the VitaSDK directory}"
export PATH="$VITASDK/bin:$PATH"
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j2

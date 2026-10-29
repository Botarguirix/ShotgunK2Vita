#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
: "${VITASDK:?Set VITASDK to the VitaSDK directory}"
export PATH="$VITASDK/bin:$PATH"
iteration=$(sed -n 's/.*set(SG_BUILD_VERSION "\([0-9][0-9]\.[0-9][0-9]\)").*/\1/p' CMakeLists.txt)
test -n "$iteration"
build_dir="build-iterations/$iteration"
source_dir="${SG_SOURCE_DIR:-$PWD}"
build_dir="${SG_BUILD_DIR:-$source_dir/$build_dir}"
cmake -S "$source_dir" -B "$build_dir" -DCMAKE_BUILD_TYPE=Release "$@"
cmake --build "$build_dir" -j2

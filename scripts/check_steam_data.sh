#!/usr/bin/env bash
# Reproducible PC checks only; nunca son aceptación de hardware Vita.
set -euo pipefail
cd "$(dirname "$0")/.."
if [ "$#" -ne 2 ]; then
  printf 'Usage: bash scripts/check_steam_data.sh PREPARED_DATA NATIVE_LUAJIT_ROOT\n' >&2
  exit 2
fi
data=$(realpath "$1")
luajit=$(realpath "$2")
test -f "$data/steam-prepared.json"
test -f "$luajit/src/libluajit.a"
mkdir -p reports/steam-feasibility
output=$(mktemp -d reports/steam-feasibility/check-XXXXXX)
output=$(realpath "$output")
build=build-host/steam-check
trap 'printf "FAIL evidence=%s\n" "$output" >&2' ERR
cmake -S . -B "$build" -DSG_HOST_PROBE=ON -DSG_LUAJIT_EXPERIMENTAL=OFF \
  -DSG_HOST_LUAJIT_ROOT="$luajit" -DSG_STEAM_SGR_EXPERIMENTAL=ON \
  -DCMAKE_BUILD_TYPE=Release > "$output/configure.log" 2>&1
cmake --build "$build" --target sg_steam_smoke sg_audio_probe -j2 > "$output/build.log" 2>&1
"$luajit/src/luajit" tests/test_steam_compat.lua > "$output/bridge.log" 2>&1
mkdir -p "$output/menu-save" "$output/game-save"
"$build/sg_steam_smoke" "$data" 600 menu "$output/menu-save" > "$output/menu.log" 2>&1
"$build/sg_steam_smoke" "$data" 1200 game "$output/game-save" > "$output/game.log" 2>&1
"$build/sg_audio_probe" "$data" "$output/audio.wav" > "$output/audio.log" 2>&1
printf 'PASS host=Steam+LuaJIT menu=600 game=1200 audio=offline hardware=pending\n' | tee "$output/result.txt"
printf 'Evidence: %s\n' "$output"

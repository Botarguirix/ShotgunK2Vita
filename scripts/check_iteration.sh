#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
iteration=$(sed -n 's/.*set(SG_BUILD_VERSION "\([0-9][0-9]\.[0-9][0-9]\)").*/\1/p' CMakeLists.txt)
test -n "$iteration"
# Unique run directories preserve previous checks and fixtures.
mkdir -p "reports/iterations/$iteration/checks"
output=$(mktemp -d "reports/iterations/$iteration/checks/run-XXXXXX")
printf 'Checks and failure logs: %s\n' "$output"
trap 'printf "FAIL iteration=%s evidence=%s\n" "$iteration" "$output" >&2' ERR
build="build-host/iteration-$iteration-checks"
cmake -S . -B "$build" -DSG_HOST_PROBE=ON -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_C_FLAGS="-O1 -fsanitize=address,undefined -fno-omit-frame-pointer" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined" > "$output/configure.log" 2>&1
cmake --build "$build" --target sg_triangles_probe sg_aim_probe sg_combat_probe sg_options_probe sg_present_probe sg_text_probe sg_pad_repeat_probe sg_diagnostics_probe -j4 > "$output/build.log" 2>&1
export ASAN_OPTIONS=detect_leaks=1
"$build/sg_triangles_probe" private/game > "$output/triangles.log" 2>&1
"$build/sg_aim_probe" private/game "$output/aim.ppm" > "$output/aim.log" 2>&1
"$build/sg_combat_probe" private/game > "$output/combat.log" 2>&1
"$build/sg_options_probe" private/game "$output/options-fixture" > "$output/options.log" 2>&1
"$build/sg_present_probe" private/game > "$output/present.log" 2>&1
"$build/sg_text_probe" private/game > "$output/text.log" 2>&1
"$build/sg_pad_repeat_probe" private/game "$output/pad-fixture" > "$output/pad.log" 2>&1
"$build/sg_diagnostics_probe" "$output/diagnostics-fixture" private/game/assets/fonts/Terminus.ttf > "$output/diagnostics.log" 2>&1
printf 'PASS iteration=%s host=ASAN+UBSAN hardware=pending\n' "$iteration" > "$output/result.txt"
cat "$output/result.txt"
printf 'Evidence: %s\n' "$output"

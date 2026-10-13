#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
# One safety gate per campaign; optimized measurements use a separate build.
python3 tests/test_campaign_analysis.py
bash scripts/check_iteration.sh
build=build-host/performance-release
cmake -S . -B "$build" -DSG_HOST_PROBE=ON -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_FLAGS_RELEASE='-O2 -DNDEBUG' > /tmp/shotgunk2vita-perf-configure.log
cmake --build "$build" --target sg_scene_bench -j4
python3 tools/performance_campaign.py --binary "$build/sg_scene_bench" "$@"

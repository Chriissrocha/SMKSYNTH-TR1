#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/.."
mkdir -p build/smklive build/su3
CC=${CC:-cc}
compile() { "$CC" -O1 -DOBV_HOST -DBOARD_SMK37=1 -Ifirmware/src -Ifirmware/gen "$@" -lm; }
compile -DSMK_LIVE=1 -DOBV_DUAL_CORE=1 tests/smklive_test.c -o build/smklive/test
build/smklive/test
for name in physical_controls octave midi_return_tuning su3_workflow sustain_panic split_layer zoom_palette; do
 compile "tests/${name}_test.c" -o "build/smklive/$name"
 "build/smklive/$name"
done
# Enforce original source/HAL boundary checks, even without the target compiler.
python3 - <<'PY'
import sys
sys.path.insert(0, 'tools')
from build import mmio_check
errors = mmio_check()
if errors: raise SystemExit('\n'.join(errors))
print('PASS: HAL/MMIO source boundary')
PY

#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/../.."
mkdir -p build/smklive-tester
CLANG=${CLANG:-clang}
"$CLANG" --target=wasm32-unknown-unknown -mcpu=generic -mbulk-memory -O2 -DSMK_LIVE=1 -DSMK_SIM_STEP=1 '-DAPP_VER="LIVE-2"' \
 -ffreestanding -fno-builtin -nostdlib -Ifirmware/src -Ifirmware/gen -Wl,--no-entry -Wl,--export-memory -Wl,--export-dynamic -Wl,-z,stack-size=262144 \
 -o build/smklive-tester/smklive.wasm web/sim/sim.c
python3 web/smklive-tester/bundle.py

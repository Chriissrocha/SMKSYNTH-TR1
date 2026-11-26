#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Build only a bank-preserving SMKLIVE update, never a first-install image.
set -eu
cd "$(dirname "$0")"
case "${1:-}" in
 pro) id=19701; model=Pro; suffix=smk37;;
 elite) id=19702; model=Elite; suffix=smk37-elite;;
 *) echo 'Usage: ./build-live.sh pro|elite' >&2; exit 2;;
esac
[ "$#" -eq 1 ] || { echo 'No extra flags accepted by this safe update wrapper.' >&2; exit 2; }
export AC79_SDK="${AC79_SDK:-$HOME/fw-AC79_AIoT_SDK}"
export JIELI_TOOLCHAIN="${JIELI_TOOLCHAIN:-$HOME/.jieli/toolchain}"
[ -x "$JIELI_TOOLCHAIN/pi32v2/bin/clang" ] || { echo 'JieLi toolchain missing. See tools/get_toolchain.sh.' >&2; exit 1; }
# Fail closed instead of silently accepting an SDK different from the original reference.
python3 - <<'PY'
import hashlib, os, sys
from pathlib import Path
sys.path.insert(0, 'tools')
from build import SDK_SHA256
root = Path(os.environ['AC79_SDK']) / 'cpu/wl82/tools'
for rel, expected in SDK_SHA256.items():
    p = root / rel
    if not p.is_file() or hashlib.sha256(p.read_bytes()).hexdigest() != expected:
        raise SystemExit(f'SDK reference mismatch: {p}. Use the original AC79NN SDK V1.2.1 (2023-12-13).')
print('SDK reference files: SHA-256 verified')
PY
export OBV_DEFS='-DSMK_LIVE=1'
export OBV_OPT='-Os'
./build.sh --board smk37 --model "$1" --voice-hardware --release 1.3.1-live2 --experimental-id "$id"
mkdir -p build/smklive-package
out="build/smklive-package/SMKLIVE-1.3.1-live2-update-SMK37-$model.fwsc"
cp "build/obervave-1.3.1-live2-$suffix.fwsc" "$out"
(cd build/smklive-package && sha256sum "$(basename "$out")" > "$(basename "$out").sha256")
python3 - "$out" "$1" "$model" "$id" <<'PYMANIFEST'
from pathlib import Path
import hashlib,json,sys
p=Path(sys.argv[1]);model,display,identity=sys.argv[2:]
raw=p.read_bytes()
manifest={'schema':'smklive-update-v1','release':'1.3.1-live2','operation':'update','model':model,'product':f'SMK-37 {display}_{identity}','filename':p.name,'size':len(raw),'sha256':hashlib.sha256(raw).hexdigest()}
p.with_suffix('.json').write_text(json.dumps(manifest,indent=2)+'\n')
PYMANIFEST
printf '\nGenerated experimental update: %s\nExpected identity: SMK-37 %s_%s\n' "$out" "$model" "$id"

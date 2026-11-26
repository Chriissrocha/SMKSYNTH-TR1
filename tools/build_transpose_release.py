#!/usr/bin/env python3
"""Build the isolated 1.3.1-TR1 candidate; never change official release files."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import zipfile
import build as upstream
import fm1_install as install
import fm1pkg_make as pkg

ROOT = Path(__file__).resolve().parents[1]
VERSION, IDENTITY = '1.3.1-TR1', '19669'
MODELS = {'pro': 'Pro', 'elite': 'Elite'}
OUT = ROOT / 'build/transpose-dist'


def run(*args):
    subprocess.run([str(a) for a in args], cwd=ROOT, check=True)


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def prepare(site):
    shutil.copytree(ROOT / 'web/install-transpose', site / 'install')
    shutil.copy2(ROOT / 'web/site/i18n.js', site / 'i18n.js')
    shutil.copytree(ROOT / 'web/backup', site / 'backup')
    (site / 'docs').mkdir()
    for name in ['TRANSPOSE-TR1.md', 'MANUAL.md', 'QUICKSTART.md', 'INSTALLATION.md']:
        shutil.copy2(ROOT / 'docs' / name, site / 'docs' / name)
    for name in ['CREDITS.md', 'LICENSE', 'LICENSING.md', 'TRANSPOSE.json']:
        shutil.copy2(ROOT / name, site / name)
    (site / 'downloads').mkdir()
    (site / 'index.html').write_text('<!doctype html><meta charset="utf-8"><title>SMKSYNTH TR1</title><a href="install/">SMKSYNTH 1.3.1-TR1 — Instalador / Installer</a>', encoding='utf-8')


def source_zip(path):
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as z:
        for f in sorted(ROOT.rglob('*')):
            rel = f.relative_to(ROOT)
            if not f.is_file() or any(x in rel.parts for x in ('.git', 'build', '__pycache__', 'node_modules')):
                continue
            if f.suffix == '.pyc':
                continue
            z.write(f, f'SMKSYNTH-{VERSION}/{rel.as_posix()}')


def build(site):
    tc = Path(os.environ.get('JIELI_TOOLCHAIN', str(Path.home() / '.jieli/toolchain')))
    sdk = Path(os.environ.get('AC79_SDK', str(Path.home() / 'fw-AC79_AIoT_SDK')))
    os.environ['JIELI_TOOLCHAIN'] = str(tc.resolve())
    # Match the 1.3.1 release's dual-core runtime and original optimization.
    os.environ['OBV_DEFS'] = '-DOBV_DUAL_CORE=1'
    os.environ['OBV_OPT'] = '-Os'
    if not os.access(tc / 'pi32v2/bin/clang', os.X_OK):
        raise SystemExit('JieLi toolchain missing; run tools/get_toolchain.sh first.')
    pkg.SDK = sdk
    for rel, sha in upstream.SDK_SHA256.items():
        if hashlib.sha256(pkg.sdk_file(rel)).hexdigest() != sha:
            raise SystemExit('SDK hash mismatch: ' + rel)
    for name in ('internal_transpose', 'octave', 'split_layer_rows', 'sustain_panic', 'midi_mode', 'midi_mode_transition', 'endless', 'endless_midi', 'ps3_onset'):
        test = ROOT / 'build/transpose' / name
        test.parent.mkdir(parents=True, exist_ok=True)
        run('cc', '-O1', '-DOBV_HOST', '-DBOARD_SMK37=1', '-DOBV_DUAL_CORE=1', '-Ifirmware/src', '-Ifirmware/gen', f'tests/{name}_test.c', '-lm', '-o', test)
        run(test)
    run(sys.executable, 'tests/release_factory_test.py')
    firmware = site / 'install/firmware'
    firmware.mkdir()
    pins = {}
    for model, label in MODELS.items():
        product = f'SMK-37 {label}_{IDENTITY}'
        pins[model] = {'model': f'SMK-37 {label}'}
        for kind, mode in [('release', 'first-install'), ('update', 'update')]:
            suffix = '' if model == 'pro' else '-elite'
            raw = ROOT / 'build' / f'obervave-{VERSION}-smk37{suffix}.fwsc'
            raw.unlink(missing_ok=True)
            args = [sys.executable, 'tools/build.py', '--board', 'smk37', '--model', model, '--release', VERSION, '--experimental-id', IDENTITY, '--voice-hardware', '--sdk', sdk]
            if kind == 'release':
                args += ['--first-install', '--factory-bank', ROOT / 'factory/bank-SMK16P1.bin']
            run(*args)
            target = firmware / f'SMKSYNTH-{VERSION}-{mode}-SMK37-{label}.fwsc'
            shutil.copy2(raw, target)
            blob = target.read_bytes()
            image, blocks = install.deinterleave(blob)
            if install.product_of(blob) != product or blocks != 20 or install.package_damage(image):
                raise SystemExit('Invalid new firmware package: ' + target.name)
            if kind == 'release':
                run(sys.executable, 'tests/release_package_test.py', target)
            pins[model][kind] = dict(name=f'SMKSYNTH {VERSION} · {mode}', product=product, url='./firmware/' + target.name, size=len(blob), sha=digest(target))
    (site / 'install/manifest.json').write_text(json.dumps(pins, indent=2) + '\n')
    run(sys.executable, 'tests/release_110_packages_test.py', site / 'install')
    (site / 'install/pins.js').write_text('// SPDX-License-Identifier: GPL-3.0-only\nexport const PINS=Object.freeze(' + json.dumps(pins) + ');\n')
    run('node', 'tests/transpose_installer_test.mjs', site / 'install', '--built')
    info = json.loads((site / 'TRANSPOSE.json').read_text())
    info.update(status='compiled-not-hardware-validated', firmware=pins)
    (site / 'TRANSPOSE.json').write_text(json.dumps(info, indent=2) + '\n')
    (site / 'SHA256SUMS.txt').write_text(''.join(digest(f) + '  install/firmware/' + f.name + '\n' for f in sorted(firmware.glob('*.fwsc'))))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--prepare-only', action='store_true', help='Generate locked installer preview without firmware.')
    args = parser.parse_args()
    destination = OUT.with_name('transpose-preview') if args.prepare_only else OUT
    destination.parent.mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='transpose-stage-', dir=destination.parent) as temp:
        stage = Path(temp) / 'site'
        stage.mkdir()
        prepare(stage)
        if args.prepare_only:
            run('node', 'tests/transpose_installer_test.mjs', stage / 'install')
        else:
            build(stage)
        source_zip(stage / 'downloads' / f'SMKSYNTH-{VERSION}-source.zip')
        if destination.exists():
            shutil.rmtree(destination)
        shutil.move(str(stage), str(destination))
    print(('LOCKED PREVIEW: ' if args.prepare_only else 'BUILT; hardware validation pending: ') + str(destination))


if __name__ == '__main__':
    main()

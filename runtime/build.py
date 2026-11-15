#!/usr/bin/env python3
"""Reproducible Linux build of the archived Bclimate Sedona sources."""
import argparse
from datetime import datetime, timezone
import os
from pathlib import Path
import shlex
import subprocess
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parent


def run(args, **kwargs):
    args = list(map(str, args))
    print('+', shlex.join(args), flush=True)
    subprocess.run(args, cwd=ROOT, check=True, **kwargs)


def compiler(*args):
    run(['java', '-Dsedona.home=' + str(ROOT), '-cp',
         os.pathsep.join(str(ROOT / 'lib' / name) for name in ('sedona.jar', 'sedonac.jar')),
         'sedonac.Main', *args])


def jars():
    for module in ('sedona', 'sedonac', 'sedonacert'):
        classes = ROOT / 'build' / 'java' / module
        classes.mkdir(parents=True, exist_ok=True)
        sources = sorted((ROOT / 'src' / module / 'src').rglob('*.java'))
        # Niagara's module metadata is not a standalone Java source.
        args = ROOT / 'build' / (module + '-javac.args')
        args.write_text('\n'.join('"' + str(p) + '"' for p in sources), encoding='utf-8')
        cp = os.pathsep.join(str(p) for p in (ROOT / 'lib').glob('*.jar')) or '.'
        run(['javac', '-encoding', 'UTF-8', '-source', '8', '-target', '8',
             '-classpath', cp, '-d', classes, '@' + str(args)])
        if module == 'sedona':
            version = next(line.split('=', 1)[1] for line in
                           (ROOT / 'lib/sedona.properties').read_text().splitlines()
                           if line.startswith('buildVersion='))
            (classes / 'version.txt').write_text('version=' + version + '\ntime=' +
                                               datetime.now(timezone.utc).isoformat() + '\n')
        run(['jar', 'cf', ROOT / 'lib' / (module + '.jar'), '-C', classes, '.'])


def kits():
    paths = list((ROOT / 'src').glob('*/kit.xml')) + list((ROOT / 'platforms/src').rglob('kit.xml'))
    by_name = {ET.parse(p).getroot().get('name'): p for p in paths
               if ET.parse(p).getroot().tag == 'sedonaKit'}
    done, visiting = set(), set()

    def visit(name):
        if name in done:
            return
        if name in visiting:
            raise RuntimeError('Cyclic kit dependency: ' + name)
        visiting.add(name)
        path = by_name[name]
        for dep in ET.parse(path).getroot().findall('depend'):
            visit(dep.get('on').split()[0])
        compiler(path)
        visiting.remove(name)
        done.add(name)

    for name in sorted(by_name):
        visit(name)


def image(arch, fast=False, asan=False):
    out = ROOT / 'build' / arch
    out.mkdir(parents=True, exist_ok=True)
    refsize = '8' if arch == 'amd64' else '4'
    manifest = ET.parse(ROOT / 'platforms/src/bclimate/unix/generic-unix.xml')
    manifest.getroot().set('vendor', 'Bclimate')
    manifest.getroot().set('id', 'bclimate-linux-' + arch + '-1.2.29')
    manifest.getroot().find('compile').set('refSize', refsize)
    # Keep original image ABI; set arch explicitly rather than using host detection.
    manifest_path = out / ('bclimate-' + arch + '.xml')
    manifest.write(manifest_path, encoding='unicode')
    stage = out / 'native'
    stage.mkdir(exist_ok=True)
    compiler(manifest_path, '-outDir', stage)
    flags = ['-m64' if arch == 'amd64' else '-m32', '-std=gnu99', '-O2', '-g',
             '-fno-strict-aliasing', '-fcommon', '-D_GNU_SOURCE', '-D__UNIX__=1', '-DSOCKET_FAMILY_INET=1',
             '-DPLAT_BUILD_VERSION="1.2.29-bclimate"', '-I/usr/include/modbus', '-I' + str(stage)]
    if fast:
        # threaded-code dispatch in vm.c and stronger optimisation; same ABI and scode
        flags = [('-O3' if f == '-O2' else f) for f in flags] + ['-DCOMPUTED_GOTO']
        print('FAST build: -O3 -DCOMPUTED_GOTO', flush=True)
    if asan:
        # diagnostic build: AddressSanitizer + LeakSanitizer, not for production
        flags = [('-O1' if f in ('-O2', '-O3') else f) for f in flags] + \
                ['-fsanitize=address', '-fno-omit-frame-pointer']
        print('ASAN build: -fsanitize=address', flush=True)
    temporary_vm = out / 'svm.new'
    run(['gcc', *flags, *sorted(stage.glob('*.c')), '-o', temporary_vm,
         '-lmodbus', '-lsqlite3', '-lm', '-pthread', *(['-fsanitize=address'] if asan else [])])
    # an ASan build never replaces the production binary
    temporary_vm.replace(out / ('svm.asan' if asan else 'svm'))
    code = ET.parse(ROOT / 'scode/bclimateDefaultKits_unix.xml')
    code.getroot().set('refSize', refsize)
    code_path = out / 'kits.xml'
    code.write(code_path, encoding='unicode')
    compiler(code_path)
    platform_dir = ROOT / 'platforms' / 'db' / 'bclimate' / 'linux' / arch / '1.2.29'
    platform_dir.mkdir(parents=True, exist_ok=True)
    import shutil
    shutil.copy2(stage / '.par/platformManifest.xml', platform_dir / 'platformManifest.xml')
    print('BUILT', out / ('svm.asan' if asan else 'svm'), flush=True)


def verify(arch, suite):
    """Run image tests with explicit selection and assert the failure counter."""
    out = ROOT / 'build' / arch
    code = ET.parse(out / 'kits.xml')
    core = {'sys', 'types', 'math', 'bclimateLib', 'bclimateMath', 'datetime'}
    for dep in code.getroot().findall('depend'):
        name = dep.get('on').split()[0]
        enabled = name in core if suite == 'core' else name not in {'inet', 'serial'}
        dep.set('test', str(enabled).lower())
    source = out / (suite + '-tests.xml')
    code.write(source, encoding='unicode')
    compiler(source)
    logfile = out / ('test-' + suite + '.log')
    with logfile.open('w') as log:
        subprocess.run([str(out / 'svm'), str(source.with_suffix('.scode')), '-test'],
                       cwd=out, stdout=log, stderr=subprocess.STDOUT, timeout=360, check=True)
    report = logfile.read_text(errors='replace')
    print(report[-1500:])
    if 'Assert Failures  = 0' not in report or 'ERROR:' in report or 'WARNING: stack imbalance' in report:
        raise RuntimeError('VM assertions failed; see ' + str(logfile))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('target', choices=['jars', 'kits', 'vm', 'all', 'verify'])
    parser.add_argument('--arch', choices=['i386', 'amd64'], default='amd64')
    parser.add_argument('--suite', choices=['core', 'offline'], default='core')
    parser.add_argument('--fast', action='store_true',
                        help='build svm with -O3 and computed-goto dispatch (optional)')
    parser.add_argument('--asan', action='store_true',
                        help='diagnostic svm with AddressSanitizer (tests only)')
    args = parser.parse_args()
    for name in ('build', 'lib', 'kits', 'manifests'):
        (ROOT / name).mkdir(exist_ok=True)
    if args.target in ('jars', 'all'):
        jars()
    if args.target in ('kits', 'all'):
        kits()
    if args.target in ('vm', 'all'):
        image(args.arch, args.fast, args.asan)
    if args.target == 'verify':
        verify(args.arch, args.suite)


if __name__ == '__main__':
    main()

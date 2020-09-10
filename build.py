#!/usr/bin/env python

import os
import subprocess
import argparse
from installer.generateFomod import FomodGenerator

assert 'MSBUILD_PATH' in os.environ
assert 'CBP_SLN_ROOT' in os.environ

assert os.path.isdir(os.environ['MSBUILD_PATH'])
assert os.path.isdir(os.environ['CBP_SLN_ROOT'])

MSBUILD_PATH = os.path.join(os.environ['MSBUILD_PATH'], 'msbuild.exe')
SLN_ROOT = os.environ['CBP_SLN_ROOT']

# relative to SLN_ROOT
OUT = 'tmp'
FOMOD = 'installer\\generateFomod.py'
SLN = 'CBP.sln'

DLL = 'CBP.dll'
CONFIGS = ['Dep-IntelAvx2MT', 'Dep-IntelSSE42MT', 'Dep-Generic']

parser = argparse.ArgumentParser()
parser.register('type', 'bool', lambda x: x.lower() in ("yes", "true", "1"))
parser.add_argument('--rebuild', action='store_true', default=False)
parser.add_argument('--nobuild', action='store_true', default=False)
parser.add_argument('-p', type='bool', nargs='?', const=True, default=True)
args = parser.parse_args()

def prepare(p):
    if os.path.exists(p):
        if not os.path.isdir(p):
            raise Exception('Invalid output path: {}'.format(p))
    else:
        os.mkdir(p)

def safe_mkdir(p):
    try:
        os.mkdir(p)
    except FileExistsError:
        pass

def test_file(p):
    return os.path.exists(p) and os.path.isfile(p) 

def build_solution(cfg, bc, path, rebuild, parallel = True):
    args = ['-p:Configuration={};OutDir={};PostBuildEventUseInBuild=no'.format(cfg, path)]
    if rebuild:
        args.append('-t:Clean;Rebuild')
    if parallel:
        args.append('-m')

    cmd = bc
    cmd.extend(args)

    print('Building {} ..'.format(cfg))

    r = subprocess.run(cmd)
    if r.returncode != 0:
        raise Exception('Build failed: {}'.format(cfg))

OUTPUT_PATH = os.path.join(SLN_ROOT, OUT)
basecmd = [MSBUILD_PATH, os.path.join(SLN_ROOT, SLN)]

prepare(OUTPUT_PATH)

package_targets = {}

for e in CONFIGS:
    path = os.path.join(OUTPUT_PATH, e)
    safe_mkdir(path)

    assert os.path.isdir(path)

    if not args.nobuild:
        build_solution(e, basecmd, path + '\\', args.rebuild, args.p)

    dll = os.path.join(path, DLL)
    if not test_file(dll):
        raise Exception('Could not find dll: {}'.format(dll))

    package_targets[e] = dll

fg = FomodGenerator(package_targets)
fg.run()


print('OK')

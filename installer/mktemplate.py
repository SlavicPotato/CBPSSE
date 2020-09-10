#!\\usr\\bin\\env python

import os
import pathlib
import pyfomod

T_C = 'SKSE\\Plugins'
T_DLL = 'SKSE\\Plugins\\cbp.dll'
T_P = 'SKSE\\Plugins\\CBP\\Default.json'

conf_force = {
    'CBP.ini',
    'CBP\\Nodes.json',
    'CBP\\Settings\\CollisionGroups.json'
}

conf_missing = [
    'CBP\\Profiles\\Node\\UNP.json',
    'CBP\\Profiles\\Node\\CBBE 3BBB.json',
    'CBP\\Profiles\\Physics\\UNP.json',
    'CBP\\Profiles\\Physics\\CBBE 3BBB.json'
]

def mkfile(a, b, c, d = pyfomod.OptionType.OPTIONAL, e = None):
    t = pyfomod.Option()
    t.name = a
    t.description = b
    t.files[c[0]] = c[1]
    t.type = d
    if e:
        t.image = e

    return t

def mkccond(p, a):
    c = pyfomod.Conditions()
    c[os.path.join(T_C, a)] = pyfomod.FileType.MISSING
    f = pyfomod.Files()
    f[os.path.join('03_baseconf\\', a)] = os.path.join(T_C, a)
    p[c] = f
    return p

root = pyfomod.Root()

pages = pyfomod.Pages()

page_exec = pyfomod.Page()
page_exec.name = 'Plugin'

group_exec = pyfomod.Group()
group_exec.name = 'DLL'
group_exec.type = pyfomod.GroupType.EXACTLYONE

group_exec.append(mkfile('Generic', 'For any x64 cpu', ('00_binaries\\generic\\cbp.dll', T_DLL)))
group_exec.append(mkfile('SSE4.2', 'For Intel Nehalem (Core 2) / AMD K10 or later', ('00_binaries\\sse42\\cbp.dll', T_DLL)))
group_exec.append(mkfile('AVX2', 'For Intel Haswell / AMD Excavator or later', ('00_binaries\\avx2\\cbp.dll', T_DLL)))

page_exec.append(group_exec)

page_profile = pyfomod.Page()
page_profile.name = 'Physics profile'

group_profile = pyfomod.Group()
group_profile.name = 'Select option to match your body'
group_profile.type = pyfomod.GroupType.EXACTLYONE

group_profile.append(mkfile('UUNP - Base', 'UUNP - Base', ('01_default\\uunp\\base\\default.json', T_P)))
group_profile.append(mkfile('UUNP - UNP', 'UUNP - UNP', ('01_default\\uunp\\unp\\default.json', T_P)))
group_profile.append(mkfile('UUNP - UNPB', 'UUNP - UNPB', ('01_default\\uunp\\unpb\\default.json', T_P)))
group_profile.append(mkfile('UUNP - 7B', 'UUNP - 7B', ('01_default\\uunp\\7b\\default.json', T_P)))
group_profile.append(mkfile('BHUUNP - Base', 'BHUUNP - Base', ('01_default\\bhuunp\\base\\default.json', T_P)))
group_profile.append(mkfile('CBBE', 'CBBE', ('01_default\\cbbe\\default.json', T_P)))
group_profile.append(mkfile('CBBE 3BBB', 'CBBE 3BBB', ('01_default\\cbbe_3bbb\\default.json', T_P)))

page_profile.append(group_profile)

pages.append(page_exec)
pages.append(page_profile)

root.name = 'CBP-A'
root.author = 'SlavicPotato'
root.description = 'CBP-A'

root.pages = pages

for v in conf_force:
    root.files[os.path.join('03_baseconf', v)] = os.path.join(T_C, v)

for v in conf_missing:
    mkccond(root.file_patterns, v)

vl = root.validate()
for warning in vl:
    print('{}{}: {} - {}'.format(
        'CRITICAL: ' if warning.critical else '', 
        warning.elem, warning.title, warning.msg))

assert len(vl) == 0

pyfomod.write(root, pathlib.Path(__file__).parent / 'package')


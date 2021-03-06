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
    'CBP\\Settings\\CollisionGroups.json',
    'CBP\\Profiles\\Node\\Blank.json'
}

conf_missing_dirs = {
    'CBP\\Templates',
    'CBP\\Exports'
}

conf_missing = [
    'CBP\\Profiles\\Node\\UUNP.json',
    'CBP\\Profiles\\Node\\UUNP - more arm colliders.json',
    'CBP\\Profiles\\Node\\BHUNP.json',
    'CBP\\Profiles\\Node\\BHUNP - more arm colliders.json',
    'CBP\\Profiles\\Node\\CBBE.json',
    'CBP\\Profiles\\Node\\3BBB.json',
    'CBP\\Profiles\\Physics\\UUNP - UNP.json',
    'CBP\\Profiles\\Physics\\UUNP - UNPB.json',
    'CBP\\Profiles\\Physics\\UUNP - 7B.json',
    'CBP\\Profiles\\Physics\\BHUNP - UNP.json',
    'CBP\\Profiles\\Physics\\BHUNP - UNPB.json',
    'CBP\\Profiles\\Physics\\BHUNP - 7B.json',
    'CBP\\Profiles\\Physics\\CBBE.json',
    'CBP\\Profiles\\Physics\\3BBB.json',
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

def mkccond(p, a, b = False):
    c = pyfomod.Conditions()
    c[os.path.join(T_C, a)] = pyfomod.FileType.MISSING
    f = pyfomod.Files()
    f[os.path.join('03_baseconf\\', a) + ('/' if b else '')] = os.path.join(T_C, a)
    p[c] = f
    return p

root = pyfomod.Root()

pages = pyfomod.Pages()

page_exec = pyfomod.Page()
page_exec.name = 'Plugin'

group_exec = pyfomod.Group()
group_exec.name = 'DLL'
group_exec.type = pyfomod.GroupType.EXACTLYONE

group_exec.append(mkfile('AVX2', 'For Intel Haswell / AMD Excavator or later. Haswell was launched in 2013, Excavator in 2015. Best performance.', ('00_binaries\\avx2\\cbp.dll', T_DLL)))
group_exec.append(mkfile('AVX', '.', ('00_binaries\\avx\\cbp.dll', T_DLL)))
group_exec.append(mkfile('SSE2', 'For any x64 processor. Use only if you run a potato built before 2008.', ('00_binaries\\sse2\\cbp.dll', T_DLL)))

page_exec.append(group_exec)

page_profile = pyfomod.Page()
page_profile.name = 'Physics profile'

group_profile = pyfomod.Group()
group_profile.name = 'Select the default global physics and node profile'
group_profile.type = pyfomod.GroupType.EXACTLYONE

group_profile.append(mkfile('UUNP - UNP', 'UUNP - UNP', ('01_default\\uunp\\unp\\default.json', T_P)))
group_profile.append(mkfile('UUNP - UNPB', 'UUNP - UNPB', ('01_default\\uunp\\unpb\\default.json', T_P)))
group_profile.append(mkfile('UUNP - 7B Original', 'UUNP - 7B Original', ('01_default\\uunp\\7b\\default.json', T_P)))
group_profile.append(mkfile('BHUNP - UNP', 'BHUNP - UNP', ('01_default\\bhunp\\unp\\default.json', T_P)))
group_profile.append(mkfile('BHUNP - UNPB', 'BHUNP - UNPB', ('01_default\\bhunp\\unpb\\default.json', T_P)))
group_profile.append(mkfile('BHUNP - 7B Original', 'BHUNP - 7B Original', ('01_default\\bhunp\\7b\\default.json', T_P)))
group_profile.append(mkfile('CBBE Curvy', 'CBBE Curvy', ('01_default\\cbbe\\default.json', T_P)))
group_profile.append(mkfile('CBBE 3BBB', 'CBBE 3BBB', ('01_default\\cbbe_3bbb\\default.json', T_P)))

page_profile.append(group_profile)

pages.append(page_exec)
pages.append(page_profile)

root.name = 'placeholder'
root.author = 'placeholder'
root.description = 'placeholder'
root.version = 'placeholder'

root.pages = pages

for v in conf_force:
    root.files[os.path.join('03_baseconf', v)] = os.path.join(T_C, v)
    
for v in conf_missing:
    mkccond(root.file_patterns, v)
    
for v in conf_missing_dirs:
    mkccond(root.file_patterns, v, True)

vl = root.validate()
for warning in vl:
    print('{}{}: {} - {}'.format(
        'CRITICAL: ' if warning.critical else '', 
        warning.elem, warning.title, warning.msg))

assert len(vl) == 0

pyfomod.write(root, pathlib.Path(__file__).parent / 'package')


"""Compile changed firmware modules and link a validation ELF using existing objects.

This is an incremental integration check, not a substitute for a clean IDE build.
Writes only mdk_v5/dma_verification; never replaces the normal flash image.
"""
from pathlib import Path
import subprocess
import xml.etree.ElementTree as ET

root = Path(__file__).resolve().parents[1]
project = root/'mdk_v5'
out = project/'dma_verification'
out.mkdir(parents=True, exist_ok=True)
binpath = Path('C:/Program Files (x86)/Arm GNU Toolchain arm-none-eabi/14.2 rel1/bin')
xml = ET.parse(project/'ITRI_Talent_A.uvprojx')
includes = xml.find('.//Carm/VariousControls/IncludePath').text.split(';')
includes = [str((project/p).resolve()) if not Path(p).is_absolute() else p for p in includes]
packs = Path.home()/'AppData/Local/Arm/Packs'
sdk = packs/'EdgeImpulse/EI-SDK/1.95.29/edgeimpulse'
model = packs/'EdgeImpulse/ITRI-Talent-A-Project/2.0.0/edgeimpulse'
includes += [str(sdk), str(sdk/'edge-impulse-sdk/classifier'), str(model),
             str(packs/'ARM/CMSIS-NN/7.0.0/Include'), str(project/'RTE/_ITRI_Talent_A')]
flags = ['-mcpu=cortex-m4', '-mthumb', '-fshort-enums', '-O2', '-g',
         '-ffunction-sections', '-fdata-sections']
flags += ['-D'+d for d in xml.find('.//Carm/VariousControls/Define').text.split(',')]
flags += ['-I'+p for p in includes]
changed = ['ei_main.cpp', 'ei_classifier_porting.cpp', 'BMC81M001.c',
           'dw1000.c', 'mr60bha2.c', 'ht32f493x5_int.c']
for name in changed:
    cpp = name.endswith('cpp')
    compiler = 'arm-none-eabi-g++.exe' if cpp else 'arm-none-eabi-gcc.exe'
    subprocess.run([str(binpath/compiler), *flags,
                    '-std=gnu++11' if cpp else '-std=gnu99', '-c', str(root/'src'/name),
                    '-o', str(out/(Path(name).stem.lower()+'.o'))], check=True, cwd=project)
response = (project/'objects/ITRI_Talent_A.lnp').read_text()
for name in changed:
    stem = Path(name).stem.lower()
    response = response.replace(f'"./objects/{stem}.o"', f'"./dma_verification/{stem}.o"')
response = response.replace('"./objects/ITRI_Talent_A.elf"', '"./dma_verification/ITRI_Talent_A.elf"')
(out/'verify.lnp').write_text(response)
subprocess.run([str(binpath/'arm-none-eabi-g++.exe'), '@'+str(out/'verify.lnp')], check=True, cwd=project)
subprocess.run([str(binpath/'arm-none-eabi-size.exe'), str(out/'ITRI_Talent_A.elf')], check=True)
print('PASS: changed firmware modules compiled and validation ELF linked')

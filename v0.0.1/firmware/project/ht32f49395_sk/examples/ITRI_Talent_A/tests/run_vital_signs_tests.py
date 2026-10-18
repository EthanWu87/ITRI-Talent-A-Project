"""Run the actual radar reference-state parser and independent image alarm with controlled time and sensor input on ARM.
Uses the same Unicorn/pyelftools installation as run_dw1000_tests.py.
"""
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'mdk_v5/dw1000_verification'
sys.path.insert(0, str(OUT / 'test_tools'))
from unicorn import Uc, UC_ARCH_ARM, UC_MODE_ARM, UC_HOOK_CODE
from unicorn.arm_const import UC_ARM_REG_SP, UC_ARM_REG_LR, UC_ARM_REG_R0
from elftools.elf.elffile import ELFFile

# Hardware initialization alone is stubbed; parsing, timestamps and gating are real.
header = (ROOT / 'inc/mr60bha2.h').read_text(encoding='utf-8')
header = header.replace('#include "ht32f493x5_board.h"', '')
(OUT / 'mr60bha2_types_under_test.h').write_text(header, encoding='utf-8')
driver = (ROOT / 'src/mr60bha2.c').read_text(encoding='utf-8')
driver = driver.replace('#include "mr60bha2.h"', '#include "mr60bha2_types_under_test.h"')
driver = driver.replace('#include "ht32f493x5_int.h"', '')
start = driver.index('static void mr60bha2_usart_configuration(void)\n{')
end = driver.index('static void mr60bha2_reset_parser(void)\n{', start)
driver = driver[:start] + 'static void mr60bha2_usart_configuration(void) {}\n' + driver[end:]
(OUT / 'mr60bha2_under_test.inc').write_text(driver, encoding='utf-8')
source = (ROOT / 'src/ei_main.cpp').read_text(encoding='utf-8')
constants = source[source.index('#define HM01B0_INITIAL_FALL_MS'):
                   source.index('extern uint8_t hm01b0_frame_buffer')]
flow = source[source.index('// Radar vital signs are reference-only'):
              source.index('int raw_feature_get_data')]
(OUT / 'vital_signs_flow_under_test.inc').write_text(constants + flow, encoding='utf-8')
compiler = Path('C:/Program Files (x86)/Arm GNU Toolchain arm-none-eabi/14.2 rel1/bin/arm-none-eabi-g++.exe')
elf_path = OUT / 'vital_signs_test.elf'
subprocess.run([str(compiler), '-mcpu=arm926ej-s', '-marm', '-O0', '-g',
               '-I'+str(ROOT/'inc'), '-I'+str(OUT), '-fno-exceptions', '-fno-rtti',
               '-nostartfiles', '--specs=nosys.specs',
               '-Wl,-Ttext=0x10000,-Tdata=0x20000000,-e,main',
               str(ROOT/'tests/vital_signs_test.cpp'), '-lm', '-o', str(elf_path)], check=True)
uc = Uc(UC_ARCH_ARM, UC_MODE_ARM)
uc.mem_map(0x10000, 0x200000)
uc.mem_map(0x20000000, 0x200000)
with elf_path.open('rb') as handle:
    elf = ELFFile(handle)
    for segment in elf.iter_segments():
        if segment['p_type'] == 'PT_LOAD':
            uc.mem_write(segment['p_vaddr'], segment.data())
    symbols = {s.name: s['st_value'] for s in elf.get_section_by_name('.symtab').iter_symbols()}
    entry = elf.header['e_entry']
stop = 0x20000C
uc.reg_write(UC_ARM_REG_SP, 0x201FFFF0)
uc.reg_write(UC_ARM_REG_LR, stop)
finished = [False]
def on_code(emulator, address, size, _):
    finished[0] = True
    emulator.emu_stop()
uc.hook_add(UC_HOOK_CODE, on_code, begin=stop, end=stop)
uc.emu_start(entry, stop+4, count=30000000)
def word(name):
    return int.from_bytes(uc.mem_read(symbols[name], 4), 'little')
if not finished[0]:
    raise AssertionError('Alarm flow did not return within instruction budget')
if uc.reg_read(UC_ARM_REG_R0):
    raise AssertionError(f'Test {word("test_number")+1} failed at C++ line {word("test_failure")}')
print(f'PASS: {word("test_number")} real UART/reference-state/fall-alarm scenarios on ARM.')

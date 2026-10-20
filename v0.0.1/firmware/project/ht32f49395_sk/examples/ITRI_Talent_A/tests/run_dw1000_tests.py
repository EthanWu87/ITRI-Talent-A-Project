"""Compile real-driver regression tests and run with fake time/UART on Unicorn ARM.
Dependencies: python -m pip install --target mdk_v5/dw1000_verification/test_tools unicorn pyelftools
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

source = (ROOT / 'src/ei_main.cpp').read_text(encoding='utf-8')
app = source.split('// BEGIN APPLICATION NETWORK PROTOCOL', 1)[1].split('// END APPLICATION NETWORK PROTOCOL', 1)[0]
source = (ROOT / 'src/bmc81m001.c').read_text(encoding='utf-8')
transport = source.split('// BEGIN BMC NONBLOCKING TCP', 1)[1].split('// END BMC NONBLOCKING TCP', 1)[0]
header = (ROOT / 'inc/bmc81m001.h').read_text(encoding='utf-8').split('// BEGIN BMC TCP API', 1)[1].split('// END BMC TCP API', 1)[0]
(OUT / 'bmc_tcp_api_under_test.inc').write_text(header, encoding='utf-8')
(OUT / 'wifi_transport_under_test.inc').write_text(transport + app, encoding='utf-8')
compiler = Path('C:/Program Files (x86)/Arm GNU Toolchain arm-none-eabi/14.2 rel1/bin/arm-none-eabi-gcc.exe')
elf_path = OUT / 'dw1000_nonblocking_test.elf'
subprocess.run([str(compiler), '-mcpu=arm926ej-s', '-marm', '-O0', '-g', '-I'+str(ROOT/'inc'),
               '-nostartfiles', '--specs=nosys.specs', '-Wl,-Ttext=0x10000,-Tdata=0x20000000,-e,main',
               str(ROOT/'tests/dw1000_nonblocking_test.c'), '-lm', '-o', str(elf_path)], check=True)
uc = Uc(UC_ARCH_ARM, UC_MODE_ARM)
uc.mem_map(0x10000, 0x200000)
uc.mem_map(0x20000000, 0x200000)
with elf_path.open('rb') as handle:
    elf = ELFFile(handle)
    for segment in elf.iter_segments():
        if segment['p_type'] == 'PT_LOAD':
            uc.mem_write(segment['p_vaddr'], segment.data())
    symtab = elf.get_section_by_name('.symtab')
    symbols = {symbol.name: symbol['st_value'] for symbol in symtab.iter_symbols()}
    entry = elf.header['e_entry']
stop = 0x20000C
uc.reg_write(UC_ARM_REG_SP, 0x201FFFF0)
uc.reg_write(UC_ARM_REG_LR, stop)
finished = [False]
def on_code(emulator, address, size, _):
    if address == stop:
        finished[0] = True
        emulator.emu_stop()
uc.hook_add(UC_HOOK_CODE, on_code, begin=stop, end=stop)
uc.emu_start(entry, stop+4, count=30000000)
def word(name):
    return int.from_bytes(uc.mem_read(symbols[name],4),'little')
if not finished[0]:
    raise AssertionError(f'Driver did not return (instruction limit), test {word("test_number")+1}')
if uc.reg_read(UC_ARM_REG_R0):
    raise AssertionError(f'Test {word("test_number")+1} failed at C line {word("test_failure")}')
print(f'PASS: {word("test_number")} real-driver ARM simulation scenarios; no real clock or UART hardware.')

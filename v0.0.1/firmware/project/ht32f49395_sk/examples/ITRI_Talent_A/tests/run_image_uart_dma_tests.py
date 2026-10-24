"""Test production image DMA and EI UART code with controlled peripheral state.

Requires a 32-bit-capable MinGW g++; pass its path as the first argument.
No board, serial port, or third-party Python packages are required.
"""
from pathlib import Path
import subprocess
import sys

root = Path(__file__).resolve().parents[1]
out = root / 'mdk_v5' / 'dma_verification'
out.mkdir(parents=True, exist_ok=True)
app = (root / 'src/ei_main.cpp').read_text(encoding='utf-8')
dma = app.split('// BEGIN IMAGE UART DMA', 1)[1].split('// END IMAGE UART DMA', 1)[0]
(out / 'image_uart_under_test.inc').write_text(dma, encoding='utf-8')
port = (root / 'src/ei_classifier_porting.cpp').read_text(encoding='utf-8')
uart = port[port.index('typedef enum'):port.index('EI_IMPULSE_ERROR ei_run_impulse_check_canceled')]
(out / 'ei_uart_under_test.inc').write_text(uart, encoding='utf-8')
compiler = sys.argv[1] if len(sys.argv) > 1 else r'C:/Program Files (x86)/Dev-Cpp/MinGW64/bin/g++.exe'
exe = out / 'image_uart_dma_test.exe'
subprocess.run([compiler, '-m32', '-std=c++11', '-O0', '-Wall', '-Wextra', '-Werror',
                '-static', '-I' + str(root / 'inc'), '-I' + str(out),
                str(root / 'tests/image_uart_dma_test.cpp'), '-o', str(exe)], check=True)
subprocess.run([str(exe)], check=True)

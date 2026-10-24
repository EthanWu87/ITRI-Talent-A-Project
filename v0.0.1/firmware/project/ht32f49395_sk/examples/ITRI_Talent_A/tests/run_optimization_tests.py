"""Run real driver logic on a 32-bit host with fake UART/DMA/time.

Uses MinGW, no extra Python packages. Also refreshes the generated includes used
by the existing ARM/Unicorn runners. Pass the MinGW bin directory optionally.
"""
from pathlib import Path
import subprocess
import sys

root = Path(__file__).resolve().parents[1]
out = root / 'mdk_v5/dw1000_verification'
out.mkdir(parents=True, exist_ok=True)
binpath = Path(sys.argv[1]) if len(sys.argv) > 1 else Path('C:/Program Files (x86)/Dev-Cpp/MinGW64/bin')
app = (root/'src/ei_main.cpp').read_text()
bmc = (root/'src/BMC81M001.c').read_text()
header = (root/'inc/BMC81M001.h').read_text()
(out/'bmc_tcp_api_under_test.inc').write_text(header.split('// BEGIN BMC TCP API')[1].split('// END BMC TCP API')[0])
(out/'wifi_transport_under_test.inc').write_text(
    bmc.split('// BEGIN BMC NONBLOCKING TCP')[1].split('// END BMC NONBLOCKING TCP')[0] +
    app.split('// BEGIN APPLICATION NETWORK PROTOCOL')[1].split('// END APPLICATION NETWORK PROTOCOL')[0])
header = (root/'inc/mr60bha2.h').read_text().replace('#include "ht32f493x5_board.h"','')
(out/'mr60bha2_types_under_test.h').write_text(header)
driver = (root/'src/mr60bha2.c').read_text().replace('#include "mr60bha2.h"','#include "mr60bha2_types_under_test.h"').replace('#include "ht32f493x5_int.h"','')
a = driver.index('static void mr60bha2_usart_configuration(void)\n{')
b = driver.index('static void mr60bha2_reset_parser(void)\n{', a)
driver = driver[:a] + 'static void mr60bha2_usart_configuration(void) {}\n' + driver[b:]
(out/'mr60bha2_under_test.inc').write_text(driver)
constants = app[app.index('#define HM01B0_INITIAL_FALL_MS'):app.index('extern uint8_t hm01b0_frame_buffer')]
flow = app[app.index('// Radar vital signs are reference-only'):app.index('int raw_feature_get_data')]
(out/'vital_signs_flow_under_test.inc').write_text(constants+flow)
timer = (root/'src/ht32f493x5_int.c').read_text()
(out/'time_under_test.inc').write_text(timer[timer.index('uint64_t SysTick_GetTimeUs'):timer.index('void SysTick_DelayMs')])
port = (root/'src/ei_classifier_porting.cpp').read_text()
(out/'format_under_test.inc').write_text(port[port.index('void ei_printf('):port.index('void *ei_malloc')])
for source, cpp in [('dw1000_nonblocking_test.c',False),('vital_signs_test.cpp',True),('optimization_test.cpp',True)]:
    exe = out/(Path(source).stem+'.exe')
    target = root/'tests'/source
    if source != 'optimization_test.cpp':
        # Existing ARM suites return the failing line via globals; report it on host.
        target = out/('host_'+source)
        prefix = '#define FLOW_LIGHTWEIGHT_TEST 1\n' if source == 'vital_signs_test.cpp' else ''
        target.write_text(prefix + '#define main suite_main\n#include "'+(root/'tests'/source).as_posix()+
            '"\n#undef main\nint main(void) { int r=suite_main(); '
            'printf("%s: '+source+' scenarios=%u failure_line=%u\\n",r?"FAIL":"PASS",test_number,test_failure); return r; }\n')
    subprocess.run([str(binpath/('g++.exe' if cpp else 'gcc.exe')), '-m32',
                    '-std=c++11' if cpp else '-std=c99', '-D__USE_MINGW_ANSI_STDIO=1', '-O0','-g','-static',
                    '-I'+str(root/'inc'),'-I'+str(out),str(target),'-lm','-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)

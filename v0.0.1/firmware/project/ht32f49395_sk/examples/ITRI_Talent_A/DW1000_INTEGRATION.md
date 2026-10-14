# DW1000 integration

UART8 uses PE1 TX / PE0 RX, 115200 8N1. UART4 uses the existing BMC81M001 PC10/PC11 configuration. TX/RX GPIO definitions follow MR60BHA2. Interrupt handlers are centralized in `ht32f493x5_int.c`: RX bytes go into rings and TX-empty interrupts drain queued command bytes.

`dw1000_process()` runs in the main loop and after each complete `INF_E` packet. Every call performs bounded RX parsing and advances state machines, then returns. It never waits for elapsed time, UART TX readiness, an AT reply, Wi-Fi connection, TCP send acknowledgement, or UWB reboot. SysTick supplies elapsed-time checks; image inference and radar processing continue between calls. Polling may be delayed by the inference itself, so reply handling occurs on the next service pass.

## Wi-Fi and UWB sequence

Set node/network settings in `inc/dw1000.h` or compiler definitions; defaults follow the supplied example. Wi-Fi progresses through AT, ATE0, CWMODE, CWJAP, CIPMUX and CIPSTART. The AP timeout is 20 seconds and TCP timeout is 10 seconds, but these deadlines do not suspend the main loop. Failure/disconnection stops measurement, drops old queued events, cancels pending role work and schedules reconnection after at least 3 seconds.

UWB sends no AT before the PC requests a role over the connected TCP session. PC commands remain `ROLE,<seq>,TAG,0`, `ROLE,<seq>,ANCHOR,<id>`, `START,<seq>,ABAC|BC|TRACK`, `STOP,<seq>`, `PING,<seq>` (newline terminated). Role AT replies, retries and reboot recovery are asynchronous. READY is queued only after verification succeeds; failure queues ERROR. The PC must wait for the reply before sending the next command.

Replies remain `NODE_<id>,<kind>,<seq>,<value>`. An eight-entry FIFO serializes CIPSEND/prompt/payload/SEND OK transactions. ABAC/BC discard two warmup readings and use the median of five; TRACK maps an2/an1/an0 to TA/TB/TC. Queue acceptance is required before a distance is marked sent. On network failure the PC must restart the measurement after reconnection.

## Ownership and timing

UART4 has one consumer: do not mix this service with BMC81M001 AT/MQTT/TCP helpers. Receive buffering is finite (2048 bytes per UART); sufficiently long inference or excessive sensor traffic can still overflow RX. This change removes network/UWB waits, not the execution time of image inference itself.

The board delay_ms/delay_us/delay_sec functions modify/disable SysTick and must not be called after the runtime alarm tick starts. Camera reset and Edge Impulse sleep use SysTick_DelayMs. DW1000 has no delay calls. PB1 buzzer and 2/5-second alarm thresholds continue using the running SysTick interrupt.

Both Keil project variants include dw1000.c. Hardware wiring, ESP-AT compatibility and timing under full sensor load require board validation.

## AP rejoin notifications

During AT/CWMODE/CWJAP setup, transient WIFI DISCONNECT/CLOSED notifications do not restart the state machine; the pending command still needs its final OK, or fails on ERROR/FAIL/timeout. After CWJAP succeeds (CIPMUX/CIPSTART/online), disconnect notifications still abort the session and schedule a retry. This handles both combined and fragmented DISCONNECT -> CONNECTED -> GOT IP -> OK responses.

UART1 logs the connection stage, the CIPSTART server address/port, TCP connection success, and timeout/error/disconnection reasons. AP credentials are not logged. Regression tests cover AP rejoin notifications as well as genuine connection loss after joining.

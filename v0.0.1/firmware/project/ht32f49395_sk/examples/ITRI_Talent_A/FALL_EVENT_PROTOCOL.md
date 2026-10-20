# Fall events over BMC81M001 TCP

## Ownership and destination

- `src/ei_main.cpp` owns PC command parsing/dispatch, control and fall packet formatting, snapshots. `inc/ei_main.h` exposes its UWB reply bridge and `FLOW_NODE_ID`.
- `inc/bmc81m001.h` defines the network configuration and generic nonblocking TCP API. `BMC81M001_SERVER_IP` defaults to `172.20.10.3`; `BMC81M001_SERVER_PORT` is **5000**. WiFi credentials use `BMC81M001_WIFI_SSID` / `BMC81M001_WIFI_PASSWORD`. The former `flow_wifi.h` has been removed.
- `src/bmc81m001.c` owns UART4, RX buffering, WiFi/TCP connection/reconnect, binary queues and chunked transmission. UART4 ISR calls `BMC81M001_TxIrqHandler()`.
- `BMC81M001_QueueData(data, length)` copies a short record while connected; these records are discarded on disconnect. `BMC81M001_QueuePacket(parts, count, token)` copies byte spans into one retained packet. The driver treats data and tokens as opaque and never formats `Node_A_S` or UWB messages. All spans are copied before the function returns; stack-allocated header/footer buffers are safe.
- Transport callbacks deliver received bytes, connection changes and completed packet tokens to the application in main context. Application callbacks assemble PC lines, dispatch to UWB and log fall delivery. Do not use the legacy blocking AT/TCP/MQTT APIs concurrently with this service.
- `src/dw1000.c` owns UART8, UWB role commands and ranging. It calls the application `flow_wifi_send_control()` bridge to format replies, then the BMC driver queues the bytes.
- `ei_main()` initializes the transport callbacks and services both networking and UWB in its loop and after inference. Neither service waits synchronously for a network response.
- UWB commands and fall events share the same TCP connection. PC ROLE commands remain required before UWB role setup. Fall events do not require an assigned UWB role.

## Wire format

```text
Node_A_S,<L1|L2>,<heart_rate>,<breath_rate>,<9216 raw image bytes>,Node_A_E\r\n
```

The angle-bracket fields are placeholders. The image is a 96 x 96, 8-bit grayscale frame in row-major order, copied from the inference input at the alarm transition. It is not JPEG, hex or Base64. Rates are the current valid radar reference averages, formatted with two decimal places; invalid reference data is encoded as `N/A,N/A`. `FLOW_NODE_ID` selects the node letter (default A).

Example headers for two separate events:

```text
Node_A_S,L1,76.40,19.80,
Node_A_S,L2,N/A,N/A,
```

Each header is immediately followed by its own 9216-byte image and the exact footer `,Node_A_E\r\n`.

## Event semantics

Entering L1 sends one notification; escalating to L2 sends another. There is no event ID. State and timing are shared by the whole scene, not tracked per person. A second person falling while L1 is already active does not send another L1 or restart the timer. Remaining in either level does not generate repeats. A frame without a qualifying fall resets the state, allowing a later detection to start again.

The current image fall thresholds remain L1 at 3 seconds and L2 at 10 seconds from the start of continuous detection, confidence >= 0.80. They are configured by `HM01B0_INITIAL_FALL_MS` and `HM01B0_FALL_HOLD_MS` in `ei_main.cpp`.

Two packet slots retain snapshots while offline or sending. If both are occupied, a new notification is dropped and logged on UART1. This bounded queue cannot retain an unlimited outage history. The internal transport token carries only the alarm level for completion logging; it is not transmitted.

## PC receiver requirements

1. Buffer TCP bytes across `recv()` calls; TCP can split or combine records arbitrarily.
2. At a record boundary, distinguish existing UWB `NODE_A,...\n` lines from `Node_A_S,` binary event records (case matters).
3. For an event, read through the first four commas to parse its header.
4. Read exactly 9216 bytes as image data, then verify the footer. Never split the image by comma/newline or search it for the footer: every byte value, including matching delimiter text, is allowed in an image.
5. Preserve unconsumed bytes for subsequent records. On socket disconnect, discard an incomplete record.

WiFi sends chunks of at most 2048 bytes using explicit lengths. UWB replies are queued until a whole image record finishes, so PC reply timeouts must allow for image transmission. A record is never intentionally interleaved with a control reply.

A failed send retains the notification, closes the old socket and retries the complete record on a reconnected stream. The receiver must tolerate duplicates. Without an event identifier, this protocol cannot reliably distinguish retries from new notifications with identical content.

`[FALL EVENT] ... queued` means the snapshot entered RAM. `[FALL TX] ... sent` means all chunks received module `SEND OK`; it is not an application acknowledgement that the PC parsed or stored the event. There is no exactly-once delivery guarantee.

The existing USB/UART1 viewer is separate from the TCP server. This change updates firmware transmission; the server on port 5000 must implement the mixed text/binary parser above.

## Verification

`tests/run_dw1000_tests.py` extracts the actual TCP implementation from `bmc81m001.c` and application protocol from `ei_main.cpp`, combining them with the real UWB driver under simulated UART/time. Tests cover binary data, immutable snapshots, no reply interleaving, offline queues and reconnect replay alongside UWB scenarios. `tests/run_vital_signs_tests.py` verifies radar validity and L1/L2 transitions and scene-level behavior with two fallen people. Hardware/network delivery still needs board-to-PC validation.

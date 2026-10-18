# MR60BHA2 reference-only vital signs

Heart and breath readings no longer trigger reports or the buzzer. Image fall
detection retains its independent configured timers (currently 3000/10000 ms).

| Wire status | Display | Meaning |
| --- | --- | --- |
| absent | -- / no person | A fresh presence frame reports no human. |
| reference | Numeric values / reference only | All conditions below are satisfied. |
| multiple | -- / multiple people, not applicable | A fresh target-info frame reports 2 or 3 targets. |
| invalid | -- / invalid measurement | Unknown, missing, moving, out of range, warming up, or stale. |

Reference display requires:
- Fresh presence = 1 and exactly one tracked target (0x0A04, NOT raw point count).
- Doppler index = 0. Nonzero values, including negative indices, invalidate data.
- Fresh distance frame with flag = 1 and finite distance > 0 and <= 150 cm.
- Positive finite heart AND breath readings. Zero means no usable reference
  measurement here; it does not generate a physiological alarm.
- A full 3000 ms acquisition window for each channel, with at least two samples
  inside the current window. The displayed numbers are 3-second sample means.

Metadata and rate freshness use 3000 ms. Every checked frame can invalidate the
history, including absence/multiple/movement followed by a valid frame within one
main-loop batch. Cluster ID changes also clear the history. Reentry therefore
requires new samples. Missing target-info support stays invalid, not assumed
single-person. There is no 6-second alarm grace period or 10-second display timer.

UART protocol:
    VITALS: heart=80.00000 breath=20.00000 status=reference
    VITALS: heart=N/A breath=N/A status=absent

The line remains inside INF_S/INF_E. Update firmware and the Python viewer
together: the new viewer treats older lines without status as invalid. The viewer
shows Chinese status labels using Pillow and Microsoft JhengHei on Windows,
with English fallback when a CJK font is unavailable. It also clears the numeric
display after 3 seconds without a complete image/inference result.

Install viewer dependencies:
    python -m pip install pyserial numpy opencv-python pillow

Implementation notes:
- Presence (0x0F09) accepts 1-byte and 2-byte payloads; its first byte is the
  0/1 presence state. The board capture 01 36 B8 00 02 0F 09 74 01 00 FE
  is a valid two-byte presence frame, covered verbatim in regression tests.
- TinyFrame header integers remain big-endian; payload uint32 values and float
  values are little-endian, as specified in the manual.
- Target info is a uint32 count followed by count 16-byte x/y, Doppler, cluster
  records. Length, count and finite coordinates are checked before accepting.
- The ISR timestamps incoming bytes. Frame arrival time is the checksum-byte
  timestamp, so queued data does not become fresh just because inference delayed
  parsing. The 1024-byte RX ring detects overflow and invalidates context.
- The averaging helper stores at most 64 samples per channel. Overflow restarts
  acquisition rather than presenting a truncated measurement window as valid.
- Main-loop service remains nonblocking and runs before and after inference.

Limitations:
Presence and target counting are sensor estimates; they can lag physical entry
or exit. Doppler zero is a reported motion indicator, not proof of complete
stillness, and correct mounting/single-subject conditions still matter. The
firmware cannot identify a repeatedly transmitted held value as old physical
data. These values are reference information only. No module firmware upgrade
or configuration changes are automatically performed.

Sources:
- https://wiki.seeedstudio.com/ha_with_mr60bha2/
- https://wiki.seeedstudio.com/getting_started_with_mr60bha2_mmwave_kit/
- https://github.com/Seeed-Projects/Seeed-mmWave-library
- https://files.seeedstudio.com/wiki/mmwave-for-xiao/mr60/datasheet/Seeed_Studio_Tiny_Frame_Interface_Breathing_and_Heartbeat.pdf

Validation:
- tests/run_vital_signs_tests.py compiles the real UART parser and state logic,
  plus telemetry and image alarm code, with fake hardware/time on Unicorn ARM.
- tests/test_vital_signs_viewer.py covers protocol gating, old/invalid packets,
  stream fragmentation, local expiry and rendering of all four status labels.

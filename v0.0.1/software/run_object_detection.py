#!/usr/bin/env python3
"""HM01B0 + Edge Impulse object detection serial viewer.

安裝: python -m pip install pyserial numpy opencv-python
執行: python run_object_detection.py --port COM3
離開: Q / Esc，或關閉視窗。
結果分頁: A / D 或左右方向鍵；左側始終顯示所有偵測框。

韌體協定（與提供的 ei_main.cpp 相容）:
IMG_S + WIDTH*HEIGHT bytes 灰階影像 + IMG_E
INF_S + Predictions (...) 與偵測文字 + INF_E
例: person (0.95000) [ x: 16, y: 24, width: 24, height: 32 ]

必須使用真正的 Edge Impulse 物件偵測模型；僅修改巨集不能轉換模型。
WIDTH / HEIGHT 必須與韌體實際傳送的裁切影像尺寸一致。
FOMO 回傳的區域不是物件完整外形，可使用 --centroids 顯示中心點。
FPS 是完整影像與推論結果的接收更新率，不是感測器原始幀率。
"""
import argparse
import math
import re
import time

SERIAL_PORT = 'COM3'
BAUD_RATE = 2250000
WIDTH = 96
HEIGHT = 96
DISPLAY_SCALE = 4
CONFIDENCE_THRESHOLD = 0.0  # 額外顯示門檻，無法恢復模型已過濾的偵測
PACKET_TIMEOUT = 30.0      # 應大於最長推論時間（秒）
PANEL_WIDTH = 560

WINDOW_NAME = 'HM01B0 + Edge Impulse Object Detection'

NUMBER = r'[+-]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?'
TIMING_RE = re.compile(
    r'Predictions\s*\(DSP:\s*(\d+)\s*ms\.,\s*'
    r'Classification:\s*(\d+)\s*ms\.,\s*Anomaly:\s*(\d+)\s*ms\.\)'
)
BOX_RE = re.compile(
    r'^[ \t]*(.+?)[ \t]+\((' + NUMBER + r')\)[ \t]*\[[ \t]*'
    r'x:\s*(\d+),\s*y:\s*(\d+),\s*width:\s*(\d+),\s*height:\s*(\d+)\s*\]',
    re.MULTILINE
)


def parse_prediction(text, threshold=CONFIDENCE_THRESHOLD):
    """回傳 timing 與偵測串列；保留同類別的多個物件。"""
    timing = TIMING_RE.search(text)
    if timing is None:
        raise ValueError('Missing or invalid Predictions timing header')
    dsp, inference, anomaly = map(int, timing.groups())
    detections = []
    for match in BOX_RE.finditer(text):
        label, value, x, y, width, height = match.groups()
        value = float(value)
        if not math.isfinite(value) or not 0 < value <= 1 or value < threshold:
            continue
        x, y, width, height = map(int, (x, y, width, height))
        if width <= 0 or height <= 0:
            continue
        detections.append(dict(label=label.strip(), value=value, x=x, y=y,
                               width=width, height=height))
    return dsp, inference, anomaly, detections


class StreamParser:
    """依固定影像長度解析，避免將像素內的標記誤認為封包邊界。"""
    def __init__(self, frame_size, timeout=PACKET_TIMEOUT):
        self.frame_size = frame_size
        self.timeout = timeout
        self.buffer = bytearray()
        self.state = 'image_header'
        self.frame = None
        self.started = None

    def reset(self):
        self.buffer.clear()
        self.state = 'image_header'
        self.frame = None
        self.started = None

    def feed(self, chunk):
        now = time.monotonic()
        if self.started is not None and now - self.started > self.timeout:
            print('Packet timeout: resynchronizing stream.')
            self.reset()
        self.buffer.extend(chunk)
        pairs = []
        while True:
            if self.state == 'image_header':
                index = self.buffer.find(b'IMG_S')
                if index < 0:
                    self.buffer[:] = self.buffer[-4:]
                    break
                del self.buffer[:index + 5]
                self.state = 'image_body'
                self.started = now

            elif self.state == 'image_body':
                if len(self.buffer) < self.frame_size + 5:
                    break
                if self.buffer[self.frame_size:self.frame_size + 5] != b'IMG_E':
                    print('Invalid IMG_E: check image dimensions or serial data loss.')
                    self.state = 'image_header'
                    self.frame = None
                    self.started = None
                    continue
                self.frame = bytes(self.buffer[:self.frame_size])
                del self.buffer[:self.frame_size + 5]
                self.state = 'inference_header'

            elif self.state == 'inference_header':
                inf = self.buffer.find(b'INF_S')
                next_img = self.buffer.find(b'IMG_S')
                if next_img >= 0 and (inf < 0 or next_img < inf):
                    print('Missing inference result: discarding unmatched image.')
                    del self.buffer[:next_img]
                    self.state = 'image_header'
                    self.frame = None
                    continue
                if inf < 0:
                    self.buffer[:] = self.buffer[-4:]
                    break
                del self.buffer[:inf + 5]
                self.state = 'inference_body'

            else:
                end = self.buffer.find(b'INF_E')
                next_img = self.buffer.find(b'IMG_S')
                if next_img >= 0 and (end < 0 or next_img < end):
                    print('Interrupted inference packet: resynchronizing stream.')
                    del self.buffer[:next_img]
                    self.state = 'image_header'
                    self.frame = None
                    continue
                if end < 0:
                    if len(self.buffer) > 262144:
                        print('Inference packet too large: discarding packet.')
                        self.reset()
                    break
                text = bytes(self.buffer[:end]).decode('utf-8', errors='replace')
                del self.buffer[:end + 5]
                pairs.append((self.frame, text))
                self.frame = None
                self.state = 'image_header'
                self.started = None
        return pairs


def page_layout(display_height, count, page=0):
    """Fixed-height table with reserved header and footer space."""
    rows = max(1, (display_height - 170) // 26)
    pages = max(1, (count + rows - 1) // rows)
    return rows, pages, max(0, min(page, pages - 1))


def draw_prediction_panel(frame, dsp, inference, anomaly, detections, fps,
                          scale=DISPLAY_SCALE, centroids=False, page=0):
    import cv2
    import numpy as np

    height, width = frame.shape
    display = cv2.cvtColor(cv2.resize(frame, (width * scale, height * scale),
                                    interpolation=cv2.INTER_NEAREST), cv2.COLOR_GRAY2BGR)
    for index, box in enumerate(detections, 1):
        x1, y1 = max(0, box['x']), max(0, box['y'])
        x2, y2 = min(width, box['x'] + box['width']), min(height, box['y'] + box['height'])
        if x1 >= x2 or y1 >= y2:
            continue
        if centroids:
            point = (int((x1 + x2) * scale / 2), int((y1 + y2) * scale / 2))
            cv2.drawMarker(display, point, (0, 255, 0), cv2.MARKER_CROSS, 12, 2)
        else:
            cv2.rectangle(display, (x1 * scale, y1 * scale),
                          (x2 * scale - 1, y2 * scale - 1), (0, 255, 0), 2)
        cv2.putText(display, f'#{index}', (x1 * scale, max(16, y1 * scale - 5)),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 1)

    display_height = height * scale
    canvas = np.zeros((display_height, width * scale + PANEL_WIDTH, 3), dtype=np.uint8)
    canvas[:, :width * scale] = display
    panel = canvas[:, width * scale:]
    panel[:] = (24, 24, 24)
    font = cv2.FONT_HERSHEY_SIMPLEX

    def put(text, x, y, size=0.5, color=(235, 235, 235), max_width=None):
        # Clip long labels within their column; full labels remain in console.
        text = str(text).encode('ascii', errors='replace').decode('ascii')
        if max_width is not None and cv2.getTextSize(text, font, size, 1)[0][0] > max_width:
            while text and cv2.getTextSize(text + '...', font, size, 1)[0][0] > max_width:
                text = text[:-1]
            text += '...'
        cv2.putText(panel, text, (x, y), font, size, color, 1, cv2.LINE_AA)

    put('Object Detection', 16, 28, 0.65)
    put(f'FPS: {fps:.2f} | DSP: {dsp} ms | Inference: {inference} ms', 16, 55,
        max_width=PANEL_WIDTH - 32)
    put(f'Anomaly: {anomaly} ms | Detections: {len(detections)}', 16, 80)
    columns = (16, 65, 200, 310, 430)
    for x, title in zip(columns, ('ID', 'Label', 'Confidence', 'x, y', 'w, h')):
        put(title, x, 112, color=(170, 190, 210))
    cv2.line(panel, (16, 121), (PANEL_WIDTH - 16, 121), (80, 80, 80), 1)
    rows, pages, page = page_layout(display_height, len(detections), page)
    start = page * rows
    for offset, box in enumerate(detections[start:start + rows]):
        y = 144 + offset * 26
        values = (f'#{start + offset + 1}', box['label'], f"{box['value'] * 100:.2f}%",
                  f"{box['x']}, {box['y']}", f"{box['width']}, {box['height']}")
        for x, value, available in zip(columns, values, (43, 125, 100, 110, 114)):
            put(value, x, y, max_width=available)
    if not detections:
        put('No objects above display threshold', 16, 144)
    cv2.line(panel, (16, display_height - 34), (PANEL_WIDTH - 16, display_height - 34),
             (80, 80, 80), 1)
    put(f'Page {page + 1}/{pages} | A/D or Left/Right | Q: quit',
        16, display_height - 12, 0.45)

    return canvas


def main():
    args_parser = argparse.ArgumentParser(description=__doc__,
                                          formatter_class=argparse.RawDescriptionHelpFormatter)
    args_parser.add_argument('--port', default=SERIAL_PORT)
    args_parser.add_argument('--baud', type=int, default=BAUD_RATE)
    args_parser.add_argument('--width', type=int, default=WIDTH)
    args_parser.add_argument('--height', type=int, default=HEIGHT)
    args_parser.add_argument('--scale', type=int, default=DISPLAY_SCALE)
    args_parser.add_argument('--threshold', type=float, default=CONFIDENCE_THRESHOLD)
    args_parser.add_argument('--timeout', type=float, default=PACKET_TIMEOUT)
    args_parser.add_argument('--centroids', action='store_true', help='Draw centers instead of boxes (FOMO)')
    args = args_parser.parse_args()
    if min(args.width, args.height, args.scale, args.baud) <= 0:
        args_parser.error('Dimensions, scale and baud must be positive')
    if not math.isfinite(args.timeout) or args.timeout <= 0 or not 0 <= args.threshold <= 1:
        args_parser.error('Timeout must be positive and finite; threshold must be between 0 and 1')
    if args.height * args.scale < 200:
        args_parser.error('Display height must be at least 200 pixels; increase --scale')
    try:
        import serial
        import numpy as np
        import cv2
    except ImportError as error:
        print(f'Missing dependency: {error}\nInstall: python -m pip install pyserial numpy opencv-python')
        return 1

    parser = StreamParser(args.width * args.height, args.timeout)
    last_result_time = None
    page = 0
    latest = None
    try:
        with serial.Serial(args.port, args.baud, timeout=0.05) as ser:
            print(f'Connected: {args.port} @ {args.baud}. Press Q or Esc to quit.')
            cv2.namedWindow(WINDOW_NAME, cv2.WINDOW_NORMAL)
            cv2.resizeWindow(WINDOW_NAME, args.width * args.scale + PANEL_WIDTH, args.height * args.scale)
            waiting = np.zeros((args.height * args.scale, args.width * args.scale + PANEL_WIDTH, 3), dtype=np.uint8)
            cv2.putText(waiting, 'Waiting for image + inference...', (20, 80),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 1)
            cv2.imshow(WINDOW_NAME, waiting)
            while True:
                chunk = ser.read(min(ser.in_waiting, 65536) or 1)
                for raw_frame, text in parser.feed(chunk):
                    try:
                        dsp, inference, anomaly, detections = parse_prediction(text, args.threshold)
                    except ValueError as error:
                        print(f'Prediction parse error: {error}')
                        continue
                    now = time.monotonic()
                    fps = 1.0 / (now - last_result_time) if last_result_time is not None else 0.0
                    last_result_time = now
                    frame = np.frombuffer(raw_frame, dtype=np.uint8).reshape(args.height, args.width).copy()
                    latest = (frame, dsp, inference, anomaly, detections, fps)
                    _, _, page = page_layout(args.height * args.scale, len(detections), page)
                    display = draw_prediction_panel(*latest, args.scale, args.centroids, page)
                    cv2.imshow(WINDOW_NAME, display)
                    print(f'\nDSP: {dsp} ms | Inference: {inference} ms | Objects: {len(detections)}')
                    for box in detections:
                        print(f"  {box['label']} ({box['value']:.5f}) "
                              f"[ x: {box['x']}, y: {box['y']}, width: {box['width']}, height: {box['height']} ]")
                key = cv2.waitKeyEx(1)
                if key in (ord('q'), ord('Q'), 27):
                    break
                previous_keys = (ord('a'), ord('A'), 2424832, 65361, 63234)
                next_keys = (ord('d'), ord('D'), 2555904, 65363, 63235)
                if latest is not None and key in previous_keys + next_keys:
                    _, pages, page = page_layout(args.height * args.scale, len(latest[4]), page)
                    page = (page + (1 if key in next_keys else -1)) % pages
                    cv2.imshow(WINDOW_NAME, draw_prediction_panel(*latest, args.scale, args.centroids, page))
                if cv2.getWindowProperty(WINDOW_NAME, cv2.WND_PROP_VISIBLE) < 1:
                    break
    except KeyboardInterrupt:
        print('\nStopped.')
    except (serial.SerialException, OSError, cv2.error) as error:
        print(f'Error: {error}')
        return 1
    finally:
        cv2.destroyAllWindows()
    return 0


if __name__ == '__main__':
    raise SystemExit(main())

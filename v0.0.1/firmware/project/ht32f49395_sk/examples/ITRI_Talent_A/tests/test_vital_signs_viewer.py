"""Viewer protocol/expiry/rendering regression tests. No serial hardware required."""
import importlib.util
import sys
from pathlib import Path
import unittest

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parents[1]
VIEWER = ROOT.parents[4] / 'software/run_object_detection.py'
spec = importlib.util.spec_from_file_location('radar_viewer', VIEWER)
viewer = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = viewer
spec.loader.exec_module(viewer)


class ReferenceViewerTests(unittest.TestCase):
    def test_parse_new_prediction_summary(self):
        result = viewer.parse_prediction(
            'PRED,person,0.950,16,24,24,32,dsp=10,cls=20,anom=0\r\n')
        self.assertEqual(result[:3], (10, 20, 0))
        self.assertEqual(result[3], [{
            'label': 'person', 'value': 0.95, 'x': 16, 'y': 24,
            'width': 24, 'height': 32,
        }])

    def test_parse_new_empty_prediction_summary(self):
        self.assertEqual(
            viewer.parse_prediction('PRED,NONE,dsp=1,cls=2,anom=3\r\n'),
            (1, 2, 3, []))

    def test_explicit_reference_only(self):
        self.assertEqual(viewer.parse_vital_report(
            'VITALS: heart=80 breath=20 status=reference\r\n'),
            viewer.VitalSigns(80, 20, 'reference'))
        for status in ('absent', 'multiple', 'invalid'):
            report = viewer.parse_vital_report(f'VITALS: heart=80 breath=20 status={status}')
            self.assertEqual(report, viewer.VitalSigns(status=status))

    def test_old_missing_or_invalid_metadata(self):
        for line in ('', 'VITALS: heart=80 breath=20',
                     'VITALS: heart=80 breath=20 status=unknown',
                     'VITALS: heart=0 breath=20 status=reference',
                     'VITALS: heart=-1 breath=20 status=reference',
                     'VITALS: heart=1e999 breath=20 status=reference',
                     'VITALS: heart=80 breath=N/A status=reference'):
            self.assertEqual(viewer.parse_vital_report(line), viewer.VitalSigns())

    def test_latest_message_clears_old_value(self):
        text = ('VITALS: heart=80 breath=20 status=reference\n'
                'VITALS: heart=N/A breath=N/A status=absent\n')
        self.assertEqual(viewer.parse_vital_report(text), viewer.VitalSigns(status='absent'))
        self.assertEqual(viewer.format_rate(None), '--')

    def test_display_expires_without_packets(self):
        value = viewer.VitalSigns(80, 20, 'reference')
        self.assertEqual(viewer.visible_vitals(value, 10, 12.99), value)
        self.assertEqual(viewer.visible_vitals(value, 10, 13), viewer.VitalSigns())
        self.assertEqual(viewer.visible_vitals(value, None, 10), viewer.VitalSigns())

    def test_fragmented_image_inference_stream(self):
        parser = viewer.StreamParser(4)
        payload = (b'IMG_S\x01\x02\x03\x04IMG_E\r\nINF_S'
                   b'Predictions (DSP: 1 ms., Classification: 2 ms., Anomaly: 0 ms.):\n'
                   b'VITALS: heart=N/A breath=N/A status=multiple\r\nINF_E')
        pairs = []
        for byte in payload:
            pairs.extend(parser.feed(bytes([byte])))
        self.assertEqual(len(pairs), 1)
        self.assertEqual(viewer.parse_vital_report(pairs[0][1]).status, 'multiple')

    def test_render_all_four_states(self):
        import numpy as np
        import cv2
        self.assertIsNotNone(viewer.status_font())
        self.assertEqual(viewer.VITAL_STATUS_LABELS['multiple'], '\u591a\u4eba\uff0c\u4e0d\u9069\u7528')
        for status in viewer.VITAL_STATUS_LABELS:
            report = viewer.VitalSigns(80, 20, status) if status == 'reference' else viewer.VitalSigns(status=status)
            panel = viewer.draw_prediction_panel(np.zeros((96, 96), dtype=np.uint8),
                1, 2, 0, [], 3.0, vitals=report)
            self.assertEqual(panel.shape, (384, 944, 3))
            self.assertGreater(np.count_nonzero(panel[139:170, 384:] != 24), 100)
        preview = viewer.draw_prediction_panel(np.zeros((96, 96), dtype=np.uint8),
            1, 2, 0, [], 3.0, vitals=viewer.VitalSigns(status='multiple'))
        self.assertTrue(cv2.imwrite(str(ROOT/'mdk_v5/dw1000_verification/vitals_preview.png'), preview))


if __name__ == '__main__':
    unittest.main()

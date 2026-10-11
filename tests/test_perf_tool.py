import sys
import tempfile
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from perf_tool import read_samples, summary


class SessionFiltering(unittest.TestCase):
    def test_stopped_frames_and_copy_time(self):
        with tempfile.TemporaryDirectory() as temp:
            path=Path(temp)/'performance.csv'
            path.write_text('frame,update_ms,draw_ms,present_ms,wait_ms,frame_ms,copy_ms,running\n0,1,2,3,4,20,5,1\n1,1,2,3,4,25,6,1\n2,1,2,3,4,10,1,0\n')
            rows=read_samples(path,0)
            self.assertEqual(len(rows),2)
            self.assertEqual(summary(rows)['copy_mean_ms'],5.5)
            self.assertAlmostEqual(summary(rows)['fps'],1000/22.5)

    def test_legacy_runtime_boundary(self):
        with tempfile.TemporaryDirectory() as temp:
            path=Path(temp)/'performance.csv'
            path.write_text('frame,update_ms,draw_ms,present_ms,wait_ms,frame_ms\n0,1,2,3,4,20\n1,1,2,3,4,25\n2,1,2,3,4,10\n')
            path.with_name('runtime.log').write_text('native_frames=2 running=0\n')
            self.assertEqual([r['frame'] for r in read_samples(path,1)],[1])

if __name__=='__main__': unittest.main()

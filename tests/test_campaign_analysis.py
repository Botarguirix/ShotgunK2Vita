import sys
import unittest
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
from performance_campaign import compare


class CampaignAnalysis(unittest.TestCase):
    def setUp(self):
        self.group = dict(scene='board', workers=1, fingerprint='1234', median_mean_ms=10,
                          min_mean_ms=9.9, max_mean_ms=10.1, median_p95_ms=15)
        self.old = {'config': {'platform': 'same'}, 'groups': [self.group]}

    def verdict(self, **changes):
        return compare(self.old['config'], [dict(self.group, **changes)], self.old)[0]['verdict']

    def test_noise_is_not_improvement(self):
        self.assertEqual(self.verdict(median_mean_ms=9.9), 'inconclusive')
        self.assertEqual(self.verdict(median_mean_ms=8, min_mean_ms=5, max_mean_ms=11), 'inconclusive')

    def test_image_difference_rejects_gain(self):
        self.assertEqual(self.verdict(median_mean_ms=8, fingerprint='different'), 'different sequence: investigate')

    def test_gain_and_regression(self):
        self.assertEqual(self.verdict(median_mean_ms=8, min_mean_ms=7.9, max_mean_ms=8.1), 'candidate improvement')
        self.assertEqual(self.verdict(median_mean_ms=12, min_mean_ms=11.9, max_mean_ms=12.1), 'regression')

    def test_tail_regression_blocks_candidate(self):
        self.assertEqual(self.verdict(median_mean_ms=8, min_mean_ms=7.9, max_mean_ms=8.1, median_p95_ms=18), 'mean improves but P95 regresses: investigate')

    def test_different_host_rejected(self):
        with self.assertRaises(ValueError):
            compare({'platform': 'other'}, [self.group], self.old)


if __name__ == '__main__':
    unittest.main()

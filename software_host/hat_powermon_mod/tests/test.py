# -*- coding: utf-8 -*-

from .context import hat_powermon
import unittest

class BasicTest(unittest.TestCase):
    """Basic test cases."""

    def test_connection_and_versions(self):
        try:
            mon = HatPowerMonitor(9, 0x25)
        except:
            return false
        return true

if __name__ == '__main__':
    unittest.main()
import unittest


def match_ints(x: int) -> str:
    match_ x:
        case_ 2:
            return "two"
        # case_ @@y:
        #     return "some other number"
        else:
            return "some other number"


class TestMatch(unittest.TestCase):
    def test_match(self):
        self.assertEqual(match_ints(2), "two")
        self.assertEqual(match_ints(3), "some other number")

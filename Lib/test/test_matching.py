import unittest


class Match:
    def __init__(self, x):
        self.x = x

    @classmethod
    def __match__(cls, instance, x):
        if not isinstance(instance, cls):
            return False
        return instance.x == x

    def __repr__(self):
        return f'Match({self.x})'


def match_ints(x: int) -> str:
    match_ x:
        case_ 2:
            return "two"
        case_ @@y:
            return f"some other number: {y}"
        else:
            return "some other number"


def match_cls(x: object) -> str:
    match_ x:
        case_ Match(3):
            return "it's a Match(3)"
        case_ @@y:
            return f"it's something else: {y}"


class TestMatch(unittest.TestCase):
    def test_match(self):
        self.assertEqual(match_ints(2), "two")
        self.assertEqual(match_ints(3), "some other number: 3")

    def test_match_cls(self):
        self.assertEqual(match_cls(Match(3)), "it's a Match(3)")
        self.assertEqual(match_cls(Match(4)), "it's something else: Match(4)")

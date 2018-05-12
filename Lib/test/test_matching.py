import unittest

# Test support for matching. See the match_* functions below for example syntax.

# TODO
# - match_ and case_ are bad keywords, but using "match" and "case" breaks too much (re.match for
#   example). To get rid of case_, we can make the syntax just be:
#       match x:
#           @@y:
#               return 3
#
# - make at-assignment within more complicated matches work. Not sure how to do that in general yet.
#   High-level plan is to make __match__ return a tuple of assigned names. To support nested
#   matches, maybe we'll need to add a builtin Assign object that gets passed. So for example
#   to match obj = X(Y(3)) against X(Y(@@z)), we would do X.__match__(obj, Assign) -> (Y(3),),
#   then Y.__match__(Y(3), Assign) -> (3,), then set z to 3.


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


def not_supported_yet(x: object) -> str:
    match_ Match(3):
        case_ Match(@@x):
            assert x == 3

    match_ Match(Match(4)):
        case_ Match(Match(@@x)):
            assert x == 4

    match_ [3, 4]:
        case_ [@@x, @@y]:
            assert x == 3
            assert y == 4

    match_ {3: 4, 5: 6}:
        # Let's not support @@ in the keys though. Also, let's not support sets (those should just check for equality).
        case_ {3: @@x, 5: @@y}:
            assert x == 4
            assert y == 6

    match_ (3, 4):
        case_ [@@x, @@y]:
            assert x == 3
            assert y == 4
        case_ _:
            # matches anything else
            assert False


class TestMatch(unittest.TestCase):
    def test_match(self):
        self.assertEqual(match_ints(2), "two")
        self.assertEqual(match_ints(3), "some other number: 3")

    def test_match_cls(self):
        self.assertEqual(match_cls(Match(3)), "it's a Match(3)")
        self.assertEqual(match_cls(Match(4)), "it's something else: Match(4)")

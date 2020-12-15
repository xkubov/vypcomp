from compiler_tests import VYPaTestCase
import unittest


def load_tests(loader, tests, pattern):
    suite = unittest.TestSuite()
    for symbol_name in globals():
        symbol = globals()[symbol_name]
        if not hasattr(symbol, "__bases__"):
            continue
        bases = symbol.__bases__
        if len(bases) != 1:
            continue
        base = bases[0]
        if base.__name__ != "VYPaTestCase":
            continue
        tests = loader.loadTestsFromTestCase(symbol)
        for test_case in tests._tests:
            test_case.compiler_path = loader.vypcomp_path
            test_case.interpret_path = loader.vypint_path
        suite.addTests(tests)
    return suite


class HelloWorldCase(VYPaTestCase):
    input_file = "hello_world.vl"
    test_stdin = b""
    test_stdout = b"Hello, world!"


class HelloWorldAdvCase(VYPaTestCase):
    input_file = "hello_world_adv.vl"
    test_stdin = b""
    test_stdout = b"Hello, world!"


class HelloName(VYPaTestCase):
    input_file = "hello_io.vl"
    test_stdin = b"Richard"
    test_stdout = b"What's your name? Hello, Richard. Your name has length: 7"


class IfTestTaken(VYPaTestCase):
    input_file = "if_test1.vl"
    test_stdin = b""
    test_stdout = b"if taken\nif exited"


class IfTestNotTaken(VYPaTestCase):
    input_file = "if_test0.vl"
    test_stdin = b""
    test_stdout = b"else taken\nif exited"


class RecursiveTest(VYPaTestCase):
    input_file = "recursive_calls.vl"
    test_stdin = b"\n\n\ny"
    test_stdout = b"enter to cont, q to quit\ncontinue\nenter to cont, q to quit\ncontinue\nenter to cont, q to quit\ncontinue\nenter to cont, q to quit\nexiting"


class WhileTest(VYPaTestCase):
    input_file = "while.vl"
    test_stdin = b"a\nb\nc\n\n"
    test_stdout = b"string is not empty\nstring is not empty\nstring is not empty\nstring is not empty\nstring empty"


# TODO: compiler error tests:
#   - non-int/object type in if
#   - non-int/object type in while

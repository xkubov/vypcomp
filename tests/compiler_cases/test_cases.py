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


class AdditionTest(VYPaTestCase):
    input_file = "addition_simple.vl"
    test_stdin = b""
    test_stdout = b"5"


class AdditionAdvTest(VYPaTestCase):
    input_file = "addition_complex.vl"
    test_stdin = b""
    test_stdout = b"6987"


class NotExpressionTest(VYPaTestCase):
    input_file = "notexp.vl"
    test_stdin = b""
    test_stdout = b'TrueFalse'


class FibRecursive(VYPaTestCase):
    input_file = "fib_recursive.vl"
    test_stdin = b""
    test_stdout = b"fib(10) = 55\n"


class FunctionRedefinition(VYPaTestCase):
    input_file = "fun_redef.vl"
    test_return = 19


class MethodRedefinition(VYPaTestCase):
    input_file = "meth_redef.vl"
    test_return = 19


class StringConcatBasic(VYPaTestCase):
    input_file = "string_concat.vl"
    test_stdin = b""
    test_stdout = b"Ahoj, svete!"


class StringConcateAdv(VYPaTestCase):
    input_file = "string_concat_adv.vl"
    test_stdin = b""
    test_stdout = b"And I say: Yeah! Yeah! Yeah! Yeah! Yeah! ...\nYeah! Yeah! Yeah! \nI said: Yeah, what's going on?!"


class StringConcateUlt(VYPaTestCase):
    input_file = "string_concat_ult.vl"
    test_stdin = b""
    test_stdout = b"super duper test of strings"


class StringCast(VYPaTestCase):
    input_file = "string_cast.vl"
    test_stdin = b""
    test_stdout = b"69"


class ComparisonCase(VYPaTestCase):
    input_file = "comparison.vl"
    test_stdin = b""
    test_stdout = b"0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n"


class SubStrCase(VYPaTestCase):
    input_file = "subStr.vl"
    test_stdin = b""
    test_stdout = b'"llo, w"\n"llo, world!llll"\n""\n""\n""\n""\n'


class DefaultInitCase(VYPaTestCase):
    input_file = "default_init.vl"
    test_stdin = b""
    test_stdout = b'i == 0: True\nf == 0.0f: True\ns == "": True\n'


class FactorialIterCase(VYPaTestCase):
    input_file = "factorial_iter.vl"
    test_stdin = b"6"
    test_stdout = b'Enter an integer to compute its factorial:\nThe result is: 720\n'


class FactorialIterCaseNegative(VYPaTestCase):
    input_file = "factorial_iter.vl"
    test_stdin = b"-3"
    test_stdout = b'Enter an integer to compute its factorial:\nFactorial of a negative integer is undefined!\n'


class FactorialRecCase(VYPaTestCase):
    input_file = "factorial_rec.vl"
    test_stdin = b"6"
    test_stdout = b'Enter an integer to compute its factorial:\nThe result is: 720\n'


class FactorialRecCaseNegative(VYPaTestCase):
    input_file = "factorial_rec.vl"
    test_stdin = b"-3"
    test_stdout = b'Enter an integer to compute its factorial:\nFactorial of a negative integer is undefined!\n'


class OopConstructors(VYPaTestCase):
    input_file = "oop_constructors.vl"
    test_stdin = b""
    test_stdout = b'constructed A\nconstructed A\nconstructed B\n'


class OopAttributes(VYPaTestCase):
    input_file = "oop_attribute_rw.vl"
    test_stdin = b""
    test_stdout = b'obja.x is 13\nobja.y is 42\nobja.z is 69\nobjb.w is 44\n'


class OopBasicMethods(VYPaTestCase):
    input_file = "oop_methods_basic.vl"
    test_stdin = b""
    test_stdout = b'foo called on A with 4\nbar called on A with 8\nresult of bar call 9\n'


class OopObjectBuiltins(VYPaTestCase):
    input_file = "oop_object_builtins.vl"
    test_stdin = b""
    test_stdout = b'o.getClass(): Object\no.toString(): worked\n'


class OopThisKeyword(VYPaTestCase):
    input_file = "oop_this_usage.vl"
    test_stdin = b""
    test_stdout = b'getX called on A 1\nresult of getX call 1\nbar called on A with 8\nresult of bar call 9\ngetXincr called on A 1\nresult of getXincr call 2\n'


class OopSubsumption(VYPaTestCase):
    input_file = "oop_subsumption.vl"
    test_stdin = b""
    test_stdout = b'o.getClass(): A\no.toString(): worked\n'


class OopOverride(VYPaTestCase):
    input_file = "oop_override.vl"
    test_stdin = b""
    test_stdout = b'foo called on A\nfoo called on B\n'


class OopSuperKeyword(VYPaTestCase):
    input_file = "oop_super.vl"
    test_stdin = b""
    test_stdout = b'toString called on A\nresult of a.toString(): "the super toString returned: 17"'


class OopVisibility(VYPaTestCase):
    input_file = "oop_visibility.vl"
    test_stdin = b""
    test_stdout = b'69'


class OopVisibilityPrivateFail(VYPaTestCase):
    input_file = "oop_visibility_fail.vl"
    test_return = 19


class OopAssignmentExample(VYPaTestCase):
    input_file = "oop_assignment_example4.vl"
    test_stdin = b"60\n70\n"
    test_stdout = b'constructor of Shapeinstance of Shape 42 - rectangle 4200'


megatest_out = b'''True: True
False: False
!True: False
!False: True
False || False: False
True || False: True
False || True: True
True || True: True
False && False: False
True && False: False
False && True: False
True && True: True
True == True: True
True != True: False
False == False: True
False != False: False
True == False: False
True != False: True
False == True: False
False != True: True
1 < 2: True
2 < 2: False
2 <= 2: True
astring < bstring: True
bstring > astring: True
astring <= bstring: True
bstring >= astring: True
astring == bstring: False
bstring == astring: False
astring >= bstring: False
bstring <= astring: False
astring > bstring: False
bstring < astring: False
astring != bstring: True
bstring != astring: True
astring != samestring: False
astring == samestring: True
f1 + f2 = 0x1.cp2: 0x1.cp2
f1 - f2 = -0x1.8p1: -0x1.8p1
f1 * f2 = 0x1.4p3: 0x1.4p3
f1 / f2 = 0x1.999999999999ap-2: 0x1.999999999999ap-2
astring+bstring = astringbstring: astringbstring
i + j = 11: 11
i - j = 7: 7
i * j = 18: 18
i / j = 7: 4
j + i = 11: 11
j - i = -7: -7
j * i = 18: 18
j / i = 0: 0
'''


class ExpressionMegatest(VYPaTestCase):
    input_file = "expression_megatest.vl"
    test_stdin = b""
    test_stdout = megatest_out


# TODO: compiler error tests:
#   - non-int/object type in if
#   - non-int/object type in while

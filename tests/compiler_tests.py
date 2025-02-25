#
# VYPa compiler project.
# Authors: Peter Kubov (xkubov06), Richard Micka (xmicka11)
#

import os
import unittest
import subprocess
import argparse
import compiler_cases.test_cases


class VYPaTestCase(unittest.TestCase):
    input_file = []
    test_stdin = b""
    test_stdout = b""
    test_stderr = b""
    test_return = 0
    test_int_retcode = 0

    output_file_path = "testout.vc"

    compiler_path = ""
    interpret_path = ""

    def __init__(self, methodname):
        super().__init__(methodname)
        self.real_stdout = b""
        self.real_stderr = b""
        self.real_return = 0
        self.real_int_retcode = 0

    def run(self, result=None):
        input_file_path = os.path.join(os.path.abspath("."), "compiler_cases", self.input_file)
        compile_command = [self.compiler_path, '-v', input_file_path, self.output_file_path]
        compiler_subproc = subprocess.Popen(compile_command, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        (stdout, stderr) = compiler_subproc.communicate(timeout=2)
        return_code = compiler_subproc.wait()
        self.real_return = return_code
        
        if return_code != 0:
            super().run(result)
            return

        run_command = ["java", "-jar", self.interpret_path, self.output_file_path]
        interpret_subproc = subprocess.Popen(run_command, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        try:
            (stdout, stderr) = interpret_subproc.communicate(self.test_stdin, timeout=2)
        except subprocess.TimeoutExpired:
            result.failures.append((self, "vypint process timed out on compiled {}".format(input_file_path)))
            return

        self.real_stdout = stdout
        self.real_stderr = stderr
        self.real_int_retcode = interpret_subproc.returncode
        super().run(result)

    def test00_check_comp_return_code(self):
        self.assertEqual(self.test_return, self.real_return)

    def test01_check_int_return_code(self):
        self.assertEqual(self.real_int_retcode, self.test_int_retcode)

    def test02_check_output(self):
        self.assertEqual(self.test_stdout, self.real_stdout)

    @classmethod
    def tearDownClass(cls):
        if os.path.exists(cls.output_file_path):
            os.remove(cls.output_file_path)


class VYPaTestLoader(unittest.TestLoader):
    def __init__(self, vypcomp_path, vypint_path, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.vypcomp_path = vypcomp_path
        self.vypint_path = vypint_path


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Run vypcomp+vypint tests')
    parser.add_argument('vypcomp_path', metavar='vypcomp_path', type=str, help='path (relative or absolute) to vypcomp binary')
    parser.add_argument('vypint_path', metavar='vypint_path', type=str, help='path (relative or absolute) to vypint jar file')
    args = parser.parse_args()
    loader = VYPaTestLoader(args.vypcomp_path, args.vypint_path)
    suite = loader.loadTestsFromModule(globals()['compiler_cases'].test_cases)
    runner = unittest.TextTestRunner()
    runner_result = runner.run(suite)
    exit(len(runner_result.failures))

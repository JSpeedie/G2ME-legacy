import filecmp
import os
import subprocess
from datetime import datetime
from pathlib import Path

produced_output_dir = Path("produced_output")
expected_output_dir = Path("correct_output")
log_file_path = Path("runtestcases.py.log")


class Test():
    def __init__(self, test_name: str, setup_cmd_list: [str],
                 test_cmd: str, redirect: bool, log_file=None):
        self.test_name = test_name
        self.setup_cmd_list = setup_cmd_list
        self.test_cmd = test_cmd
        self.redirect = redirect
        self.log_file = log_file

    def run_setup(self) -> bool:
        # {{{
        # Run the setup commands
        for sc in self.setup_cmd_list:
            if self.log_file != None:
                print("[{}]: ".format(datetime.now()), end="", file=self.log_file)
                print("{}: ".format(self.test_name), end="", file=self.log_file)
                print(f"Executing setup command \"{sc}\"", file=self.log_file)
            ret = subprocess.call(sc.split(" "),
                                stdout=subprocess.DEVNULL,
                                stderr=subprocess.STDOUT)
            if ret != 0:
                print("ERROR: the script was unable to successfully execute " +
                    "all the setup commands for the test. Exiting test...");
                return False

        return True
        # }}}

    def run(self) -> (int, int):
        # {{{
        passes = 0
        fails = 0

        # Run the setup commands for the Test. If executing the setup commands
        # failed, exit early
        if self.run_setup() == False:
            return (passes, fails)

        # Create Path vars for the produced and expected output
        produced = str(produced_output_dir / self.test_name)
        expected = str(expected_output_dir / self.test_name)
        # Construct the test command
        cmd = []
        cmd.extend(self.test_cmd.split(" "))
        # By default, ignore stdout output from the command. This will be
        # changed later if we wish to save the stdout output
        subprocess_stdout = subprocess.DEVNULL

        # If the test requires it, add output redirection by taking the
        # testcommand and adding a redirection to it so its output lands in a
        # file for comparison
        if self.redirect:
            if self.log_file != None:
                print("[{}]: ".format(datetime.now()), end="", file=self.log_file)
                print("{}: ".format(self.test_name), end="", file=self.log_file)
                print(f"Executing test command \"{self.test_cmd}\", saving output to \"{produced}\"", file=self.log_file)
            # Call command, sending output to the dedicated command output file
            stdout_file = open(produced, 'w')
            subprocess_stdout = stdout_file
        else:
            cmd.extend(produced.split(" "))
            if self.log_file != None:
                print("[{}]: ".format(datetime.now()), end="", file=self.log_file)
                print("{}: ".format(self.test_name), end="", file=self.log_file)
                print(f"Executing test command \"{self.test_cmd} {produced}\"", file=self.log_file)

        # Run the test command
        ret = subprocess.call(cmd,
                              stdout=subprocess_stdout,
                              stderr=subprocess.DEVNULL)
        if ret != 0:
            print("ERROR: the script was unable to successfully execute " +
                "the test command. Exiting test...");
            if self.log_file != None:
                print("[{}]: ".format(datetime.now()), end="", file=self.log_file)
                print("{}: ".format(self.test_name), end="", file=self.log_file)
                print(f"ERROR: failure experienced when executing test command.", file=self.log_file)
            return (passes, fails)

        # Compare the output from the test command to the expected output
        test_result = filecmp.cmp(produced, expected, shallow=False)
        if test_result:
            passes += 1
        else:
            fails += 1
            if self.log_file != None:
                print("[{}]: ".format(datetime.now()), end="", file=self.log_file)
                print("{}: ".format(self.test_name), end="", file=self.log_file)
                print(f"test failed.", file=self.log_file)

        return (passes, fails)
        # }}}


# Run a list/collection of tests
def run_test_list(tests: [Test]) -> (int, int):
    # {{{
    list_total_passes = 0
    list_total_fails = 0

    # Run all the tests
    for t in tests:
        (passes, fails) = t.run()
        if fails == 0:
            print(".", end="")
        else:
            print("E")
            print(f"Files \"{produced_output_dir}/{t.test_name}\" and " +
                  f"\"{expected_output_dir}/{t.test_name}\" differ")
            # TODO: ? not sure how to diff the files from python, diff won't
            # be available on windows environments?
            # diff -U 0 ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} | tail -n +4

        list_total_passes += passes
        list_total_fails += fails

    return (list_total_passes, list_total_fails)
    # }}}


# 2 tests
def test_A(log_file=None) -> (int, int):
    # {{{
    # Setup
    g2me_exec = ""
    if os.name == "posix": # If this is being run on linux/macOS
        g2me_exec = "../G2ME"
        input_dir = Path("input")
    elif os.name == "nt": # If this is being run on windows
        g2me_exec = "..\\G2ME64.exe"
        input_dir = Path(".\\input")
    season1 = str(input_dir / Path("TEST.sea"))

    # Define the B1 test collection
    B1_test_collection=[
        Test(test_name="testB1A1",
             setup_cmd_list=[
                # ../G2ME -B input/TEST.sea
                g2me_exec + " -B " + season1,
             ],
             # ../G2ME -A Jayden
             test_cmd=g2me_exec + " -A Jayden",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testB1A2",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -A Harper
             test_cmd=g2me_exec + " -A Harper",
             redirect=True,
             log_file=log_file,
        ),
    ]

    # Run all the test collections associated with the test suite
    test_collection_list = [
        B1_test_collection,
    ]

    total_passes = 0
    total_fails = 0

    for tc in test_collection_list:
        (passes, fails) = run_test_list(tc)
        total_passes += passes
        total_fails += fails

    return (total_passes, total_fails)
    # }}}


# 3 tests
def test_c(log_file=None) -> (int, int):
    # {{{
    # Setup
    g2me_exec = ""
    if os.name == "posix": # If this is being run on linux/macOS
        g2me_exec = "../G2ME"
        input_dir = Path("input")
    elif os.name == "nt": # If this is being run on windows
        g2me_exec = "..\\G2ME64.exe"
        input_dir = Path(".\\input")
    season1 = str(input_dir / Path("TEST.sea"))

    # Define the B1 test collection
    B1_test_collection=[
        Test(test_name="testB1c1",
             setup_cmd_list=[
                # ../G2ME -B input/TEST.sea
                g2me_exec + " -B " + season1,
             ],
             # ../G2ME -c Dylan
             test_cmd=g2me_exec + " -c Dylan",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testB1c2",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -c Michael
             test_cmd=g2me_exec + " -c Michael",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testB1c3",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -c Valerie
             test_cmd=g2me_exec + " -c Valerie",
             redirect=True,
             log_file=log_file,
        ),
    ]

    # Run all the test collections associated with the test suite
    test_collection_list = [
        B1_test_collection,
    ]

    total_passes = 0
    total_fails = 0

    for tc in test_collection_list:
        (passes, fails) = run_test_list(tc)
        total_passes += passes
        total_fails += fails

    return (total_passes, total_fails)
    # }}}


# 12 tests
def test_C(log_file=None) -> (int, int):
    # {{{
    # Setup
    g2me_exec = ""
    if os.name == "posix": # If this is being run on linux/macOS
        g2me_exec = "../G2ME"
        input_dir = Path("input")
    elif os.name == "nt": # If this is being run on windows
        g2me_exec = "..\\G2ME64.exe"
        input_dir = Path(".\\input")
    season1 = str(input_dir / Path("TEST.sea"))
    bracket1 = str(input_dir / Path("TEST1"))
    bracket2 = str(input_dir / Path("TEST2"))
    filter1 = str(input_dir / Path("filter1"))

    # Define the b1 test collection
    b1_test_collection=[
        Test(test_name="testb1C",
             setup_cmd_list=[
                # ../G2ME -b input/TEST1
                g2me_exec + " -b " + bracket1,
             ],
             # ../G2ME -C
             test_cmd=g2me_exec + " -C",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testb1mC",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -m 2 -C
             test_cmd=g2me_exec + " -m 2 -C",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testb1fC",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -f input/filter1 -C
             test_cmd=g2me_exec + " -f " + filter1 + " -C",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testb1mfC",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -m 2 -f input/filter1 -C
             test_cmd=g2me_exec + " -m 2 -f " + filter1 + " -C",
             redirect=True,
             log_file=log_file,
        ),
    ]

    # Define the b2 test collection
    b2_test_collection=[
        Test(test_name="testb2C",
             setup_cmd_list=[
                # ../G2ME -b input/TEST2
                g2me_exec + " -b " + bracket2,
             ],
             # ../G2ME -C
             test_cmd=g2me_exec + " -C",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testb2mC",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -m 2 -C
             test_cmd=g2me_exec + " -m 2 -C",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testb2fC",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -f input/filter1 -C
             test_cmd=g2me_exec + " -f " + filter1 + " -C",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testb2mfC",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -m 2 -f input/filter1 -C
             test_cmd=g2me_exec + " -m 2 -f " + filter1 + " -C",
             redirect=True,
             log_file=log_file,
        ),
    ]

    # Define the B1 test collection
    B1_test_collection=[
        Test(test_name="testB1C",
             setup_cmd_list=[
                # ../G2ME -B input/TEST.sea
                g2me_exec + " -B " + season1,
             ],
             # ../G2ME -C
             test_cmd=g2me_exec + " -C",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testB1mC",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -m 2 -C
             test_cmd=g2me_exec + " -m 2 -C",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testB1fC",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -f input/filter1 -C
             test_cmd=g2me_exec + " -f " + filter1 + " -C",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testB1mfC",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -m 2 -f input/filter1 -C
             test_cmd=g2me_exec + " -m 2 -f " + filter1 + " -C",
             redirect=True,
             log_file=log_file,
        ),
    ]

    # Run all the test collections associated with the test suite
    test_collection_list = [
        b1_test_collection,
        b2_test_collection,
        B1_test_collection,
    ]

    total_passes = 0
    total_fails = 0

    for tc in test_collection_list:
        (passes, fails) = run_test_list(tc)
        total_passes += passes
        total_fails += fails

    return (total_passes, total_fails)
    # }}}


# 12 tests
def test_M(log_file=None) -> (int, int):
    # {{{
    # Setup
    g2me_exec = ""
    if os.name == "posix": # If this is being run on linux/macOS
        g2me_exec = "../G2ME"
        input_dir = Path("input")
    elif os.name == "nt": # If this is being run on windows
        g2me_exec = "..\\G2ME64.exe"
        input_dir = Path(".\\input")
    season1 = str(input_dir / Path("TEST.sea"))
    bracket1 = str(input_dir / Path("TEST1"))
    bracket2 = str(input_dir / Path("TEST2"))
    filter1 = str(input_dir / Path("filter1"))

    # Define the b1 test collection
    b1_test_collection=[
        Test(test_name="testb1M",
             setup_cmd_list=[
                # ../G2ME -b input/TEST1
                g2me_exec + " -b " + bracket1,
             ],
             # ../G2ME -M
             test_cmd=g2me_exec + " -M",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testb1mM",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -m 2 -M
             test_cmd=g2me_exec + " -m 2 -M",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testb1fM",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -f input/filter1 -M
             test_cmd=g2me_exec + " -f " + filter1 + " -M",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testb1mfM",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -m 2 -f input/filter1 -M
             test_cmd=g2me_exec + " -m 2 -f " + filter1 + " -M",
             redirect=True,
             log_file=log_file,
        ),
    ]

    # Define the b2 test collection
    b2_test_collection=[
        Test(test_name="testb2M",
             setup_cmd_list=[
                # ../G2ME -b input/TEST2
                g2me_exec + " -b " + bracket2,
             ],
             # ../G2ME -M
             test_cmd=g2me_exec + " -M",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testb2mM",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -m 2 -M
             test_cmd=g2me_exec + " -m 2 -M",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testb2fM",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -f input/filter1 -M
             test_cmd=g2me_exec + " -f " + filter1 + " -M",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testb2mfM",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -m 2 -f input/filter1 -M
             test_cmd=g2me_exec + " -m 2 -f " + filter1 + " -M",
             redirect=True,
             log_file=log_file,
        ),
    ]

    # Define the B1 test collection
    B1_test_collection=[
        Test(test_name="testB1M",
             setup_cmd_list=[
                # ../G2ME -B input/TEST.sea
                g2me_exec + " -B " + season1,
             ],
             # ../G2ME -M
             test_cmd=g2me_exec + " -M",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testB1mM",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -m 2 -M
             test_cmd=g2me_exec + " -m 2 -M",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testB1fM",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -f input/filter1 -M
             test_cmd=g2me_exec + " -f " + filter1 + " -M",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testB1mfM",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -m 2 -f input/filter1 -M
             test_cmd=g2me_exec + " -m 2 -f " + filter1 + " -M",
             redirect=True,
             log_file=log_file,
        ),
    ]

    # Run all the test collections associated with the test suite
    test_collection_list = [
        b1_test_collection,
        b2_test_collection,
        B1_test_collection,
    ]

    total_passes = 0
    total_fails = 0

    for tc in test_collection_list:
        (passes, fails) = run_test_list(tc)
        total_passes += passes
        total_fails += fails

    return (total_passes, total_fails)
    # }}}


# 24 tests
def test_o(log_file=None) -> (int, int):
    # {{{
    # Setup
    g2me_exec = ""
    if os.name == "posix": # If this is being run on linux/macOS
        g2me_exec = "../G2ME"
        input_dir = Path("input")
    elif os.name == "nt": # If this is being run on windows
        g2me_exec = "..\\G2ME64.exe"
        input_dir = Path(".\\input")
    filter1 = str(input_dir / Path("filter1"))
    bracket1 = str(input_dir / Path("TEST1"))
    bracket2 = str(input_dir / Path("TEST2"))
    season1 = str(input_dir / Path("TEST.sea"))

    # Define the b1 test collection
    b1_test_collection=[
        Test(test_name="testb1o",
             setup_cmd_list=[
                # ../G2ME -b input/TEST1
                g2me_exec + " -b " + bracket1,
             ],
             # ../G2ME -o
             test_cmd=g2me_exec + " -o",
             redirect=False,
             log_file=log_file,
        ),
        Test(test_name="testb1vo",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -vo
             test_cmd=g2me_exec + " -vo",
             redirect=False,
             log_file=log_file,
        ),
        Test(test_name="testb1mo",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -m 2 -o
             test_cmd=g2me_exec + " -m 2 -o",
             redirect=False,
             log_file=log_file,
        ),
        Test(test_name="testb1mvo",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -m 2 -vo
             test_cmd=g2me_exec + " -m 2 -vo",
             redirect=False,
             log_file=log_file,
        ),
        Test(test_name="testb1fo",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -f input/filter1 -o
             test_cmd=g2me_exec + " -f " + filter1 + " -o",
             redirect=False,
             log_file=log_file,
        ),
        Test(test_name="testb1fvo",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -f input/filter1 -vo
             test_cmd=g2me_exec + " -f " + filter1 + " -vo",
             redirect=False,
             log_file=log_file,
        ),
        Test(test_name="testb1mfo",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -m 2 -f input/filter1 -o
             test_cmd=g2me_exec + " -m 2 -f " + filter1 + " -o",
             redirect=False,
             log_file=log_file,
        ),
        Test(test_name="testb1mfvo",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # /G2ME -m 2 -f input/filter1 -vo
             test_cmd=g2me_exec + " -m 2 -f " + filter1 + " -vo",
             redirect=False,
             log_file=log_file,
        ),
    ]

    # Define the b2 test collection
    b2_test_collection=[
        Test(test_name="testb2o",
             setup_cmd_list=[
                # ../G2ME -b input/TEST2
                g2me_exec + " -b " + bracket2,
             ],
             # ../G2ME -o
             test_cmd=g2me_exec + " -o",
             redirect=False,
             log_file=log_file,
        ),
        Test(test_name="testb2vo",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -vo
             test_cmd=g2me_exec + " -vo",
             redirect=False,
             log_file=log_file,
        ),
        Test(test_name="testb2mo",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -m 2 -o
             test_cmd=g2me_exec + " -m 2 -o",
             redirect=False,
             log_file=log_file,
        ),
        Test(test_name="testb2mvo",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -m 2 -vo
             test_cmd=g2me_exec + " -m 2 -vo",
             redirect=False,
             log_file=log_file,
        ),
        Test(test_name="testb2fo",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -f input/filter1 -o
             test_cmd=g2me_exec + " -f " + filter1 + " -o",
             redirect=False,
             log_file=log_file,
        ),
        Test(test_name="testb2fvo",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -f input/filter1 -vo
             test_cmd=g2me_exec + " -f " + filter1 + " -vo",
             redirect=False,
             log_file=log_file,
        ),
        Test(test_name="testb2mfo",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -m 2 -f input/filter1 -o
             test_cmd=g2me_exec + " -m 2 -f " + filter1 + " -o",
             redirect=False,
             log_file=log_file,
        ),
        Test(test_name="testb2mfvo",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # /G2ME -m 2 -f input/filter1 -vo
             test_cmd=g2me_exec + " -m 2 -f " + filter1 + " -vo",
             redirect=False,
             log_file=log_file,
        ),
    ]

    # Define the B1 test collection
    B1_test_collection=[
        Test(test_name="testB1o",
             setup_cmd_list=[
                # ../G2ME -B input/TEST.sea
                g2me_exec + " -B " + season1,
             ],
             # ../G2ME -o
             test_cmd=g2me_exec + " -o",
             redirect=False,
             log_file=log_file,
        ),
        Test(test_name="testB1vo",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -vo
             test_cmd=g2me_exec + " -vo",
             redirect=False,
             log_file=log_file,
        ),
        Test(test_name="testB1mo",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -m 2 -o
             test_cmd=g2me_exec + " -m 2 -o",
             redirect=False,
             log_file=log_file,
        ),
        Test(test_name="testB1mvo",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -m 2 -vo
             test_cmd=g2me_exec + " -m 2 -vo",
             redirect=False,
             log_file=log_file,
        ),
        Test(test_name="testB1fo",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -f input/filter1 -o
             test_cmd=g2me_exec + " -f " + filter1 + " -o",
             redirect=False,
             log_file=log_file,
        ),
        Test(test_name="testB1fvo",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -f input/filter1 -vo
             test_cmd=g2me_exec + " -f " + filter1 + " -vo",
             redirect=False,
             log_file=log_file,
        ),
        Test(test_name="testB1mfo",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -m 2 -f input/filter1 -o
             test_cmd=g2me_exec + " -m 2 -f " + filter1 + " -o",
             redirect=False,
             log_file=log_file,
        ),
        Test(test_name="testB1mfvo",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # /G2ME -m 2 -f input/filter1 -vo
             test_cmd=g2me_exec + " -m 2 -f " + filter1 + " -vo",
             redirect=False,
             log_file=log_file,
        ),
    ]

    # Run all the test collections associated with the test suite
    test_collection_list = [
        b1_test_collection,
        b2_test_collection,
        B1_test_collection,
    ]

    total_passes = 0
    total_fails = 0

    for tc in test_collection_list:
        (passes, fails) = run_test_list(tc)
        total_passes += passes
        total_fails += fails

    return (total_passes, total_fails)
    # }}}


# 24 tests
def test_O(log_file=None) -> (int, int):
    # {{{
    # Setup
    g2me_exec = ""
    if os.name == "posix": # If this is being run on linux/macOS
        g2me_exec = "../G2ME"
        input_dir = Path("input")
    elif os.name == "nt": # If this is being run on windows
        g2me_exec = "..\\G2ME64.exe"
        input_dir = Path(".\\input")
    filter1 = str(input_dir / Path("filter1"))
    bracket1 = str(input_dir / Path("TEST1"))
    bracket2 = str(input_dir / Path("TEST2"))
    season1 = str(input_dir / Path("TEST.sea"))

    # Define the b1 test collection
    b1_test_collection=[
        Test(test_name="testb1O",
             setup_cmd_list=[
                # ../G2ME -b input/TEST1
                g2me_exec + " -b " + bracket1,
             ],
             # ../G2ME -O
             test_cmd=g2me_exec + " -O",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testb1vO",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -vO
             test_cmd=g2me_exec + " -vO",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testb1mO",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -m 2 -O
             test_cmd=g2me_exec + " -m 2 -O",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testb1mvO",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -m 2 -vO
             test_cmd=g2me_exec + " -m 2 -vO",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testb1fO",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -f input/filter1 -O
             test_cmd=g2me_exec + " -f " + filter1 + " -O",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testb1fvO",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -f input/filter1 -vO
             test_cmd=g2me_exec + " -f " + filter1 + " -vO",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testb1mfO",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -m 2 -f input/filter1 -O
             test_cmd=g2me_exec + " -m 2 -f " + filter1 + " -O",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testb1mfvO",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # /G2ME -m 2 -f input/filter1 -vO
             test_cmd=g2me_exec + " -m 2 -f " + filter1 + " -vO",
             redirect=True,
             log_file=log_file,
        ),
    ]

    # Define the b2 test collection
    b2_test_collection=[
        Test(test_name="testb2O",
             setup_cmd_list=[
                # ../G2ME -b input/TEST2
                g2me_exec + " -b " + bracket2,
             ],
             # ../G2ME -O
             test_cmd=g2me_exec + " -O",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testb2vO",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -vO
             test_cmd=g2me_exec + " -vO",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testb2mO",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -m 2 -O
             test_cmd=g2me_exec + " -m 2 -O",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testb2mvO",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -m 2 -vO
             test_cmd=g2me_exec + " -m 2 -vO",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testb2fO",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -f input/filter1 -O
             test_cmd=g2me_exec + " -f " + filter1 + " -O",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testb2fvO",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -f input/filter1 -vO
             test_cmd=g2me_exec + " -f " + filter1 + " -vO",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testb2mfO",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -m 2 -f input/filter1 -O
             test_cmd=g2me_exec + " -m 2 -f " + filter1 + " -O",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testb2mfvO",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # /G2ME -m 2 -f input/filter1 -vO
             test_cmd=g2me_exec + " -m 2 -f " + filter1 + " -vO",
             redirect=True,
             log_file=log_file,
        ),
    ]

    # Define the B1 test collection
    B1_test_collection=[
        Test(test_name="testB1O",
             setup_cmd_list=[
                # ../G2ME -B input/TEST.sea
                g2me_exec + " -B " + season1,
             ],
             # ../G2ME -O
             test_cmd=g2me_exec + " -O",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testB1vO",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -vO
             test_cmd=g2me_exec + " -vO",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testB1mO",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -m 2 -O
             test_cmd=g2me_exec + " -m 2 -O",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testB1mvO",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -m 2 -vO
             test_cmd=g2me_exec + " -m 2 -vO",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testB1fO",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -f input/filter1 -O
             test_cmd=g2me_exec + " -f " + filter1 + " -O",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testB1fvO",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -f input/filter1 -vO
             test_cmd=g2me_exec + " -f " + filter1 + " -vO",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testB1mfO",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # ../G2ME -m 2 -f input/filter1 -O
             test_cmd=g2me_exec + " -m 2 -f " + filter1 + " -O",
             redirect=True,
             log_file=log_file,
        ),
        Test(test_name="testB1mfvO",
             setup_cmd_list=[
                # Nothing - Use setup from previous test
             ],
             # /G2ME -m 2 -f input/filter1 -vO
             test_cmd=g2me_exec + " -m 2 -f " + filter1 + " -vO",
             redirect=True,
             log_file=log_file,
        ),
    ]

    # Run all the test collections associated with the test suite
    test_collection_list = [
        b1_test_collection,
        b2_test_collection,
        B1_test_collection,
    ]

    total_passes = 0
    total_fails = 0

    for tc in test_collection_list:
        (passes, fails) = run_test_list(tc)
        total_passes += passes
        total_fails += fails

    return (total_passes, total_fails)
    # }}}


if __name__ == "__main__":
    total_passes = 0
    total_fails = 0

	# if [[ ! -d "${PRODUCED_OUTPUT}" ]]; then
	# 	mkdir ${PRODUCED_OUTPUT}
	# fi

    test_suite_list = [
        test_A,
        test_c,
        test_C,
        test_M,
        test_o,
        test_O,
    ]

    log_file = open(log_file_path, 'w')

    for ts in test_suite_list:
        (passes, fails) = ts(log_file)
        total_passes += passes
        total_fails += fails
        total_tests = total_fails + total_passes

    if total_fails == 0:
        print("{}/{} All tests passed!".format(total_passes, total_tests))
    else:
        print("{}/{} passed. {} tests failed.".format(total_passes, total_tests, total_fails))


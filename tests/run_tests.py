#!/usr/bin/env python3
"""Builds the simulator and checks it against the programs in tests/.

Each program in tests/programs has a line like
    # expect: R1=4 R3=10 MEM[1024]=10 PC=10 cycles=61
Registers that are not listed must end as 0, and so must every word of the data
segment (addresses 1024 to 2047) that is not listed.

Each program in tests/errors has a line like
    # error: unknown instruction
and must be rejected with that message.
"""
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TESTS = os.path.join(ROOT, "tests")
EXE = os.path.join(ROOT, "processor.exe" if os.name == "nt" else "processor")


def build():
    cc = os.environ.get("CC", "gcc")
    cmd = [cc, "-O2", "-std=c11", "-Wall", "-Wno-unused-function", "-Wno-unused-variable",
           os.path.join(ROOT, "translate.c"), "-o", EXE]
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        sys.exit("Build failed:\n" + result.stderr)
    if result.stderr.strip():
        print(result.stderr)


def run(path):
    result = subprocess.run([EXE, path, "--delay", "0"], capture_output=True, cwd=ROOT)
    return result.returncode, result.stdout.decode("utf-8", errors="replace")


def to_signed(bits):
    value = int(bits, 2)
    return value - (1 << 32) if bits[0] == "1" else value


def final_state(output):
    """Registers, PC, memory and cycle count from the dump printed after the last cycle."""
    match = re.search(r"Final state after (\d+) clock cycles", output)
    if not match:
        raise AssertionError("no final state in the output")
    dump = output[match.end():]
    regs = {int(n): int(v) for n, v in re.findall(r"^Reg(\d+): (-?\d+)$", dump, re.M)}
    pc = int(re.search(r"^PC: (-?\d+)$", dump, re.M).group(1))
    mem = {int(a): to_signed(b) for a, b in re.findall(r"^Ram\[(\d+)\]: ([01]{32})$", dump, re.M)}
    if len(regs) != 32 or len(mem) != 2048:
        raise AssertionError(f"incomplete dump: {len(regs)} registers, {len(mem)} memory words")
    return int(match.group(1)), regs, pc, mem


def check_program(path):
    text = open(path, encoding="utf-8").read()
    expect = re.search(r"#\s*expect:(.*)", text).group(1).split()
    code, output = run(path)
    if code != 0:
        raise AssertionError(f"exit code {code}:\n{output[-2000:]}")
    cycles, regs, pc, mem = final_state(output)

    want_regs, want_mem = {}, {}
    for item in expect:
        key, value = item.split("=")
        if key == "cycles":
            if cycles != int(value):
                raise AssertionError(f"took {cycles} cycles, expected {value}")
        elif key == "PC":
            if pc != int(value):
                raise AssertionError(f"PC is {pc}, expected {value}")
        elif key.startswith("MEM["):
            want_mem[int(key[4:-1])] = int(value)
        else:
            want_regs[int(key[1:])] = int(value)

    for n in range(32):
        if regs[n] != want_regs.get(n, 0):
            raise AssertionError(f"R{n} is {regs[n]}, expected {want_regs.get(n, 0)}")
    for address in list(want_mem) + list(range(1024, 2048)):
        if mem[address] != want_mem.get(address, 0):
            raise AssertionError(f"MEM[{address}] is {mem[address]}, expected {want_mem.get(address, 0)}")
    return f"{cycles} cycles"


def check_error(path):
    text = open(path, encoding="utf-8").read()
    message = re.search(r"#\s*error:\s*(.*)", text).group(1).strip()
    code, output = run(path)
    if code == 0:
        raise AssertionError("the program was accepted")
    if message.lower() not in output.lower():
        raise AssertionError(f"expected the error '{message}', got:\n{output[-500:]}")
    return "rejected"


def main():
    build()
    failures = 0
    for folder, check in (("programs", check_program), ("errors", check_error)):
        for name in sorted(os.listdir(os.path.join(TESTS, folder))):
            try:
                detail = check(os.path.join(TESTS, folder, name))
                print(f"PASS  {folder}/{name} ({detail})")
            except Exception as error:  # noqa: BLE001 - report every failure
                failures += 1
                print(f"FAIL  {folder}/{name}: {error}")
    if failures:
        sys.exit(f"{failures} test(s) failed")
    print("All tests passed.")


if __name__ == "__main__":
    main()

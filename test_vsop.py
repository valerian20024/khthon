#!/usr/bin/env python3
"""
VSOP Compiler Test Runner

Automatically runs your compiler on all test cases in examples/automatic/
and compares the output with the expected .out files.

Supports:
- -l  (lexical)
- -p  (syntax / parse)
- -c  (semantic analysis)

Error messages are compared only on the first line (header), as requested.
"""

import sys
import subprocess
from pathlib import Path
from typing import Tuple, Optional

# ========================= CONFIG =========================

COMPILER = "./vsopc"
EXAMPLES_ROOT = Path("examples/automatic")

CATEGORY_TO_MODE = {
    "1_lexical": "-l",
    "2_syntax":  "-p",
    #"3_semantic": "-c",
    #"4_generation": "-i",
}

# =========================================================

def run_compiler(vsop_file: Path, mode_flag: str) -> Tuple[int, str, str]:
    """Run the compiler and return (returncode, stdout, stderr)"""
    try:
        result = subprocess.run(
            [COMPILER, mode_flag, str(vsop_file)],
            capture_output=True,
            text=True,
            timeout=5.0
        )
        return result.returncode, result.stdout, result.stderr
    except subprocess.TimeoutExpired:
        return -1, "", f"Timeout while running {vsop_file}"
    except FileNotFoundError:
        print(f"Error: Compiler '{COMPILER}' not found. Did you run 'make'?")
        sys.exit(1)


def normalize_output(output: str) -> str:
    """Keep only the first line for error messages (as per your spec)"""
    lines = output.strip().splitlines()
    if not lines:
        return ""
    
    first_line = lines[0].strip()
    
    # If it's an error line (contains ": lexical error:", ": syntax error:", etc.)
    if any(x in first_line for x in [": lexical error:", ": syntax error:", ": semantic error:"]):
        return first_line
    else:
        # For successful outputs (token list or AST), keep everything
        return output.strip()


def compare_outputs(actual: str, expected: str, test_name: str) -> bool:
    """Compare actual vs expected output. Returns True if they match."""
    norm_actual = normalize_output(actual)
    norm_expected = normalize_output(expected)

    if norm_actual == norm_expected:
        print(f"✅ {test_name}")
        return True
    else:
        print(f"❌ {test_name}")
        print("   Expected first line / output:")
        print(f"     {repr(norm_expected[:200])}")
        print("   Got:")
        print(f"     {repr(norm_actual[:200])}")
        return False


def main():
    if not Path(COMPILER).exists():
        print(f"Error: Compiler '{COMPILER}' not found.")
        print("       Please run 'make' first.")
        sys.exit(1)

    total = 0
    passed = 0

    # Walk through all categories
    for category_dir in sorted(EXAMPLES_ROOT.iterdir()):
        if not category_dir.is_dir():
            continue

        mode_flag = CATEGORY_TO_MODE.get(category_dir.name)
        if not mode_flag:
            print(f"Skipping unknown category: {category_dir.name}")
            continue

        print(f"\n=== Running {category_dir.name} tests ({mode_flag}) ===")

        input_dir = category_dir / "input" if (category_dir / "input").exists() else category_dir
        # Some students put .vsop files directly in 1_lexical/, others in input/
        # We support both

        vsop_files = list(input_dir.glob("*.vsop"))
        if not vsop_files:
            vsop_files = list(category_dir.glob("*.vsop"))

        output_dir = EXAMPLES_ROOT.parent / "output" / category_dir.name

        for vsop_file in sorted(vsop_files):
            total += 1
            test_name = f"{category_dir.name}/{vsop_file.name}"

            # Find corresponding .out file
            out_file = output_dir / vsop_file.with_suffix(".out").name

            if not out_file.exists():
                print(f"⚠️  Missing expected output: {out_file}")
                continue

            # Run compiler
            retcode, stdout, stderr = run_compiler(vsop_file, mode_flag)

            # Combine stdout + stderr (most tests put everything on stderr for errors)
            actual_output = stdout + stderr

            # Read expected
            expected_output = out_file.read_text(encoding="utf-8")

            if compare_outputs(actual_output, expected_output, test_name):
                passed += 1

    # Final summary
    print("\n" + "="*60)
    print(f"Test summary: {passed}/{total} passed")
    if passed == total:
        print("🎉 All tests passed!")
    else:
        print(f"💥 {total - passed} test(s) failed")
        sys.exit(1)


if __name__ == "__main__":
    main()
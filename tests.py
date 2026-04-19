#!/usr/bin/env python3

"""
Script for testing each tests from the submission platform
that has been cached in the repository. Tree structure required to parse correctly : 

examples
└── automatic
    ├── input
    │   ├── 1_lexical
    │   ├── 2_syntax
    │   ├── 3_semantic
    │   └── 4_generation
    └── output
        ├── 1_lexical
        ├── 2_syntax
        ├── 3_semantic
        └── 4_generation

!!!
This script is buggy.
Don't take its output seriously.
I will debug it when I have time.
!!!

"""

import sys
import subprocess
from pathlib import Path
from typing import Tuple

# ========================= CONFIG =========================

COMPILER = "./vsopc"
EXAMPLES_ROOT = Path("examples/automatic")
INPUT         = (EXAMPLES_ROOT / "input")
OUTPUT        = (EXAMPLES_ROOT / "output")

CATEGORY_TO_MODE = {
    "1_lexical":  "-l",
    "2_syntax":   "-p",
    "3_semantic": "-c",
    "4_generation": "-i",
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
    except Exception as e:
        return -1, "", f"Failed to run compiler: {e}"


def strip_path_prefix(line: str, vsop_file: Path) -> str:
    """Remove the full path so we only keep the filename."""
    filename = vsop_file.name

    # Replace any occurrence of the full path with just the filename
    if str(vsop_file.parent) in line:
        line = line.replace(str(vsop_file.parent) + "/", "")
    
    if str(vsop_file) in line:
        line = line.replace(str(vsop_file), filename)
    
    return line


def normalize_output(output: str, vsop_file: Path) -> str:
    """
    Normalize output for comparison:
      - Error messages: keep only first line + strip path
      - Successful dumps (tokens or AST): remove all whitespace (\n \t spaces)
    """
    if not output.strip():
        return ""

    lines = output.strip().splitlines()

    first_line = lines[0].strip()

    # If it's an error message, keep only the first line and strip path
    if any(err in first_line.lower() for err in [
            ": lexical error", 
            ": syntax error", 
            ": semantic error"
        ]):

        cleaned = strip_path_prefix(first_line, vsop_file)
        
        # Remove the trailing colon that sometimes appears in your current output
        if cleaned.endswith(":"):
            cleaned = cleaned[:-1].strip()
        return cleaned

    # Otherwise it's a successful dump
    # Remove whitespaces for robust comparison
    normalized = "".join(output.split())
    return normalized


def compare_outputs(actual: str, expected: str, vsop_file: Path) -> bool:
    """Compare normalized outputs"""
    norm_actual = normalize_output(actual, vsop_file)
    norm_expected = normalize_output(expected, vsop_file)

    if norm_actual == norm_expected:
        print(f"[V] {vsop_file.parent.name}/{vsop_file.name}")
        return True
    else:
        print(f"[X] {vsop_file.parent.name}/{vsop_file.name}")
        print("   Expected (normalized):")
        print(f"     {norm_expected[:300]}...")
        print("   Got (normalized):")
        print(f"     {norm_actual[:300]}...")
        return False


def main():
    if not Path(COMPILER).exists():
        print(f"Error: Compiler '{COMPILER}' not found. Run 'make' first.")
        sys.exit(1)

    # Number of total and passed tests
    total = 0
    passed = 0

    for category_dir in sorted(INPUT.iterdir()):
        if not category_dir.is_dir():
            continue

        mode_flag = CATEGORY_TO_MODE.get(category_dir.name)
        if not mode_flag:
            continue

        print(f"\n=== Running {category_dir.name} tests ===")

        vsop_files = sorted(category_dir.glob("*.vsop"))

        output_dir = OUTPUT / category_dir.name

        for vsop_file in vsop_files:
            total += 1

            expected_file = output_dir / vsop_file.with_suffix(".out").name
            if not expected_file.exists():
                print(f"[!]  Missing expected output: {expected_file.name}")
                continue

            retcode, stdout, stderr = run_compiler(vsop_file, mode_flag)
            
            # First stderr, then actual dump.
            actual_output = stderr + stdout
            expected_output = expected_file.read_text(encoding="utf-8")

            if compare_outputs(actual_output, expected_output, vsop_file):
                passed += 1

    print("\n" + "=" * 60)
    print(f"Test summary: {passed}/{total} passed")


if __name__ == "__main__":
    main()
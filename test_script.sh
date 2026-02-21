#!/bin/bash

COMPILER="./vsopc"
SOURCE_FILE="examples/automatic/1_lexical/11-multiline-string.vsop"
EXPECTED_OUTPUT="output/1_lexical/11-multiline-string.out"

# todo fix the colors
COLOR_R="\e[0;31m"
COLOR_G="\e[0;32m"
COLOR_B="\e[0;34m"
COLOR_C="\e[0;36m"
COLOR_M="\e[0;35m"
COLOR_Y="\e[0;33m"
COLOR_END="\e[0m"


# We use <() to capture the stdout of the compiler command
if diff <($COMPILER -l $SOURCE_FILE) "$EXPECTED_OUTPUT"; then
    echo "Test Passed!";
else
    echo "Test Failed: Output does not match expected file."
fi
#!/bin/sh
python3 `realpath ${0%/*}`/tools/builder/scripts/builder.py $*

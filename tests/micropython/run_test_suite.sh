#!/bin/bash

# deduce the root path based on this shell script location
ports=$*
if [[ ${ports} ]]; then ports_opt="--ports ${ports}"; fi

root_path=`realpath ${0%/*}/../..`
# get the current working directory and save it
curr_dir=$PWD
# change location to the root directory to not miss other relative locations
cd ${root_path}
tester=./tools/tester/tester.py

# prepare micropython directories containing the test cases
mpy_sub_dirs=(basics float extmod misc micropython)
for sub_dir in ${mpy_sub_dirs[@]}; do
    test_dirs="${test_dirs} ext/micropython/tests/${sub_dir}"
done

# run the test engine
python3 ${tester} \
    --test-suites "tests/micropython/test_suite_manifest.json" \
    --disable-colors \
    ${ports_opt}
exit_code=$?

# change directory back to the previous working dir
cd ${curr_dir}

exit ${exit_code}

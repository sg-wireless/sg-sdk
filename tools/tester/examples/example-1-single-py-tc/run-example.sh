#!/bin/bash

# deduce the root path based on this shell script location
ports=$*
if [[ ${ports} ]]; then ports_opt="--ports ${ports}"; fi

root_path=`realpath ${0%/*}/../../../..`
example_path=`realpath ${0%/*}`
example_path=${example_path#"$root_path/"}
# get the current working directory and save it
curr_dir=$PWD
# change location to the root directory to not miss other relative locations
cd ${root_path}
tester=./tools/tester/tester.py

# run the test engine
python3 ${tester} --test-cases ${example_path}/test-case-1-simple.py \
    ${ports_opt}
exit_code=$?

# change directory back to the previous working dir
cd ${curr_dir}

exit ${exit_code}

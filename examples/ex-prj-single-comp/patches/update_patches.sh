
__root_dir=../../..
__modified_dir=./modified_sources
__patch_dir=.
__main_update_script=${__root_dir}/tools/builder/cmake/update_patches.sh

__original_dir=${__root_dir}/src/libs/utils
__search_path="utils_time.c"
${__main_update_script} \
    ${__original_dir} \
    ${__modified_dir} \
    ${__patch_dir}/utils_lib \
    ${__search_path}

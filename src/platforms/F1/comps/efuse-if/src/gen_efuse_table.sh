#!/bin/bash

script_path=`dirname "$0"`

$IDF_PATH/components/efuse/efuse_table_gen.py \
    --idf_target esp32s3 \
    $IDF_PATH/components/efuse/esp32s3/esp_efuse_table.csv \
    ${script_path}/f1_efuse_table.csv


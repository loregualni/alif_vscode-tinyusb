#!/usr/bin/bash

build_dir="out"
app_name="tinyusb"
core_name="HE"
build_type="debug"
setools_dir=$HOME/"app-release-exec-linux"

cp $build_dir/$app_name/$core_name/$build_type/$app_name.bin $setools_dir/build/images/alif-img.bin
cd $setools_dir
./app-gen-toc -f "build/config/M55_"$core_name"_cfg.json"
./app-write-mram -p

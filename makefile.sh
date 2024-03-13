#!/bin/bash
cur_path=$(pwd)
for dir in $(ls)
do
    if [ -d "$dir" ];
    then
        cd $dir/src/ && make clean;make
        cd $cur_path
    fi
done
#!/bin/bash

# This script runs an experiment with the fasten_client program using different resource types.
# It loops through a list of resource types with different suffixes and runs the fasten_client program 10 times for each resource type.

rt_prefix=fasten_
rt_suffix_list=("128" "256" "512" "1K" "2K" "4K")
rt_list=()

for suffix in ${rt_suffix_list[@]}; do
    temp=$rt_prefix$suffix
    rt_list+=($temp)
done

# Run the server in the background
./fasten_server &

for rt in ${rt_list[@]};
do
    for i in {1..10};
    do
        ./fasten_client $rt
    done
done

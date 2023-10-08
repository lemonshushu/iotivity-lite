#!/bin/bash

rt_list=("fasten_128" "fasten_256" "fasten_512" "fasten_1K" "fasten_2K" "fasten_4K")

# Run the server in the background
./fasten_server &

for rt in ${rt_list[@]};
do
    for i in {1..10};
    do
        ./fasten_client $rt
    done
done
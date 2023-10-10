#!/bin/bash

rt_list=("fasten_128" "fasten_256" "fasten_512" "fasten_1k" "fasten_2k" "fasten_4k")

# Run the server in the background
./fasten_server &

for rt in ${rt_list[@]};
do
    for i in {1..10};
    do
        ./fasten_client $rt
    done
done
#!/bin/bash

if test "$#" -ne 1; then
    echo "Invalid number of arguments to $0"
    exit 1
fi

mode="$1"
valid_modes=("c" "cpp" "python")

mode_is_valid=0
for vm in "${valid_modes[@]}"; do
    if [[ "$mode" == "$vm" ]]; then
        mode_is_valid=1
        break
    fi
done

if [[ $mode_is_valid -eq 0 ]]; then
    echo "Invalid mode: $mode (expected one of: ${valid_modes[*]})"
    exit 2
fi

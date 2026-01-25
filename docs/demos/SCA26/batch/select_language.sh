#!/bin/bash

script_name="$0"

if test "$#" -gt 1; then
    echo "Invalid number of arguments to $script_name"
    exit 1
elif test "$#" -eq 0; then
    mode="c"
else
    mode="$1"
fi

valid_modes=("c" "cpp" "python")
mode_is_valid=0
for vm in "${valid_modes[@]}"; do
    if [[ $mode_is_valid -eq 1 ]] || [[ "$mode" == "$vm" ]]; then
        mode_is_valid=1
    else
        mode_is_valid=0
    fi
done

if [[ $mode_is_valid -eq 0 ]]; then
    echo "Invalid arg for language mode: $mode"
    echo 'Choose either "c", "cpp" or "python"'
    exit 2
fi
echo "Language: ${mode}"

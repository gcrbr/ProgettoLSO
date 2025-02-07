#!/bin/bash
if [ "$1" == "client" ] || [ "$1" == "server" ]; then
    printf "=%.0s" $(seq 10)
    echo ""
    echo "Building $1..."
    common=$(find common -iname '*.c' | tr -s '\n' ' ' | sed 's/.$//')
    build=$(find "$1" -iname '*.c' | tr -s '\n' ' ' | sed 's/.$//')
    rm -f "run_$1"
    gcc $common $build -o "run_$1"
    locs=$(cat $common $build | wc -l | awk '{print $1}')
    echo "Built ($locs lines of code)"
    printf "=%.0s" $(seq 10)
    echo ""
else
    echo "Invalid arguments"
fi
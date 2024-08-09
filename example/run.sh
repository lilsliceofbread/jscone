#!/bin/bash

# allow any directory to call this
parent_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )

cd "$parent_path"

mkdir -p ../build
gcc -g -Wall -Wpedantic -I.. example.c -o ../build/example

../build/example
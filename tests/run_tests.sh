#!/bin/bash

# allow any directory to call this
parent_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )

cd "$parent_path"

make

../build/tests/tests
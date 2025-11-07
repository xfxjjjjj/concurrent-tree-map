#!/usr/bin/env bash

set -e
set -u
set -x

./scripts/check_build.sh

./temp/build-release/demo_coarse_grained 8 1000 10000 0.7
./temp/build-release/demo_fine_grained 8 1000 10000 0.7

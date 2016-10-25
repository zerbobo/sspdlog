#!/bin/bash

script=$(readlink -f "$0")
route=$(dirname "$script")

sh $route/build.sh

cd $route/../build
ctest -C Debug
ctest -C Release

#!/bin/bash
g++ -g $(find . -type f -iregex ".*\.cpp") -o build/luminar
cd build

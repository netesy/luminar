#!/bin/bash
g++ -g $(find src -type f -iregex ".*\.cpp") -o build/luminar -I/usr/lib/gcc/x86_64-linux-gnu/13/include -L/usr/lib/gcc/x86_64-linux-gnu/13/ -lgccjit
g++ -g $(find test -type f -iregex ".*\.cpp") -o build/runUnitTests -I/usr/include/gtest/ -L/usr/lib/x86_64-linux-gnu/ -lgtest -lgtest_main -pthread

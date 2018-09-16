#!/usr/bin/env bash

mkdir -p ./build/
g++ --std=c++14 -o ./build/gfb_find main.cpp \
    -lpthread -lboost_system -lboost_filesystem -lboost_regex

valgrind --log-file="./build/leaks-report.log" --tool=memcheck ./build/gfb_find ~/ -content "([\w]{4}[\s]{1})"
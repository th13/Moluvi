#!/usr/bin/env bash
clang -Wall -Wextra -I./vendor/raylib-5.5_macos/include/ -o main ./vendor/raylib-5.5_macos/lib/libraylib.a main.c

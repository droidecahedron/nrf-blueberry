cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(blueberry)

zephyr_include_directories(inc)

target_sources(app PRIVATE src/main.c)
cmake_minimum_required(VERSION 3.16)
project(rzhttpd C)

set(CMAKE_C_STANDARD 99)

add_executable(rzhttpd src/rzhttpd.c)
add_executable(showip src/showip.c)
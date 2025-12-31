# Minimum CMake version required by ESP-IDF
cmake_minimum_required(VERSION 3.16)

# This includes the standard ESP-IDF build logic/tools
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Name of project
project(EchoHand_Firmware)
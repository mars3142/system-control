cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(system_control)

target_add_binary_data(${PROJECT_NAME}.elf "main/isrgrootx1.pem" TEXT)
## Systen Control

### ESP32-S3 (folder: main)

This is an implementation of my custom system control project (custom pcb with Lolin ESP32-S3 Mini) and LED strip.

The build process is straight forward with ESP-IDF. We used version 5.4 while development and the github actions tried to compile for multiple ESP-IDF versions, so we are safe.

### Desktop (folder: src)

It's included also a desktop application (with SDL3), so you can test the project without any MCU.

### Global Information

The projects can be generated from the root, because here is the starting CMakeLists.txt file.

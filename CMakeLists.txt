cmake_minimum_required(VERSION 3.26)
project(qr_encoder C)

set(CMAKE_C_STANDARD 11)

add_executable(qr_encoder
        src/FPToolkit.c
        src/test_graphics.c
        src/animate.c
        src/encoder.c
        src/reed_solomon.c
        src/clipper.c
)

# Name: ARDuino Freertos ToCKer
cmake_minimum_required(VERSION 3.21)
project(unofreertos VERSION 0.1.0)
include(cmake/CPM.cmake)
add_library(freertos_config INTERFACE)
target_include_directories(freertos_config SYSTEM INTERFACE src)
target_compile_definitions(freertos_config INTERFACE projCOVERAGE_TEST=0)
set(FREERTOS_HEAP "4" CACHE STRING "" FORCE)
set(FREERTOS_PORT "GCC_ATMEGA" CACHE STRING "" FORCE)
file(DOWNLOAD "https://raw.githubusercontent.com/Optiboot/optiboot/master/optiboot/bootloaders/optiboot/optiboot_atmega328.hex" optiboot_atmega328.hex)
CPMAddPackage("gh:freertos/freertos-kernel#V11.0.0")
add_avr_executable(${PROJECT_NAME}
    src/main.cpp
    src/hooks.cpp
)
target_compile_definitions(${PROJECT_NAME}.elf PUBLIC F_CPU=16000000UL)
target_include_directories(${PROJECT_NAME}.elf PUBLIC
    src
    ${freertos-kernel_SOURCE_DIR}/include
    ${freertos-kernel_SOURCE_DIR}/portable/ThirdParty/GCC/ATmega
)
target_link_libraries(${PROJECT_NAME}.elf freertos_kernel)

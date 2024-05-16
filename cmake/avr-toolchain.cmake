find_program(AVR_CC avr-gcc REQUIRED)
find_program(AVR_CXX avr-g++ REQUIRED)
find_program(AVR_OBJCOPY avr-objcopy REQUIRED)
find_program(AVR_OBJDUMP avr-objdump REQUIRED)
find_program(AVR_SIZE avr-size REQUIRED)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR avr)
set(CMAKE_C_COMPILER ${AVR_CC})
set(CMAKE_CXX_COMPILER ${AVR_CXX})
set(AVR 1)

if(NOT AVR_MCU)
  set(AVR_MCU atmega328p CACHE STRING "Default: atmega328p (arduino uno) (see 'avr-gcc --target-help' for avalable options)")
endif()

add_compile_options(-mmcu=${AVR_MCU})
add_link_options(-mmcu=${AVR_MCU} -Wl,--gc-sections -mrelax)

function(add_avr_executable EXECUTABLE_NAME)
  if(NOT ARGN)
    message(FATAL_ERROR "No source filed given for ${EXECUTABLE_NAME}")
  endif()

  set(elf_file ${EXECUTABLE_NAME}.elf)
  set(hex_file ${EXECUTABLE_NAME}.hex)
  set(lst_file ${EXECUTABLE_NAME}.lst)
  set(map_file ${EXECUTABLE_NAME}.map)
  set(eeprom_image ${EXECUTABLE_NAME}-eeprom.hex)

  set(${EXECUTABLE_NAME}_ELF_TARGET ${elf_file} PARENT_SCOPE)
  set(${EXECUTABLE_NAME}_HEX_TARGET ${hex_file} PARENT_SCOPE)
  set(${EXECUTABLE_NAME}_LST_TARGET ${lst_file} PARENT_SCOPE)
  set(${EXECUTABLE_NAME}_MAP_TARGET ${map_file} PARENT_SCOPE)
  set(${EXECUTABLE_NAME}_EEPROM_TARGET ${eeprom_file} PARENT_SCOPE)
  add_executable(${elf_file} EXCLUDE_FROM_ALL ${ARGN})

  set_target_properties(${elf_file} 
    PROPERTIES LINK_FLAGS "-Wl,--gc-sections -mrelax -Wl,-Map,${map_file}")

  add_custom_command(OUTPUT ${hex_file}
    COMMAND ${AVR_OBJCOPY} -j .text -j .data -O ihex ${elf_file} ${hex_file}
    COMMAND ${AVR_SIZE} ${AVR_SIZE_ARGS} ${elf_file}
    DEPENDS ${elf_file})

  add_custom_command(OUTPUT ${lst_file}
    COMMAND ${AVR_OBJDUMP} -d ${elf_file} > ${lst_file}
    DEPENDS ${elf_file})

  add_custom_command(OUTPUT ${eeprom_image}
    COMMAND ${AVR_OBJCOPY} -j .eeprom --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0 --no-change-warnings -O ihex ${elf_file} ${eeprom_image}
    DEPENDS ${elf_file})

  add_custom_target(${EXECUTABLE_NAME} ALL
    DEPENDS ${hex_file} ${lst_file} ${eeprom_image})

  set_target_properties(${EXECUTABLE_NAME} PROPERTIES OUTPUT_NAME "${elf_file}")

  get_directory_property(clean_files ADDITIONAL_MAKE_CLEAN_FILES)
  set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${map_file}")

  add_custom_target(disassemble_${EXECUTABLE_NAME} ${AVR_OBJDUMP} -h -S ${elf_file} > ${EXECUTABLE_NAME}.lst DEPENDS ${elf_file})
endfunction()

set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

# project(m68kmake)

# cmake_minimum_required(VERSION 2.6)
if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif (MSVC)

add_executable(m68kmake m68kmake.c)

add_custom_command( OUTPUT m68kops.c m68kops.h
                    COMMAND m68kmake ${CMAKE_CURRENT_BINARY_DIR}/ ${CMAKE_CURRENT_SOURCE_DIR}/m68k_in.c
                    DEPENDS m68kmake.c
                    COMMENT "Generating musashi 68k core"
                    VERBATIM)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(musashi m68kcpu.c m68kdasm.c m68kops.c m68kops.h softfloat/softfloat.c)
target_include_directories(musashi PUBLIC "${PROJECT_SOURCE_DIR}/Musashi")
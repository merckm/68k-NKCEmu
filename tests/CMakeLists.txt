# FILEPATH: /D:/Sandbox/Historical/ndr-klein/68k-NKCEmu/tests/CMakeList.txt

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdl2)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${SDL2_IMAGE_INCLUDE_DIR})

list(APPEND EXTRA_LIBS Ayumi)
list(APPEND EXTRA_INCLUDES "${PROJECT_BUILD_DIR}/ayumi")

list(APPEND EXTRA_LIBS musashi)
list(APPEND EXTRA_INCLUDES "${PROJECT_BUILD_DIR}/Musashi" )


# Add the test executable
add_executable( ConfigTest config_test.c
                ../nkcgui.c
                ../gui_button.c
                ../gui_file.c
                ../gui_group.c
                ../util.c
                ../directory.c
                ../config.c
                ../log.c
                ../bankboot.c
                ../gdp64.c
                ../col256.c
                ../key.c
                ../cas.c
                ../ioe.c
                ../centronics.c
                ../flo2.c
                ../crc.c
                ../promer.c 
                ../sound.c
                ../uhr.c
)

# Add the test to CTest
# add_test(ConfigTest ConfigTest)
add_test(NAME ConfigTest COMMAND ConfigTest 1)

target_link_libraries(ConfigTest ${EXTRA_LIBS} yaml SDL2::Main SDL2::Image SDL2::TTF -lm)

# Set the test properties
set_tests_properties(ConfigTest PROPERTIES TIMEOUT 10)

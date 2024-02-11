cmake_minimum_required(VERSION 3.10)
project(68k-nkcemu VERSION 1.0.0)

if (MSVC)
    # warning level 4
    add_compile_options(/W4)
else()
    # additional warnings
    add_compile_options(-O3)
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdl2)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${SDL2_IMAGE_INCLUDE_DIR})

add_subdirectory(ayumi)
list(APPEND EXTRA_LIBS Ayumi)
list(APPEND EXTRA_INCLUDES "${PROJECT_SOURCE_DIR}/ayumi")

add_subdirectory(Musashi)
list(APPEND EXTRA_LIBS musashi)
list(APPEND EXTRA_INCLUDES "${PROJECT_SOURCE_DIR}/Musashi" )

if ( APPLE )
        set(HOMEBREW_PREFIX "/opt/homebrew"
                CACHE PATH "Path to Homebrew installation")
        include_directories( ${HOMEBREW_PREFIX}/include )
        link_directories(${HOMEBREW_PREFIX}/lib)
endif ()

add_executable(${PROJECT_NAME} 68k-nkcemu.c
                      nkcgui.c
                      gui_button.c
                      gui_file.c
                      gui_group.c
                      util.c
                      directory.c
                      config.c
                      log.c
                      bankboot.c
                      gdp64.c
                      col256.c
                      key.c
                      cas.c
                      ioe.c
                      centronics.c
                      flo2.c
                      crc.c
                      promer.c 
                      sound.c
                      uhr.c
                      m68kconf.h)

target_link_libraries(${PROJECT_NAME} ${EXTRA_LIBS} ${SDL2_LIBRARIES} yaml SDL2::Image SDL2::TTF -lm)
if(MINGW)
        if(CMAKE_BUILD_TYPE MATCHES Debug)
                message("DEBUG BUILD")
                target_link_libraries(${PROJECT_NAME} ${EXTRA_LIBS} ${SDL2_LIBRARIES} yaml SDL2::Image SDL2::TTF -lm)
        else()
                message("RELEASE BUILD ${CMAKE_BUILD_TYPE}")
                target_link_libraries(${PROJECT_NAME} ${EXTRA_LIBS} ${SDL2_LIBRARIES} yaml SDL2::Image SDL2::TTF -mwindows -lm)
        endif()
endif()

add_custom_command(TARGET 68k-nkcemu POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/resources
        ${PROJECT_BINARY_DIR}/resources
        COMMENT "Copying resources into binary directory")       

add_custom_command(TARGET 68k-nkcemu POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${PROJECT_SOURCE_DIR}/config.yaml
        ${PROJECT_BINARY_DIR}/config.yaml
        COMMENT "Copying configuration file")       

add_custom_command(TARGET 68k-nkcemu POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${PROJECT_SOURCE_DIR}/config_43.yaml
        ${PROJECT_BINARY_DIR}/config_43.yaml
        COMMENT "Copying configuration file sample 4.3")       

add_custom_command(TARGET 68k-nkcemu POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${PROJECT_SOURCE_DIR}/config_622.yaml
        ${PROJECT_BINARY_DIR}/config_622.yaml
        COMMENT "Copying configuration file sample 6.22")       

add_custom_command(TARGET 68k-nkcemu POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${PROJECT_SOURCE_DIR}/config_701.yaml
        ${PROJECT_BINARY_DIR}/config_701.yaml
        COMMENT "Copying configuration file sample 7.01")       

add_custom_command(TARGET 68k-nkcemu POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${PROJECT_SOURCE_DIR}/config_710.yaml
        ${PROJECT_BINARY_DIR}/config_710.yaml
        COMMENT "Copying configuration file sample 7.10")       

add_custom_command(TARGET 68k-nkcemu POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${PROJECT_SOURCE_DIR}/list.lst
        ${PROJECT_BINARY_DIR}/list.lst
        COMMENT "Copying empty listing file")       


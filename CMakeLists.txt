cmake_minimum_required(VERSION 2.8.4)
project(highcaliber)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wpedantic")

set(SOURCE_FILES
    application.h
    clock.h
    common.h
    display.h
    image.h
    painter.h
    log.h
    main.cpp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "/projects/highcaliber_build")

add_executable(highcaliber ${SOURCE_FILES})

#set_target_properties(highcaliber PROPERTIES LINK_FLAGS "-mwindows")

target_link_libraries(highcaliber
                      mingw32
                      SDL2main
                      SDL2
                      SDL2_image
                      )

add_custom_command(TARGET highcaliber PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/external $<TARGET_FILE_DIR:highcaliber>)

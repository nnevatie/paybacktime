cmake_minimum_required(VERSION 2.8.4)
project(highcaliber)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}
    -std=c++11 -Wall -Wno-unused-function -fopenmp")

set(HEADER_FILES
    geometry.h
    mc_mesher.h
    sn_mesher.h
    )

set(SOURCE_FILES
    main.cpp
    application.cpp
    display.cpp
    painter.cpp
    image.cpp
    image_cube.cpp
    image_mesher.cpp
    common.cpp
    clock.cpp
    log.cpp
    sdf_primitives.cpp
    ref_mesher.cpp
    image_mesher.cpp
    gl_buffers.cpp
    gl_shaders.cpp
    gl_mesh.cpp
    render_stats.cpp
    nanovg.c
    )

# System dependencies
set(SYS_LIBS
    mingw32
    )

# Boost dependencies
set(BOOST_LIBS
    boost_system
    boost_filesystem
    )

# SDL dependencies
set(SDL_LIBS
    SDL2main
    SDL2
    SDL2_image
    )

# OpenGL dependencies
set(OPENGL_LIBS
    glew32
    opengl32
    )

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME}
                      ${SYS_LIBS}
                      ${BOOST_LIBS}
                      ${SDL_LIBS}
                      ${OPENGL_LIBS}
                      )

################################################################################
# Installation
################################################################################
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/install)

# Binaries
install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION .)

# Runtime
file(GLOB runtime_files
    "external/*.dll")

install(FILES ${runtime_files}
        DESTINATION .)

# Data
file(GLOB data_files
    "data/*")

install(FILES ${data_files}
        DESTINATION data)

cmake_minimum_required(VERSION 2.8.4)
project(highcaliber)

# Installer target/macro
add_custom_target(install_files_target COMMENT "Installing files...")
macro(install_files src dest)
    message(STATUS "Installing files ${src} -> ${dest}")
    add_custom_command(TARGET install_files_target POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/${src} ${DEPLOY_DIR}/${dest})
endmacro(install_files)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wpedantic")

set(SOURCE_FILES
    main.cpp
    application.cpp
    display.cpp
    painter.cpp
    image.cpp
    image_cube.cpp
    common.cpp
    clock.cpp
    log.cpp
    sdf_primitives.cpp
    reference_extractor.cpp
    gl_buffers.cpp
    gl_shaders.cpp
    gl_mesh.cpp
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

set(DEPLOY_DIR /projects/highcaliber_build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${DEPLOY_DIR})

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME}
                      ${SYS_LIBS}
                      ${BOOST_LIBS}
                      ${SDL_LIBS}
                      ${OPENGL_LIBS}
                      )

install_files(external .)
install_files(data data)

add_dependencies(${PROJECT_NAME} install_files_target)

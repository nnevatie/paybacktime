cmake_minimum_required(VERSION 2.8.4)
project(highcaliber)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}
    -std=c++11 -Wall -Wextra -Wno-unused-function -fopenmp")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}
    -Wl,--stack,16777216")

set(HEADER_FILES
    optional.h
    file_system.h
    application.h
    display.h
    painter.h
    geometry.h
    image.h
    image_cube.h
    image_mesher.h
    image_atlas.h
    texture_atlas.h
    common.h
    clock.h
    log.h
    sdf_primitives.h
    ref_mesher.h
    image_mesher.h
    image_atlas.h
    gl_uniforms.h
    gl_texture.h
    gl_buffers.h
    gl_shaders.h
    gl_primitive.h
    gl_fbo.h
    gl_rbo.h
    render_stats.h
    mesh.h
    sn_mesher.h
    ssao.h
    scene.h
    object.h
    object_store.h
    )

set(SOURCE_FILES
    main.cpp
    application.cpp
    display.cpp
    painter.cpp
    image.cpp
    image_cube.cpp
    image_mesher.cpp
    image_atlas.cpp
    texture_atlas.cpp
    common.cpp
    clock.cpp
    log.cpp
    sdf_primitives.cpp
    ref_mesher.cpp
    image_mesher.cpp
    gl_texture.cpp
    gl_buffers.cpp
    gl_shaders.cpp
    gl_primitive.cpp
    gl_fbo.cpp
    gl_rbo.cpp
    render_stats.cpp
    mesh.cpp
    ssao.cpp
    scene.cpp
    object.cpp
    object_store.cpp
    )

set(EXT_HEADER_FILES
    ext/stb_image.h
    ext/stb_image_write.h
    ext/stb_truetype.h
    ext/nanovg.h
    ext/nanovg_gl.h
    ext/nanovg_gl_utils.h
    ext/fontstash.h
    )

set(EXT_SOURCE_FILES
    ext/nanovg.c
    )

set(SHADER_FILES
    shaders/simple.vs.glsl
    shaders/geometry.vs.glsl
    shaders/geometry.fs.glsl
    shaders/passthrough.vs.glsl
    shaders/color.fs.glsl
    shaders/screenspace.fs.glsl
    shaders/lambert.fs.glsl
    shaders/phong.fs.glsl
    shaders/ssao.fs.glsl
    shaders/blur.fs.glsl
    shaders/texture.fs.glsl
    shaders/wireframe.gs.glsl
    )

# System dependencies
set(SYS_LIBS
    mingw32
    )

# Boost dependencies
set(BOOST_LIBS
    boost_system
    boost_filesystem
    boost_program_options
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

# Build definitions
add_definitions(-D_USE_MATH_DEFINES
                -DGLEW_STATIC)

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES}
                               ${EXT_HEADER_FILES} ${EXT_SOURCE_FILES}
                               ${SHADER_FILES})

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

# Shaders
file(GLOB shader_files
    "shaders/*")

install(FILES ${shader_files}
        DESTINATION shaders)

# Runtime
file(GLOB runtime_files
    "runtime/*.dll")

install(FILES ${runtime_files}
        DESTINATION .)

# Objects
file(GLOB data_files
    "data/*")

install(FILES ${data_files}
        DESTINATION data)

# Objects
file(GLOB object_files
    "objects/*")

install(FILES ${object_files}
        DESTINATION objects)

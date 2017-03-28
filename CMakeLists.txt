cmake_minimum_required(VERSION 2.8.4)
project(paybacktime)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}
    -Wall \
    -Wno-unused-function \
    -Wno-ignored-attributes \
    -Wno-deprecated-declarations \
    -Wno-misleading-indentation \
    -fopenmp -ffast-math -march=core2")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}
    -Wl,--stack,16777216")

file(GLOB HEADER_FILES
    *.h
    platform/*.h
    img/*.h
    geom/*.h
    scene/*.h
    common/*.h
    gfx/*.h
    ui/*.h
    gl/*.h
    )

file(GLOB SOURCE_FILES
    *.cpp
    platform/*.cpp
    img/*.cpp
    geom/*.cpp
    scene/*.cpp
    common/*.cpp
    gfx/*.cpp
    ui/*.cpp
    gl/*.cpp
    )

file(GLOB EXT_HEADER_FILES
    )

file(GLOB EXT_SOURCE_FILES
    ext/nanovg/src/*.c
    ext/nanogui-sdl/src/*.cpp
    )

file(GLOB SHADER_FILES
    shaders/*.glsl
    )

file(GLOB RESOURCE_FILES
    res/*.rc
    )

# System dependencies
set(SYS_LIBS
    mingw32
    )

# Boost dependencies
set(BOOST_LIBS
    boost_system
    boost_chrono
    boost_thread
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
    glbinding
    opengl32
    )

# Ozz dependencies
set(OZZ_LIBS
    ozz_animation_offline_r
    ozz_base_r
    )

# Include paths
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/ext
    ${CMAKE_SOURCE_DIR}/ext/stb
    ${CMAKE_SOURCE_DIR}/ext/eigen
    ${CMAKE_SOURCE_DIR}/ext/nanovg/src
    ${CMAKE_SOURCE_DIR}/ext/nanogui-sdl/include
    ${CMAKE_SOURCE_DIR}/ext/gsl-lite/include
    )

# Build definitions
add_definitions(-DEIGEN_NO_DEBUG
                -DEIGEN_DONT_PARALLELIZE
                -DGL_GLEXT_PROTOTYPES
                -DNANOVG_GL3_IMPLEMENTATION
                -DBOOST_THREAD_VERSION=4
                -DGLM_FORCE_SWIZZLE)

add_library(${PROJECT_NAME}_ext ${EXT_HEADER_FILES} ${EXT_SOURCE_FILES})

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES}
                               ${SHADER_FILES} ${RESOURCE_FILES})

target_link_libraries(${PROJECT_NAME}
                      ${PROJECT_NAME}_ext
                      ${SYS_LIBS}
                      ${BOOST_LIBS}
                      ${SDL_LIBS}
                      ${OPENGL_LIBS}
                      ${OZZ_LIBS}
                      )

################################################################################
# Installation
################################################################################
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/install)

function(install_files path dst)
    file(GLOB entries ${path})
    foreach(entry ${entries})
       IF(IS_DIRECTORY "${entry}" )
          LIST(APPEND install_dirs "${entry}")
       ELSE()
          LIST(APPEND install_files "${entry}")
       ENDIF()
    endforeach()
    install(DIRECTORY ${install_dirs}  DESTINATION ${dst})
    install(FILES     ${install_files} DESTINATION ${dst})
endfunction()

# Binaries
install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION .)

# Runtime and data
install_files("runtime/*.dll" .)
install_files("data/*"        data)
install_files("shaders/*"     shaders)
install_files("objects/*"     objects)
install_files("characters/*"  characters)
install_files("horizons/*"    horizons)
install_files("scenes/*"      scenes)

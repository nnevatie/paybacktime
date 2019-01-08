# General
cmake_minimum_required(VERSION 3.1)
project(paybacktime DESCRIPTION "Payback Time" LANGUAGES C CXX)

# Language standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Package dependencies
find_package(Boost REQUIRED COMPONENTS filesystem program_options)
find_package(cereal CONFIG REQUIRED)
find_package(Eigen3 CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(nanovg CONFIG REQUIRED)

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

file(GLOB EXT_SOURCE_FILES
    ext/glad/src/*.c
    ext/nanovg/src/*.c
    ext/nanogui-sdl/src/*.cpp
    ext/ozz-animation/src_fused/*.cc
    )

file(GLOB SHADER_FILES
    shaders/*.glsl
    )

file(GLOB RESOURCE_FILES
    #res/*.rc
    )

# OpenGL dependencies
set(OPENGL_LIBS
    opengl32
    )

# Include paths
include_directories(${CMAKE_SOURCE_DIR})

find_path(NANOVG_INCLUDE_DIR nanovg.h)
include_directories(${NANOVG_INCLUDE_DIR})

include_directories(
    ${CMAKE_SOURCE_DIR}/ext
    ${CMAKE_SOURCE_DIR}/ext/glad/include
    ${CMAKE_SOURCE_DIR}/ext/glad/include/glad
    ${CMAKE_SOURCE_DIR}/ext/glad/include/KHR
    ${CMAKE_SOURCE_DIR}/ext/nanogui-sdl/include
    ${CMAKE_SOURCE_DIR}/ext/ozz-animation/include
    )

# Build definitions
add_definitions(-DEIGEN_NO_DEBUG
                -DEIGEN_DONT_PARALLELIZE
                -DGL_GLEXT_PROTOTYPES
                -DNANOVG_GL3_IMPLEMENTATION
                -DBOOST_THREAD_VERSION=4
                -DGLM_FORCE_CXX14
                -DGLM_FORCE_SWIZZLE
                -DGLM_FORCE_CTOR_INIT
                -DGLM_ENABLE_EXPERIMENTAL
                )

# Compile options
add_compile_options(-Wno-pragma-pack -Wno-deprecated-declarations)

add_executable(${PROJECT_NAME} ${HEADER_FILES}
                               ${SOURCE_FILES} ${EXT_SOURCE_FILES}
                               ${SHADER_FILES} ${RESOURCE_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE
                      ${OPENGL_LIBS}
                      Boost::filesystem
                      Boost::program_options
                      nanovg::nanovg
                      Eigen3::Eigen
                      SDL2::SDL2
                      SDL2::SDL2main
                      cereal
                      glm
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
install_files("animations/*"  animations)
install_files("horizons/*"    horizons)
install_files("scenes/*"      scenes)

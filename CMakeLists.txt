cmake_minimum_required(VERSION 2.8.4)
project(paybacktime)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}
    -std=c++11 -Wall -Wextra -Wno-unused-function -fopenmp")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}
#    -std=c++11 -Wall -Wextra -Wno-unused-function -fopenmp -Ofast -m64")

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
    gl/*.cpp
    )

file(GLOB EXT_HEADER_FILES
    ext/*.h
    )

file(GLOB EXT_SOURCE_FILES
    ext/*.c
    ext/*.cpp
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
    opengl32
    )

# Include paths
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/ext
    )

# Build definitions
add_definitions(-D_USE_MATH_DEFINES
                -DBOOST_THREAD_VERSION=4)

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES}
                               ${EXT_HEADER_FILES} ${EXT_SOURCE_FILES}
                               ${SHADER_FILES} ${RESOURCE_FILES})

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
file(GLOB object_entries
    "objects/*")

foreach(entry ${object_entries})
   IF(IS_DIRECTORY "${entry}" )
      LIST(APPEND object_dirs "${entry}")
   ELSE()
      LIST(APPEND object_files "${entry}")
   ENDIF()
endforeach()

install(DIRECTORY ${object_dirs}
        DESTINATION objects)
install(FILES ${object_files}
        DESTINATION objects)

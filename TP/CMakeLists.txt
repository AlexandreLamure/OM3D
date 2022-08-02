cmake_minimum_required(VERSION 3.20)
project(TP)

# CPP setup
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(EXTRA_WARNINGS -Wfloat-conversion -Woverloaded-virtual -Wnon-virtual-dtor)
set(COMPILE_OPTIONS -pedantic -Wall -Wextra ${EXTRA_WARNINGS})


# setup external libraries
add_subdirectory(external/glfw)
include_directories(external/glfw/include)

include_directories(external/glad/include) # glad generated using https://gen.glad.sh/#generator=c&api=gl%3D4.4&profile=gl%3Dcore%2Cgles1%3Dcommon&options=HEADER_ONLY

file(GLOB_RECURSE SOURCE_FILES
        "src/*.h"
        "src/*.cpp"
    )

#file(GLOB_RECURSE EXTERNAL_FILES
#        "external/imgui/*.cpp"
#        "external/imgui/*.h"
#    )

# Shader files
file(GLOB_RECURSE SHADER_FILES
        "shaders/*.frag"
        "shaders/*.vert"
        "shaders/*.geom"
        "shaders/*.comp"
        "shaders/*.glsl"
    )



add_executable(TP ${SOURCE_FILES} ${SHADER_FILES})
target_link_libraries(TP glfw)
target_compile_options(TP PUBLIC ${COMPILE_OPTIONS})

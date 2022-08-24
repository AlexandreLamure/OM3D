cmake_minimum_required(VERSION 3.20)
project(TP)

# CPP setup
set(CMAKE_CXX_STANDARD 17)
if(MSVC)
    set(COMPILE_OPTIONS /permissive- /W3 /WX /fp:fast -D_CRT_SECURE_NO_DEPRECATE /Zc:preprocessor /wd5105)
else()
    set(EXTRA_WARNINGS -Wfloat-conversion -Woverloaded-virtual -Wnon-virtual-dtor)
    set(COMPILE_OPTIONS -pedantic -Wall -Wextra ${EXTRA_WARNINGS})
endif()


# setup external libraries
add_subdirectory(external/glfw)
add_subdirectory(external/glm)

include_directories(external/glfw/include)
include_directories(external/glad/include)
include_directories(external/glm)
include_directories(external)

include_directories(${TP_SOURCE_DIR}/src)


file(GLOB_RECURSE SOURCE_FILES
        "src/*.h"
        "src/*.cpp"
    )

file(GLOB_RECURSE EXTERNAL_FILES
        "external/glad/*.c"
        "external/glad/*.h"
        "external/imgui/*.cpp"
        "external/imgui/*.h"
    )

# Shader files
file(GLOB_RECURSE SHADER_FILES
        "shaders/*.frag"
        "shaders/*.vert"
        "shaders/*.geom"
        "shaders/*.comp"
        "shaders/*.glsl"
    )



add_executable(TP ${SOURCE_FILES} ${EXTERNAL_FILES} ${SHADER_FILES})
target_link_libraries(TP glfw)
target_compile_options(TP PUBLIC ${COMPILE_OPTIONS})

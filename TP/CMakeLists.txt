cmake_minimum_required(VERSION 3.18)
project(TP)

# CPP setup
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(EXTRA_WARNINGS
            -Wzero-as-null-pointer-constant
            -Wfloat-conversion
            -Woverloaded-virtual
            -Wnon-virtual-dtor
            -Wlogical-op
            -Wredundant-decls
            -Wundef
            -Wno-gnu-zero-variadic-macro-arguments
        )
set(COMPILE_OPTIONS -pedantic -Wall -Wextra ${EXTRA_WARNINGS})


# setup includes
# add_subdirectory(external/spirv_cross)


file(GLOB_RECURSE SOURCE_FILES
        "src/*.h"
        "src/*.cpp"
    )


file(GLOB_RECURSE EXTERNAL_FILES
        # "external/imgui/*.cpp"
        # "external/imgui/*.h"

        # "external/tinygltf/*.hpp"
        # "external/tinygltf/*.h"
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
target_compile_options(TP PUBLIC ${COMPILE_OPTIONS})

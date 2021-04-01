cmake_minimum_required(VERSION 3.15)
include(FetchContent)

string(FIND "${CMAKE_CXX_COMPILER}" "clang++" GRAVITY_SIMULATION_COMPILER_CLANGPP)
if(GRAVITY_SIMULATION_COMPILER_CLANGPP GREATER -1)
  set(GRAVITY_SIMULATION_COMPILER_CLANGPP ON)
else()
  set(GRAVITY_SIMULATION_COMPILER_CLANGPP OFF)
endif()

if(GRAVITY_SIMULATION_COMPILER_CLANGPP)
    set(GRAVITY_SIMULATION_COMPILE_FLAGS
        -Wall -Wextra -pedantic
        -ferror-limit=1
        -Wnon-virtual-dtor
        -Werror=return-type
        -fno-char8_t # Yea, just no.
        -fno-rtti 
        -fno-exceptions
        -fno-math-errno
    )
endif()

project(gravity_simulation)

# Add anton_types
FetchContent_Declare(
    anton_types
    GIT_REPOSITORY https://github.com/kociap/anton_types.git
    GIT_TAG 1b25551af3b508a2dd560b15f4b2f1ac1879542c
)
FetchContent_MakeAvailable(anton_types)
# Add anton_math
FetchContent_Declare(
    anton_math
    GIT_REPOSITORY https://github.com/kociap/anton_math.git
    GIT_TAG d28ae9df306f8ec1a93ff5bbc293917b7119897e
)
FetchContent_MakeAvailable(anton_math)
# Add anton_core
FetchContent_Declare(
    anton_core
    GIT_REPOSITORY https://github.com/kociap/anton_core.git
    GIT_TAG 2c310a1d7b4e88b9822cf67fe417f0d1695d1f73
)
FetchContent_MakeAvailable(anton_core)
# Add mimas
FetchContent_Declare(
    mimas
    GIT_REPOSITORY https://github.com/kociap/mimas.git
    GIT_TAG 3b81b83e1b19e0bf570742d6475153283436ca88
)
FetchContent_MakeAvailable(mimas)
# Add glad
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/kociap/glad.git
    GIT_TAG b4d4f69539196fdb08a56b88fd15b7287c838b87
)
FetchContent_MakeAvailable(glad)

add_executable(gravity_simulation
    "${CMAKE_CURRENT_SOURCE_DIR}/source/build.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/handle.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/input.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/mesh.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/physics.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/point_mass.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/rendering.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/shader.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/world.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/entity.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/input.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/mesh.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/physics.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/rendering.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/shader.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/transform.hpp"
)
set_target_properties(gravity_simulation PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF)
target_compile_options(gravity_simulation PRIVATE ${GRAVITY_SIMULATION_COMPILE_FLAGS})
target_link_libraries(gravity_simulation PUBLIC anton_core mimas glad)
target_include_directories(gravity_simulation
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/source"
)

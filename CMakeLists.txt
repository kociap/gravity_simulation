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

# Add anton_core
FetchContent_Declare(
    anton_core
    GIT_REPOSITORY https://github.com/kociap/anton_core.git
)
FetchContent_MakeAvailable(anton_core)
# Add mimas
FetchContent_Declare(
    mimas
    GIT_REPOSITORY https://github.com/kociap/mimas.git
)
FetchContent_MakeAvailable(mimas)
# Add glad
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/kociap/glad.git
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

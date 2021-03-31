#pragma once

#include <anton/math/mat4.hpp>
#include <build.hpp>
#include <world.hpp>

void init_rendering();

void render(World& world, Mat4 const& view, Mat4 const& proj);

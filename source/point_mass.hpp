#pragma once

#include <anton/math/vec2.hpp>
#include <build.hpp>

struct Point_Mass {
    Vec2 position;
    Vec2 velocity;
    f32 mass;
};

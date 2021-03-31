#pragma once

#include <anton/math/quat.hpp>
#include <anton/math/vec3.hpp>
#include <build.hpp>

struct Transform {
    Vec3 postion;
    Vec3 scale = Vec3{1.0f, 1.0f, 1.0f};
    Quat orientation;
};

#pragma once

#include <world.hpp>

struct Physics_World;

[[nodiscard]] Physics_World* create_physics_world();
void destory_physics_world(Physics_World* physics_world);

// run_physics
// Run n steps of physics simulation with a fixed delta time of 1/60 seconds.
//
void run_physics(Physics_World& physics_world, World& world, f32 delta_time);

#include <physics.hpp>

#include <point_mass.hpp>

constexpr f32 fixed_delta_time = 1.0f / 240.0f;
constexpr f32 gravitational_constant = 6.67408e-11f;

struct Physics_World {
    f32 delta_time = 0.0f;
};

Physics_World* create_physics_world() {
    Physics_World* physics_world = new Physics_World;
    return physics_world;
}

void destory_physics_world(Physics_World* physics_world) {
    delete physics_world;
}

void run_physics(Physics_World& physics_world, World& world, f32 const delta_time) {
    physics_world.delta_time += delta_time;
    while(physics_world.delta_time >= fixed_delta_time) {
        physics_world.delta_time -= fixed_delta_time;
        Slice<Point_Mass> point_masses = world.components<Point_Mass>();
        Array<Point_Mass> point_masses_next{reserve, point_masses.size()};
        for(Point_Mass const& e: point_masses) {
            Point_Mass point_mass_next = e;
            // Sum of accelerations at t
            Vec2 acceleration1;
            for(Point_Mass const& point_mass: point_masses) {
                // Skip self
                if(&e == &point_mass) {
                    continue;
                }

                Vec2 const distance_vec = point_mass.position - point_mass_next.position;
                f32 const distance = math::length(distance_vec);
                if(!is_almost_zero(distance, 1.0f)) {
                    Vec2 const direction_vec = distance_vec / distance;
                    f32 const acceleration_magnitude = point_mass.mass / distance / distance * gravitational_constant;
                    acceleration1 += direction_vec * acceleration_magnitude;
                }
            }

            point_mass_next.position =
                point_mass_next.position + point_mass_next.velocity * fixed_delta_time + 0.5 * acceleration1 * fixed_delta_time * fixed_delta_time;

            // Sum of accelerations at t+dt
            Vec2 acceleration2;
            for(Point_Mass const& point_mass: point_masses) {
                // Skip self
                if(&e == &point_mass) {
                    continue;
                }

                Vec2 const distance_vec = point_mass.position - point_mass_next.position;
                f32 const distance = math::length(distance_vec);
                if(!is_almost_zero(distance, 1.0f)) {
                    Vec2 const direction_vec = distance_vec / distance;
                    f32 const acceleration_magnitude = point_mass.mass / distance / distance * gravitational_constant;
                    acceleration2 += direction_vec * acceleration_magnitude;
                }
            }

            point_mass_next.velocity = point_mass_next.velocity + 0.5 * (acceleration1 + acceleration2) * fixed_delta_time;
            point_masses_next.emplace_back(point_mass_next);
        }
        copy(point_masses_next.begin(), point_masses_next.end(), point_masses.begin());
    }
}

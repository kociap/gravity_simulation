#version 450 core

struct Point_Mass {
    vec2 position;
    float mass;
};

layout(binding = 1, std430) readonly buffer PM_Buffer {
    Point_Mass point_masses[];
};

uniform float max_field;
uniform int render_mode;
const float isoline_levels = 32;

in vec2 world_position;
out vec4 out_color;

const vec4[32] color_lut = vec4[32](vec4(255.0 / 255.0, 0.0 / 255.0, 0.0 / 255.0, 1.0),
                                    vec4(253.0 / 255.0, 28.0 / 255.0, 0.0 / 255.0, 1.0),
                                    vec4(250.0 / 255.0, 55.0 / 255.0, 0.0 / 255.0, 1.0),
                                    vec4(247.0 / 255.0, 83.0 / 255.0, 0.0 / 255.0, 1.0),
                                    vec4(245.0 / 255.0, 110.0 / 255.0, 0.0 / 255.0, 1.0),
                                    vec4(242.0 / 255.0, 138.0 / 255.0, 0.0 / 255.0, 1.0),
                                    vec4(239.0 / 255.0, 165.0 / 255.0, 0.0 / 255.0, 1.0),
                                    vec4(236.0 / 255.0, 193.0 / 255.0, 0.0 / 255.0, 1.0),
                                    vec4(229.0 / 255.0, 215.0 / 255.0, 5.0 / 255.0, 1.0),
                                    vec4(208.0 / 255.0, 220.0 / 255.0, 25.0 / 255.0, 1.0),
                                    vec4(188.0 / 255.0, 226.0 / 255.0, 45.0 / 255.0, 1.0),
                                    vec4(167.0 / 255.0, 231.0 / 255.0, 65.0 / 255.0, 1.0),
                                    vec4(147.0 / 255.0, 237.0 / 255.0, 85.0 / 255.0, 1.0),
                                    vec4(126.0 / 255.0, 242.0 / 255.0, 105.0 / 255.0, 1.0),
                                    vec4(106.0 / 255.0, 248.0 / 255.0, 125.0 / 255.0, 1.0),
                                    vec4(85.0 / 255.0, 253.0 / 255.0, 145.0 / 255.0, 1.0),
                                    vec4(70.0 / 255.0, 243.0 / 255.0, 161.0 / 255.0, 1.0),
                                    vec4(60.0 / 255.0, 217.0 / 255.0, 174.0 / 255.0, 1.0),
                                    vec4(51.0 / 255.0, 192.0 / 255.0, 187.0 / 255.0, 1.0),
                                    vec4(41.0 / 255.0, 166.0 / 255.0, 200.0 / 255.0, 1.0),
                                    vec4(31.0 / 255.0, 141.0 / 255.0, 213.0 / 255.0, 1.0),
                                    vec4(22.0 / 255.0, 115.0 / 255.0, 226.0 / 255.0, 1.0),
                                    vec4(12.0 / 255.0, 90.0 / 255.0, 239.0 / 255.0, 1.0),
                                    vec4(2.0 / 255.0, 64.0 / 255.0, 253.0 / 255.0, 1.0),
                                    vec4(0.0 / 255.0, 52.0 / 255.0, 231.0 / 255.0, 1.0),
                                    vec4(0.0 / 255.0, 45.0 / 255.0, 198.0 / 255.0, 1.0),
                                    vec4(0.0 / 255.0, 37.0 / 255.0, 165.0 / 255.0, 1.0),
                                    vec4(0.0 / 255.0, 30.0 / 255.0, 132.0 / 255.0, 1.0),
                                    vec4(0.0 / 255.0, 22.0 / 255.0, 99.0 / 255.0, 1.0),
                                    vec4(0.0 / 255.0, 15.0 / 255.0, 66.0 / 255.0, 1.0),
                                    vec4(0.0 / 255.0, 7.0 / 255.0, 33.0 / 255.0, 1.0),
                                    vec4(0.0 / 255.0, 0.0 / 255.0, 0.0 / 255.0, 1.0));

void main() {
    vec2 accel = vec2(0, 0);
    for(int i = 0; i < point_masses.length(); ++i) {
        vec2 dist_vec = point_masses[i].position - world_position;
        float dist = length(dist_vec);
        float accel_strength = 6.67408E-11 * point_masses[i].mass / dist / dist;
        vec2 dist_vec_norm = dist_vec / dist;
        accel += accel_strength * dist_vec_norm;
    }

    float field_strength = length(accel);
    float linear_strength = log(field_strength);
    float interval = log(max_field) / isoline_levels;
    float interval_index = linear_strength / interval;
    switch(render_mode) {
        case 1: {
            float frac = fract(interval_index);
            float upper = smoothstep(0.9, 1.0, frac);
            float lower = 1.0 - smoothstep(0.0, 0.1, frac);
            float v = frac < 0.5 ? lower : upper;
            out_color = vec4(v, v, v, 1.0);
            break;
        }
        case 2: {
            int index = (int(isoline_levels) - 1) - max(int(interval_index), 0);
            out_color = color_lut[index];
            break;
        }
        case 3: {
            int index = max(int(interval_index), 0);
            out_color = color_lut[index];
            break;
        }
        case 4: {
            float v = sqrt(min(1.0 / field_strength, 1.0));
            out_color = vec4(vec3(1.0 - v), 1.0);
            break;
        }
    }
}

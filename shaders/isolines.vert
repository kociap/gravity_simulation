#version 450 core

layout(location = 0) in vec3 position;

uniform mat4 vp;

out vec2 world_position;

void main() {
    gl_Position = vec4(position, 1.0);
    world_position = vec2(inverse(vp) * vec4(position, 1.0));
}

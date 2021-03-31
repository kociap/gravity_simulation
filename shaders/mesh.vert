#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;

uniform mat4 mvp;

layout(location = 0) out vec4 out_color;

void main() {
    out_color = color;
    gl_Position = mvp * vec4(position, 1.0);
}

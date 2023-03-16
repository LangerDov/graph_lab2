#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 fragColor;

layout( push_constant ) uniform constants
{
    mat4 mvp;
} mvp;


void main() {
    gl_Position = mvp.mvp * vec4(pos, 1.f);
    fragColor = color;
}
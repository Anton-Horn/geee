#version 450 core

layout(location = 0) out vec4 color_out;
layout(location = 0) in vec4 color;
layout(location = 1) in vec2 uv;

layout (set = 1, binding = 1) uniform sampler2D texture2d;

void main() {

    vec4 sampled = texture(texture2d, uv);

    color_out = color * sampled;
}

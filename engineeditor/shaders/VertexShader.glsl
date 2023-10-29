#version 450 core

layout(location = 0) in vec3 vertexPosition; 
layout(location = 1) in vec2 uv; 

layout(location = 0) out vec4 color_out;
layout(location = 1) out vec2 uv_out;

layout(set = 0, binding = 0) uniform global_transforms {
    mat4 cameraViewProj;
} u_globals;

layout(set = 1, binding = 0) uniform _dynamic_object_transforms {
    mat4 model;
    vec4 color;
} _dynamic_u_object;

void main() {
    color_out = _dynamic_u_object.color;
    uv_out = uv;
    gl_Position = u_globals.cameraViewProj * _dynamic_u_object.model * vec4(vertexPosition, 1.0f);
}
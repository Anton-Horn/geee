#version 450 core

layout(location = 0) in vec3 vertexPosition; 
layout(location = 1) in vec2 uv; 

layout(location = 0) out vec4 color_out;
layout(location = 1) out vec2 uv_out;
layout(location = 2) out vec4 p1_out;
layout(location = 3) out vec4 p2_out;
layout(location = 4) out vec4 c1_out;
layout(location = 5) out vec4 c2_out;
layout(location = 6) out vec2 screenSize_out;
layout(location = 7) out mat4 cameraViewProj_out;

layout(set = 0, binding = 0) uniform global_transforms {
    mat4 cameraViewProj;
    vec2 screenSize;
} u_globals;

layout(set = 1, binding = 0) uniform _dynamic_object_transforms {
    vec4 p1;
    vec4 p2;
    vec4 c1;
    vec4 c2;
    mat4 model;
    vec4 color;
} _dynamic_u_object;

void main() {
    color_out = _dynamic_u_object.color;
    uv_out = uv;
    p1_out = _dynamic_u_object.p1;
    p2_out = _dynamic_u_object.p2;
    c1_out = _dynamic_u_object.c1;
    c2_out = _dynamic_u_object.c2;
    screenSize_out = u_globals.screenSize;
    cameraViewProj_out = u_globals.cameraViewProj;
    gl_Position = u_globals.cameraViewProj * _dynamic_u_object.model * vec4(vertexPosition, 1.0f);

}
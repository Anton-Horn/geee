#version 450 core

layout(location = 0) in vec3 vertexPosition; 
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv; 

layout(location = 0) out vec2 uv_out;
layout(location = 1) out vec3 cameraPosition_out;
layout(location = 2) out vec3 lightPosition_out;
layout(location = 3) out vec3 normal_out;
layout(location = 4) out vec3 position_out;

layout(set = 0, binding = 0) uniform global_transforms {
    mat4 cameraProj;
    mat4 cameraView;
    vec4 cameraPosition;
    vec4 lightPosition;
} u_globals;

layout(set = 1, binding = 0) uniform _dynamic_object_transforms {
    mat4 model;
} _dynamic_u_object;

void main() {
    uv_out = uv;
    cameraPosition_out =  u_globals.cameraPosition.xyz;
    lightPosition_out = u_globals.lightPosition.xyz;
    normal_out = mat3(transpose(inverse(_dynamic_u_object.model))) * normal;
    position_out = (_dynamic_u_object.model * vec4(vertexPosition,1.0)).xyz;
    gl_Position = (u_globals.cameraProj * u_globals.cameraView) * _dynamic_u_object.model * vec4(vertexPosition, 1.0);
}
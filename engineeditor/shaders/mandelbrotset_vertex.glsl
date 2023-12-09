#version 450 core

layout(location = 0) in vec3 vertexPosition; 
layout(location = 1) in vec2 uv; 

layout(location = 0) out vec2 out_uv;
layout(location = 1) out vec2 out_cstart;
layout(location = 2) out float out_iterations;
layout(location = 3) out float out_zoom;


layout(set = 0, binding = 0) uniform object_transforms {
    mat4 cameraViewProj;
    mat4 model;
    vec4 spec; //xy = c_start, z = iterations, w = zoom
} uniforms;

void main() {
    out_uv = uv;
    out_cstart = uniforms.spec.xy;
    out_iterations = uniforms.spec.z;
    out_zoom = uniforms.spec.w;
    gl_Position = uniforms.cameraViewProj * uniforms.model * vec4(vertexPosition, 1.0f);
}
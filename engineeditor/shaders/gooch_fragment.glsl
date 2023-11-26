#version 450 core

layout(location = 0) out vec4 color_out;

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 cameraPosition;
layout(location = 2) in vec3 lightPosition;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec3 position;

layout (set = 1, binding = 1) uniform sampler2D albedo;

void main() {

    vec3 n = normalize(normal);
    vec3 v = normalize(cameraPosition - position);
    vec3 l = normalize(lightPosition - position);     

    vec4 sampled = texture(albedo, uv);

    vec3 ch = vec3(1., 1., 1.);
    vec3 cc = vec3(0.1, 0.1, 0.1) * 0.25 + 0.75 * sampled.xyz;
    vec3 cw = vec3(0.6, 0.5, 0.3) * 0.25  + 0.75 * sampled.xyz;
    vec3 r = 2 * dot(n, l) * n - l;
    float s = clamp(100 * dot(r, v) - 97, 0, 1);
    float t = (dot(n, l) + 1) / 2;

    vec3 shaded = vec3(s) * ch + vec3(1 - s) * (vec3(t)*cw + vec3(1 - t) * cc);

    color_out = vec4(shaded.xyz, 1.0);
    //color_out = vec4(n, 1.0);
}

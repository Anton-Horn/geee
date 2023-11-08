#version 450 core

layout(location = 0) out vec4 color_out;
layout(location = 0) in vec4 color;
layout(location = 1) in vec2 uv_in;
layout(location = 2) in vec4 p1_in;
layout(location = 3) in vec4 p2_in;
layout(location = 4) in vec4 c1_in;
layout(location = 5) in vec4 c2_in;
layout(location = 6) in vec2 screenSize_in;
layout(location = 7) in mat4 cameraViewProj_in;

float line(vec2 p, vec2 a, vec2 b) {

    vec2 pa = p-a, ba = b-a;
    float t = clamp(dot(pa, ba) / dot(ba, ba), 0., 1.);
    vec2 c = a+ba*t;
    float d = length(c-p);
    return smoothstep(fwidth(d), 0., d-0.002);

}

float point(vec2 uv, vec2 p, float size) {
    return 1 - step(size, length(uv - p));
}

vec2 bezier(vec2 A, vec2 B, vec2 C, vec2 D, float t) {
  vec2 E = mix(A, B, t);
  vec2 F = mix(B, C, t);
  vec2 G = mix(C, D, t);

  vec2 H = mix(E, F, t);
  vec2 I = mix(F, G, t);

  vec2 P = mix(H, I, t);

  return P;
}

void main() {

    vec2 uv = gl_FragCoord.xy / screenSize_in;

    uv -= 0.5;
    uv *= 2;
    vec2 aspectRatio = vec2(screenSize_in.x / screenSize_in.y, 1.0);
    uv *= aspectRatio;

    float point_size = 0.01;
    vec3 c = vec3(0.0);
    
    vec2 p1 = (cameraViewProj_in * p1_in).xy * aspectRatio;
    vec2 p2 = (cameraViewProj_in * p2_in).xy * aspectRatio;
    vec2 c1 = (cameraViewProj_in * c1_in).xy * aspectRatio;
    vec2 c2 = (cameraViewProj_in * c2_in).xy * aspectRatio;

    float steps = 50;

    c += point(uv, p1, point_size) * vec3(1., 0., 0.);
    c += point(uv, p2, point_size) * vec3(1., 0., 0.);
    c += point(uv, c1, point_size) * vec3(1., 0., 0.);
    c += point(uv, c2, point_size) * vec3(1., 0., 0.);

    vec2 p;
    vec2 pp = p1;

    for (float i = 1; i <= steps; i++) {

        float t = float(i) / float(steps);

        p = bezier(p1, c1, c2, p2, t);

        c += line(uv, pp, p);
       // c += point(uv, p, point_size);
        pp = p;
    }

    c += line(uv, p1, c1) * vec3(0., 1., 0.);
    c += line(uv, c2, p2) * vec3(0., 1., 0.);
    
    color_out = vec4(c, length(c));

}

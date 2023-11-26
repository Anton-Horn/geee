#version 450 core

layout(location = 0) out vec4 color_out;
layout(location = 0) in vec2 uv;
layout(location = 1) in vec2 cstart;
layout(location = 2) in float iterations;
layout(location = 3) in float zoom;

layout (set = 1, binding = 1) uniform sampler2D texture2d;


vec2 multComplex(vec2 z1, vec2 z2) {

    return vec2(z1.x * z2.x - z1.y * z2.y, z1.x * z2.y + z2.x * z1.y);

}

// from https://stackoverflow.com/questions/15095909/from-rgb-to-hsv-in-opengl-glsl
vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

int mandelbrot(vec2 c, int MAX_ITER) {
    vec2 z = vec2(0);
    int i = 0;

    for (i = 0; i < MAX_ITER; i++) {

        z = multComplex(z, z) + c;
        if (length(z) > 2.0) break;
    }
    return i;
}

vec3 calculate_color(int iter_count, int max_iter) {
    if (iter_count == max_iter) {
        return vec3(0.0, 0.0, 0.0); // Farbe für Punkte, die im Set sind (z.B. Schwarz)
    }
    float hue = float(iter_count) / float(max_iter);
    return hsv2rgb(vec3(hue, 1.0, 1.0));
}

void main() {

    int MAX_ITER = int(iterations);
    vec2 c = cstart + (uv - 0.5) * 2 * vec2(16./9., 1.) * zoom;

    int iterations = mandelbrot(c, MAX_ITER);

    vec3 col = calculate_color(iterations, MAX_ITER);

    color_out = vec4(col, 1.0);
}

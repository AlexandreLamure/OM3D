#version 450

#include "debug.glsl"

layout(location = 0) out vec4 out_color;

uniform float delta_time;

void main() {
    _px_coord = vec2(10, 30);

    putc(CHAR_F, CHAR_P, CHAR_S, CHAR_COLON);
    putf(1.0 / delta_time, 4.0, 1.0);

    _px_coord = vec2(10, 10);

    putc(CHAR_M, CHAR_S, CHAR_SPACE, CHAR_COLON);
    putf(delta_time * 1000.0, 4.0, 1.0);

    out_color = vec4(1.0);
    if(print() < 0.5) {
        discard;
    }
}


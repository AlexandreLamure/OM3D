#version 450

layout(location = 0) out vec4 out_color;

uniform float red = 1.0;
uniform float green = 1.0;
uniform float blue = 1.0;

void main() {
    out_color = vec4(red, green, blue, 1.0);
}


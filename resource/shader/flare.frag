#version 330 core

in vec2 texture_coord;
//out vec4 out_color;
layout (location = 0) out vec4 out_color;

uniform sampler2D flare_texture;

void main() {
    //out_color = vec4(texture(flare_texture, texture_coord).rgb, 1.0);
    out_color = vec4(1,0,0,1);
}

#version 330 core

in vec2 texture_coord;
in float flare_opacity;
layout (location = 0) out vec4 out_color;

uniform sampler2D flare_texture;
uniform float flare_opacity_override;

void main() {
    out_color = texture(flare_texture, texture_coord);
    out_color.a *= flare_opacity * flare_opacity_override;
}

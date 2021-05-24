// skybox.frag

#version 330 core

in vec3 texture_coord;

out vec4 out_color;

uniform samplerCube tex;
uniform float brightness;

void main() {
	out_color = brightness * texture(tex, texture_coord);
}

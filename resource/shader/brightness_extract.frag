// brightness_extract.frag

#version 330 core

in vec2 texture_coord;

layout (location = 0) out vec4 out_color;

uniform sampler2D texture0;
uniform float factor;

// https://learnopengl.com/Advanced-Lighting/Bloom
void main() {
	vec4 color = texture(texture0, texture_coord);
	float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    out_color = color * brightness * factor;
}

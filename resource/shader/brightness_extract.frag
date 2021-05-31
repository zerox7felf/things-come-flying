// brightness_extract.frag

#version 330 core

in vec2 texture_coord;

layout (location = 0) out vec4 out_color;

uniform sampler2D texture0;
uniform float factor;
uniform bool keep_color;

// https://learnopengl.com/Advanced-Lighting/Bloom
void main() {
	vec4 color = texture(texture0, texture_coord);
	float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
	if (!keep_color) {
		color = brightness * color;
	}
	else {
		color = vec4(brightness);
	}
	out_color = color * factor;
}

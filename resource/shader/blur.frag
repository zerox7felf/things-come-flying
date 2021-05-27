// blur.frag

#version 330 core

in vec2 texture_coord;

layout (location = 0) out vec4 out_color;

uniform sampler2D texture0;
uniform bool vertical;	// Dictates if we should blur vertically or horizontally

// Gaussian kernel is calculated with https://dev.theomader.com/gaussian-kernel-calculator/
float weights[5] = float[] (0.382928, 0.241732, 0.060598, 0.005977, 0.000229);

void main() {
	vec4 color = vec4(0);
	vec2 texel_size = 1.0 / textureSize(texture0, 0);
	if (vertical) {
		for (int i = 0; i < 5; i++) {
			color += weights[i] * texture(texture0, texture_coord + vec2(0, texel_size.x * i));
			color += weights[i] * texture(texture0, texture_coord - vec2(0, texel_size.x * i));
		}
	}
	else {
		for (int i = 0; i < 5; i++) {
			color += weights[i] * texture(texture0, texture_coord + vec2(texel_size.y * i, 0));
			color += weights[i] * texture(texture0, texture_coord - vec2(texel_size.y * i, 0));
		}
	}
	out_color = color;
}

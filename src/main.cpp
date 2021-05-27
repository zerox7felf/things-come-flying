// main.cpp

#include <math.h>

#include "engine.hpp"

static float kernel(float sigma);
static float gaussian(i32 index, float sigma);
static void write_gaussian_kernel(FILE* fp, float sigma, i32 count);

float kernel(float sigma) {
	return 1 + 2.0f * sqrt(-2.0f * sigma * sigma * log(0.005f));
}

float gaussian(i32 index, float sigma) {
	return expf(-(index * index) / (2.0f * sigma * sigma));
}

void write_gaussian_kernel(FILE* fp, float sigma, i32 count) {
	float kernel_size = kernel(sigma);
	fprintf(fp, "// kernel size: %g, sigma: %g\n", kernel_size, sigma);
	for (i32 i = 0; i < count; i++) {
		float value = gaussian(i, sigma);
		fprintf(fp, "%lf", value);
		if (i + 1 < count) {
			fprintf(fp, ", ");
		}
		else {
			fprintf(fp, "\n");
		}
	}
}

int main(int argc, char** argv) {
	// write_gaussian_kernel(stdout, 2.0f, 11);
	return engine_start();
}

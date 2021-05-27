// gaussian_kernel.cpp
// generates gaussian kernel
//
// compile:
//   g++ gaussian_kernel.cpp -o gaussian_kernel -lm
//
// based on: https://blog.demofox.org/2015/08/19/gaussian-blur/

#include <stdio.h>
#include <math.h>

static float kernel(float sigma);
static float gaussian(int index, float sigma);
static void write_gaussian_kernel(FILE* fp, float sigma, int count);

float kernel(float sigma) {
    return 1 + 2.0f * sqrt(-2.0f * sigma * sigma * log(0.005f));
}

float gaussian(int index, float sigma) {
    return expf(-(index * index) / (2.0f * sigma * sigma));
}

void write_gaussian_kernel(FILE* fp, float sigma, int count) {
    float kernel_size = kernel(sigma);
    // fprintf(fp, "// kernel size: %g, sigma: %g\n", kernel_size, sigma);
    for (int i = 0; i < count; i++) {
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

int main(void) {
	write_gaussian_kernel(stdout, 1.3f, 11);
	return 0;
}

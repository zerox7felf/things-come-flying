// image.cpp

#include <png.h>

#include "common.hpp"
#include "memory.hpp"
#include "image.hpp"

i32 load_image_from_file(const char* path, Image* image) {
	i32 result = NoError;
	i32 row_size = 0;
	u8* pixels = NULL;
	png_bytep* rows = NULL;
	FILE* fp = fopen(path, "r");
	if (!fp) {
		fprintf(stderr, "No such image file '%s'\n", path);
		return Error;
	}
	png_structp png;
	png_infop info;

	u8 png_signature[8];
	if (fread(png_signature, 1, sizeof(png_signature), fp) < 8) {
		result = Error;
		goto done;
	}

	if (png_sig_cmp(png_signature, 0, 8)) {
		result = Error;
		goto done;
	}

	png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) {
		result = Error;
		goto done;
	}

	info = png_create_info_struct(png);
	if (!info) {
		result = Error;
		goto done;
	}
	if (setjmp(png_jmpbuf(png))) {
		png_destroy_read_struct(&png, &info, NULL);
		result = Error;
		goto done;
	}
	png_init_io(png, fp);
	png_set_sig_bytes(png, sizeof(png_signature));
	png_read_info(png, info);

	image->width = png_get_image_width(png, info);
	image->height = png_get_image_height(png, info);
	image->depth = png_get_bit_depth(png, info);

	if (image->depth < 8) {
		png_set_packing(png);
	}

	if (png_get_valid(png, info, PNG_INFO_tRNS)) {
		png_set_tRNS_to_alpha(png);
	}

	switch (png_get_color_type(png, info)) {
		case PNG_COLOR_TYPE_RGB: {
			image->pitch = 3 * image->width;
			break;
		}
		case PNG_COLOR_TYPE_RGBA: {
			image->pitch = 4 * image->width;
			break;
		}
		default: {
			image->pitch = 3 * image->width;
			break;
		}
	}

	image->bytes_per_pixel = png_get_rowbytes(png, info) / image->width;

	png_set_interlace_handling(png);
	png_read_update_info(png, info);

	image->buffer = (u8*)m_malloc(sizeof(u8) * image->width * image->height * image->bytes_per_pixel);

	row_size = sizeof(png_bytep) * image->height;
	rows = (png_bytep*)m_malloc(row_size);
	pixels = image->buffer;
	for (i32 row = 0; row < image->height; ++row) {
		rows[row] = pixels;
		pixels += image->width * image->bytes_per_pixel;
	}

	png_read_image(png, rows);
	png_read_end(png, NULL);
	png_destroy_read_struct(&png, &info, NULL);
	m_free(rows, row_size);
done:
	return result;
}

void unload_image(Image* image) {
	if (image->buffer) {
		m_free(image->buffer, sizeof(u8) * image->width * image->height * image->bytes_per_pixel);
		memset(image, 0, sizeof(Image));
	}
}

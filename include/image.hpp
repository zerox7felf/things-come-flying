// image.hpp

#ifndef _IMAGE_HPP
#define _IMAGE_HPP

typedef struct Image {
	u8* buffer;
	i32 width, height;
	u16 depth;
	u16 pitch;
	u16 bytes_per_pixel;
} Image;

i32 load_image_from_file(const char* path, Image* image);

void unload_image(Image* image);

#endif

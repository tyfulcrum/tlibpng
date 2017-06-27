#ifndef TLIBPNG_PNG_DECODER_H_
#define TLIBPNG_PNG_DECODER_H_
#include <stdint.h>
#include <stdbool.h>

typedef struct PngContent {
  uint32_t pic_width;
  uint32_t pic_height;
  uint32_t *rgba_pixel_array;
} PngContent;

bool DecodePng(char const *file_path, PngContent *png_content);

#endif

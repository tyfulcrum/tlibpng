#ifndef TLIBPNG_PNG_DECODER_H_
#define TLIBPNG_PNG_DECODER_H_
#include <stdint.h>

typedef struct PngContent {
  uint32_t pic_width;
  uint32_t pic_height;
  uint32_t *rgba_pixel_array;
} PngContent;

PngContent DecodePng(char *file_path);

#endif

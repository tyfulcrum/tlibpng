#include "png_decoder.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

PngContent DecodePng(char *file_path) {
  if (NULL == file_path) {
    puts("File path can't be empty.");
    exit(EXIT_FAILURE);
  }
  uint32_t width = 0;
  uint32_t height = 0;
  PngContent result;
  result.pic_width = width;
  result.pic_height = height;
  result.rgba_pixel_array = malloc(width * height * sizeof(uint32_t));
  return result;
}

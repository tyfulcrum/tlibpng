#include "png_decoder.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <stdarg.h>

bool CheckType(uint8_t *buffer, int length, ...) {
  va_list ap;
  va_start(ap, length);
  int i = 0;
  for (i = 0; i < length; ++i) {
    if (buffer[i] != va_arg(ap, uint8_t)) {
      return false;
    }
  }
  va_end(ap);
  return true;
}

bool DecodePng(char *file_path, PngContent *png_content) {
  if (NULL == file_path) {
    fprintf(stderr, "You must give the png file path as shell parameter!\n");
    return false;
  }
  FILE *file_ptr = fopen(file_path, "rb");
  if (NULL == file_ptr) {
    fprintf(stderr, "Failed to open file!\n");
    return false;
  }

  struct stat file_info;
  stat(file_path, &file_info);
  uint64_t file_size = file_info.st_size;
  uint8_t binary_buffer[file_size];
  fprintf(stdout, "File size: %fMiB\n", (double)file_size/(1024*1024));

  fread(binary_buffer, sizeof(binary_buffer), 1, file_ptr);
  
  if (false == CheckType(&binary_buffer[0], 8, 137, 80, 78, 71, 13, 10, 26, 10)) {
    fprintf(stderr, "Given file is not PNG file!\n");
    return false;
  } else {
    puts("This is a PNG file!\n");
  }


  fclose(file_ptr);
  return false;

  uint32_t width = 0;
  uint32_t height = 0;
  png_content->pic_width = width;
  png_content->pic_height = height;
  png_content->rgba_pixel_array = malloc(width * height * sizeof(uint32_t));
  fclose(file_ptr);
  return true;
}

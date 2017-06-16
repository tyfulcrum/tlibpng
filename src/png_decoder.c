#include "png_decoder.h"
#include "dbg.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <stdarg.h>

bool CheckType(uint8_t *buffer, int length, ...) {
  va_list ap;
  va_start(ap, length);
  uint8_t j = 0;
  int i = 0;
  for (i = 0; i < length; ++i) {
    j = va_arg(ap, uint8_t);
    log_info("buffer[%d]: %u VS %u\n", i, buffer[i], j);
    if (buffer[i] != j) {
      return false;
    }
  }
  va_end(ap);
  return true;
}

bool DecodePng(char *file_path, PngContent *png_content) {
  FILE *file_ptr = NULL;
  uint8_t *binary_buffer = NULL;
  check(file_path, "You must give the png file path as shell parameter!\n");

  file_ptr = fopen(file_path, "rb");
  check(file_ptr, "Failed to open file!");

  struct stat file_info;
  stat(file_path, &file_info);
  uint64_t file_size = file_info.st_size;
  binary_buffer = malloc(file_size * sizeof(uint8_t));
  check(binary_buffer, "Alloc memory for picture file failed!");

  fprintf(stdout, "File size: %fMiB\n", (double)file_size/(1024*1024));

  fread(binary_buffer, sizeof(binary_buffer), 1, file_ptr);
  
  check(CheckType(&binary_buffer[0], 8, 137, 80, 78, 71, 13, 10, 26, 10),
        "Given file is not PNG file!");

  puts("This is a PNG file!\n");


  fclose(file_ptr);
  return false;

  uint32_t width = 0;
  uint32_t height = 0;
  png_content->pic_width = width;
  png_content->pic_height = height;
  png_content->rgba_pixel_array = malloc(width * height * sizeof(uint32_t));
  fclose(file_ptr);
  return true;

error:
  if (NULL != file_ptr) {
    fclose(file_ptr);
  }
  if (NULL != binary_buffer) {
    free(binary_buffer);
  }
  if (NULL != png_content->rgba_pixel_array) {
    free(png_content->rgba_pixel_array);
  }
  return false;
}

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
  int i = 0;
  uint8_t j = 0;
  for (i = 0; i < length; ++i) {
    j = va_arg(ap, uint8_t);
    log_info("buffer[i]: %x VS %x", buffer[i], j);
    if (buffer[i] != j) {
      return false;
    }
  }
  va_end(ap);
  return true;
}

uint32_t swap_uint32(uint32_t p) {
  p = ((p << 8) & 0xFF00FF00) | ((p >> 8) & 0xFF00FF);
  return (p << 16) | (p >> 16);
}

bool DecodePng(char *file_path, PngContent *png_content) {
  int i = 0; /* iterator */
  FILE *file_ptr = NULL;
  uint8_t *binary_buffer = NULL; /* global pointer to file content */
  check(file_path, "You must give the png file path as shell parameter!\n");

  file_ptr = fopen(file_path, "rb");
  check(file_ptr, "Failed to open file!");

  /* Read file size and alloc memory */
  struct stat file_info;
  stat(file_path, &file_info);
  uint64_t file_size = file_info.st_size;
  binary_buffer = malloc(file_size * sizeof(uint8_t));
  check(binary_buffer, "Alloc memory for picture file failed!");

  fprintf(stdout, "File size: %fMiB\n", (double)file_size/(1024*1024));

  fread(binary_buffer, file_size, 1, file_ptr);
  
  /* Check PNG signature */
  check(CheckType(&binary_buffer[0], 8, 137, 80, 78, 71, 13, 10, 26, 10),
        "Given file is not PNG file!");
  puts("This is a PNG file!\n");

  /* Read IHDR chunk */
  uint8_t *ihdr_ptr = &binary_buffer[8];
  uint32_t ihdr_length = ((uint32_t*) ihdr_ptr)[0];
  ihdr_length = swap_uint32(ihdr_length);
  log_info("IHDR length: %x", ihdr_length);
  
  check(CheckType(&ihdr_ptr[4], 4, 73, 72, 68, 82), 
        "IHDR Image header doesn't exist!");

  uint8_t *ihdr_data_ptr = &ihdr_ptr[8];
  uint32_t width  = ((uint32_t*) ihdr_data_ptr)[0];
  uint32_t height = ((uint32_t*) ihdr_data_ptr)[1];
  width = swap_uint32(width);
  height = swap_uint32(height);
  log_info("Width: %d, height: %d\n", width, height);
  uint8_t bit_depth = ihdr_data_ptr[8];
  uint8_t colour_type = ihdr_data_ptr[9];
  uint8_t compression_method = ihdr_data_ptr[10];
  uint8_t filter_method = ihdr_data_ptr[11];
  uint8_t interlace_method = ihdr_data_ptr[12];
  log_info("bit depth: %d", bit_depth);
  log_info("colour type: %d", colour_type);
  log_info("compression_method: %d", compression_method);
  log_info("filter method: %d", filter_method);
  log_info("interlace method: %d", interlace_method);
  png_content->pic_width = width;
  png_content->pic_height = height;
  //png_content->rgba_pixel_array = malloc(width * height * sizeof(uint32_t));

  free(binary_buffer);
  fclose(file_ptr);
  return false;

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

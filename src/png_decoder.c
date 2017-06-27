#include "png_decoder.h"
#include "dbg.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <zlib.h>

static const int32_t buffer_size =  52428800;
static const int8_t png_sign_length = 8;
static const int8_t length_field_length = 4;
static const int8_t type_field_length = 4;
static const int8_t crc_field_length = 4;
static const int8_t except_data_length = 12;

bool CheckType(uint8_t const *buffer, int length, ...) {
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

/* Table of CRCs of all 8-bit messages. */
unsigned long crc_table[256];

/* Flag: has the table been computed? Initially false. */
int crc_table_computed = 0;

/* Make the table for a fast CRC. */
void make_crc_table(void) {
  unsigned long c;
  int n, k;

  for (n = 0; n < 256; n++) {
    c = (unsigned long) n;
    for (k = 0; k < 8; k++) {
      if (c & 1)
        c = 0xedb88320L ^ (c >> 1);
      else
        c = c >> 1;
    }
    crc_table[n] = c;
  }
  crc_table_computed = 1;
}

/* Update a running CRC with the bytes buf[0..len-1]--the CRC
 *       should be initialized to all 1's, and the transmitted value
 *             is the 1's complement of the final running CRC (see the
 *                   crc() routine below). */

unsigned long update_crc(unsigned long crc, unsigned char *buf,
    int len) {
  unsigned long c = crc;
  int n;

  if (!crc_table_computed)
    make_crc_table();
  for (n = 0; n < len; n++) {
    c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
  }
  return c;
}

/* Return the CRC of the bytes buf[0..len-1]. */
unsigned long crc(unsigned char *buf, int len) {
  return update_crc(0xffffffffL, buf, len) ^ 0xffffffffL;
}

uint32_t GetChunkLength(uint8_t const *chunk_ptr) {
  uint32_t chunk_length = ((uint32_t*) chunk_ptr)[0];
  chunk_length = swap_uint32(chunk_length);
  return chunk_length;
}

bool CheckCRC(uint8_t const *chunk_ptr) {
  uint32_t chunk_length = GetChunkLength(chunk_ptr);
  uint32_t crc_caled_value = crc(&chunk_ptr[length_field_length], 
      type_field_length+chunk_length);
  uint8_t *crc_ptr = &chunk_ptr[length_field_length+type_field_length+chunk_length];
  uint32_t crc_value = ((uint32_t*)crc_ptr)[0];
  crc_value = swap_uint32(crc_value);
  log_info("Calculated CRC: %x CRC in file: %x", crc_caled_value, crc_value);
  return crc_caled_value == crc_value;
}

bool DecodeIDAT(uint8_t const *chunk_ptr, uint32_t *rgba_pixel_array) {
  check(CheckCRC(chunk_ptr), "IDAT CRC error!");
  uint32_t idat_data_field_length = GetChunkLength(chunk_ptr);
  log_info("IDAT data length: %d", idat_data_field_length);

  return true;
error:
  return false;
}
bool DecodePng(char const *file_path, PngContent *png_content) {
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
  uint8_t *ihdr_ptr = &binary_buffer[png_sign_length];
  check(CheckType(&ihdr_ptr[length_field_length], 4, 73, 72, 68, 82), 
        "IHDR Image header doesn't exist!");
  uint32_t ihdr_data_field_length = GetChunkLength(ihdr_ptr);
  log_info("IHDR Data field length: %x", ihdr_data_field_length);
  
  uint8_t *ihdr_data_ptr = &ihdr_ptr[length_field_length+type_field_length];
  /* Check CRC Value */
  check(CheckCRC(ihdr_ptr), "CRC of IHDR chunk error!");
  /* Read chunk data */
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
  png_content->rgba_pixel_array = malloc(width * height * sizeof(uint32_t));
  check(png_content->rgba_pixel_array, "RGBA array alloc failed!");

  uint8_t *next_chunk_ptr = &ihdr_ptr[ihdr_data_field_length+12];
  for(;;) {
    uint32_t next_chunk_data_length = GetChunkLength(next_chunk_ptr);
    uint8_t *next_chunk_type = &next_chunk_ptr[length_field_length];
    log_info("========================================");
    log_info("Next chunk type: %d %d %d %d", next_chunk_ptr[4], 
        next_chunk_ptr[5], next_chunk_ptr[6], next_chunk_ptr[7]);
    log_info("Next chunk data field length: %d", next_chunk_data_length);
    if(CheckType(next_chunk_type, 4, 73, 68, 65, 84)) {
      /* Counter IDAT chunk */
      check(DecodeIDAT(next_chunk_ptr, png_content->rgba_pixel_array), 
          "Decoding IDAT failed!");
    }
    if(CheckType(next_chunk_type, 4, 73, 69, 78, 68)) {
      break;
    }
    next_chunk_ptr = &next_chunk_ptr[except_data_length+next_chunk_data_length];
  }


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

#include "png_decoder.h"
#include "dbg.h"
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

int main(int argc, char *argv[]) {
  Uint32 *pixel_buf            = NULL;
  SDL_Texture *texture         = NULL;
  SDL_PixelFormat *format      = NULL;
  SDL_Window *window           = NULL;
  SDL_Renderer *renderer       = NULL;
  PngContent png_content;
  png_content.pic_width        = 0;
  png_content.pic_height       = 0;
  png_content.rgba_pixel_array = NULL;

  /*
   * This is for integration test
   * of SDL and tlibpng. 
   * 
  png_content.pic_width = 1024;
  png_content.pic_height = 768;
  puts("aaa");
  png_content.rgba_pixel_array = malloc(1024*768*sizeof(uint32_t));
  check(png_content.rgba_pixel_array, "png error");
  int k = 0;
  for (; k < 1024*768; ++k) {
    png_content.rgba_pixel_array[k] = 0xFFFFFF;
  }

  */

  char *pic_path = argv[1];
  check(pic_path, "You must give the png file path as shell parameter!");

  check(DecodePng(pic_path, &png_content), "Decode failed.");

  int width = png_content.pic_width;
  int height = png_content.pic_height;
  int n_of_pixels = width * height;
  

  check(SDL_Init(SDL_INIT_VIDEO) >= 0, "SDL_Init error: %s\n", SDL_GetError());

  pixel_buf = malloc(width * height * sizeof(Uint32));
  check(pixel_buf, "Failed to allocate buffer for pixels.\n");

  window = SDL_CreateWindow(
      "tlibpng", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
      width, height, 0);
  check(window, "Failed to create window: %s\n", SDL_GetError());

  format = SDL_AllocFormat(SDL_GetWindowPixelFormat(window));
  check(format, "Failed to alloc pixel format: %s\n", SDL_GetError());

  renderer = SDL_CreateRenderer(window, -1, 0);
  check(renderer, "Failed to created renderer: %s\n", SDL_GetError());

  SDL_RenderSetLogicalSize(renderer, width, height);

  texture = SDL_CreateTexture( 
      renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 
      width, height);
  check(texture, "Failed to create screen texture: %s\n", SDL_GetError());
  log_info("All right before rendering picture.");

  int i = 0;
  uint8_t *j = NULL;
  for (i = 0; i < n_of_pixels; ++i) {
    j = (uint8_t*) &png_content.rgba_pixel_array[i];
    pixel_buf[i] = SDL_MapRGBA(format, j[0], j[1], j[2], j[3]);
    //pixel_buf[i] = SDL_MapRGBA(format, 0xFF, 0xFF, 0xFF, 0xFF);
  }

  SDL_UpdateTexture(texture, NULL, pixel_buf, width * sizeof(Uint32));
  check(0 == SDL_RenderClear(renderer), "Failed to clear renderer: %s\n", SDL_GetError());

  check(0 == SDL_RenderCopy(renderer, texture, NULL, NULL), 
        "Failed to blit surface: %s\n", SDL_GetError());

  SDL_RenderPresent(renderer);

  bool quit = false;
  SDL_Event e;

  while(!quit) {
    while(0 != SDL_PollEvent(&e)) {
      if (SDL_QUIT == e.type) {
        quit = true;
      }
    }
  }

  if (NULL != png_content.rgba_pixel_array) {
    free(png_content.rgba_pixel_array);
  }
  SDL_DestroyTexture(texture);
  free(pixel_buf);
  SDL_DestroyRenderer(renderer);
  SDL_FreeFormat(format);
  SDL_DestroyWindow(window);
  SDL_Quit();
  exit(EXIT_SUCCESS);

error:
  if (NULL != png_content.rgba_pixel_array) {
    free(png_content.rgba_pixel_array);
  }
  SDL_DestroyTexture(texture);
  free(pixel_buf);
  SDL_DestroyRenderer(renderer);
  SDL_FreeFormat(format);
  SDL_DestroyWindow(window);
  SDL_Quit();
  exit(EXIT_FAILURE);
}

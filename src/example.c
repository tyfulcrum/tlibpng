#include "png_decoder.h"
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

Uint32 *pixel_buf        = NULL;
SDL_Texture *texture    = NULL;
SDL_PixelFormat *format = NULL;
SDL_Window *window      = NULL;
SDL_Renderer *renderer  = NULL;
PngContent png_content;

void CleanInDetail(void) {
  if (NULL != png_content.rgba_pixel_array) {
    free(png_content.rgba_pixel_array);
  }
  SDL_DestroyTexture(texture);
  free(pixel_buf);
  SDL_DestroyRenderer(renderer);
  SDL_FreeFormat(format);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

void CleanUp(void) {
  CleanInDetail();
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {

  if (NULL == argv[1]) {
    fprintf(stderr, "You must give the png file path as shell parameter!\n");
    exit(EXIT_SUCCESS);
  }
  char *pic_path = argv[1];

  if (false == DecodePng(pic_path, &png_content)) {
    CleanUp();
  }

  int width = png_content.pic_width;
  int height = png_content.pic_height;
  int n_of_pixels = width * height;

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
    CleanUp();
  }

  pixel_buf = malloc(width * height * sizeof(Uint32));
  if (NULL == pixel_buf) {
    fprintf(stderr, "Failed to allocate buffer for pixels.\n");
    CleanUp();
  }

  window = SDL_CreateWindow(
      "tlibpng", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
      width, height, 0);
  if (NULL == window) {
    fprintf(stderr, "Failed to create window: %s\n", SDL_GetError());
    CleanUp();
  }

  format = SDL_AllocFormat(SDL_GetWindowPixelFormat(window));
  if (NULL == format) {
    fprintf(stderr, "Failed to alloc pixel format: %s\n", SDL_GetError());
    CleanUp();
  }

  renderer = SDL_CreateRenderer(window, -1, 0);
  if (NULL == renderer) {
    fprintf(stderr, "Failed to created renderer: %s\n", SDL_GetError());
    CleanUp();
  }

  SDL_RenderSetLogicalSize(renderer, width, height);

  texture = SDL_CreateTexture( 
      renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 
      width, height);
  if (NULL == texture) {
    fprintf(stderr, "Failed to create screen texture: %s\n", SDL_GetError());
    CleanUp();
  }

  int i = 0;
  uint8_t *j = 0;
  for (i = 0; i < n_of_pixels; ++i) {
    j = (uint8_t) png_content.rgba_pixel_array[i];
    pixel_buf[i] = SDL_MapRGBA(format, j[0], j[1], j[2], j[3]);
  }

  SDL_UpdateTexture(texture, NULL, pixel_buf, width * sizeof(Uint32));
  if (0 != SDL_RenderClear(renderer)) {
    fprintf(stderr, "Failed to clear renderer: %s\n", SDL_GetError());
    CleanUp();
  }

  if (0 !=SDL_RenderCopy(renderer, texture, NULL, NULL)) {
    fprintf(stderr, "Failed to blit surface: %s\n", SDL_GetError());
    CleanUp();
  }

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

  CleanInDetail();
  exit(EXIT_SUCCESS);
}


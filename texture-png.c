#include "texture-png.h"

#include "texture.h"
#include "renderer.h"

#include <png.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

bool kl_texture_loadpng(char* path, kl_texture_t *texture) {
  strncpy(texture->path, path, KL_TEXTURE_PATHLEN);
  texture->path[KL_TEXTURE_PATHLEN-1] = '\0';
  texture->w  = 0;
  texture->h  = 0;
  texture->id = 0;

  FILE    *file   = NULL;
  uint8_t *buffer = NULL;

  png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png == NULL) {
    fprintf(stderr, "image-png: Failed to create png struct!\n");
    return false;
  }

  png_infop info = png_create_info_struct(png);
  if (info == NULL) {
    fprintf(stderr, "image-png: Failed to create info struct!\n");
    png_destroy_read_struct(&png, NULL, NULL);
    return false;
  }
 
  /* error handling & cleanup routine */ 
  if (setjmp(png_jmpbuf(png)) != 0) {
    fprintf(stderr, "image-png: Failed to parse %s\n", path);
    png_destroy_read_struct(&png, &info, NULL);
    if (file != NULL) fclose(file);
    if (buffer != NULL) free(buffer);
    return false;
  }

  file = fopen(path, "rb");
  if (file == NULL) {
    fprintf(stderr, "image-png: %s does not exist\n", path);
    longjmp(png_jmpbuf(png), 1);
  }
  png_init_io(png, file);

  png_read_info(png, info);

  png_set_expand(png); /* palette -> RGB */
  png_set_packing(png); /* 1/2/4 bpp -> 8bpp */

  png_read_update_info(png, info);

  int w = png_get_image_width(png, info);
  int h = png_get_image_height(png, info);
  int format;
  int channels;
  switch (png_get_color_type(png, info)) {
    case PNG_COLOR_TYPE_GRAY:
      format   = KL_TEXFMT_I;
      channels = 1;
      break;
    case PNG_COLOR_TYPE_GRAY_ALPHA:
      format   = KL_TEXFMT_IA;
      channels = 2;
      break;
    case PNG_COLOR_TYPE_RGB:
      format   = KL_TEXFMT_RGB;
      channels = 3;
      break;
    case PNG_COLOR_TYPE_RGB_ALPHA:
      format   = KL_TEXFMT_RGBA;
      channels = 4;
      break;
    default:
      fprintf(stderr, "image-png: Bad image format for %s\n", path);
      fclose(file);
      longjmp(png_jmpbuf(png), 1);
  }
  if (png_get_bit_depth(png, info) != 8) {
    fprintf(stderr, "image-png: Unsupported pixel format for %s\n", path);
    fclose(file);
    longjmp(png_jmpbuf(png), 1);
  }
  
  buffer = malloc(w * h * channels);

  png_bytep rows[h];
  for (uint32_t i=0; i < h; i++) {
    rows[i] = buffer + i * w * channels;
  }
  
  png_read_image(png, rows);

  texture->w  = w;
  texture->h  = h;
  texture->id = kl_render_upload_texture(buffer, w, h, format, false, true);

  png_destroy_read_struct(&png, &info, NULL);
  fclose(file);
  free(buffer);

  return true;
}
/* vim: set ts=2 sw=2 et */

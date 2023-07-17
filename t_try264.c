#include "t_try264.h"
#include <vips/vips.h>

int t_try264(int argc, char **argv) {
  VipsImage *image;
  VipsImage *memory;
  VipsImage *memory2;
  char *data;
  size_t size;
  int width;
  int height;
  int bands;
  VipsBandFormat format;

  if (!(image = vips_image_new_from_file(argv[1], NULL)))
    vips_error_exit(NULL);

  size = VIPS_IMAGE_SIZEOF_IMAGE(image);
  width = image->Xsize;
  height = image->Ysize;
  bands = image->Bands;
  format = image->BandFmt;

  if (!(data = malloc(size)))
    vips_error_exit(NULL);
  if (!(memory = vips_image_new_from_memory(data, size, width, height, bands,
                                            format)))
    vips_error_exit(NULL);

  if (vips_image_write(image, memory))
    vips_error_exit(NULL);

  g_object_unref(image);
  g_object_unref(memory);

  printf("written %zd bytes to %p\n", size, data);

  if (!(memory2 = vips_image_new_from_memory(data, size, width, height, bands,
                                             format)))
    vips_error_exit(NULL);
  if (vips_image_write_to_file(memory2, argv[2], NULL))
    vips_error_exit(NULL);

  g_object_unref(memory2);

  free(data);

  return (0);
}

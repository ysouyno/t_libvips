#include "t_generate.h"
#include <vips/vips.h>

int travel_pixels_generate(VipsRegion * or, void *vseq, void *a, void *b,
                           gboolean *stop) {
  VipsImage *in = VIPS_IMAGE(a);
  VipsRect *r = & or->valid;
  VipsRegion *ir = VIPS_REGION(vseq);
  int line_size = r->width * in->Bands;
  int x, y;

  if (vips_region_prepare(ir, r))
    return -1;

  for (y = 0; y < r->height; ++y) {
    unsigned char *p =
        (unsigned char *)VIPS_REGION_ADDR(ir, r->left, r->top + y);
    unsigned char *q =
        (unsigned char *)VIPS_REGION_ADDR(or, r->left, r->top + y);

    memcpy((unsigned char *)b + line_size * (r->top + y) + r->left, p,
           line_size);
  }

  return 0;
}

int travel_pixels(VipsImage *in, VipsImage **out, unsigned char *addr) {
  if (vips_check_uncoded("negative", in) ||
      vips_check_format("negative", in, VIPS_FORMAT_UCHAR))
    return -1;

  *out = vips_image_new();

  if (vips_image_pipelinev(*out, VIPS_DEMAND_STYLE_THINSTRIP, in, NULL)) {
    g_object_unref(*out);
    return -1;
  }

  if (vips_image_generate(*out, vips_start_one, travel_pixels_generate,
                          vips_stop_one, in, addr)) {
    g_object_unref(*out);
    return -1;
  }

  return 0;
}

int t_generate(int argc, char **argv) {
  if (argc < 4) {
    printf("Usage: exec src.jpg out1.jpg out2.jpg\n");
    return -1;
  }

  VipsImage *in;
  VipsImage *out;
  VipsImage *out2;

  if (vips_init("t_generate"))
    vips_error_exit("unable to init");

  vips_leak_set(TRUE);

  if (!(in = vips_image_new_from_file(argv[1], "access", VIPS_ACCESS_SEQUENTIAL,
                                      NULL)))
    vips_error_exit("unable to open");

  int size = VIPS_IMAGE_SIZEOF_IMAGE(in);
  int width = in->Xsize;
  int height = in->Ysize;
  int bands = in->Bands;
  int format = in->BandFmt;
  unsigned char *data = (unsigned char *)malloc(size);

  int beg, end;

  beg = clock();
  if (travel_pixels(in, &out, data))
    vips_error_exit("unable to travel_pixels");

  out2 = vips_image_new_from_memory(data, size, width, height, bands, format);
  if (!out2)
    vips_error_exit("unable new image from memory");

  if (vips_image_write_to_file(out, argv[2], NULL))
    vips_error_exit("unable to write");
  end = clock();
  printf("time(travel_pixels): %dms\n", end - beg);

  if (vips_image_write_to_file(out2, argv[3], NULL))
    vips_error_exit("unable to write");

  g_object_unref(in);

  // Need to read it from file again or error when call
  // `vips_image_write_to_memory`
  if (!(in = vips_image_new_from_file(argv[1], "access", VIPS_ACCESS_SEQUENTIAL,
                                      NULL)))
    vips_error_exit("unable to open");

  beg = clock();
  const void *p = vips_image_write_to_memory(in, NULL);
  if (!p) {
    vips_error_exit("unable to write to memory");
  }
  end = clock();
  printf("time(vips_image_write_to_memory): %dms\n", end - beg);

  g_object_unref(in);
  g_object_unref(out);
  g_object_unref(out2);

  return 0;
}

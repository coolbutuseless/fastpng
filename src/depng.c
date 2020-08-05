

#include <R.h>
#include <Rinternals.h>
#include <Rdefines.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//#include "R-finalizers.h"


#include "spng.h"


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// dpng
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP depng_(SEXP raw_vec_) {

  size_t out_size;
  int buf_size = length(raw_vec_);

  unsigned char *buf = (unsigned char *)RAW(raw_vec_);

  /* Create a context */
  spng_ctx *ctx = spng_ctx_new(0);


  /* Set memory usage limits for storing standard and unknown chunks,
   this is important when reading arbitrary files! */
  size_t limit = 1024 * 1024 * 64;
  spng_set_chunk_limits(ctx, limit, limit);

  /* Set an input buffer */
  spng_set_png_buffer(ctx, buf, buf_size);

  /* Determine output image size */
  spng_decoded_image_size(ctx, SPNG_FMT_RGBA8, &out_size);


  SEXP out_ = PROTECT(allocVector(RAWSXP, out_size));
  unsigned char * out = (unsigned char *)RAW(out_);

  /* Decode to 8-bit RGBA */
  spng_decode_image(ctx, out, out_size, SPNG_FMT_RGBA8, 0);

  /* Free context memory */
  spng_ctx_free(ctx);


  UNPROTECT(1);
  return out_;
}

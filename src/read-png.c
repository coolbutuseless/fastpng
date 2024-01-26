

#include <R.h>
#include <Rinternals.h>
#include <Rdefines.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "spng.h"


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Read image data from PNG stored in a raw vector
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP read_png_raw_(SEXP raw_vec_, SEXP fmt_, SEXP flags_) {

  size_t out_size;
  int buf_size = length(raw_vec_);

  unsigned char *buf = (unsigned char *)RAW(raw_vec_);
  int fmt = INTEGER(fmt_)[0];
  int flags = INTEGER(flags_)[0];


  /* Create a context */
  spng_ctx *ctx = spng_ctx_new(0);


  /* Set memory usage limits for storing standard and unknown chunks,
   this is important when reading arbitrary files! */
  size_t limit = 1024 * 1024 * 64;
  spng_set_chunk_limits(ctx, limit, limit);

  /* Set an input buffer */
  spng_set_png_buffer(ctx, buf, buf_size);

  /* Determine output image size */
  spng_decoded_image_size(ctx, fmt, &out_size);


  SEXP out_ = PROTECT(allocVector(RAWSXP, out_size));
  unsigned char * out = (unsigned char *)RAW(out_);

  /* Decode to 8-bit RGBA */
  int r = spng_decode_image(ctx, out, out_size, fmt, flags);
  if (r) {
    spng_ctx_free(ctx);
    error("spng_decode_image() error: %s\n", spng_strerror(r));
  }

  /* Free context memory */
  spng_ctx_free(ctx);


  UNPROTECT(1);
  return out_;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Read PNG as a nativeraster
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP read_png_nara_(SEXP raw_vec_, SEXP flags_) {
  size_t out_size;
  int buf_size = length(raw_vec_);
  
  unsigned char *buf = (unsigned char *)RAW(raw_vec_);
  int fmt   = SPNG_FMT_RGBA8;
  int flags = INTEGER(flags_)[0];
  
  
  /* Create a context */
  spng_ctx *ctx = spng_ctx_new(0);
  
  
  /* Set memory usage limits for storing standard and unknown chunks,
   this is important when reading arbitrary files! */
  size_t limit = 1024 * 1024 * 64;
  spng_set_chunk_limits(ctx, limit, limit);
  
  /* Set an input buffer */
  spng_set_png_buffer(ctx, buf, buf_size);
  
  /* Determine output image size */
  spng_decoded_image_size(ctx, fmt, &out_size);
  
  
  SEXP out_ = PROTECT(allocVector(INTSXP, out_size >> 2));
  unsigned char * out = (unsigned char *)INTEGER(out_);
  
  // get info
  struct spng_ihdr ihdr;
  int r = spng_get_ihdr(ctx, &ihdr);
  if (r) {
    spng_ctx_free(ctx);
    error("spng_get_ihdr() error: %s\n", spng_strerror(r));
  }
  
  /* Decode to 8-bit RGBA */
  r = spng_decode_image(ctx, out, out_size, fmt, flags);
  if (r) {
    spng_ctx_free(ctx);
    UNPROTECT(1);
    error("spng_decode_image() error: %s\n", spng_strerror(r));
  }
  
  /* Free context memory */
  spng_ctx_free(ctx);
  
  // dim(nara) <- c(76, 100)
  //   class(nara) <- 'nativeRaster'
  // attr(nara, 'channels') <- 4L
  // grid::grid.raster(nara, interpolate = FALSE)
  
  SEXP dims_ = PROTECT(allocVector(INTSXP, 2));
  INTEGER(dims_)[0] = ihdr.height;
  INTEGER(dims_)[1] = ihdr.width;
  
  setAttrib(out_, R_DimSymbol, dims_);
  setAttrib(out_, R_ClassSymbol, mkString("nativeRaster"));
  setAttrib(out_, mkString("channels"), ScalarInteger(4));
  
  
  UNPROTECT(2);
  return out_;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Read PNG as RGBA array
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP read_png_rgba_(SEXP raw_vec_, SEXP fmt_, SEXP flags_) {
  return R_NilValue;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Read PNG as RGBA array
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP read_png_rgb_(SEXP raw_vec_, SEXP fmt_, SEXP flags_) {
  return R_NilValue;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Read PNG as RGBA array
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP read_png_grey_(SEXP raw_vec_, SEXP fmt_, SEXP flags_) {
  return R_NilValue;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Read PNG as RGBA array
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP read_png_raster_(SEXP raw_vec_, SEXP fmt_, SEXP flags_) {
  return R_NilValue;
}

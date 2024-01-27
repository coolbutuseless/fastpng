

#include <R.h>
#include <Rinternals.h>
#include <Rdefines.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "spng.h"




//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Write image data (stored as raw) into PNG (also stored as raw)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP write_png_from_raw_(SEXP raw_vec_, SEXP width_, SEXP height_, 
                         SEXP use_filter_, SEXP compression_level_) {
  int fmt;
  int err = 0;
  spng_ctx *ctx = NULL;
  struct spng_ihdr ihdr = {0}; /* zero-initialize to set valid defaults */
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Options
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  void *image = (void *)RAW(raw_vec_);
  size_t length = length(raw_vec_);
  uint32_t width = (uint32_t)asInteger(width_);
  uint32_t height = (uint32_t)asInteger(height_);
  enum spng_color_type color_type = SPNG_COLOR_TYPE_TRUECOLOR_ALPHA;
  int bit_depth = 8;
  
  int use_filter        = asLogical(use_filter_);
  int compression_level = asInteger(compression_level_);
  if (compression_level < -1 || compression_level > 9) {
    error("Invalid compression level. Must be in range [0, 9] not %i", compression_level);
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Creating an encoder context requires a flag */
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ctx = spng_ctx_new(SPNG_CTX_ENCODER);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Encode to internal buffer managed by the library 
  // Alternatively you can set an output FILE* or stream with 
  //   spng_set_png_file() or spng_set_png_stream() 
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  spng_set_option(ctx, SPNG_ENCODE_TO_BUFFER, 1);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Extra options
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (!use_filter) {
    spng_set_option(ctx, SPNG_FILTER_CHOICE, SPNG_FILTER_CHOICE_NONE);
  }
  spng_set_option(ctx, SPNG_IMG_COMPRESSION_LEVEL , compression_level);
  spng_set_option(ctx, SPNG_TEXT_COMPRESSION_LEVEL, compression_level);
  
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set image properties, this determines the destination image format 
  // Valid color type, bit depth combinations: 
  //   https://www.w3.org/TR/2003/REC-PNG-20031110/#table111 
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ihdr.width      = width;
  ihdr.height     = height;
  ihdr.color_type = color_type;
  ihdr.bit_depth  = bit_depth;
  spng_set_ihdr(ctx, &ihdr);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // When encoding fmt is the source format */
  // SPNG_FMT_PNG is a special value that matches the format in ihdr 
  // SPNG_ENCODE_FINALIZE will finalize the PNG with the end-of-file marker 
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  fmt = SPNG_FMT_PNG;
  
  err = spng_encode_image(ctx, image, length, fmt, SPNG_ENCODE_FINALIZE);
  if (err) {
    spng_ctx_free(ctx);
    error("spng_encode_image() error: %s\n", spng_strerror(err));
  }
  
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Get the internal buffer of the finished PNG 
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  size_t png_size;
  void *png_buf = NULL;
  png_buf = spng_get_png_buffer(ctx, &png_size, &err);
  if(png_buf == NULL) {
    spng_ctx_free(ctx);
    error("spng_get_png_buffer() error: %s\n", spng_strerror(err));
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Copy into R raw vector
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP res_ = PROTECT(allocVector(RAWSXP, png_size));
  memcpy(RAW(res_), png_buf, png_size);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Tidy and return
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  free(png_buf); // User owns this buffer
  spng_ctx_free(ctx);
  UNPROTECT(1);
  return res_;
}




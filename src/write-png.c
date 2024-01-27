

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
    spng_set_option(ctx, SPNG_IMG_COMPRESSION_STRATEGY, 0); // Z_DEFAULT_STRATEGY = 0
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




//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Write image data (stored as native raster) into PNG (also stored as raw)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP write_png_from_nara_(SEXP nara_, SEXP file_, SEXP use_filter_, SEXP compression_level_) {
  int fmt;
  int err = 0;
  spng_ctx *ctx = NULL;
  struct spng_ihdr ihdr = {0}; /* zero-initialize to set valid defaults */
  
  FILE *fp = NULL;
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Options
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  void *image = (void *)INTEGER(nara_);
  size_t length = length(nara_) * 4;
  
  SEXP dims_ = getAttrib(nara_, R_DimSymbol);
  uint32_t width  = (uint32_t)INTEGER(dims_)[1];
  uint32_t height = (uint32_t)INTEGER(dims_)[0];
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
  if (isNull(file_)) {
    spng_set_option(ctx, SPNG_ENCODE_TO_BUFFER, 1);
  } else {
    fp = fopen(CHAR(STRING_ELT(file_, 0)), "wb");
    if (fp == NULL) {
      spng_ctx_free(ctx);
      error("Couldn't open file: %s", CHAR(STRING_ELT(file_, 0)));
    }
    err = spng_set_png_file(ctx, fp); 
    if (err) {
      spng_ctx_free(ctx);
      error("Couldn't set file for output: %s", CHAR(STRING_ELT(file_, 0)));
    }
  } 
  
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Extra options
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (!use_filter) {
    spng_set_option(ctx, SPNG_FILTER_CHOICE, SPNG_FILTER_CHOICE_NONE);
    spng_set_option(ctx, SPNG_IMG_COMPRESSION_STRATEGY, 0); // Z_DEFAULT_STRATEGY = 0
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
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // IF writing to file, can now just return to the caller
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (!isNull(file_)) {
    spng_ctx_free(ctx);
    fclose(fp);
    return R_NilValue;
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
  free(png_buf); // User owns this buffer
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Tidy and return
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  spng_ctx_free(ctx);
  UNPROTECT(1);
  return res_;
}


static unsigned int hexdigit(int digit) {
  if('0' <= digit && digit <= '9') return      digit - '0';
  if('A' <= digit && digit <= 'F') return 10 + digit - 'A';
  if('a' <= digit && digit <= 'f') return 10 + digit - 'a';
  error("invalid hex");
  return digit; 
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Write image data (stored as native raster) into PNG (also stored as raw)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP write_png_from_raster_(SEXP ras_, SEXP file_, SEXP use_filter_, SEXP compression_level_) {
  int fmt;
  int err = 0;
  spng_ctx *ctx = NULL;
  struct spng_ihdr ihdr = {0}; /* zero-initialize to set valid defaults */
  
  FILE *fp = NULL;
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Options
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  size_t length = length(ras_) * 4;
  
  SEXP dims_ = getAttrib(ras_, R_DimSymbol);
  uint32_t width  = (uint32_t)INTEGER(dims_)[1];
  uint32_t height = (uint32_t)INTEGER(dims_)[0];
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
  if (isNull(file_)) {
    spng_set_option(ctx, SPNG_ENCODE_TO_BUFFER, 1);
  } else {
    fp = fopen(CHAR(STRING_ELT(file_, 0)), "wb");
    if (fp == NULL) {
      spng_ctx_free(ctx);
      error("Couldn't open file: %s", CHAR(STRING_ELT(file_, 0)));
    }
    err = spng_set_png_file(ctx, fp); 
    if (err) {
      spng_ctx_free(ctx);
      error("Couldn't set file for output: %s", CHAR(STRING_ELT(file_, 0)));
    }
  } 
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Extra options
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (!use_filter) {
    spng_set_option(ctx, SPNG_FILTER_CHOICE, SPNG_FILTER_CHOICE_NONE);
    spng_set_option(ctx, SPNG_IMG_COMPRESSION_STRATEGY, 0); // Z_DEFAULT_STRATEGY = 0
  }
  spng_set_option(ctx, SPNG_IMG_COMPRESSION_LEVEL , compression_level);
  spng_set_option(ctx, SPNG_TEXT_COMPRESSION_LEVEL, compression_level);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Convert from raster to image
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  unsigned char *image = (unsigned char *)malloc(length);
  if (image == NULL) {
    spng_ctx_free(ctx);
    error("Could not allocate image buffer");
  }
  unsigned char *im_ptr = image;
  for (int i = 0; i < Rf_xlength(ras_); i++) {
    const char *col = CHAR(STRING_ELT(ras_, i));
    *im_ptr++ = (hexdigit(col[1]) << 4) + hexdigit(col[2]); // R
    *im_ptr++ = (hexdigit(col[3]) << 4) + hexdigit(col[4]); // G
    *im_ptr++ = (hexdigit(col[5]) << 4) + hexdigit(col[6]); // B
    *im_ptr++ = (hexdigit(col[7]) << 4) + hexdigit(col[8]); // A
  }
  
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
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // IF writing to file, can now just return to the caller
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (!isNull(file_)) {
    spng_ctx_free(ctx);
    fclose(fp);
    return R_NilValue;
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
  free(png_buf); // User owns this buffer
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Tidy and return
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  spng_ctx_free(ctx);
  UNPROTECT(1);
  return res_;
}

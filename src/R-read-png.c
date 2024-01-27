

#include <R.h>
#include <Rinternals.h>
#include <Rdefines.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "spng.h"




//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Initialise a context
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
spng_ctx *read_png_core(SEXP src_, FILE *fp, int fmt, int *width, int *height, size_t *out_size) {
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create context
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  spng_ctx *ctx = spng_ctx_new(0);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set memory usage limits for storing standard and unknown chunks,
  // this is important when reading arbitrary files! 
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  size_t limit = 1024 * 1024 * 64;
  spng_set_chunk_limits(ctx, limit, limit);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set an input buffer 
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  int buf_size = 0;
  unsigned char *buf = 0;
  if (TYPEOF(src_) == RAWSXP) {
    buf_size = length(src_);
    buf = (unsigned char *)RAW(src_);
    spng_set_png_buffer(ctx, buf, buf_size);
  } else if (TYPEOF(src_) == STRSXP) {
    const char *filename = CHAR(STRING_ELT(src_, 0));
    fp = fopen(filename, "rb");
    if (fp == NULL) {
      spng_ctx_free(ctx);
      error("read_png_core(): Couldn't open file '%s'", filename);
    }
    
    int err = spng_set_png_file(ctx, fp); 
    if (err) {
      fclose(fp);
      spng_ctx_free(ctx);
      error("read_png_core(): Couldn't set file for input: %s", filename);
    }
    
  } else {
    spng_ctx_free(ctx);
    error("read_png_core(): Data source not handled");
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // get info
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  struct spng_ihdr ihdr;
  int err = spng_get_ihdr(ctx, &ihdr);
  if (err) {
    spng_ctx_free(ctx);
    error("spng_get_ihdr() error: %s\n", spng_strerror(err));
  }
  *height = ihdr.height;
  *width  = ihdr.width;
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Determine output image size 
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  spng_decoded_image_size(ctx, fmt, out_size);
  
  return ctx;
}





//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Read image data from PNG stored in a raw vector
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP read_png_as_raw_(SEXP src_, SEXP fmt_, SEXP flags_) {
  
  FILE *fp = NULL;
  int fmt   = asInteger(fmt_);
  int flags = asInteger(flags_);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create a context 
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  int width  = 0;
  int height = 0;
  size_t out_size = 0;
  spng_ctx *ctx = read_png_core(src_, fp, fmt, &width, &height, &out_size);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Initialise memory into which the PNG will be decoded
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP res_ = PROTECT(allocVector(RAWSXP, out_size));
  unsigned char *decode_buf = (unsigned char *)RAW(res_);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Decode to given format
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  int err = spng_decode_image(ctx, decode_buf, out_size, fmt, flags);
  if (err) {
    if (fp) fclose(fp);
    spng_ctx_free(ctx);
    UNPROTECT(1);
    error("spng_decode_image() error: %s\n", spng_strerror(err));
  }
  
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Tidy and return
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (fp) fclose(fp);
  spng_ctx_free(ctx);
  UNPROTECT(1);
  return res_;
}





//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Read PNG as a nativeraster
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP read_png_as_nara_(SEXP src_, SEXP flags_) {
  
  FILE *fp = NULL;
  int fmt   = SPNG_FMT_RGBA8;
  int flags = asInteger(flags_);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create a context 
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  int width  = 0;
  int height = 0;
  size_t out_size = 0;
  spng_ctx *ctx = read_png_core(src_, fp, fmt, &width, &height, &out_size);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Initialise memory into which the PNG will be decoded
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP res_ = PROTECT(allocVector(INTSXP, out_size >> 2));
  unsigned char *decode_buf = (unsigned char *)INTEGER(res_);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Decode 
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  int err = spng_decode_image(ctx, decode_buf, out_size, fmt, flags);
  if (err) {
    if (fp) fclose(fp);
    spng_ctx_free(ctx);
    UNPROTECT(1);
    error("spng_decode_image() error: %s\n", spng_strerror(err));
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set attributs on return value
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP dims_ = PROTECT(allocVector(INTSXP, 2));
  INTEGER(dims_)[0] = height;
  INTEGER(dims_)[1] = width;
  
  setAttrib(res_, R_DimSymbol, dims_);
  setAttrib(res_, R_ClassSymbol, mkString("nativeRaster"));
  setAttrib(res_, mkString("channels"), ScalarInteger(4));
  
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Tidy and return
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (fp) fclose(fp);
  spng_ctx_free(ctx);
  UNPROTECT(2);
  return res_;
}




//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Read PNG as RGBA array
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP read_png_as_raster_(SEXP src_, SEXP flags_) {
  
  FILE *fp = NULL;
  int fmt   = SPNG_FMT_RGBA8;
  int flags = asInteger(flags_);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create a context 
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  int width  = 0;
  int height = 0;
  size_t out_size = 0;
  spng_ctx *ctx = read_png_core(src_, fp, fmt, &width, &height, &out_size);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Initialise memory into which the PNG will be decoded
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  unsigned char *decode_buf = (unsigned char *)malloc(out_size);
  if (decode_buf == NULL) {
    if (fp) fclose(fp);
    spng_ctx_free(ctx);
    error("Couldn't allocate PNG buffer");
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Decode
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  int err = spng_decode_image(ctx, decode_buf, out_size, fmt, flags);
  if (err) {
    if (fp) fclose(fp);
    free(decode_buf);
    spng_ctx_free(ctx);
    error("spng_decode_image() error: %s\n", spng_strerror(err));
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Format raw bytes as #RRGGBBAA string
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP res_ = PROTECT(allocVector(STRSXP, out_size >> 2));
  
  uint32_t *buf_ptr = (uint32_t *)decode_buf;
  
  unsigned char hex_lookup[]= "0123456789ABCDEF"; // Lookup table
  char col[10] = "#00000000"; // template
  
  for (int i = 0; i < length(res_); i++) {
    col[1] = hex_lookup[(*buf_ptr >>  4) & 0x0F];
    col[2] = hex_lookup[(*buf_ptr >>  0) & 0x0F];
    col[3] = hex_lookup[(*buf_ptr >> 12) & 0x0F];
    col[4] = hex_lookup[(*buf_ptr >>  8) & 0x0F];
    col[5] = hex_lookup[(*buf_ptr >> 20) & 0x0F];
    col[6] = hex_lookup[(*buf_ptr >> 16) & 0x0F];
    col[7] = hex_lookup[(*buf_ptr >> 28) & 0x0F];
    col[8] = hex_lookup[(*buf_ptr >> 24) & 0x0F];
    
    SET_STRING_ELT(res_, i, mkChar(col));
    buf_ptr++;
  }
  
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set attributes on result
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP dims_ = PROTECT(allocVector(INTSXP, 2));
  INTEGER(dims_)[0] = height;
  INTEGER(dims_)[1] = width;
  
  setAttrib(res_, R_DimSymbol, dims_);
  setAttrib(res_, R_ClassSymbol, mkString("raster"));
  
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Tidy and return
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (fp) fclose(fp);
  spng_ctx_free(ctx);
  free(decode_buf);
  UNPROTECT(2);
  return res_;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Read PNG as RGBA array
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP read_png_as_rgba_(SEXP src_, SEXP flags_) {
  
  FILE *fp = NULL;
  int fmt   = SPNG_FMT_RGBA8;
  int flags = asInteger(flags_);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create a context 
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  int width  = 0;
  int height = 0;
  size_t out_size = 0;
  spng_ctx *ctx = read_png_core(src_, fp, fmt, &width, &height, &out_size);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Initialise memory into which the PNG will be decoded
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  unsigned char *decode_buf = (unsigned char *)malloc(out_size);
  if (decode_buf == NULL) {
    if (fp) fclose(fp);
    spng_ctx_free(ctx);
    error("Couldn't allocate PNG buffer");
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Decode
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  int err = spng_decode_image(ctx, decode_buf, out_size, fmt, flags);
  if (err) {
    if (fp) fclose(fp);
    free(decode_buf);
    spng_ctx_free(ctx);
    error("spng_decode_image() error: %s\n", spng_strerror(err));
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Format raw bytes as 3D array:  width, height, depth
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP res_ = PROTECT(allocVector(REALSXP, out_size));
  
  double *res_ptr = REAL(res_);
  unsigned char *buf_ptr = decode_buf;
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Need to switch from raw data (row-major) to R array (column-major)
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  for (int row = 0; row < height; row++) {
    
    double *r   = res_ptr + row;
    double *g   = res_ptr + row + (out_size >> 2) * 1;
    double *b   = res_ptr + row + (out_size >> 2) * 2;
    double *a   = res_ptr + row + (out_size >> 2) * 3;
    
    for (int col = 0; col < width; col++) {
      *r = *buf_ptr++ / 255.0;
      *g = *buf_ptr++ / 255.0;
      *b = *buf_ptr++ / 255.0;
      *a = *buf_ptr++ / 255.0;
      r += height;
      g += height;
      b += height;
      a += height;
    }
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set attributes on result
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP dims_ = PROTECT(allocVector(INTSXP, 3));
  INTEGER(dims_)[0] = height;
  INTEGER(dims_)[1] = width;
  INTEGER(dims_)[2] = 4;
  
  setAttrib(res_, R_DimSymbol, dims_);
  setAttrib(res_, R_ClassSymbol, mkString("array"));
  
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Tidy and return
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (fp) fclose(fp);
  spng_ctx_free(ctx);
  free(decode_buf);
  UNPROTECT(2);
  return res_;
}





//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Read PNG as RGBA array
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP read_png_as_rgb_(SEXP src_, SEXP flags_) {
  
  FILE *fp = NULL;
  int fmt   = SPNG_FMT_RGB8;
  int flags = asInteger(flags_);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create a context 
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  int width  = 0;
  int height = 0;
  size_t out_size = 0;
  spng_ctx *ctx = read_png_core(src_, fp, fmt, &width, &height, &out_size);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Initialise memory into which the PNG will be decoded
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  unsigned char *decode_buf = (unsigned char *)malloc(out_size);
  if (decode_buf == NULL) {
    if (fp) fclose(fp);
    spng_ctx_free(ctx);
    error("Couldn't allocate PNG buffer");
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Decode
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  int err = spng_decode_image(ctx, decode_buf, out_size, fmt, flags);
  if (err) {
    if (fp) fclose(fp);
    free(decode_buf);
    spng_ctx_free(ctx);
    error("spng_decode_image() error: %s\n", spng_strerror(err));
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Format raw bytes as 3D array:  width, height, depth
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP res_ = PROTECT(allocVector(REALSXP, out_size));
  
  double *res_ptr = REAL(res_);
  unsigned char *buf_ptr = decode_buf;
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Need to switch from raw data (row-major) to R array (column-major)
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  int npixels = width * height;
  
  for (int row = 0; row < height; row++) {
    
    double *r   = res_ptr + row;
    double *g   = res_ptr + row + npixels * 1;
    double *b   = res_ptr + row + npixels * 2;
    
    for (int col = 0; col < width; col++) {
      *r = *buf_ptr++ / 255.0;
      *g = *buf_ptr++ / 255.0;
      *b = *buf_ptr++ / 255.0;
      r += height;
      g += height;
      b += height;
    }
  }
  
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set attributes on result
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP dims_ = PROTECT(allocVector(INTSXP, 3));
  INTEGER(dims_)[0] = height;
  INTEGER(dims_)[1] = width;
  INTEGER(dims_)[2] = 3;
  
  setAttrib(res_, R_DimSymbol, dims_);
  setAttrib(res_, R_ClassSymbol, mkString("array"));
  
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Tidy and return
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (fp) fclose(fp);
  spng_ctx_free(ctx);
  free(decode_buf);
  UNPROTECT(2);
  return res_;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP read_png_(SEXP src_, SEXP type_, SEXP flags_) {
  
  const char *image_type = CHAR(STRING_ELT(type_, 0));
  
  if (strcmp(image_type, "nara") == 0) {
    return read_png_as_nara_(src_, flags_);
  } else if (strcmp(image_type, "raster") == 0) {
    return read_png_as_raster_(src_, flags_);
  } else if (strcmp(image_type, "rgba") == 0) {
    return read_png_as_rgba_(src_, flags_);
  } else if (strcmp(image_type, "rgb") == 0) {
    return read_png_as_rgb_(src_, flags_);
  }
  
  error("image type not understood: %s", image_type);
  return R_NilValue;
}






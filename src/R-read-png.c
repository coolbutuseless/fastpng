

#include <R.h>
#include <Rinternals.h>
#include <Rdefines.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "spng.h"


#define R_IMAGE_NARA 0
#define R_IMAGE_RASTER 1
#define R_IMAGE_ARRAY 2

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Initialise a context
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
spng_ctx *read_png_core(SEXP src_, FILE **fp, int rgba, int *fmt, int image_type, 
                        uint32_t *width, uint32_t *height, size_t *out_size) {
  
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
  size_t buf_size = 0;
  unsigned char *buf = 0;
  if (TYPEOF(src_) == RAWSXP) {
    buf_size = (size_t)length(src_);
    buf = (unsigned char *)RAW(src_);
    spng_set_png_buffer(ctx, buf, buf_size);
  } else if (TYPEOF(src_) == STRSXP) {
    const char *filename = CHAR(STRING_ELT(src_, 0));
    *fp = fopen(filename, "rb");
    if (fp == NULL) {
      spng_ctx_free(ctx);
      error("read_png_core(): Couldn't open file '%s'", filename);
    }
    
    int err = spng_set_png_file(ctx, *fp); 
    if (err) {
      spng_ctx_free(ctx);
      fclose(*fp);
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
    if (*fp) fclose(*fp);
    error("spng_get_ihdr() error: %s\n", spng_strerror(err));
  }
  *height = ihdr.height;
  *width  = ihdr.width;
  
  
  // SPNG_COLOR_TYPE_GRAYSCALE = 0,
  // SPNG_COLOR_TYPE_TRUECOLOR = 2,
  // SPNG_COLOR_TYPE_INDEXED = 3,
  // SPNG_COLOR_TYPE_GRAYSCALE_ALPHA = 4,
  // SPNG_COLOR_TYPE_TRUECOLOR_ALPHA = 6
  // enum spng_format
  // {
  //   SPNG_FMT_RGBA8 = 1,
  //   SPNG_FMT_RGBA16 = 2,
  //   SPNG_FMT_RGB8 = 4,
  //   
  //   /* Partially implemented, see documentation */
  //   SPNG_FMT_GA8 = 16,
  //   SPNG_FMT_GA16 = 32,
  //   SPNG_FMT_G8 = 64,
  //   
  //   /* No conversion or scaling */
  //   SPNG_FMT_PNG = 256,
  //   SPNG_FMT_RAW = 512  /* big-endian (everything else is host-endian) */
  // };
  if (rgba || image_type == R_IMAGE_NARA) {
    // Set to RGBA if asked
    // NativeRaster can only be RGBA
    *fmt = SPNG_FMT_RGBA8;
  } else if (image_type == R_IMAGE_RASTER) {
    // Raster can only process RGBA or RGB
    if (ihdr.color_type == SPNG_COLOR_TYPE_TRUECOLOR_ALPHA) {
      *fmt = SPNG_FMT_RGBA8;
    } else {
      *fmt = SPNG_FMT_RGB8;
    }
  } else if (image_type == R_IMAGE_ARRAY) {
    if (ihdr.color_type == SPNG_COLOR_TYPE_TRUECOLOR_ALPHA || ihdr.color_type == SPNG_COLOR_TYPE_INDEXED) {
      *fmt = SPNG_FMT_RGBA8;
    } else if (ihdr.color_type == SPNG_COLOR_TYPE_TRUECOLOR) {
      *fmt = SPNG_FMT_RGB8;
    } else if (ihdr.color_type == SPNG_COLOR_TYPE_GRAYSCALE_ALPHA) {
      *fmt = SPNG_FMT_GA8;
    } else if (ihdr.color_type == SPNG_COLOR_TYPE_GRAYSCALE) {
      *fmt = SPNG_FMT_G8;
    } else {
      *fmt = SPNG_FMT_RGBA8;
    }
  } else {
    error("Image type not understood: %i", image_type);
  }
  
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Determine output image size 
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  spng_decoded_image_size(ctx, *fmt, out_size);
  
  return ctx;
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
  uint32_t width  = 0;
  uint32_t height = 0;
  size_t out_size = 0;
  spng_ctx *ctx = read_png_core(src_, &fp, 1, &fmt, R_IMAGE_NARA, &width, &height, &out_size);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Initialise memory into which the PNG will be decoded
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP res_ = PROTECT(allocVector(INTSXP, (R_xlen_t)(out_size >> 2)));
  unsigned char *decode_buf = (unsigned char *)INTEGER(res_);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Decode 
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  int err = spng_decode_image(ctx, decode_buf, out_size, fmt, flags);
  if (err) {
    spng_ctx_free(ctx);
    if (fp) fclose(fp);
    UNPROTECT(1);
    error("spng_decode_image() error: %s\n", spng_strerror(err));
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set attributs on return value
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP dims_ = PROTECT(allocVector(INTSXP, 2));
  INTEGER(dims_)[0] = (int)height;
  INTEGER(dims_)[1] = (int)width;
  
  setAttrib(res_, R_DimSymbol, dims_);
  setAttrib(res_, R_ClassSymbol, mkString("nativeRaster"));
  setAttrib(res_, mkString("channels"), ScalarInteger(4));
  
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Tidy and return
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  spng_ctx_free(ctx);
  if (fp) fclose(fp);
  UNPROTECT(2);
  return res_;
}




//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Read PNG as RGBA array
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP read_png_as_raster_(SEXP src_, SEXP rgba_, SEXP flags_) {
  
  FILE *fp = NULL;
  int fmt   = SPNG_FMT_RGBA8;
  int flags = asInteger(flags_);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create a context 
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  uint32_t width  = 0;
  uint32_t height = 0;
  size_t out_size = 0;
  spng_ctx *ctx = read_png_core(src_, &fp, asInteger(rgba_), &fmt, R_IMAGE_RASTER, &width, &height, &out_size);
  
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
  int npixels = (int)(width * height);
  SEXP res_ = PROTECT(allocVector(STRSXP, (R_xlen_t)npixels));
  
  
  char hex_lookup[]= "0123456789ABCDEF"; // Lookup table
  
  if (fmt == SPNG_FMT_RGBA8) {
    char col[10] = "#00000000"; // template
    uint32_t *buf_ptr = (uint32_t *)decode_buf;
    
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
  } else if (fmt == SPNG_FMT_RGB8) {
    char col[8] = "#000000"; // template
    unsigned char *buf_ptr = (unsigned char *)decode_buf;
    for (int i = 0; i < npixels; i++) {
      col[1] = hex_lookup[(*buf_ptr   >>  4) & 0x0F];
      col[2] = hex_lookup[(*buf_ptr++ >>  0) & 0x0F];
      col[3] = hex_lookup[(*buf_ptr   >>  4) & 0x0F];
      col[4] = hex_lookup[(*buf_ptr++ >>  0) & 0x0F];
      col[5] = hex_lookup[(*buf_ptr   >>  4) & 0x0F];
      col[6] = hex_lookup[(*buf_ptr++ >>  0) & 0x0F];
      
      SET_STRING_ELT(res_, i, mkChar(col));
    }
  }
  
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set attributes on result
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP dims_ = PROTECT(allocVector(INTSXP, 2));
  INTEGER(dims_)[0] = (int)height;
  INTEGER(dims_)[1] = (int)width;
  
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
SEXP read_png_as_array_(SEXP src_, SEXP rgba_, SEXP flags_, SEXP avoid_transpose_) {
  
  FILE *fp = NULL;
  int fmt   = SPNG_FMT_RGBA8;
  int flags = asInteger(flags_);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create a context 
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  uint32_t width  = 0;
  uint32_t height = 0;
  size_t out_size = 0;
  spng_ctx *ctx = read_png_core(src_, &fp, asInteger(rgba_), &fmt, R_IMAGE_ARRAY, &width, &height, &out_size);
  
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // 
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  int npixels = (int)(width * height);
  int nchannels;
  
  switch(fmt) {
  case SPNG_FMT_G8:
    nchannels = 1;
    break;
  case SPNG_FMT_GA8:
    // 2024-02-02 For unknown reasons SPNG doesn't seem to want to read
    // a gray+alpha image back as the 'GA8' format.
    // Error:  spng_decode_image() error: invalid format
    nchannels = 2;
    error("Note: reading Grey+Alpha images currently unsupported");
    break;
  case SPNG_FMT_RGB8:
    nchannels = 3;
    break;
  case SPNG_FMT_RGBA8:
    nchannels = 4;
    break;
  default:
    error("Unhandled format for read_png_as_array: %i", fmt);
  }
  
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
  // Format raw bytes as array:  width, height, depth
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP res_ = PROTECT(allocVector(REALSXP, (R_xlen_t)out_size));
  
  double *res_ptr = REAL(res_);
  unsigned char *buf_ptr = decode_buf;
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Copy pixel data from PNG buffer to R 'res_'
  // Need to switch from raw data (row-major) to R array (column-major)
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (nchannels == 4) {
  for (int row = 0; row < height; row++) {
    
    double *r   = res_ptr + row;
    double *g   = res_ptr + row + npixels * 1;
    double *b   = res_ptr + row + npixels * 2;
    double *a   = res_ptr + row + npixels * 3;
    
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
  } else if (nchannels == 3) {
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
  } else if (nchannels == 2) {
    for (int row = 0; row < height; row++) {
      
      double *g   = res_ptr + row;
      double *a   = res_ptr + row + npixels * 1;
      
      for (int col = 0; col < width; col++) {
        *g = *buf_ptr++ / 255.0;
        *a = *buf_ptr++ / 255.0;
        g += height;
        a += height;
      }
    }
  } else if (nchannels == 1) {
    if (asLogical(avoid_transpose_)) {
      double *r   = res_ptr;
      for (int idx = 0; idx < width * height; idx++) {
        *r++ = *buf_ptr++ / 255.0;
      }
    } else {
      for (int row = 0; row < height; row++) {
        double *r   = res_ptr + row;
        for (int col = 0; col < width; col++) {
          *r = *buf_ptr++ / 255.0;
          r += height;
        }
      }
    }
  } else {
    error("unhandled nchannels: %i", nchannels);
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set attributes on result
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (nchannels == 1) {
    SEXP dims_ = PROTECT(allocVector(INTSXP, 2));
    if (asLogical(avoid_transpose_)) {
      INTEGER(dims_)[0] = (int)width;
      INTEGER(dims_)[1] = (int)height;
    } else {
      INTEGER(dims_)[0] = (int)height;
      INTEGER(dims_)[1] = (int)width;
    }
    
    setAttrib(res_, R_DimSymbol, dims_);
    setAttrib(res_, R_ClassSymbol, mkString("matrix"));
  } else {
    SEXP dims_ = PROTECT(allocVector(INTSXP, 3));
    INTEGER(dims_)[0] = (int)height;
    INTEGER(dims_)[1] = (int)width;
    INTEGER(dims_)[2] = nchannels;
    
    setAttrib(res_, R_DimSymbol, dims_);
    setAttrib(res_, R_ClassSymbol, mkString("array"));
  }
  
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
// 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP read_png_(SEXP src_, SEXP type_, SEXP flags_, SEXP rgba_, SEXP avoid_transpose_) {
  
  const char *image_type = CHAR(STRING_ELT(type_, 0));
  
  if (strcmp(image_type, "nara") == 0) {
    return read_png_as_nara_(src_, flags_);
  } else if (strcmp(image_type, "raster") == 0) {
    return read_png_as_raster_(src_, rgba_, flags_);
  } else if (strcmp(image_type, "array") == 0) {
    return read_png_as_array_(src_, rgba_, flags_, avoid_transpose_);
  }
  
  error("image type not understood: %s", image_type);
  return R_NilValue;
}






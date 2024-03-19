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
#define R_IMAGE_INDEXED 3

//=============================================================================
// Core PNG Read function (used by all readers)
//
// * Create a context
// * Set source to data in memory or file
// * Read the image header to determine metainformation
//       * width, height
//       * number of bits
//       * number of channels
//       * colour format
//       * total decoded size
//
// @param src_ raw vector or filename
// @param **fp pointer to a file pointer - populated if `src_` is a filename, 
//        otherwise NULL
// @param rgba boolean. Should read mode be forced to be TRUECOLOR RGBA?
// @param *fmt detected image format to be filled in here
// @param image_type the coded image type i.e. R_IMAGE_NARA etc
// @return spng_ctx
//=============================================================================
spng_ctx *read_png_core(SEXP src_, FILE **fp, int rgba, int *fmt, int image_type, 
                        uint32_t *width, uint32_t *height, size_t *out_size,
                        uint8_t *bits, uint32_t *nchannels) {
  
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
    const char *filename = R_ExpandFileName(CHAR(STRING_ELT(src_, 0)));
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
    error("read_png_core(): Data source must be a raw vector or path to an existing file");
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
  *bits   = ihdr.bit_depth;
  
  
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
  
  if (image_type == R_IMAGE_INDEXED && ihdr.color_type != SPNG_COLOR_TYPE_INDEXED) {
    error("type='indexed' cannot be used as this is not an indexed PNG");
  }
  
  struct spng_trns trns;
  int has_trns = (spng_get_trns(ctx, &trns) == 0);
  
  if (image_type == R_IMAGE_NARA) {
    // NativeRaster can only be RGBA
    *fmt = SPNG_FMT_RGBA8;
  } else if (rgba) {
    // Set to RGBA if asked
    if (*bits == 16) {
      *fmt = SPNG_FMT_RGBA16;  
    } else {
      *fmt = SPNG_FMT_RGBA8;
    }
    if (nchannels) {
      *nchannels = 4;
    }
  } else if (image_type == R_IMAGE_RASTER) {
    // Raster can only process RGBA or RGB
    if (ihdr.color_type == SPNG_COLOR_TYPE_TRUECOLOR_ALPHA || ihdr.color_type == SPNG_COLOR_TYPE_GRAYSCALE_ALPHA || has_trns) {
      *fmt = SPNG_FMT_RGBA8;
    } else {
      *fmt = SPNG_FMT_RGB8;
    }
  } else if (image_type == R_IMAGE_ARRAY) {
    if (*bits == 16) {
      if (ihdr.color_type == SPNG_COLOR_TYPE_TRUECOLOR_ALPHA) {
        *fmt = SPNG_FMT_RGBA16;
        *nchannels = 4;
      } else if (ihdr.color_type == SPNG_COLOR_TYPE_TRUECOLOR) {
        *fmt = SPNG_FMT_PNG;
        *nchannels = 3;
      } else if (ihdr.color_type == SPNG_COLOR_TYPE_GRAYSCALE_ALPHA) {
        *fmt = SPNG_FMT_PNG;
        *nchannels = 2;
      } else if (ihdr.color_type == SPNG_COLOR_TYPE_GRAYSCALE) {
        *fmt = SPNG_FMT_PNG;
        *nchannels = 1;
      } else {
        *fmt = SPNG_FMT_RGBA16;
        *nchannels = 4;
      }
    } else {
      if (ihdr.color_type == SPNG_COLOR_TYPE_TRUECOLOR_ALPHA || (ihdr.color_type == SPNG_COLOR_TYPE_INDEXED  && has_trns)) {
        *fmt = SPNG_FMT_RGBA8;
        *nchannels = 4;
      } else if (ihdr.color_type == SPNG_COLOR_TYPE_TRUECOLOR || ihdr.color_type == SPNG_COLOR_TYPE_INDEXED) {
        *fmt = SPNG_FMT_RGB8;
        *nchannels = 3;
      } else if (ihdr.color_type == SPNG_COLOR_TYPE_GRAYSCALE_ALPHA) {
        *fmt = SPNG_FMT_PNG;
        *nchannels = 2;
      } else if (ihdr.color_type == SPNG_COLOR_TYPE_GRAYSCALE) {
        *fmt = SPNG_FMT_G8;
        *nchannels = 1;
      } else {
        *fmt = SPNG_FMT_RGBA8;
        *nchannels = 4;
      }
    }
  } else if (ihdr.color_type == SPNG_COLOR_TYPE_INDEXED) {
    *fmt = SPNG_FMT_PNG;
  } else {
    error("Image type not understood: %i", image_type);
  }
  
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Determine output image size 
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  spng_decoded_image_size(ctx, *fmt, out_size);
  
  return ctx;
}



//=============================================================================
// Read PNG as raw() vector
//=============================================================================
SEXP read_png_as_raw_(SEXP src_, SEXP rgba_, SEXP flags_) {
  
  FILE *fp = NULL;
  int fmt   = SPNG_FMT_RGBA8;
  uint8_t bits  = 8; 
  int flags = asInteger(flags_);
  uint32_t nchannels;
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create a context 
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  uint32_t width  = 0;
  uint32_t height = 0;
  size_t out_size = 0;
  spng_ctx *ctx = read_png_core(src_, &fp, asInteger(rgba_), &fmt, R_IMAGE_ARRAY, &width, &height, &out_size, &bits, &nchannels);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // 
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  uint32_t npixels = (uint32_t)(width * height);
  if (bits == 8) {
    nchannels = (uint32_t)(out_size / (size_t)npixels);
  } else if (bits == 16) {
    nchannels = (uint32_t)(out_size / (size_t)npixels / 2);
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
  // Prep space for raw bytes
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP res_ = PROTECT(allocVector(RAWSXP, (R_xlen_t)out_size));
  
  unsigned char *res_ptr = RAW(res_);
  unsigned char *buf_ptr = decode_buf;
  
  memcpy(res_ptr, buf_ptr, out_size);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set attributes on result
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  setAttrib(res_, mkString("width") , ScalarInteger((int)width));
  setAttrib(res_, mkString("height"), ScalarInteger((int)height));
  setAttrib(res_, mkString("depth") , ScalarInteger((int)nchannels));
  setAttrib(res_, mkString("bits" ) , ScalarInteger(bits));

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Tidy and return
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (fp) fclose(fp);
  spng_ctx_free(ctx);
  free(decode_buf);
  UNPROTECT(1);
  return res_;
}




//=============================================================================
// Read PNG as a nativeraster
//=============================================================================
SEXP read_png_as_nara_(SEXP src_, SEXP flags_) {
  
  FILE *fp = NULL;
  int fmt   = SPNG_FMT_RGBA8;
  uint8_t bits  = 8; 
  int flags = asInteger(flags_);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create a context 
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  uint32_t width  = 0;
  uint32_t height = 0;
  size_t out_size = 0;
  spng_ctx *ctx = read_png_core(src_, &fp, 1, &fmt, R_IMAGE_NARA, &width, &height, &out_size, &bits, NULL);
  
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




//=============================================================================
// Read PNG as RGBA array
//=============================================================================
SEXP read_png_as_raster_(SEXP src_, SEXP rgba_, SEXP flags_) {
  
  FILE *fp = NULL;
  uint8_t bits  = 8; 
  int fmt   = SPNG_FMT_RGBA8;
  int flags = asInteger(flags_);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create a context 
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  uint32_t width  = 0;
  uint32_t height = 0;
  size_t out_size = 0;
  spng_ctx *ctx = read_png_core(src_, &fp, asInteger(rgba_), &fmt, R_IMAGE_RASTER, &width, &height, &out_size, &bits, NULL);
  
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
// Forward declaration
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP read_png_as_array16_(SEXP src_, SEXP rgba_, SEXP flags_, SEXP avoid_transpose_, SEXP array_type_);

//=============================================================================
// Read PNG as RGBA array
//=============================================================================
SEXP read_png_as_array_(SEXP src_, SEXP rgba_, SEXP flags_, SEXP avoid_transpose_, SEXP array_type_) {
  
  FILE *fp = NULL;
  int fmt   = SPNG_FMT_RGBA8;
  uint8_t bits  = 8; 
  int flags = asInteger(flags_);
  uint32_t nchannels;
  
  // Rprintf(">> read_png_as_array_()\n");
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create a context 
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  uint32_t width  = 0;
  uint32_t height = 0;
  size_t out_size = 0;
  spng_ctx *ctx = read_png_core(src_, &fp, asInteger(rgba_), &fmt, R_IMAGE_ARRAY, &width, &height, &out_size, &bits, &nchannels);
  
  if (bits == 16) {
    return read_png_as_array16_(src_, rgba_, flags_, avoid_transpose_, array_type_);
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // 
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  int npixels = (int)(width * height);

  
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
  SEXP res_;
  unsigned char *buf_ptr = decode_buf;
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Copy pixel data from PNG buffer to R 'res_'
  // Need to switch from raw data (row-major) to R array (column-major)
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (strcmp(CHAR(STRING_ELT(array_type_, 0)), "dbl") == 0) {
    res_ = PROTECT(allocVector(REALSXP, (R_xlen_t)out_size));
    double *res_ptr = REAL(res_);
    if (nchannels == 1 && asLogical(avoid_transpose_)) {
      double *r   = res_ptr;
      for (int idx = 0; idx < width * height; idx++) {
        *r++ = *buf_ptr++ / 255.0;
      }
    } else {
      for (int row = 0; row < height; row++) {
        
        double *p0   = res_ptr + row + npixels * 0;
        double *p1   = res_ptr + row + npixels * 1;
        double *p2   = res_ptr + row + npixels * 2;
        double *p3   = res_ptr + row + npixels * 3;
        
        for (int col = 0; col < width; col++, p0+=height, p1+=height, p2+=height, p3+=height, buf_ptr+=nchannels) {
          switch(nchannels) {
          case 4:
            *p3 = buf_ptr[3] / 255.0;
          case 3:
            *p2 = buf_ptr[2] / 255.0;
          case 2:
            *p1 = buf_ptr[1] / 255.0;
          case 1:
            *p0 = buf_ptr[0] / 255.0;
          }
        }
      }
    } 
  } else {
    res_ = PROTECT(allocVector(INTSXP, (R_xlen_t)out_size));
    int32_t *res_ptr = INTEGER(res_);
    if (nchannels == 1 && asLogical(avoid_transpose_)) {
      int32_t *r   = res_ptr;
      for (int idx = 0; idx < width * height; idx++) {
        *r++ = *buf_ptr++;
      }
    } else {
      for (int row = 0; row < height; row++) {
        
        int32_t *p0   = res_ptr + row + npixels * 0;
        int32_t *p1   = res_ptr + row + npixels * 1;
        int32_t *p2   = res_ptr + row + npixels * 2;
        int32_t *p3   = res_ptr + row + npixels * 3;
        
        for (int col = 0; col < width; col++, p0+=height, p1+=height, p2+=height, p3+=height, buf_ptr+=nchannels) {
          switch(nchannels) {
          case 4:
            *p3 = buf_ptr[3];
          case 3:
            *p2 = buf_ptr[2];
          case 2:
            *p1 = buf_ptr[1];
          case 1:
            *p0 = buf_ptr[0];
          }
        }
      }
    } 
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
  } else {
    SEXP dims_ = PROTECT(allocVector(INTSXP, 3));
    INTEGER(dims_)[0] = (int)height;
    INTEGER(dims_)[1] = (int)width;
    INTEGER(dims_)[2] = (int)nchannels;
    
    setAttrib(res_, R_DimSymbol, dims_);
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



//=============================================================================
// Read PNG as RGBA array.  16 bits
//=============================================================================
SEXP read_png_as_array16_(SEXP src_, SEXP rgba_, SEXP flags_, SEXP avoid_transpose_, SEXP array_type_) {
  
  FILE *fp = NULL;
  uint8_t bits  = 8; 
  int fmt   = SPNG_FMT_RGBA8;
  int flags = asInteger(flags_);
  uint32_t nchannels;
  
  // Rprintf("read_png_as_array16_()\n");
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create a context 
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  uint32_t width  = 0;
  uint32_t height = 0;
  size_t out_size = 0;
  spng_ctx *ctx = read_png_core(src_, &fp, asInteger(rgba_), &fmt, R_IMAGE_ARRAY, &width, &height, &out_size, &bits, &nchannels);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // 
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  uint32_t npixels = (uint32_t)(width * height);

  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Initialise memory into which the PNG will be decoded
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  uint16_t *decode_buf = (uint16_t *)malloc(out_size);
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
  SEXP res_;
  uint16_t *buf_ptr = decode_buf;
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Copy pixel data from PNG buffer to R 'res_'
  // Need to switch from raw data (row-major) to R array (column-major)
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (strcmp(CHAR(STRING_ELT(array_type_, 0)), "dbl") == 0) {
    res_ = PROTECT(allocVector(REALSXP, (R_xlen_t)(npixels * nchannels)));
    double *res_ptr = REAL(res_);
    if (nchannels == 1 && asLogical(avoid_transpose_)) {
      double *r   = res_ptr;
      for (int idx = 0; idx < width * height; idx++) {
        *r++ = *buf_ptr++ / 65535.0;
      }
    } else {
      for (int row = 0; row < height; row++) {
        
        double *p0   = res_ptr + row + npixels * 0;
        double *p1   = res_ptr + row + npixels * 1;
        double *p2   = res_ptr + row + npixels * 2;
        double *p3   = res_ptr + row + npixels * 3;
        
        for (int col = 0; col < width; col++, p0+=height, p1+=height, p2+=height, p3+=height, buf_ptr+=nchannels) {
          switch(nchannels) {
          case 4:
            *p3 = buf_ptr[3] / 65535.0;
          case 3:
            *p2 = buf_ptr[2] / 65535.0;
          case 2:
            *p1 = buf_ptr[1] / 65535.0;
          case 1:
            *p0 = buf_ptr[0] / 65535.0;
          }
        }
      }
    }
  } else {
    res_ = PROTECT(allocVector(INTSXP, npixels * nchannels));
    int32_t *res_ptr = INTEGER(res_);
    if (nchannels == 1 && asLogical(avoid_transpose_)) {
      int32_t *r   = res_ptr;
      for (int idx = 0; idx < width * height; idx++) {
        *r++ = *buf_ptr++;
      }
    } else {
      for (int row = 0; row < height; row++) {
        
        int32_t *p0   = res_ptr + row + npixels * 0;
        int32_t *p1   = res_ptr + row + npixels * 1;
        int32_t *p2   = res_ptr + row + npixels * 2;
        int32_t *p3   = res_ptr + row + npixels * 3;
        
        for (int col = 0; col < width; col++, p0+=height, p1+=height, p2+=height, p3+=height, buf_ptr+=nchannels) {
          switch(nchannels) {
          case 4:
            *p3 = buf_ptr[3];
          case 3:
            *p2 = buf_ptr[2];
          case 2:
            *p1 = buf_ptr[1];
          case 1:
            *p0 = buf_ptr[0];
          }
        }
      }
    }
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
  } else {
    SEXP dims_ = PROTECT(allocVector(INTSXP, 3));
    INTEGER(dims_)[0] = (int)height;
    INTEGER(dims_)[1] = (int)width;
    INTEGER(dims_)[2] = (int)nchannels;
    
    setAttrib(res_, R_DimSymbol, dims_);
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



//=============================================================================
// Read PNG as Indexed
//=============================================================================
SEXP read_indexed_png_as_indexed_(SEXP src_, SEXP rgba_, SEXP flags_, SEXP avoid_transpose_) {
  
  FILE *fp = NULL;
  uint8_t bits  = 8; 
  int fmt   = SPNG_FMT_G8;
  int flags = asInteger(flags_);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create a context 
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  uint32_t width  = 0;
  uint32_t height = 0;
  size_t out_size = 0;
  spng_ctx *ctx = read_png_core(src_, &fp, asInteger(rgba_), &fmt, R_IMAGE_INDEXED, &width, &height, &out_size, &bits, NULL);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Initialise memory into which the PNG will be decoded
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  unsigned char *decode_buf = (unsigned char *)malloc(out_size);
  if (decode_buf == NULL) {
    if (fp) fclose(fp);
    spng_ctx_free(ctx);
    error("Couldn't allocate PNG buffer");
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Decode all chunks
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  int err = spng_decode_chunks(ctx);
  if (err) {
    if (fp) fclose(fp);
    free(decode_buf);
    spng_ctx_free(ctx);
    error("spng_decode_image() chunks error: %s\n", spng_strerror(err));
  }
  
  // int spng_get_plte(spng_ctx *ctx, struct spng_plte *plte)
  struct spng_plte plte;
  err = spng_get_plte(ctx, &plte);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // tRNS
  // Note: there is no guarantee that the number of trNS entries matches
  // the number of palette entries
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  struct spng_trns trns;
  int trns_err = spng_get_trns(ctx, &trns); 
  
  char hex_lookup[]= "0123456789ABCDEF"; // Lookup table
  SEXP palette_ = PROTECT(allocVector(STRSXP, plte.n_entries));
  char col[10] = "#000000FF"; // template
  for (int i = 0; i < plte.n_entries; i++) {
    col[1] = hex_lookup[(plte.entries[i].red   >>  4) & 0x0F];
    col[2] = hex_lookup[(plte.entries[i].red   >>  0) & 0x0F];
    col[3] = hex_lookup[(plte.entries[i].green >>  4) & 0x0F];
    col[4] = hex_lookup[(plte.entries[i].green >>  0) & 0x0F];
    col[5] = hex_lookup[(plte.entries[i].blue  >>  4) & 0x0F];
    col[6] = hex_lookup[(plte.entries[i].blue  >>  0) & 0x0F];
    
    if (trns_err == 0 && i < trns.n_type3_entries) {
      // There is a tRNS entry for this palette entry
      // Rprintf("TRNS %i = %i\n", i, trns.type3_alpha[i]);
      col[7] = hex_lookup[(trns.type3_alpha[i]  >>  4) & 0x0F];
      col[8] = hex_lookup[(trns.type3_alpha[i]  >>  0) & 0x0F];
    } else {
      // There is no tRNS
      col[7] = 'F';
      col[8] = 'F';
    }
    
    SET_STRING_ELT(palette_, i, mkChar(col));
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Decode
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  err = spng_decode_image(ctx, decode_buf, out_size, fmt, flags);
  if (err) {
    if (fp) fclose(fp);
    free(decode_buf);
    spng_ctx_free(ctx);
    error("spng_decode_image() error: %s\n", spng_strerror(err));
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Format raw bytes as array:  width, height, depth
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP res_ = PROTECT(allocVector(INTSXP, (R_xlen_t)out_size));
  
  int32_t *res_ptr = INTEGER(res_);
  unsigned char *buf_ptr = decode_buf;
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Copy pixel data from PNG buffer to R 'res_'
  // Need to switch from raw data (row-major) to R array (column-major)
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (asLogical(avoid_transpose_)) {
    int32_t *r   = res_ptr;
    for (int idx = 0; idx < width * height; idx++) {
      *r++ = *buf_ptr++;
    }
  } else {
    for (int row = 0; row < height; row++) {
      int32_t *r   = res_ptr + row;
      for (int col = 0; col < width; col++) {
        *r = *buf_ptr++;
        r += height;
      }
    }
  }
  
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set attributes on result
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP dims_ = PROTECT(allocVector(INTSXP, 2));
  if (asLogical(avoid_transpose_)) {
    INTEGER(dims_)[0] = (int)width;
    INTEGER(dims_)[1] = (int)height;
  } else {
    INTEGER(dims_)[0] = (int)height;
    INTEGER(dims_)[1] = (int)width;
  }
  
  setAttrib(res_, R_DimSymbol, dims_);
  

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Return named list object
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP list_ = PROTECT(allocVector(VECSXP, 2));
  SET_VECTOR_ELT(list_, 0, res_);
  SET_VECTOR_ELT(list_, 1, palette_);
  
  SEXP nms_ = PROTECT(allocVector(STRSXP, 2));
  SET_STRING_ELT(nms_, 0, mkChar("index"));
  SET_STRING_ELT(nms_, 1, mkChar("palette"));
  setAttrib(list_, R_NamesSymbol, nms_);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Tidy and return
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (fp) fclose(fp);
  spng_ctx_free(ctx);
  free(decode_buf);
  UNPROTECT(5);
  return list_;
}




//=============================================================================
// C function called from R
//=============================================================================
SEXP read_png_(SEXP src_, SEXP type_, SEXP rgba_, SEXP flags_, SEXP avoid_transpose_,
               SEXP array_type_) {
  
  const char *image_type = CHAR(STRING_ELT(type_, 0));
  
  if (strcmp(image_type, "native_raster") == 0) {
    return read_png_as_nara_(src_, flags_);
  } else if (strcmp(image_type, "raster") == 0) {
    return read_png_as_raster_(src_, rgba_, flags_);
  } else if (strcmp(image_type, "array") == 0) {
    return read_png_as_array_(src_, rgba_, flags_, avoid_transpose_, array_type_);
  } else if (strcmp(image_type, "indexed") == 0) {
    return read_indexed_png_as_indexed_(src_, rgba_, flags_, avoid_transpose_);
  } else if (strcmp(image_type, "raw") == 0) {
    return read_png_as_raw_(src_, rgba_, flags_);
  }
  
  error("read_png(): Image type not understood: '%s'", image_type);
  return R_NilValue;
}

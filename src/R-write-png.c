

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
SEXP write_png_core_(void *image, size_t nbytes, uint32_t width, uint32_t height, 
                     SEXP file_,
                     enum spng_color_type color_type,
                     SEXP use_filter_, SEXP compression_level_,
                     int free_image_on_error) {
  
  int fmt;
  int err = 0;
  spng_ctx *ctx = NULL;
  struct spng_ihdr ihdr = {0}; /* zero-initialize to set valid defaults */
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Options
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  int bit_depth = 8;
  int use_filter        = asLogical(use_filter_);
  int compression_level = asInteger(compression_level_);
  if (compression_level < -1 || compression_level > 9) {
    if (free_image_on_error) free(image);
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
  FILE *fp = NULL;
  if (isNull(file_)) {
    spng_set_option(ctx, SPNG_ENCODE_TO_BUFFER, 1);
  } else {
    fp = fopen(CHAR(STRING_ELT(file_, 0)), "wb");
    if (fp == NULL) {
      if (free_image_on_error) free(image);
      spng_ctx_free(ctx);
      error("Couldn't open file: %s", CHAR(STRING_ELT(file_, 0)));
    }
    err = spng_set_png_file(ctx, fp); 
    if (err) {
      fclose(fp);
      if (free_image_on_error) free(image);
      spng_ctx_free(ctx);
      error("Couldn't set file for output: %s", CHAR(STRING_ELT(file_, 0)));
    }
  } 
  
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
  
  err = spng_encode_image(ctx, image, nbytes, fmt, SPNG_ENCODE_FINALIZE);
  if (err) {
    if (fp) fclose(fp);
    if (free_image_on_error) free(image);
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
    if (free_image_on_error) free(image);
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
// Write image data (stored as raw) into PNG (also stored as raw)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP write_png_from_raw_(SEXP src_, SEXP file_, SEXP width_, SEXP height_, 
                         SEXP use_filter_, SEXP compression_level_) {
  
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Options
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  size_t nbytes = length(src_);
  uint32_t width = (uint32_t)asInteger(width_);
  uint32_t height = (uint32_t)asInteger(height_);
  void *image = (void *)RAW(src_);
  
  return write_png_core_(
    image, nbytes, width, height, file_,
    SPNG_COLOR_TYPE_TRUECOLOR_ALPHA,
    use_filter_, compression_level_,
    FALSE // free_image_on_error
  );
}




//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Write image data (stored as native raster) into PNG (also stored as raw)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP write_png_from_nara_(SEXP nara_, SEXP file_, SEXP use_filter_, SEXP compression_level_) {
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Options
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  void *image = (void *)INTEGER(nara_);
  size_t nbytes = length(nara_) * 4;
  SEXP dims_ = getAttrib(nara_, R_DimSymbol);
  uint32_t width  = (uint32_t)INTEGER(dims_)[1];
  uint32_t height = (uint32_t)INTEGER(dims_)[0];
  
  return write_png_core_(
    image, nbytes, width, height, file_,
    SPNG_COLOR_TYPE_TRUECOLOR_ALPHA,
    use_filter_, compression_level_,
    FALSE // free_image_on_error
  );
}


static unsigned int hexdigit(int digit) {
  if('0' <= digit && digit <= '9') return      digit - '0';
  if('A' <= digit && digit <= 'F') return 10 + digit - 'A';
  if('a' <= digit && digit <= 'f') return 10 + digit - 'a';
  error("invalid hex: %i", digit);
  return digit; 
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Write image data (stored as native raster) into PNG (also stored as raw)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP write_png_from_raster_(SEXP ras_, SEXP file_, SEXP use_filter_, SEXP compression_level_) {
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Options
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  size_t nbytes   = length(ras_) * 4;
  SEXP dims_      = getAttrib(ras_, R_DimSymbol);
  uint32_t width  = (uint32_t)INTEGER(dims_)[1];
  uint32_t height = (uint32_t)INTEGER(dims_)[0];
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Convert from raster to image
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  unsigned char *image = (unsigned char *)malloc(nbytes);
  if (image == NULL) {
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
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Encode
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP res_ = PROTECT(write_png_core_(
    image, nbytes, width, height, file_,
    SPNG_COLOR_TYPE_TRUECOLOR_ALPHA,
    use_filter_, compression_level_,
    TRUE // free_image_on_error
  ));
  
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Tidy and return
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  free(image);
  UNPROTECT(1);
  return res_;
}




//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Write image data (stored as native raster) into PNG (also stored as raw)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP write_png_from_rgba_(SEXP arr_, SEXP file_, SEXP use_filter_, SEXP compression_level_) {
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Options
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  size_t nbytes = length(arr_);
  SEXP dims_    = getAttrib(arr_, R_DimSymbol);
  if (length(dims_) != 3) {
    error("Must be 3d array");
  }
  if (INTEGER(dims_)[2] != 4) {
    error("Must be RGBA array");
  }
  uint32_t width  = (uint32_t)INTEGER(dims_)[1];
  uint32_t height = (uint32_t)INTEGER(dims_)[0];
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Convert from array to raw vec
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  unsigned char *image = (unsigned char *)malloc(nbytes);
  if (image == NULL) {
    error("Could not allocate image buffer");
  }
  unsigned char *im_ptr = image;
  
  for (int row = 0; row < height; row++) {
    double *r = REAL(arr_) + row + (width * height) * 0;
    double *g = REAL(arr_) + row + (width * height) * 1;
    double *b = REAL(arr_) + row + (width * height) * 2;
    double *a = REAL(arr_) + row + (width * height) * 3;
    for (int col = 0; col < width; col++) {
      *im_ptr++ = *r * 255;
      *im_ptr++ = *g * 255;
      *im_ptr++ = *b * 255;
      *im_ptr++ = *a * 255;
      r += height;
      g += height;
      b += height;
      a += height;
    }
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Encode
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP res_ = PROTECT(write_png_core_(
    image, nbytes, width, height, file_,
    SPNG_COLOR_TYPE_TRUECOLOR_ALPHA,
    use_filter_, compression_level_,
    TRUE // free_image_on_error
  ));
  
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Tidy and return
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  free(image);
  UNPROTECT(1);
  return res_;
}




//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Write image data (stored as native raster) into PNG (also stored as raw)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP write_png_from_rgb_(SEXP arr_, SEXP file_, SEXP use_filter_, SEXP compression_level_) {
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Options
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  size_t nbytes = length(arr_);
  SEXP dims_    = getAttrib(arr_, R_DimSymbol);
  if (length(dims_) != 3) {
    error("Must be 3d array");
  }
  if (INTEGER(dims_)[2] != 3) {
    error("Must be RGB array");
  }
  uint32_t width  = (uint32_t)INTEGER(dims_)[1];
  uint32_t height = (uint32_t)INTEGER(dims_)[0];
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Convert from array to raw vec
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  unsigned char *image = (unsigned char *)malloc(nbytes);
  if (image == NULL) {
    error("Could not allocate image buffer");
  }
  unsigned char *im_ptr = image;
  
  for (int row = 0; row < height; row++) {
    double *r = REAL(arr_) + row + (width * height) * 0;
    double *g = REAL(arr_) + row + (width * height) * 1;
    double *b = REAL(arr_) + row + (width * height) * 2;
    for (int col = 0; col < width; col++) {
      *im_ptr++ = *r * 255;
      *im_ptr++ = *g * 255;
      *im_ptr++ = *b * 255;
      r += height;
      g += height;
      b += height;
    }
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Encode
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP res_ = PROTECT(write_png_core_(
    image, nbytes, width, height, file_,
    SPNG_COLOR_TYPE_TRUECOLOR,
    use_filter_, compression_level_,
    TRUE // free_image_on_error
  ));
  
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Tidy and return
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  free(image);
  UNPROTECT(1);
  return res_;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Write image data 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP write_png_(SEXP image_, SEXP file_, SEXP use_filter_, SEXP compression_level_) {
  
  if (inherits(image_, "nativeRaster")) {
    return write_png_from_nara_(image_, file_, use_filter_, compression_level_);
  } else if (inherits(image_, "raster")) {
    return write_png_from_raster_(image_, file_, use_filter_, compression_level_);
  } else if (inherits(image_, "array") && isReal(image_)) {
    SEXP dims_ = getAttrib(image_, R_DimSymbol);
    if (length(dims_) == 3) {
      if (INTEGER(dims_)[2] == 4) {
        return write_png_from_rgba_(image_, file_, use_filter_, compression_level_);
      } else if (INTEGER(dims_)[2] == 3) {
        return write_png_from_rgb_(image_, file_, use_filter_, compression_level_);
      }
    }
  }
  
  error("write_png(): R image container not understood");
  return R_NilValue;
}














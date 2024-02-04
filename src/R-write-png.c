

#include <R.h>
#include <Rinternals.h>
#include <Rdefines.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "spng.h"



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Convert a hex digit to a nibble
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static unsigned char hexdigit(int digit) {
  if('0' <= digit && digit <= '9') return (unsigned char)(     digit - '0');
  if('A' <= digit && digit <= 'F') return (unsigned char)(10 + digit - 'A');
  if('a' <= digit && digit <= 'f') return (unsigned char)(10 + digit - 'a');
  error("Invalid hex: %i.  Only 6-char and 8 char hex colours supported e.g. '#RRGGBB' and '#RRGGBBAA' \nR colours in a raster image (e.g. 'white') are not supported", digit);
  return (unsigned char)digit; 
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Write image data (stored as raw) into PNG (also stored as raw)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP write_png_core_(void *image, size_t nbytes, uint32_t width, uint32_t height, 
                     SEXP file_,
                     enum spng_color_type color_type,
                     SEXP palette_,
                     SEXP use_filter_, SEXP compression_level_,
                     int free_image_on_error) {
  
  int fmt;
  int err = 0;
  spng_ctx *ctx = NULL;
  struct spng_ihdr ihdr = {0}; /* zero-initialize to set valid defaults */
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Options
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  uint8_t bit_depth = 8;
  int use_filter        = asLogical(use_filter_);
  int compression_level = asInteger(compression_level_);
  if (compression_level < -1 || compression_level > 9) {
    if (free_image_on_error) free(image);
    error("Invalid compression level. Must be in range [0, 9] not %i", compression_level);
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Creating an encoder context requires a flag 
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
  ihdr.color_type = (uint8_t)color_type;
  ihdr.bit_depth  = bit_depth;
  spng_set_ihdr(ctx, &ihdr);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Write Palette:  PLTE Chunk
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (!isNull(palette_)) {
    
    struct spng_plte plte;
    plte.n_entries = length(palette_); // length is checked prior to calling core func
    for (int i = 0; i < length(palette_); i++) {
      const char *col = CHAR(STRING_ELT(palette_, i));
      if (col[0] != '#') {
        error("Palettes may only contain hex colours of the form '#RRGGBB' or '#RRGGBBAA'");
      }
      plte.entries[i].red   = (uint8_t)( (hexdigit(col[1]) << 4) + hexdigit(col[2]) ); // R
      plte.entries[i].green = (uint8_t)( (hexdigit(col[3]) << 4) + hexdigit(col[4]) ); // G
      plte.entries[i].blue  = (uint8_t)( (hexdigit(col[5]) << 4) + hexdigit(col[6]) ); // B
    }
    
    // int spng_set_plte(spng_ctx *ctx, struct spng_plte *plte)
    err = spng_set_plte(ctx, &plte);
    if (err) {
      if (fp) fclose(fp);
      if (free_image_on_error) free(image);
      spng_ctx_free(ctx);
      error("spng_encode_image() PLTE error: %s\n", spng_strerror(err));
    }
    
    err = spng_encode_chunks(ctx);
    if (err) {
      if (fp) fclose(fp);
      if (free_image_on_error) free(image);
      spng_ctx_free(ctx);
      error("spng_encode_image() chunks error: %s\n", spng_strerror(err));
    }
  }
  
  
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
  SEXP res_ = PROTECT(allocVector(RAWSXP, (R_xlen_t)png_size));
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
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Options
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  void *image = (void *)INTEGER(nara_);
  size_t nbytes = (size_t)(length(nara_) * 4.0);
  SEXP dims_ = getAttrib(nara_, R_DimSymbol);
  uint32_t width  = (uint32_t)INTEGER(dims_)[1];
  uint32_t height = (uint32_t)INTEGER(dims_)[0];
  
  return write_png_core_(
    image, nbytes, width, height, file_,
    SPNG_COLOR_TYPE_TRUECOLOR_ALPHA,
    R_NilValue, // Palette
    use_filter_, compression_level_,
    FALSE // free_image_on_error
  );
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Write image data (stored as native raster) 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP write_png_from_raster_(SEXP ras_, SEXP file_, SEXP use_filter_, SEXP compression_level_) {
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Options
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  size_t nbytes   = (size_t)(length(ras_) * 4.0);
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
    if (col[0] != '#') {
      error("Valid rasters may only contain hex colours of the form '#RRGGBB' or '#RRGGBBAA'");
    }
    *im_ptr++ = (unsigned char)( (hexdigit(col[1]) << 4) + hexdigit(col[2]) ); // R
    *im_ptr++ = (unsigned char)( (hexdigit(col[3]) << 4) + hexdigit(col[4]) ); // G
    *im_ptr++ = (unsigned char)( (hexdigit(col[5]) << 4) + hexdigit(col[6]) ); // B
    *im_ptr++ = (unsigned char)( (hexdigit(col[7]) << 4) + hexdigit(col[8]) ); // A
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Encode
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP res_ = PROTECT(write_png_core_(
    image, nbytes, width, height, file_,
    SPNG_COLOR_TYPE_TRUECOLOR_ALPHA,
    R_NilValue, // Palette
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
// Write image data stored as hex colours in a character matrix
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP write_png_from_raster_rgb_(SEXP ras_, SEXP file_, SEXP use_filter_, SEXP compression_level_) {
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Options
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  size_t nbytes   = (size_t)(length(ras_) * 3.0);
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
    if (col[0] != '#') {
      error("Valid rasters may only contain hex colours of the form '#RRGGBB' or '#RRGGBBAA'");
    }
    *im_ptr++ = (unsigned char)( (hexdigit(col[1]) << 4) + hexdigit(col[2]) ); // R
    *im_ptr++ = (unsigned char)( (hexdigit(col[3]) << 4) + hexdigit(col[4]) ); // G
    *im_ptr++ = (unsigned char)( (hexdigit(col[5]) << 4) + hexdigit(col[6]) ); // B
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Encode
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP res_ = PROTECT(write_png_core_(
    image, nbytes, width, height, file_,
    SPNG_COLOR_TYPE_TRUECOLOR,
    R_NilValue, // Palette
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
// Write image data stored as RGBA numeric array
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP write_png_from_array_(SEXP arr_, SEXP file_, SEXP use_filter_, SEXP compression_level_, 
                           SEXP avoid_transpose_) {
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Options
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  size_t nbytes = (size_t)(length(arr_));
  
  enum spng_color_type fmt;
  SEXP dims_ = getAttrib(arr_, R_DimSymbol);
  if (length(dims_) == 2) {
    fmt = SPNG_COLOR_TYPE_GRAYSCALE;
    // SPNG_COLOR_TYPE_TRUECOLOR_ALPHA
    // error("Must be 3d array");
  } else if (length(dims_) == 3) {
    switch(INTEGER(dims_)[2]) {
    case 2:
      fmt = SPNG_COLOR_TYPE_GRAYSCALE_ALPHA;
      break;
    case 3:
      fmt = SPNG_COLOR_TYPE_TRUECOLOR;
      break;
    case 4:
      fmt = SPNG_COLOR_TYPE_TRUECOLOR_ALPHA;
      break;
    default:
      error("Unknown 3rd dimension length: %i", INTEGER(dims_)[2]);
    }
  } else {
    error("Unknown dims length: %i", length(dims_));
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
  
  int npixels = (int)(width * height);
  double *arr_ptr = REAL(arr_);
  unsigned char *im_ptr = image;
  
  if (fmt == SPNG_COLOR_TYPE_TRUECOLOR_ALPHA) {
    for (int row = 0; row < height; row++) {
      double *r = arr_ptr + row + npixels * 0;
      double *g = arr_ptr + row + npixels * 1;
      double *b = arr_ptr + row + npixels * 2;
      double *a = arr_ptr + row + npixels * 3;
      for (int col = 0; col < width; col++) {
        *im_ptr++ = (unsigned char)(*r * 255.0 + 0.5);
        *im_ptr++ = (unsigned char)(*g * 255.0 + 0.5);
        *im_ptr++ = (unsigned char)(*b * 255.0 + 0.5);
        *im_ptr++ = (unsigned char)(*a * 255.0 + 0.5);
        r += height;
        g += height;
        b += height;
        a += height;
      }
    }
  } else if (fmt == SPNG_COLOR_TYPE_TRUECOLOR) {
    for (int row = 0; row < height; row++) {
      double *r = arr_ptr + row + npixels * 0;
      double *g = arr_ptr + row + npixels * 1;
      double *b = arr_ptr + row + npixels * 2;
      for (int col = 0; col < width; col++) {
        *im_ptr++ = (unsigned char)(*r * 255.0 + 0.5);
        *im_ptr++ = (unsigned char)(*g * 255.0 + 0.5);
        *im_ptr++ = (unsigned char)(*b * 255.0 + 0.5);
        r += height;
        g += height;
        b += height;
      }
    }
  } else if (fmt == SPNG_COLOR_TYPE_GRAYSCALE_ALPHA) {
    for (int row = 0; row < height; row++) {
      double *g = arr_ptr + row + npixels * 0;
      double *a = arr_ptr + row + npixels * 1;
      for (int col = 0; col < width; col++) {
        *im_ptr++ = (unsigned char)(*g * 255.0 + 0.5);
        *im_ptr++ = (unsigned char)(*a * 255.0 + 0.5);
        g += height;
        a += height;
      }
    }
  } else if (fmt == SPNG_COLOR_TYPE_GRAYSCALE) {
    if (asLogical(avoid_transpose_)) {
      double *r = arr_ptr;
      for (int idx = 0; idx < npixels; idx ++) {
        *im_ptr++ = (unsigned char)(*r++ * 255.0 + 0.5);
      }
    } else {
      for (int row = 0; row < height; row++) {
        double *r = arr_ptr + row;
        for (int col = 0; col < width; col++) {
          *im_ptr++ = (unsigned char)(*r * 255.0 + 0.5);
          r += height;
        }
      }
    }
  } else {
    error("Unknown fmt: %i", fmt);
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Encode
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP res_ = PROTECT(write_png_core_(
    image, nbytes, width, height, file_,
    fmt,
    R_NilValue, // Palette
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


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP write_png_indexed_(SEXP arr_, SEXP file_, SEXP palette_, SEXP use_filter_, 
                        SEXP compression_level_, SEXP avoid_transpose_) {
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Options
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  size_t nbytes = (size_t)(length(arr_));
  
  enum spng_color_type fmt = SPNG_COLOR_TYPE_INDEXED;
  SEXP dims_ = getAttrib(arr_, R_DimSymbol);
  if (length(dims_) != 2) {
    error("write_png_indexed_(): Must be 2-D array");
  } 
  uint32_t width  = (uint32_t)INTEGER(dims_)[1];
  uint32_t height = (uint32_t)INTEGER(dims_)[0];
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Convert from column-major R matrix  to row major unsigned char
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  unsigned char *image = (unsigned char *)malloc(nbytes);
  if (image == NULL) {
    error("Could not allocate image buffer");
  }
  
  int npixels = (int)(width * height);
  unsigned char *im_ptr = image;
  
  if (isInteger(arr_)) {
    if (asLogical(avoid_transpose_)) {
      int32_t *r = INTEGER(arr_);
      for (int idx = 0; idx < npixels; idx ++) {
        *im_ptr++ = (unsigned char)(*r++);
      }
    } else {
      int32_t *arr_ptr = INTEGER(arr_);
      for (int row = 0; row < height; row++) {
        int32_t *r = arr_ptr + row;
        for (int col = 0; col < width; col++) {
          *im_ptr++ = (unsigned char)(*r);
          r += height;
        }
      }
    }
  } else if (isReal(arr_)) {
    if (asLogical(avoid_transpose_)) {
      double *r = REAL(arr_);
      for (int idx = 0; idx < npixels; idx ++) {
        *im_ptr++ = (unsigned char)(*r++);
      }
    } else {
      double *arr_ptr = REAL(arr_);
      for (int row = 0; row < height; row++) {
        double *r = arr_ptr + row;
        for (int col = 0; col < width; col++) {
          *im_ptr++ = (unsigned char)(*r);
          r += height;
        }
      }
    }
  } else {
    error("Index type not understood");
  }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // No tranposition, so swap width/height value
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (asLogical(avoid_transpose_)) {
    int tmp = height;
    height = width;
    width = tmp;
  }
  
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Encode
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP res_ = PROTECT(write_png_core_(
    image, nbytes, width, height, file_,
    fmt,
    palette_,
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
SEXP write_png_(SEXP image_, SEXP file_, SEXP palette_, SEXP use_filter_, SEXP compression_level_, SEXP avoid_transpose_) {

  if (!isNull(palette_)) {
    if (!isMatrix(image_)) {
      error("write_png(): When palette provided image must be a matrix.");
    }
    if (!isNumeric(image_)) {
      error("write_png(): When writing paletted PNG, image must be integer or numeric matrix with values in range [0, 255]");
    }
    if (length(palette_) > 256 || TYPEOF(palette_) != STRSXP) {
      error("Palette must be a character vector of hex colours. length <= 256 elements");
    }
    return write_png_indexed_(image_, file_, palette_, use_filter_, compression_level_, avoid_transpose_);
  } else if (inherits(image_, "nativeRaster")) {
    return write_png_from_nara_(image_, file_, use_filter_, compression_level_);
  } else if (inherits(image_, "raster") && TYPEOF(image_) == STRSXP) {
    if (length(image_) == 0) {
      error("Zero length rasters not valid");
    }
    const char *first_elem = CHAR(STRING_ELT(image_, 0));
    if (first_elem[0] != '#') {
      error("Valid rasters may only contain hex colours of the form '#RRGGBB' or '#RRGGBBAA'");  
    }
    if (strlen(first_elem) == 9) {
      return write_png_from_raster_(image_, file_, use_filter_, compression_level_);
    } else if (strlen(first_elem) == 7) {
      return write_png_from_raster_rgb_(image_, file_, use_filter_, compression_level_);
    } else {
      error("Raster encoding not understood");
    }
  } else if ((isArray(image_) || isMatrix(image_)) && isReal(image_)) {
    return write_png_from_array_(image_, file_, use_filter_, compression_level_, avoid_transpose_);
  }
  
  error("write_png(): R image container not understood");
  return R_NilValue;
}














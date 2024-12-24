
#define R_NO_REMAP

#include <R.h>
#include <Rinternals.h>
#include <Rdefines.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>


#include "spng.h"
#include "colorfast.h"


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Write image data (stored as raw) into PNG (also stored as raw)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP write_png_core_(void *image, size_t nbytes, uint32_t width, uint32_t height, 
                     SEXP file_,
                     enum spng_color_type color_type,
                     SEXP palette_,
                     SEXP use_filter_, SEXP compression_level_,
                     int free_image_on_error, 
                     uint8_t bit_depth, 
                     SEXP trns_) {
  
  int fmt;
  int err = 0;
  spng_ctx *ctx = NULL;
  struct spng_ihdr ihdr = {0}; /* zero-initialize to set valid defaults */
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Options
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  int use_filter        = Rf_asLogical(use_filter_);
  int compression_level = Rf_asInteger(compression_level_);
  if (compression_level < -1 || compression_level > 9) {
    if (free_image_on_error) free(image);
    Rf_error("Invalid compression level. Must be in range [0, 9] not %i", compression_level);
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
  if (Rf_isNull(file_)) {
    spng_set_option(ctx, SPNG_ENCODE_TO_BUFFER, 1);
  } else {
    const char *filename = R_ExpandFileName(CHAR(STRING_ELT(file_, 0)));
    fp = fopen(filename, "wb");
    if (fp == NULL) {
      if (free_image_on_error) free(image);
      spng_ctx_free(ctx);
      Rf_error("Couldn't open file: %s", filename);
    }
    err = spng_set_png_file(ctx, fp); 
    if (err) {
      fclose(fp);
      if (free_image_on_error) free(image);
      spng_ctx_free(ctx);
      Rf_error("Couldn't set file for output: %s", filename);
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
  // Write TRNS chunk if
  //  (a) it is provided
  //  (b) it makes sense.  i.e. image is RGB (raster/array) or grayscale (array).
  // SPNG_COLOR_TYPE_GRAYSCALE = 0,
  //   SPNG_COLOR_TYPE_TRUECOLOR = 2,
  //   SPNG_COLOR_TYPE_INDEXED = 3,
  //   SPNG_COLOR_TYPE_GRAYSCALE_ALPHA = 4,
  //   SPNG_COLOR_TYPE_TRUECOLOR_ALPHA = 6
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (!Rf_isNull(trns_)) {
    struct spng_trns trns;
    int has_trns = 0;
    if (color_type == SPNG_COLOR_TYPE_GRAYSCALE) {
      // Rprintf("trns given and valid for image type GRAYSCALE\n");
      trns.gray = (uint16_t)(Rf_asInteger(trns_));
      has_trns = 1;
    } else if (color_type == SPNG_COLOR_TYPE_TRUECOLOR) {
      // Rprintf("trns given and valid for image type RGB\n");
      if (TYPEOF(trns_) == STRSXP) {
        // Rprintf("Hex color for trns\n");
        const char *col = CHAR(STRING_ELT(trns_, 0));
        uint32_t icol = col_to_int(col);
        trns.red   = (uint16_t)CF_RED(icol);
        trns.green = (uint16_t)CF_GREEN(icol);
        trns.blue  = (uint16_t)CF_BLUE(icol);
        has_trns = 1;
      } else if (Rf_isInteger(trns_) && Rf_length(trns_) == 3) {
        // Rprintf("Integer vector for trns\n");
        int *ptr = INTEGER(trns_);
        trns.red   = (uint16_t)ptr[0]; // R
        trns.green = (uint16_t)ptr[1]; // G
        trns.blue  = (uint16_t)ptr[2]; // B
        has_trns = 1;
      } else if (Rf_isReal(trns_) && Rf_length(trns_) == 3) {
        // Rprintf("Real vector for trns\n");
        double *ptr = REAL(trns_);
        trns.red   = (uint16_t)ptr[0]; // R
        trns.green = (uint16_t)ptr[1]; // G
        trns.blue  = (uint16_t)ptr[2]; // B
        has_trns = 1;
      } else {
      //   Rf_warning("Unknown argument for 'trns' %s\n", type2char((SEXPTYPE)TYPEOF(trns_)));
      }
    } else {
      // has_trns = 0
      // Rprintf("trns given and --NOT-- valid for image type\n");
    }
    
    // Set tRNS chunk
    if (has_trns) {
      err = spng_set_trns(ctx, &trns);
      if (err) {
        Rf_warning("spng_encode_image() TRNS error: %s\n", spng_strerror(err));
      }
    }
    
  }
  
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Write Palette:  
  //   tRNS CHunk for alpha channel
  //   PLTE Chunk for RGB palette
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (!Rf_isNull(palette_)) {
    
    // struct spng_trns
    // {
    //   uint16_t gray;
    //   
    //   uint16_t red;
    //   uint16_t green;
    //   uint16_t blue;
    //   
    //   uint32_t n_type3_entries;
    //   uint8_t type3_alpha[256];
    // };
    // int spng_set_trns(spng_ctx *ctx, struct spng_trns *trns)
    
    struct spng_trns trns;
    struct spng_plte plte;
    
    plte.n_entries = (uint32_t)Rf_length(palette_); // length is checked prior to calling core func
    trns.n_type3_entries = (uint32_t)Rf_length(palette_); // always match palette length
    
    for (int i = 0; i < Rf_length(palette_); i++) {
      const char *col = CHAR(STRING_ELT(palette_, i));
      uint32_t icol = col_to_int(col);
      plte.entries[i].red   = CF_RED(icol);
      plte.entries[i].green = CF_GREEN(icol);
      plte.entries[i].blue  = CF_BLUE(icol);
      trns.type3_alpha[i]   = CF_ALPHA(icol);
    }
    
    // Set PLTE chunk
    err = spng_set_plte(ctx, &plte);
    if (err) {
      if (fp) fclose(fp);
      if (free_image_on_error) free(image);
      spng_ctx_free(ctx);
      Rf_error("spng_encode_image() PLTE error: %s\n", spng_strerror(err));
    }
    
    // Set tRNS chunk
    err = spng_set_trns(ctx, &trns);
    if (err) {
      if (fp) fclose(fp);
      if (free_image_on_error) free(image);
      spng_ctx_free(ctx);
      Rf_error("spng_encode_image() TRNS error: %s\n", spng_strerror(err));
    }
    
    err = spng_encode_chunks(ctx);
    if (err) {
      if (fp) fclose(fp);
      if (free_image_on_error) free(image);
      spng_ctx_free(ctx);
      Rf_error("spng_encode_image() chunks error: %s\n", spng_strerror(err));
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
    Rf_error("spng_encode_image() error: %s\n", spng_strerror(err));
  }
  
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // IF writing to file, can now just return to the caller
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (!Rf_isNull(file_)) {
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
    Rf_error("spng_get_png_buffer() error: %s\n", spng_strerror(err));
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Copy into R raw vector
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP res_ = PROTECT(Rf_allocVector(RAWSXP, (R_xlen_t)png_size));
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
// Write PNG from raw data
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP write_png_from_raw_vec_(SEXP image_, SEXP file_, SEXP use_filter_, 
                             SEXP compression_level_, SEXP trns_, 
                             SEXP raw_spec_) {
  
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Unpack the 'raw_spec' list into variables
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (Rf_isNull(raw_spec_) || TYPEOF(raw_spec_) != VECSXP || Rf_length(raw_spec_) < 4) {
    Rf_error("'raw_spec' must be a named list with 4 elements");
  }
  
  uint32_t width  = 0;
  uint32_t height = 0;
  uint32_t depth  = 0;
  uint32_t bits   = 0;
  
  SEXP nms_ = Rf_getAttrib(raw_spec_, R_NamesSymbol);
  if (Rf_isNull(nms_) || Rf_length(nms_) != Rf_length(raw_spec_)) {
    Rf_error("'raw_spec' must be a named list with 4 elements.");
  }
  
  for (int i = 0; i < Rf_length(nms_); i++) {
    const char *nm = CHAR(STRING_ELT(nms_, i));
    if (strcmp(nm, "width") == 0) {
      width = (uint32_t)Rf_asInteger(VECTOR_ELT(raw_spec_, i));
    } else if (strcmp(nm, "height") == 0) {
      height = (uint32_t)Rf_asInteger(VECTOR_ELT(raw_spec_, i));
    } else if (strcmp(nm, "depth") == 0) {
      depth = (uint32_t)Rf_asInteger(VECTOR_ELT(raw_spec_, i));
    } else if (strcmp(nm, "bits") == 0) {
      bits = (uint32_t)Rf_asInteger(VECTOR_ELT(raw_spec_, i));
    }
  }
  
  if (width == 0 || height == 0 || depth == 0 || bits == 0) {
    Rf_error("'raw_spec' must contain 'width', 'height', 'depth', 'bits'");
  }
  
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Calculate number of bytes specified by the 'raw_spec'
  // And sanity check against the actual data provided
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  uint32_t size = height * width * depth;
  if (bits == 16) {
    size *= 2; 
  }
  
  if (size != Rf_length(image_)) {
    Rf_error("Mismatch between length of raw vector (%i) and raw_spec (%i x %i x %i x %i/8)", 
          Rf_length(image_), width, height, depth, bits);
  }
  
  enum spng_color_type color_type = SPNG_COLOR_TYPE_TRUECOLOR_ALPHA;
  
  switch (depth) {
  case 4:
    color_type = SPNG_COLOR_TYPE_TRUECOLOR_ALPHA;
    break;
  case 3:
    color_type = SPNG_COLOR_TYPE_TRUECOLOR;
    break;
  case 2:
    color_type = SPNG_COLOR_TYPE_GRAYSCALE_ALPHA;
    break;
  case 1:
    color_type = SPNG_COLOR_TYPE_GRAYSCALE;
    break;
  default:
    Rf_error("Depth not understood: %i", depth);
  }
  
  return write_png_core_(
    RAW(image_), (size_t)Rf_length(image_), width, height, file_,
    color_type,
    R_NilValue, // Palette
    use_filter_, compression_level_,
    FALSE, // free_image_on_error
    (uint8_t)bits, // bit depth
    trns_ // trns
  );
  
  return R_NilValue;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Write image data (stored as native raster) into PNG (also stored as raw)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP write_png_from_nara_(SEXP nara_, SEXP file_, SEXP use_filter_, SEXP compression_level_) {
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Options
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  void *image = (void *)INTEGER(nara_);
  size_t nbytes = (size_t)(Rf_length(nara_) * 4.0);
  SEXP dims_ = Rf_getAttrib(nara_, R_DimSymbol);
  uint32_t width  = (uint32_t)INTEGER(dims_)[1];
  uint32_t height = (uint32_t)INTEGER(dims_)[0];
  
  return write_png_core_(
    image, nbytes, width, height, file_,
    SPNG_COLOR_TYPE_TRUECOLOR_ALPHA,
    R_NilValue, // Palette
    use_filter_, compression_level_,
    FALSE, // free_image_on_error
    8, // bit depth
    R_NilValue // trns
  );
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Write image data (stored as native raster) 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP write_png_from_raster_(SEXP ras_, SEXP file_, SEXP use_filter_, SEXP compression_level_) {
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Options
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  size_t nbytes   = (size_t)(Rf_length(ras_) * 4.0);
  SEXP dims_      = Rf_getAttrib(ras_, R_DimSymbol);
  uint32_t width  = (uint32_t)INTEGER(dims_)[1];
  uint32_t height = (uint32_t)INTEGER(dims_)[0];
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Convert from raster to image
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  unsigned char *image = (unsigned char *)malloc(nbytes);
  
  if (image == NULL) {
    Rf_error("Could not allocate image buffer");
  }
  uint32_t *im_ptr = (uint32_t *)image;
  for (int i = 0; i < Rf_length(ras_); i++) {
    const char *col = CHAR(STRING_ELT(ras_, i));
    im_ptr[i] = col_to_int(col);
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Encode
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP res_ = PROTECT(write_png_core_(
    image, nbytes, width, height, file_,
    SPNG_COLOR_TYPE_TRUECOLOR_ALPHA,
    R_NilValue, // Palette
    use_filter_, compression_level_,
    TRUE,  // free_image_on_error
    8, // bit depth
    R_NilValue // trns
  ));
  
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Tidy and return
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  free(image);
  UNPROTECT(1);
  return res_;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Write image data stored as hex colors in a character matrix
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP write_png_from_raster_rgb_(SEXP ras_, SEXP file_, SEXP use_filter_, SEXP compression_level_,
                                SEXP trns_) {
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Options
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  size_t nbytes   = (size_t)(Rf_length(ras_) * 3.0);
  SEXP dims_      = Rf_getAttrib(ras_, R_DimSymbol);
  uint32_t width  = (uint32_t)INTEGER(dims_)[1];
  uint32_t height = (uint32_t)INTEGER(dims_)[0];
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Convert from raster to image
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  unsigned char *image = (unsigned char *)malloc(nbytes);
  if (image == NULL) {
    Rf_error("Could not allocate image buffer");
  }
  unsigned char *im_ptr = image;
  for (int i = 0; i < Rf_length(ras_); i++) {
    const char *col = CHAR(STRING_ELT(ras_, i));
    uint32_t icol = col_to_int(col);
    *im_ptr++ = CF_RED(icol);
    *im_ptr++ = CF_GREEN(icol);
    *im_ptr++ = CF_BLUE(icol);
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Encode
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP res_ = PROTECT(write_png_core_(
    image, nbytes, width, height, file_,
    SPNG_COLOR_TYPE_TRUECOLOR,
    R_NilValue, // Palette
    use_filter_, compression_level_,
    TRUE,  // free_image_on_error
    8, // bit depth
    trns_ // trns
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
                           SEXP avoid_transpose_, SEXP trns_) {
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Options
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  size_t nbytes = (size_t)(Rf_length(arr_));
  
  enum spng_color_type fmt;
  int nchannels;
  SEXP dims_ = Rf_getAttrib(arr_, R_DimSymbol);
  if (Rf_length(dims_) == 2) {
    fmt = SPNG_COLOR_TYPE_GRAYSCALE;
    nchannels = 1;
    // SPNG_COLOR_TYPE_TRUECOLOR_ALPHA
    // Rf_error("Must be 3d array");
  } else if (Rf_length(dims_) == 3) {
    nchannels = INTEGER(dims_)[2];
    switch(nchannels) {
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
      Rf_error("Unknown 3rd dimension length: %i", INTEGER(dims_)[2]);
    }
  } else {
    Rf_error("Unknown dims length: %i", Rf_length(dims_));
  }
  uint32_t width  = (uint32_t)INTEGER(dims_)[1];
  uint32_t height = (uint32_t)INTEGER(dims_)[0];
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Convert from array to raw vec
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  unsigned char *image = (unsigned char *)malloc(nbytes);
  if (image == NULL) {
    Rf_error("Could not allocate image buffer");
  }
  
  int npixels = (int)(width * height);
  unsigned char *im_ptr = image;
  
  if (Rf_isReal(arr_)) {
    double *arr_ptr = REAL(arr_);
    if (fmt == SPNG_COLOR_TYPE_GRAYSCALE && Rf_asLogical(avoid_transpose_)) {
      double *r = arr_ptr;
      for (int idx = 0; idx < npixels; idx ++) {
        *im_ptr++ = (uint8_t)(*r++ * 255.0 + 0.5);
      }
      uint32_t tmp = height;
      height = width;
      width = tmp;
    } else {
      for (int row = 0; row < height; row++) {
        double *p0 = arr_ptr + row + npixels * 0;
        double *p1 = arr_ptr + row + npixels * 1;
        double *p2 = arr_ptr + row + npixels * 2;
        double *p3 = arr_ptr + row + npixels * 3;
        for (int col = 0; col < width; col++, p0+=height, p1+=height, p2+=height, p3+=height, im_ptr+=nchannels) {
          switch(nchannels) {
          case 4:
            im_ptr[3] = (uint8_t)(*p3 * 255.0 + 0.5);
          case 3:
            im_ptr[2] = (uint8_t)(*p2 * 255.0 + 0.5);
          case 2:
            im_ptr[1] = (uint8_t)(*p1 * 255.0 + 0.5);
          case 1:
            im_ptr[0] = (uint8_t)(*p0 * 255.0 + 0.5);
          }
        }
      }
    } 
  } else if (Rf_isInteger(arr_)) {
    int32_t *arr_ptr = INTEGER(arr_);
    if (fmt == SPNG_COLOR_TYPE_GRAYSCALE && Rf_asLogical(avoid_transpose_)) {
      int32_t *r = arr_ptr;
      for (int idx = 0; idx < npixels; idx ++) {
        *im_ptr++ = (uint8_t)(*r++);
      }
      uint32_t tmp = height;
      height = width;
      width = tmp;
    } else {
      for (int row = 0; row < height; row++) {
        int32_t *p0 = arr_ptr + row + npixels * 0;
        int32_t *p1 = arr_ptr + row + npixels * 1;
        int32_t *p2 = arr_ptr + row + npixels * 2;
        int32_t *p3 = arr_ptr + row + npixels * 3;
        for (int col = 0; col < width; col++, p0+=height, p1+=height, p2+=height, p3+=height, im_ptr+=nchannels) {
          switch(nchannels) {
          case 4:
            im_ptr[3] = (uint8_t)(*p3);
          case 3:
            im_ptr[2] = (uint8_t)(*p2);
          case 2:
            im_ptr[1] = (uint8_t)(*p1);
          case 1:
            im_ptr[0] = (uint8_t)(*p0);
          }
        }
      }
    } 
  } 
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Encode
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP res_ = PROTECT(write_png_core_(
    image, nbytes, width, height, file_,
    fmt,
    R_NilValue, // Palette
    use_filter_, compression_level_,
    TRUE,  // free_image_on_error
    8, // bit depth
    trns_ // trns
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
SEXP write_png_from_array16_(SEXP arr_, SEXP file_, SEXP use_filter_, SEXP compression_level_, 
                             SEXP avoid_transpose_) {
  
  // Rprintf("write_png_from_array16_()  %i\n", Rf_isReal(arr_));
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Options
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  size_t nbytes = (size_t)(2.0 * Rf_length(arr_));
  
  enum spng_color_type fmt;
  SEXP dims_ = Rf_getAttrib(arr_, R_DimSymbol);
  int nchannels;
  if (Rf_length(dims_) == 2) {
    fmt = SPNG_COLOR_TYPE_GRAYSCALE;
    nchannels = 1;
    // SPNG_COLOR_TYPE_TRUECOLOR_ALPHA
    // Rf_error("Must be 3d array");
  } else if (Rf_length(dims_) == 3) {
    nchannels = INTEGER(dims_)[2];
    switch(nchannels) {
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
      Rf_error("Unknown 3rd dimension length: %i", INTEGER(dims_)[2]);
    }
  } else {
    Rf_error("Unknown dims length: %i", Rf_length(dims_));
  }
  uint32_t width  = (uint32_t)INTEGER(dims_)[1];
  uint32_t height = (uint32_t)INTEGER(dims_)[0];
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Convert from array to raw vec
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  uint16_t *image = (uint16_t *)malloc(nbytes);
  if (image == NULL) {
    Rf_error("Could not allocate image buffer");
  }
  
  int npixels = (int)(width * height);
  uint16_t *im_ptr = image;
  
  if (Rf_isReal(arr_)) {
    double *arr_ptr = REAL(arr_);
    if (fmt == SPNG_COLOR_TYPE_GRAYSCALE && Rf_asLogical(avoid_transpose_)) {
      double *r = arr_ptr;
      for (int idx = 0; idx < npixels; idx ++) {
        *im_ptr++ = (uint16_t)(*r++ * 65535.0 + 0.5);
      }
      uint32_t tmp = height;
      height = width;
      width = tmp;
    } else {
      for (int row = 0; row < height; row++) {
        double *p0 = arr_ptr + row + npixels * 0;
        double *p1 = arr_ptr + row + npixels * 1;
        double *p2 = arr_ptr + row + npixels * 2;
        double *p3 = arr_ptr + row + npixels * 3;
        for (int col = 0; col < width; col++, p0+=height, p1+=height, p2+=height, p3+=height, im_ptr+=nchannels) {
          switch(nchannels) {
          case 4:
            im_ptr[3] = (uint16_t)(*p3 * 65535.0 + 0.5);
          case 3:
            im_ptr[2] = (uint16_t)(*p2 * 65535.0 + 0.5);
          case 2:
            im_ptr[1] = (uint16_t)(*p1 * 65535.0 + 0.5);
          case 1:
            im_ptr[0] = (uint16_t)(*p0 * 65535.0 + 0.5);
          }
        }
      }
    } 
  } else if (Rf_isInteger(arr_)) {
    int32_t *arr_ptr = INTEGER(arr_);
    if (fmt == SPNG_COLOR_TYPE_GRAYSCALE && Rf_asLogical(avoid_transpose_)) {
      int32_t *r = arr_ptr;
      for (int idx = 0; idx < npixels; idx ++) {
        *im_ptr++ = (uint16_t)(*r++);
      }
      uint32_t tmp = height;
      height = width;
      width = tmp;
    } else {
      for (int row = 0; row < height; row++) {
        int32_t *p0 = arr_ptr + row + npixels * 0;
        int32_t *p1 = arr_ptr + row + npixels * 1;
        int32_t *p2 = arr_ptr + row + npixels * 2;
        int32_t *p3 = arr_ptr + row + npixels * 3;
        for (int col = 0; col < width; col++, p0+=height, p1+=height, p2+=height, p3+=height, im_ptr+=nchannels) {
          switch(nchannels) {
          case 4:
            im_ptr[3] = (uint16_t)(*p3);
            // Rprintf("%i %i\n", im_ptr[3], (uint16_t)(*p3));
          case 3:
            im_ptr[2] = (uint16_t)(*p2);
          case 2:
            im_ptr[1] = (uint16_t)(*p1);
          case 1:
            im_ptr[0] = (uint16_t)(*p0);
          }
        }
      }
    } 
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Encode
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP res_ = PROTECT(write_png_core_(
    image, nbytes, width, height, file_,
    fmt,
    R_NilValue, // Palette
    use_filter_, compression_level_,
    TRUE,  // free_image_on_error
    16, // bit depth
    R_NilValue // trns
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
  size_t nbytes = (size_t)(Rf_length(arr_));
  
  enum spng_color_type fmt = SPNG_COLOR_TYPE_INDEXED;
  SEXP dims_ = Rf_getAttrib(arr_, R_DimSymbol);
  if (Rf_length(dims_) != 2) {
    Rf_error("write_png_indexed_(): Must be 2-D array");
  } 
  uint32_t width  = (uint32_t)INTEGER(dims_)[1];
  uint32_t height = (uint32_t)INTEGER(dims_)[0];
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Convert from column-major R matrix  to row major unsigned char
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  unsigned char *image = (unsigned char *)malloc(nbytes);
  if (image == NULL) {
    Rf_error("Could not allocate image buffer");
  }
  
  int npixels = (int)(width * height);
  unsigned char *im_ptr = image;
  
  if (Rf_isInteger(arr_)) {
    if (Rf_asLogical(avoid_transpose_)) {
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
  } else if (Rf_isReal(arr_)) {
    if (Rf_asLogical(avoid_transpose_)) {
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
    Rf_error("Index type not understood");
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // No tranposition, so swap width/height value
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (Rf_asLogical(avoid_transpose_)) {
    uint32_t tmp = height;
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
    TRUE,  // free_image_on_error
    8, // bit depth
    R_NilValue // trns
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
SEXP write_png_(SEXP image_, SEXP file_, SEXP palette_, SEXP use_filter_, 
                SEXP compression_level_, SEXP avoid_transpose_, SEXP bits_,
                SEXP trns_, SEXP raw_spec_) {
  
  if (!Rf_isNull(palette_)) {
    if (!Rf_isMatrix(image_)) {
      Rf_error("write_png(): When palette provided image must be a matrix.");
    }
    if (!Rf_isNumeric(image_)) {
      Rf_error("write_png(): When writing paletted PNG, image must be integer or numeric matrix with values in range [0, 255]");
    }
    if (Rf_length(palette_) > 256 || TYPEOF(palette_) != STRSXP) {
      Rf_error("Palette must be a character vector of hex colors. length <= 256 elements");
    }
    return write_png_indexed_(image_, file_, palette_, use_filter_, compression_level_, avoid_transpose_);
  } else if (Rf_inherits(image_, "nativeRaster")) {
    return write_png_from_nara_(image_, file_, use_filter_, compression_level_);
  } else if (Rf_inherits(image_, "raster") && TYPEOF(image_) == STRSXP) {
    if (Rf_length(image_) == 0) {
      Rf_error("Zero length rasters not valid");
    }
    if (Rf_isNull(trns_)) {
      return write_png_from_raster_(image_, file_, use_filter_, compression_level_);
    } else {
      return write_png_from_raster_rgb_(image_, file_, use_filter_, compression_level_, trns_);
    } 
  } else if ((Rf_isArray(image_) || Rf_isMatrix(image_)) && (Rf_isReal(image_) || Rf_isInteger(image_))) {
    if (Rf_asInteger(bits_) == 16) {
      return write_png_from_array16_(image_, file_, use_filter_, compression_level_, avoid_transpose_);
    } else {
      return write_png_from_array_(image_, file_, use_filter_, compression_level_, avoid_transpose_, trns_);
    }
  } else if (TYPEOF(image_) == RAWSXP) {
    return write_png_from_raw_vec_(image_, file_, use_filter_, compression_level_, trns_, 
                                   raw_spec_);
  }
  
  Rf_error("write_png(): R image container not understood");
  return R_NilValue;
}














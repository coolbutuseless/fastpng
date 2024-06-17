

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
#define hex2nibble(x) ( (((x) & 0xf) + ((x) >> 6) + ((x >> 6) << 3)) & 0xf )


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
    const char *filename = R_ExpandFileName(CHAR(STRING_ELT(file_, 0)));
    fp = fopen(filename, "wb");
    if (fp == NULL) {
      if (free_image_on_error) free(image);
      spng_ctx_free(ctx);
      error("Couldn't open file: %s", filename);
    }
    err = spng_set_png_file(ctx, fp); 
    if (err) {
      fclose(fp);
      if (free_image_on_error) free(image);
      spng_ctx_free(ctx);
      error("Couldn't set file for output: %s", filename);
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
  if (!isNull(trns_)) {
    struct spng_trns trns;
    int has_trns = 0;
    if (color_type == SPNG_COLOR_TYPE_GRAYSCALE) {
      // Rprintf("trns given and valid for image type GRAYSCALE\n");
      trns.gray = (uint16_t)(asInteger(trns_));
      has_trns = 1;
    } else if (color_type == SPNG_COLOR_TYPE_TRUECOLOR) {
      // Rprintf("trns given and valid for image type RGB\n");
      if (TYPEOF(trns_) == STRSXP) {
        // Rprintf("Hex color for trns\n");
        const char *col = CHAR(STRING_ELT(trns_, 0));
        if (col[0] == '#') {
          switch (strlen(col)) {
          case 9:
          case 7:
            trns.red   = (uint16_t)( (hex2nibble(col[1]) << 4) + hex2nibble(col[2]) ); // R
            trns.green = (uint16_t)( (hex2nibble(col[3]) << 4) + hex2nibble(col[4]) ); // G
            trns.blue  = (uint16_t)( (hex2nibble(col[5]) << 4) + hex2nibble(col[6]) ); // B
            break;
          case 5:
          case 4:
            trns.red   = (uint16_t)( hex2nibble(col[1]) * (16 + 1) ); // R
            trns.green = (uint16_t)( hex2nibble(col[2]) * (16 + 1) ); // G
            trns.blue  = (uint16_t)( hex2nibble(col[3]) * (16 + 1) ); // B
            break;
          default:
            error("TRNS colour not understood: '%s'", col);
          }
          has_trns = 1;
        } else {
          error("Character colours can only be hex strings, no '%s'", col);
        }
      } else if (isInteger(trns_) && length(trns_) == 3) {
        // Rprintf("Integer vector for trns\n");
        int *ptr = INTEGER(trns_);
        trns.red   = (uint16_t)ptr[0]; // R
        trns.green = (uint16_t)ptr[1]; // G
        trns.blue  = (uint16_t)ptr[2]; // B
        has_trns = 1;
      } else if (isReal(trns_) && length(trns_) == 3) {
        // Rprintf("Real vector for trns\n");
        double *ptr = REAL(trns_);
        trns.red   = (uint16_t)ptr[0]; // R
        trns.green = (uint16_t)ptr[1]; // G
        trns.blue  = (uint16_t)ptr[2]; // B
        has_trns = 1;
      } else {
        warning("Unknown argument for 'trns' %s\n", type2char((SEXPTYPE)TYPEOF(trns_)));
      }
    } else {
      // Rprintf("trns given and --NOT-- valid for image type\n");
    }
    
    // Set tRNS chunk
    if (has_trns) {
      err = spng_set_trns(ctx, &trns);
      if (err) {
        warning("spng_encode_image() TRNS error: %s\n", spng_strerror(err));
      }
    }
    
  }
  
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Write Palette:  
  //   tRNS CHunk for alpha channel
  //   PLTE Chunk for RGB palette
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (!isNull(palette_)) {
    
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
    
    plte.n_entries = (uint32_t)length(palette_); // length is checked prior to calling core func
    trns.n_type3_entries = (uint32_t)length(palette_); // always match palette length
    
    for (int i = 0; i < length(palette_); i++) {
      const char *col = CHAR(STRING_ELT(palette_, i));
      if (col[0] != '#') {
        error("Palettes may only contain hex colors of the form '#RRGGBB' or '#RRGGBBAA'");
      }
      switch (strlen(col)) {
      case 9:
        plte.entries[i].red   = (uint8_t)( (hex2nibble(col[1]) << 4) + hex2nibble(col[2]) ); // R
        plte.entries[i].green = (uint8_t)( (hex2nibble(col[3]) << 4) + hex2nibble(col[4]) ); // G
        plte.entries[i].blue  = (uint8_t)( (hex2nibble(col[5]) << 4) + hex2nibble(col[6]) ); // B
        trns.type3_alpha[i]   = (uint8_t)( (hex2nibble(col[7]) << 4) + hex2nibble(col[8]) ); // A
        break;
      case 7:
        plte.entries[i].red   = (uint8_t)( (hex2nibble(col[1]) << 4) + hex2nibble(col[2]) ); // R
        plte.entries[i].green = (uint8_t)( (hex2nibble(col[3]) << 4) + hex2nibble(col[4]) ); // G
        plte.entries[i].blue  = (uint8_t)( (hex2nibble(col[5]) << 4) + hex2nibble(col[6]) ); // B
        trns.type3_alpha[i]   = 255; // opaque
        break;
      case 4:
        plte.entries[i].red   = (uint8_t)( hex2nibble(col[1]) * (16 + 1)); // R
        plte.entries[i].green = (uint8_t)( hex2nibble(col[2]) * (16 + 1)); // G
        plte.entries[i].blue  = (uint8_t)( hex2nibble(col[3]) * (16 + 1)); // B
        trns.type3_alpha[i]   = (uint8_t)( hex2nibble(col[4]) * (16 + 1)); // A
        break;
      case 3:
        plte.entries[i].red   = (uint8_t)( hex2nibble(col[1]) * (16 + 1)); // R
        plte.entries[i].green = (uint8_t)( hex2nibble(col[2]) * (16 + 1)); // G
        plte.entries[i].blue  = (uint8_t)( hex2nibble(col[3]) * (16 + 1)); // B
        trns.type3_alpha[i]   = 255; // opaque
        break;
      default:
        error("Unknown hex colour '%s'", col);
      }
    }
    
    // Set PLTE chunk
    err = spng_set_plte(ctx, &plte);
    if (err) {
      if (fp) fclose(fp);
      if (free_image_on_error) free(image);
      spng_ctx_free(ctx);
      error("spng_encode_image() PLTE error: %s\n", spng_strerror(err));
    }
    
    // Set tRNS chunk
    err = spng_set_trns(ctx, &trns);
    if (err) {
      if (fp) fclose(fp);
      if (free_image_on_error) free(image);
      spng_ctx_free(ctx);
      error("spng_encode_image() TRNS error: %s\n", spng_strerror(err));
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
// Write PNG from raw data
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP write_png_from_raw_vec_(SEXP image_, SEXP file_, SEXP use_filter_, 
                             SEXP compression_level_, SEXP trns_, 
                             SEXP raw_spec_) {
  
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Unpack the 'raw_spec' list into variables
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (isNull(raw_spec_) || TYPEOF(raw_spec_) != VECSXP || length(raw_spec_) < 4) {
    error("'raw_spec' must be a named list with 4 elements");
  }
  
  uint32_t width  = 0;
  uint32_t height = 0;
  uint32_t depth  = 0;
  uint32_t bits   = 0;
  
  SEXP nms_ = getAttrib(raw_spec_, R_NamesSymbol);
  if (isNull(nms_) || length(nms_) != length(raw_spec_)) {
    error("'raw_spec' must be a named list with 4 elements.");
  }
  
  for (int i = 0; i < length(nms_); i++) {
    const char *nm = CHAR(STRING_ELT(nms_, i));
    if (strcmp(nm, "width") == 0) {
      width = (uint32_t)asInteger(VECTOR_ELT(raw_spec_, i));
    } else if (strcmp(nm, "height") == 0) {
      height = (uint32_t)asInteger(VECTOR_ELT(raw_spec_, i));
    } else if (strcmp(nm, "depth") == 0) {
      depth = (uint32_t)asInteger(VECTOR_ELT(raw_spec_, i));
    } else if (strcmp(nm, "bits") == 0) {
      bits = (uint32_t)asInteger(VECTOR_ELT(raw_spec_, i));
    }
  }
  
  if (width == 0 || height == 0 || depth == 0 || bits == 0) {
    error("'raw_spec' must contain 'width', 'height', 'depth', 'bits'");
  }
  
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Calculate number of bytes specified by the 'raw_spec'
  // And sanity check against the actual data provided
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  uint32_t size = height * width * depth;
  if (bits == 16) {
    size *= 2; 
  }
  
  if (size != length(image_)) {
    error("Mismatch between length of raw vector (%i) and raw_spec (%i x %i x %i x %i/8)", 
          length(image_), width, height, depth, bits);
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
    error("Depth not understood: %i", depth);
  }
  
  return write_png_core_(
    RAW(image_), (size_t)length(image_), width, height, file_,
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
  size_t nbytes = (size_t)(length(nara_) * 4.0);
  SEXP dims_ = getAttrib(nara_, R_DimSymbol);
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
      error("Valid rasters may only contain hex colors of the form '#RRGGBB' or '#RRGGBBAA'. \
Try removing any named R colors using 'normalize_colors()'");  
    }
    switch(strlen(col)) {
    case 9:
      *im_ptr++ = (unsigned char)( (hex2nibble(col[1]) << 4) + hex2nibble(col[2]) ); // R
      *im_ptr++ = (unsigned char)( (hex2nibble(col[3]) << 4) + hex2nibble(col[4]) ); // G
      *im_ptr++ = (unsigned char)( (hex2nibble(col[5]) << 4) + hex2nibble(col[6]) ); // B
      *im_ptr++ = (unsigned char)( (hex2nibble(col[7]) << 4) + hex2nibble(col[8]) ); // A
      break;
    case 7:
      *im_ptr++ = (unsigned char)( (hex2nibble(col[1]) << 4) + hex2nibble(col[2]) ); // R
      *im_ptr++ = (unsigned char)( (hex2nibble(col[3]) << 4) + hex2nibble(col[4]) ); // G
      *im_ptr++ = (unsigned char)( (hex2nibble(col[5]) << 4) + hex2nibble(col[6]) ); // B
      *im_ptr++ = 255; // A
      // Rprintf("%02X %02X %02X %02X\n", *(im_ptr - 4), *(im_ptr - 3), *(im_ptr - 2), *(im_ptr - 1));
      break;
    case 5:
      *im_ptr++ = (unsigned char)( hex2nibble(col[1]) * (16 + 1) ); // R
      *im_ptr++ = (unsigned char)( hex2nibble(col[2]) * (16 + 1) ); // G
      *im_ptr++ = (unsigned char)( hex2nibble(col[3]) * (16 + 1) ); // B
      *im_ptr++ = (unsigned char)( hex2nibble(col[4]) * (16 + 1) ); // A
      break;
    case 4:
      *im_ptr++ = (unsigned char)( hex2nibble(col[1]) * (16 + 1) ); // R
      *im_ptr++ = (unsigned char)( hex2nibble(col[2]) * (16 + 1) ); // G
      *im_ptr++ = (unsigned char)( hex2nibble(col[3]) * (16 + 1) ); // B
      *im_ptr++ = 255; // A
      break;
    default:
      error("hex colour in raster not understood: '%s'", col);
    }
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
      error("Valid rasters may only contain hex colors of the form '#RRGGBB' or '#RRGGBBAA'. \
Try removing any named R colors using 'normalize_colors()'");  
    }
    switch(strlen(col)) {
    case 9:
    case 7:
      *im_ptr++ = (unsigned char)( (hex2nibble(col[1]) << 4) + hex2nibble(col[2]) ); // R
      *im_ptr++ = (unsigned char)( (hex2nibble(col[3]) << 4) + hex2nibble(col[4]) ); // G
      *im_ptr++ = (unsigned char)( (hex2nibble(col[5]) << 4) + hex2nibble(col[6]) ); // B
      break;
    case 5:
    case 4:
      *im_ptr++ = (unsigned char)( hex2nibble(col[1]) * (16 + 1) ); // R
      *im_ptr++ = (unsigned char)( hex2nibble(col[2]) * (16 + 1) ); // G
      *im_ptr++ = (unsigned char)( hex2nibble(col[3]) * (16 + 1) ); // B
      break;
    default:
      error("write_png_from_raster_rgb_(): hex colour not understood: '%s'", col);
    }
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
  size_t nbytes = (size_t)(length(arr_));
  
  enum spng_color_type fmt;
  int nchannels;
  SEXP dims_ = getAttrib(arr_, R_DimSymbol);
  if (length(dims_) == 2) {
    fmt = SPNG_COLOR_TYPE_GRAYSCALE;
    nchannels = 1;
    // SPNG_COLOR_TYPE_TRUECOLOR_ALPHA
    // error("Must be 3d array");
  } else if (length(dims_) == 3) {
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
  unsigned char *im_ptr = image;
  
  if (isReal(arr_)) {
    double *arr_ptr = REAL(arr_);
    if (fmt == SPNG_COLOR_TYPE_GRAYSCALE && asLogical(avoid_transpose_)) {
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
  } else if (isInteger(arr_)) {
    int32_t *arr_ptr = INTEGER(arr_);
    if (fmt == SPNG_COLOR_TYPE_GRAYSCALE && asLogical(avoid_transpose_)) {
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
  
  // Rprintf("write_png_from_array16_()  %i\n", isReal(arr_));
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Options
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  size_t nbytes = (size_t)(2.0 * length(arr_));
  
  enum spng_color_type fmt;
  SEXP dims_ = getAttrib(arr_, R_DimSymbol);
  int nchannels;
  if (length(dims_) == 2) {
    fmt = SPNG_COLOR_TYPE_GRAYSCALE;
    nchannels = 1;
    // SPNG_COLOR_TYPE_TRUECOLOR_ALPHA
    // error("Must be 3d array");
  } else if (length(dims_) == 3) {
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
  uint16_t *image = (uint16_t *)malloc(nbytes);
  if (image == NULL) {
    error("Could not allocate image buffer");
  }
  
  int npixels = (int)(width * height);
  uint16_t *im_ptr = image;
  
  if (isReal(arr_)) {
    double *arr_ptr = REAL(arr_);
    if (fmt == SPNG_COLOR_TYPE_GRAYSCALE && asLogical(avoid_transpose_)) {
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
  } else if (isInteger(arr_)) {
    int32_t *arr_ptr = INTEGER(arr_);
    if (fmt == SPNG_COLOR_TYPE_GRAYSCALE && asLogical(avoid_transpose_)) {
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
  
  if (!isNull(palette_)) {
    if (!isMatrix(image_)) {
      error("write_png(): When palette provided image must be a matrix.");
    }
    if (!isNumeric(image_)) {
      error("write_png(): When writing paletted PNG, image must be integer or numeric matrix with values in range [0, 255]");
    }
    if (length(palette_) > 256 || TYPEOF(palette_) != STRSXP) {
      error("Palette must be a character vector of hex colors. length <= 256 elements");
    }
    return write_png_indexed_(image_, file_, palette_, use_filter_, compression_level_, avoid_transpose_);
  } else if (inherits(image_, "nativeRaster")) {
    return write_png_from_nara_(image_, file_, use_filter_, compression_level_);
  } else if (inherits(image_, "raster") && TYPEOF(image_) == STRSXP) {
    if (length(image_) == 0) {
      error("Zero length rasters not valid");
    }
    if (isNull(trns_)) {
      return write_png_from_raster_(image_, file_, use_filter_, compression_level_);
    } else {
      return write_png_from_raster_rgb_(image_, file_, use_filter_, compression_level_, trns_);
    } 
  } else if ((isArray(image_) || isMatrix(image_)) && (isReal(image_) || isInteger(image_))) {
    if (asInteger(bits_) == 16) {
      return write_png_from_array16_(image_, file_, use_filter_, compression_level_, avoid_transpose_);
    } else {
      return write_png_from_array_(image_, file_, use_filter_, compression_level_, avoid_transpose_, trns_);
    }
  } else if (TYPEOF(image_) == RAWSXP) {
    return write_png_from_raw_vec_(image_, file_, use_filter_, compression_level_, trns_, 
                                   raw_spec_);
  }
  
  error("write_png(): R image container not understood");
  return R_NilValue;
}














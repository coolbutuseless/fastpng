

#include <R.h>
#include <Rinternals.h>
#include <Rdefines.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>



#include "spng.h"


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Extract PNG header information
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP get_png_info_(SEXP src_) {

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
  FILE *fp = NULL;
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
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // get info
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  struct spng_ihdr ihdr;
  int err = spng_get_ihdr(ctx, &ihdr);
  if (err) {
    if (fp) fclose(fp);
    spng_ctx_free(ctx);
    error("spng_get_ihdr() error: %s\n", spng_strerror(err));
  }
  if (fp) fclose(fp);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create an R list
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP res_ = PROTECT(allocVector(VECSXP, 7));
  SET_VECTOR_ELT(res_, 0, ScalarInteger(ihdr.width));
  SET_VECTOR_ELT(res_, 1, ScalarInteger(ihdr.height));
  SET_VECTOR_ELT(res_, 2, ScalarInteger(ihdr.bit_depth));
  SET_VECTOR_ELT(res_, 3, ScalarInteger(ihdr.color_type));
  SET_VECTOR_ELT(res_, 4, ScalarInteger(ihdr.compression_method));
  SET_VECTOR_ELT(res_, 5, ScalarInteger(ihdr.filter_method));
  SET_VECTOR_ELT(res_, 6, ScalarInteger(ihdr.interlace_method));
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set the names on the list.
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP names = PROTECT(allocVector(STRSXP, 7));
  SET_STRING_ELT(names, 0, mkChar("width"));
  SET_STRING_ELT(names, 1, mkChar("height"));
  SET_STRING_ELT(names, 2, mkChar("bit_depth"));
  SET_STRING_ELT(names, 3, mkChar("color_type"));
  SET_STRING_ELT(names, 4, mkChar("compression_method"));
  SET_STRING_ELT(names, 5, mkChar("filter_method"));
  SET_STRING_ELT(names, 6, mkChar("interlace_method"));
  setAttrib(res_, R_NamesSymbol, names);

  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // tidy and return 
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  spng_ctx_free(ctx);
  UNPROTECT(2);
  return res_;
}











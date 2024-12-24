
#define R_NO_REMAP

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
  size_t buf_size = 0;
  unsigned char *buf = 0;
  FILE *fp = NULL;
  if (TYPEOF(src_) == RAWSXP) {
    buf_size = (size_t)Rf_length(src_);
    buf = (unsigned char *)RAW(src_);
    spng_set_png_buffer(ctx, buf, buf_size);
  } else if (TYPEOF(src_) == STRSXP) {
    const char *filename = R_ExpandFileName(CHAR(STRING_ELT(src_, 0)));
    fp = fopen(filename, "rb");
    if (fp == NULL) {
      spng_ctx_free(ctx);
      Rf_error("read_png_core(): Couldn't open file '%s'", filename);
    }
    
    int err = spng_set_png_file(ctx, fp); 
    if (err) {
      fclose(fp);
      spng_ctx_free(ctx);
      Rf_error("read_png_core(): Couldn't set file for input: %s", filename);
    }
    
  } else {
    spng_ctx_free(ctx);
    Rf_error("read_png_core(): Data source not handled");
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // get info
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  struct spng_ihdr ihdr;
  int err = spng_get_ihdr(ctx, &ihdr);
  if (err) {
    if (fp) fclose(fp);
    spng_ctx_free(ctx);
    Rf_error("spng_get_ihdr() error: %s\n", spng_strerror(err));
  }
  if (fp) fclose(fp);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create an R list
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP res_ = PROTECT(Rf_allocVector(VECSXP, 7));
  SET_VECTOR_ELT(res_, 0, Rf_ScalarInteger((int)ihdr.width));
  SET_VECTOR_ELT(res_, 1, Rf_ScalarInteger((int)ihdr.height));
  SET_VECTOR_ELT(res_, 2, Rf_ScalarInteger(ihdr.bit_depth));
  SET_VECTOR_ELT(res_, 3, Rf_ScalarInteger(ihdr.color_type));
  SET_VECTOR_ELT(res_, 4, Rf_ScalarInteger(ihdr.compression_method));
  SET_VECTOR_ELT(res_, 5, Rf_ScalarInteger(ihdr.filter_method));
  SET_VECTOR_ELT(res_, 6, Rf_ScalarInteger(ihdr.interlace_method));
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set the names on the list.
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP names = PROTECT(Rf_allocVector(STRSXP, 7));
  SET_STRING_ELT(names, 0, Rf_mkChar("width"));
  SET_STRING_ELT(names, 1, Rf_mkChar("height"));
  SET_STRING_ELT(names, 2, Rf_mkChar("bit_depth"));
  SET_STRING_ELT(names, 3, Rf_mkChar("color_type"));
  SET_STRING_ELT(names, 4, Rf_mkChar("compression_method"));
  SET_STRING_ELT(names, 5, Rf_mkChar("filter_method"));
  SET_STRING_ELT(names, 6, Rf_mkChar("interlace_method"));
  Rf_setAttrib(res_, R_NamesSymbol, names);

  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // tidy and return 
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  spng_ctx_free(ctx);
  UNPROTECT(2);
  return res_;
}











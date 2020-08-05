

#include <R.h>
#include <Rinternals.h>
#include <Rdefines.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//#include "R-finalizers.h"


#include "spng.h"



SEXP png_info_(SEXP raw_vec_) {

  size_t out_size;
  int buf_size = length(raw_vec_);

  unsigned char *buf = (unsigned char *)RAW(raw_vec_);

  /* Create a context */
  spng_ctx *ctx = spng_ctx_new(0);


  /* Set memory usage limits for storing standard and unknown chunks,
   this is important when reading arbitrary files! */
  size_t limit = 1024 * 1024 * 64;
  spng_set_chunk_limits(ctx, limit, limit);

  /* Set an input buffer */
  spng_set_png_buffer(ctx, buf, buf_size);


  // get info
  struct spng_ihdr ihdr;
  int r = spng_get_ihdr(ctx, &ihdr);
  if (r) {
    error("spng_get_ihdr() error: %s\n", spng_strerror(r));
  }

  struct spng_ihdr *ptr = &ihdr;

  if (ptr == NULL) error("spng_ihdr pointer is invalid/NULL");
  // Final list
  SEXP result_ = PROTECT(NEW_LIST(7));
  SET_VECTOR_ELT(result_, 0, ScalarInteger(ptr->width));
  SET_VECTOR_ELT(result_, 1, ScalarInteger(ptr->height));
  SET_VECTOR_ELT(result_, 2, ScalarInteger(ptr->bit_depth));
  SET_VECTOR_ELT(result_, 3, ScalarInteger(ptr->color_type));
  SET_VECTOR_ELT(result_, 4, ScalarInteger(ptr->compression_method));
  SET_VECTOR_ELT(result_, 5, ScalarInteger(ptr->filter_method));
  SET_VECTOR_ELT(result_, 6, ScalarInteger(ptr->interlace_method));

  // Set the names on the list.
  SEXP names = PROTECT(allocVector(STRSXP, 7));
  SET_STRING_ELT(names, 0, mkChar("width"));
  SET_STRING_ELT(names, 1, mkChar("height"));
  SET_STRING_ELT(names, 2, mkChar("bit_depth"));
  SET_STRING_ELT(names, 3, mkChar("color_type"));
  SET_STRING_ELT(names, 4, mkChar("compression_method"));
  SET_STRING_ELT(names, 5, mkChar("filter_method"));
  SET_STRING_ELT(names, 6, mkChar("interlace_method"));
  setAttrib(result_, R_NamesSymbol, names);


  /* Free context memory */
  spng_ctx_free(ctx);


  UNPROTECT(2);
  return result_;
}











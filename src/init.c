
// #define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>

extern SEXP read_png_(SEXP src_, SEXP type_, SEXP flags_,SEXP rgba_, SEXP avoid_transpose_, SEXP array_type_);
extern SEXP write_png_(SEXP image_ , SEXP file_, SEXP palette_, SEXP use_filter_, 
                       SEXP compression_level_, SEXP avoid_transpose_, SEXP bits_, SEXP trns_,
                       SEXP raw_spec_);
extern SEXP get_png_info_(SEXP src_);


static const R_CallMethodDef CEntries[] = {
  {"read_png_"   , (DL_FUNC) &read_png_ , 6},
  {"write_png_"  , (DL_FUNC) &write_png_, 9},

  {"get_png_info_", (DL_FUNC) &get_png_info_, 1},
  {NULL , NULL, 0}
};


void R_init_fastpng(DllInfo *info) {
  R_registerRoutines(
    info,      // DllInfo
    NULL,      // .C
    CEntries,  // .Call
    NULL,      // Fortran
    NULL       // External
  );
  R_useDynamicSymbols(info, FALSE);
}




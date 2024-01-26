
// #define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>

extern SEXP read_png_raw_(SEXP raw_vec_, SEXP fmt_, SEXP flags_);
extern SEXP read_png_nara_(SEXP raw_vec_, SEXP flags_);
extern SEXP extract_png_info_(SEXP raw_vec_);

static const R_CallMethodDef CEntries[] = {
  
  {"read_png_raw_"   , (DL_FUNC) &read_png_raw_   , 3},
  {"read_png_nara_"    , (DL_FUNC) &read_png_nara_    , 2},
  {"extract_png_info_", (DL_FUNC) &extract_png_info_, 1},
  {NULL , NULL, 0}
};


void R_init_spng(DllInfo *info) {
  R_registerRoutines(
    info,      // DllInfo
    NULL,      // .C
    CEntries,  // .Call
    NULL,      // Fortran
    NULL       // External
  );
  R_useDynamicSymbols(info, FALSE);
}





// #define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>

extern SEXP depng_(SEXP raw_vec_, SEXP fmt_, SEXP flags_);
extern SEXP png_info_(SEXP raw_vec_);

static const R_CallMethodDef CEntries[] = {

  {"depng_", (DL_FUNC) &depng_, 3},
  {"png_info_", (DL_FUNC) &png_info_, 1},
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




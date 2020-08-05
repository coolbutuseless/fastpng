
// #define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>

extern SEXP depng_();

static const R_CallMethodDef CEntries[] = {

  {"depng_", (DL_FUNC) &depng_, 1},

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




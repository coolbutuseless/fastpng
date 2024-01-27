
// #define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>

extern SEXP read_png_as_raw_   (SEXP raw_vec_, SEXP fmt_, SEXP flags_);
extern SEXP read_png_as_nara_  (SEXP raw_vec_, SEXP flags_);
extern SEXP read_png_as_raster_(SEXP raw_vec_, SEXP flags_);
extern SEXP read_png_as_rgba_  (SEXP raw_vec_, SEXP flags_);
extern SEXP read_png_as_rgb_   (SEXP raw_vec_, SEXP flags_);

extern SEXP write_png_from_raw_(SEXP raw_vec_, SEXP width_, SEXP height_, SEXP use_filter_, SEXP compression_level_);

extern SEXP extract_png_info_(SEXP raw_vec_);

static const R_CallMethodDef CEntries[] = {
  
  {"read_png_as_raw_"   , (DL_FUNC) &read_png_as_raw_   , 3},
  {"read_png_as_nara_"  , (DL_FUNC) &read_png_as_nara_  , 2},
  {"read_png_as_raster_", (DL_FUNC) &read_png_as_raster_, 2},
  {"read_png_as_rgba_"  , (DL_FUNC) &read_png_as_rgba_  , 2},
  {"read_png_as_rgb_"   , (DL_FUNC) &read_png_as_rgb_   , 2},
  
  {"write_png_from_raw_", (DL_FUNC) &write_png_from_raw_, 5},
  
  
  
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




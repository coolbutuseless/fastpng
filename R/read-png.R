#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Read a PNG 
#'
#' @param src PNG image provided as either a file path, or a raw vector 
#'        containing encoded PNG data
#' @param type type of R object in which to store image data. Valid types are
#'        'array', 'raster', 'native_raster', 'indexed' and 'raw'.  Note that indexed
#'        image objects can only be loaded from indexed PNGs.
#' @param rgba Should the result be forced into RGBA?  Default: FALSE  
#'        means to use the most appropriate format of the given R image type 
#'        to store the data.  If \code{TRUE}, then the image will be forced
#'        into RGBA colour mode.
#' @param flags Flags to apply when reading PNG. Default: 1 (always decode transparency from tRNS chunks). 
#'        See \code{?spng_decode_flags} for other options.  Must be an integer.
#' @param avoid_transpose Default: FALSE. If \code{TRUE}, then transposing the image
#'        from row-major (in the PNG), into column-major (in R) will be avoided
#'        if possible.  This option only applies when reading grayscale or
#'        indexed images.  Since the transposition is avoided, the decode 
#'        step can be faster, but the image will not be in the correct orientation.
#' @param array_type 'dbl' or 'int'. Default: dbl.  When reading PNG into an array,
#'        should the data be stored as a double (i.e. real) in the range [0, 1] 
#'        or an integer in the range [0,255] (for 8 bit images) or 
#'        [0,65535] (for 16 bit images).
#'
#' @return R image object of the specified type
#'
#' @export
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
read_png <- function(src, type = c('array', 'raster', 'native_raster', 'indexed', 'raw'), 
                     rgba = FALSE, flags = 1L, avoid_transpose = FALSE,
                     array_type = c('dbl', 'int')) {
  .Call(read_png_, src, match.arg(type), rgba, flags, avoid_transpose, array_type)
}

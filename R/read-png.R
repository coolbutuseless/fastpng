#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Read a PNG from a raw vector in memory
#'
#' See \url{https://libspng.org/docs/decode/} for more information on which
#' combinations of formats and flags are permissable.
#'
#' @param src raw vector containing encoded PNG data or file path
#' @param type type of object in which to store image data. Valid types are
#'        'array', 'raster', 'native_raster', 'indexed' and 'raw'.  Note that indexed
#'        image objects can only be loaded from indexed PNGs.
#' @param rgba Should the result be forced into RGBA?  Default: FALSE  
#'        Example 1: When reading with \code{type = 'native_raster'} the result
#'        will always be RGBA, so this variable is ignored.
#'        Example 2: When reading a grey+alpha image, if \code{rgba = TRUE}, 
#'        then image will be read into an RGBA array with 4 planes.  Otherwise
#'        it will be read in as grey+alpha data in an array with 2 planes.
#' @param flags flags. default: 1 (always decode transparency from tRNS chunks). 
#'        See \code{?spng_decode_flags} for other options.  Must be an integer.
#' @param avoid_transpose default: FALSE.
#'
#' @return R image object of the specified type
#'
#' @export
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
read_png <- function(src, type = c('array', 'raster', 'native_raster', 'indexed', 'raw'), 
                     rgba = FALSE, flags = 1L, 
                     avoid_transpose = FALSE) {
  .Call(read_png_, src, match.arg(type), rgba, flags, avoid_transpose)
}

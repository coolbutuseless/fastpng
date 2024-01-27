
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Read a PNG from a raw vector in memory
#'
#' See \url{https://libspng.org/docs/decode/} for more information on which
#' combinations of formats and flags are permissable.
#'
#' @param src raw vector containing encoded PNG data or file path
#' @param type type of object in which to store image data
#' @param flags flags. default: 0. See \code{?spng_decode_flags} for other
#'        options.  Must be an integer.
#'
#' @return R image object of the specified type
#'
#' @export
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
read_png <- function(src, type = c('nara', 'raster', 'rgba', 'rgb'), flags = 0L) {
  .Call(read_png_, src, match.arg(type), flags)
}
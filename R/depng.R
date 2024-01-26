

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Decompress a PNG in memory
#'
#' See \url{https://libspng.org/docs/decode/} for more information on which
#' combinations of formats and flags are permissable.
#'
#' @param raw_vec raw vector containing encoded PNG data
#' @param fmt what format the decoded bytes should be in. default: \code{SPNG_FMT_RGBA8}.
#'        See \code{?spng_format} for other formats.
#' @param flags flags. default: 0. See \code{?spng_decode_flags} for other
#'        options.  Must be an integer.
#'
#' @return vector of raw bytes representing pixel data in the requested format.
#'         Note that the pixels will be in row-major format
#'
#' @export
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
read_png_raw <- function(raw_vec, fmt = spng_format$SPNG_FMT_RGBA8, flags = 0) {
  .Call(read_png_raw_, raw_vec, as.integer(fmt), as.integer(flags))
}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Write PNG
#' 
#' @param image image data.  raster, rgba array, rgb array, nativeraster object,
#'        2D grayscale matrix
#' @param file If NULL then return result as raw vector, otherwise write
#'        to the given filepath.
#' @param use_filter Use PNG filtering to help reduce size. Default: TRUE.
#'        If FALSE, then filtering will be disabled which can make 
#'        image writing faster.
#' @param compression_level compression level for zlib.  Default: -1 means
#'        to use the default compression level.  Other valid
#'        values are in range [0, 9].  In general, lower compression levels
#'        result in faster compression, but larger image sizes.  For fastest
#'        image writing, set \code{compression_level}
#'        to 0 to completely disable compression.
#' @param avoid_traanspose Should transposition be avoided if possible so as to 
#'        maximise the speed of writing the PNG?  Default: FALSE.  
#'        PNG is a row-major image format, where R stored data in column-major
#'        ordering.  When writing data to PNG, it is often necessary to transpose
#'        the R data to match what PNG requires.  If this option is set 
#'        to \code{TRUE} then the image is written without this transposition and 
#'        should speed up PNG creation.  Currently this option is only
#'        used when writing greyscale PNGs from 2D matrix data.
#' @param palette character vector of up to 256 colours in RGB hex format
#'        i.e. \code{#RRGGBB}
#' @param bits bit depth. default 8.  Valid values are 8 and 16.  This option
#'        only has an effect when image to output is a numeric array.
#' 
#' @export
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
write_png <- function(image, file = NULL, palette = NULL, use_filter = TRUE, 
                      compression_level = -1L, avoid_traanspose = FALSE, bits=8) {
  .Call(write_png_, image, file, palette, use_filter, compression_level, avoid_traanspose, bits)
}

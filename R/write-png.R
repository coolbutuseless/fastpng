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
#'        used when reading/writing greyscale PNGs with 2D matrix data.
#' @param palette character vector of up to 256 colours in RGB hex format
#'        i.e. \code{#RRGGBB}
#' @param bits bit depth. default 8.  Valid values are 8 and 16.  This option
#'        only has an effect when image to output is a numeric array.
#' @param trns Colour to be treated as transparent
#'        in RGB and Greyscale images - without specifying a full alpha channel.  
#'        Only a single colour can be specified and it will be treated as a 
#'        fully transparent colour in the image.  This setting is only used 
#'        when writing RGB and Greyscale images.  For 8-bit RGB images, the value
#'        may be a hex colour value i.e. \code{"#RRGGBB"} or a vector of 3 numeric
#'        values in the range [0, 255].  For 8-bit greyscale images,
#'        must be a single integer value in the range [0, 255].
#'        For 16-bit RGB images, the value
#'        may be a vector of 3 numeric
#'        values in the range [0, 65535].  For 16-bit greyscale images,
#'        must be a single integer value in the range [0, 65535].
#'        Default: NULL - means to not add a transparency colour. 
#' @param raw_spec list of image specifications for encoding a raw vector
#'        to PNG. This list must contain the following elements in this order:
#'        width, height, nchannels, bits e.g. \code{raw_spec = list(400, 300, 4, 8)}
#' 
#' @export
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
write_png <- function(image, file = NULL, palette = NULL, use_filter = TRUE, 
                      compression_level = -1L, avoid_traanspose = FALSE, bits=8, 
                      trns = NULL, raw_spec = NULL) {
  .Call(write_png_, image, file, palette, use_filter, compression_level, 
        avoid_traanspose, bits, trns, raw_spec)
}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Write PNG
#' 
#' @param image image.  Supported image types:  
#' \describe{
#'   \item{Numeric arrays}{Numeric arrays with values in the range [0, 1], with
#'   1, 2, 3 or 4 colour planes to represent gray, gray+alpha, rgb
#'   and rgba pixels, respectively}
#'   \item{Rasters}{Rasters with a mixture of named colours (e.g. 'red'),
#'   and hex colours of the form #RGB, #RGBA, #RRGGBB and #RRGGBBAA}
#'   \item{Integer arrays}{Integer arrays with values in [0,255] treated as
#'   8-bit image data.  Integer arrays with values in [0, 65535] treated as
#'   16-bit image data}
#'   \item{Native rasters}{Integer matrices containing colurs in native format
#'   i.e. 8-bit RGBA values packed into a single integer}
#'   \item{Integer matrix + an indexed palette of colors}{Can be saved as 
#'   an indexed PNG}
#'   \item{Raw vectors}{Vectors of raw bytes must be accompanied by a 
#'   \code{raw_spec} which details how the bytes are to be interpreted
#'   e.g. colour depth, width and height}
#' }
#' @param file If NULL (the default) then return PNG data as raw vector, otherwise write
#'        to the given file path.
#' @param use_filter Use PNG filtering to help reduce size? Default: TRUE.
#'        If FALSE, then filtering will be disabled which can make 
#'        image writing faster.
#' @param compression_level compression level for PNG. Default: -1 means
#'        to use the default compression level.  Other valid
#'        values are in range [0, 9].  In general, lower compression levels
#'        result in faster compression, but larger image sizes.  For fastest
#'        image writing, set \code{compression_level}
#'        to 0 to completely disable compression.
#' @param avoid_transpose Should transposition be avoided if possible so as to 
#'        maximise the speed of writing the PNG?  Default: FALSE.  
#'        PNG is a row-major image format, but R stores data in column-major
#'        ordering.  When writing data to PNG, it is often necessary to transpose
#'        the R data to match what PNG requires.  If this option is set 
#'        to \code{TRUE} then the image is written without this transposition and 
#'        should speed up PNG creation.  This option only has an effect
#'        for 2D integer and numeric matrix formats.
#' @param palette character vector of up to 256 colors. If specified, and the
#'        image is a 2D matrix of integer or numeric values, then an indexed 
#'        PNG is written where the matrix values indicate the colour palette
#'        value to use. The values in the matrix must range from 0 (for the 
#'        first colour)
#' @param bits bit depth. default 8.  Valid values are 8 and 16.  This option
#'        only has an effect when image to output is a numeric array.
#' @param trns color to be treated as transparent
#'        in RGB and Greyscale images - without specifying a full alpha channel.  
#'        Only a single color can be specified and it will be treated as a 
#'        fully transparent color in the image.  This setting is only used 
#'        when writing RGB and Greyscale images.  For 8-bit RGB images, the value
#'        may be a hex color value i.e. \code{"#RRGGBB"} or a vector of 3 numeric
#'        values in the range [0, 255].  For 8-bit greyscale images,
#'        must be a single integer value in the range [0, 255].
#'        For 16-bit RGB images, the value
#'        may be a vector of 3 numeric
#'        values in the range [0, 65535].  For 16-bit greyscale images,
#'        must be a single integer value in the range [0, 65535].
#'        Default: NULL - means to not add a transparency color. 
#' @param raw_spec named list of image specifications for encoding a raw vector
#'        to PNG. Use \code{raw_spec()} to create such a list in the correct format.
#'        This argument is only required if the \code{image} argument is a 
#'        raw vector.
#' 
#' @return If \code{file} argument provided, function writes to file and returns 
#'         nothing, otherwise it returns a raw vector holding the PNG
#'         encoded data.
#'         
#' @examples
#' # create a small greyscale integer matrix, and write it to a PNG file
#' mat <- matrix(c(0L, 255L), 3, 4)
#' pngfile <- tempfile()
#' write_png(mat, file = pngfile)
#' im <- read_png(pngfile, type = 'raster') 
#' plot(im, interpolate = FALSE)
#' @export
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
write_png <- function(image, file = NULL, palette = NULL, use_filter = TRUE, 
                      compression_level = -1L, avoid_transpose = FALSE, bits=8, 
                      trns = NULL, raw_spec = NULL) {
  res <- .Call(write_png_, image, file, palette, use_filter, compression_level, 
        avoid_transpose, bits, trns, raw_spec)
  if (is.null(file)) {
    res
  } else {
    invisible(file)
  }
}


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Create a specification for how raw bytes should be interpreted when passed
#' to \code{write_png()}
#' 
#' @param width,height image dimensions
#' @param depth number of colour channels. Integer value in range [1, 4]
#' @param bits number of bits for each colour channel. Either 8 or 16.
#' 
#' @return named list to pass to the \code{write_png(..., raw_spec = )}
#' @examples
#' raw_spec(100, 20, 3, 8)
#' @export
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
raw_spec <- function(width, height, depth, bits) {
  list(
    width  = width,
    height = height,
    depth  = depth,
    bits   = bits
  )  
}



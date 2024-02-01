
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
#' @param avoid_transpose default: FALSE.
#'
#' @return R image object of the specified type
#'
#' @export
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
read_png <- function(src, type = c('nara', 'raster', 'rgba', 'rgb', 'gray'), flags = 0L, 
                     avoid_transpose = FALSE) {
  .Call(read_png_, src, match.arg(type), flags, avoid_transpose)
}



if (FALSE) {
  library(png)
  library(grid)
  
  png_file <- system.file("img", "Rlogo.png", package="png")
  png_data <- readBin(png_file, 'raw', n = file.size(png_file))
  
  nara <- read_png(png_data, type = 'nara')
  # ras  <- read_png(png_data, type = 'raster')
  # rgba <- read_png(png_data, type = 'rgba')
  # rgb  <- read_png(png_data, type = 'rgb')
  # 
  nara2 <- read_png(png_file, type = 'nara')
  # ras2  <- read_png(png_file, type = 'raster')
  # rgba2 <- read_png(png_file, type = 'rgba')
  # rgb2  <- read_png(png_file, type = 'rgb')
  # 
  # identical(nara, nara2)
  # identical(ras, ras2)
  # identical(rgba, rgba2)
  # identical(rgb, rgb2)
    
  
  
  bench::mark(
    read_png(png_file, type = 'nara'),
    read_png(png_file, type = 'raster'),
    read_png(png_file, type = 'rgba'),
    read_png(png_file, type = 'rgb'),
    readPNG(png_file),
    readPNG(png_file, native = TRUE),
    check = FALSE
  )[, c(1, 4)]
   
  bench::mark( 
    read_png(png_data, type = 'nara'),
    read_png(png_data, type = 'raster'),
    read_png(png_data, type = 'rgba'),
    read_png(png_data, type = 'rgb'),
    readPNG(png_data),
    readPNG(png_data, native = TRUE),
    check = FALSE
  )[, c(1, 4)]
  
}

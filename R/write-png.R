

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Write PNG
#' 
#' @param image image data.  raster, rgba array, rgb array, nativeraster object
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
#' 
#' @export
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
write_png <- function(image, file = NULL, use_filter = TRUE, 
                      compression_level = -1L) {
  .Call(write_png_, image, file, use_filter, compression_level)
}





if (FALSE) {
  library(png)
  library(grid)
  
  png_file <- system.file("img", "Rlogo.png", package="png")
  png_data <- readBin(png_file, 'raw', n = file.size(png_file))
  
  nara <- read_png(png_data, type = 'nara')
  ras  <- read_png(png_data, type = 'raster')
  rgba <- read_png(png_data, type = 'rgba')
  rgb  <- read_png(png_data, type = 'rgb')
  
  
  
  bench::mark(
    write_png(nara),
    write_png(nara, compression_level = 0, use_filter = FALSE),
    write_png(ras),
    write_png(ras, compression_level = 0, use_filter = FALSE),
    writePNG(rgba),
    check = FALSE
  )[, c(1, 4)]
  
}

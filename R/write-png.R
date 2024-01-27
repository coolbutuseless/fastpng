

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Write PNG
#' 
#' @param raw_vec raw vector containing RGBA bytes
#' @param width,height dimensions of the image
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
write_png_from_raw <- function(raw_vec, width, height, use_filter = TRUE, 
                               compression_level = -1L) {
  .Call(write_png_from_raw_, raw_vec, width, height, use_filter, compression_level);
}


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Write PNG from NativeRaster
#' 
#' @inheritParams write_png_from_raw
#' @param nara native raster object. I.e.  An integer matrix.
#' @param file If NULL then return result as raw vector, otherwise write
#'        to the given filepath.
#' @export
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
write_png_from_nara <- function(nara, file = NULL, use_filter = TRUE, compression_level = -1L) {
  .Call(write_png_from_nara_, nara, file, use_filter, compression_level);
}


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Write PNG from Raster (character matrix of hex colours)
#' 
#' @inheritParams write_png_from_raw
#' @inheritParams write_png_from_nara
#' @param ras raster object. I.e.  A character matrix.
#' 
#' @export
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
write_png_from_raster <- function(ras, file = NULL, use_filter = TRUE, compression_level = -1L) {
  .Call(write_png_from_raster_, ras, file, use_filter, compression_level);
}



if (FALSE) {
  library(png)
  library(grid)
  
  png_file <- system.file("img", "Rlogo.png", package="png")
  png_data <- readBin(png_file, 'raw', n = file.size(png_file))
  
  nara <- read_png_as_nara(png_data)
  ras  <- read_png_as_raster(png_data)
  rgba <- read_png_as_rgba(png_data)
  
  bench::mark(
    write_png_from_nara(nara, compression_level = 0),
    write_png_from_raster(ras, compression_level = 0),
    writePNG(rgba),
    check = FALSE
  )
  
  
  write_png_from_raster(ras, "working/ras.png")
  
  
  grid.newpage()
  grid.raster(im, interpolate = FALSE)
  
  
  png2 <- write_png_from_nara(im) 
  
  im2 <- read_png_as_nara(png2)
  grid.newpage()
  grid.raster(im2, interpolate = FALSE)
  
  writeBin(png2, "working/test.png")
  
  im3 <- read_png_as_rgba(png2)
  
  library(png)
  bench::mark(
    # write_png_from_raw(im, width = info$width, height = info$height),
    # # write_png_from_raw(im, width = info$width, height = info$height, compression_level = 9),
    # write_png_from_raw(im, width = info$width, height = info$height, use_filter = FALSE),
    # write_png_from_raw(im, width = info$width, height = info$height, use_filter = TRUE , compression_level = 0),
    # write_png_from_raw(im, width = info$width, height = info$height, use_filter = FALSE, compression_level = 0),
    write_png_from_nara(im, use_filter = FALSE, compression_level = 0),
    writePNG(im3),
    check = FALSE,
    relative = TRUE
  ) [,c(1,4)]
  
  
  writePNG(im3) |> length()
  write_png_from_raw(im, width = info$width, height = info$height) |> length()
  write_png_from_raw(im, width = info$width, height = info$height, compression_level = 9) |> length()
  write_png_from_raw(im, width = info$width, height = info$height, use_filter = FALSE) |> length()
  write_png_from_raw(im, width = info$width, height = info$height, use_filter = TRUE , compression_level = 0) |> length()
  write_png_from_raw(im, width = info$width, height = info$height, use_filter = FALSE, compression_level = 0) |> length()
  
  
  identical(
    writePNG(im3),
    write_png_from_raw(im, width = info$width, height = info$height)
  )
  
  
  file <- "working/test.png"
  bench::mark(
    write_png_from_nara(im, file),
    write_png_from_nara(im, file, use_filter=FALSE, compression_level = 0),
    writePNG(im3, file)
  )  
  
  
  
  
  
  
}
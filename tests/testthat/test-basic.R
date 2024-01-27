


if (FALSE) {
  
  ras_test <- matrix(tolower(grDevices::rainbow(100 * 75, alpha = 1)), 75, 100)
  class(ras_test) <- 'raster'
  write_png(ras_test, "tests/testthat/image/rainbow.png")
  
  
  
  ras_test <- matrix(tolower(grDevices::rainbow(100 * 75)), 75, 100)
  class(ras_test) <- 'raster'
  write_png(ras_test, "tests/testthat/image/rainbow.png")
}





test_that("Write PNG works", {

  png_file <- testthat::test_path("image/rainbow.png")
  png_data <- readBin(png_file, raw(), n = file.size(png_file))
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Read in as specific formats
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  nara <- read_png_as_nara  (png_data)
  ras  <- read_png_as_raster(png_data)
  rgb  <- read_png_as_rgb   (png_data)
  rgba <- read_png_as_rgba  (png_data)
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Vanilla 'write_png' works
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  png_data_nara <- write_png(nara)
  png_data_ras  <- write_png(ras)
  png_data_rgb  <- write_png(rgb)
  png_data_rgba <- write_png(rgba)
  
  expect_identical(png_data_nara, png_data_ras)
  expect_identical(png_data_nara, png_data_rgba)
  
  
  nara3 <- read_png_as_nara  (png_data_nara)
  ras3  <- read_png_as_raster(png_data_ras)
  rgb3  <- read_png_as_rgb   (png_data_rgb)
  rgba3 <- read_png_as_rgba  (png_data_rgba)
  
  expect_identical(nara, nara3)
  expect_identical(ras, ras3)
  expect_identical(rgb, rgb3)
  expect_identical(rgba, rgba3)
})

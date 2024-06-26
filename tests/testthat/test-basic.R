


if (FALSE) {
  
  ras_test <- matrix(tolower(grDevices::rainbow(100 * 75, alpha = 1)), 75, 100)
  class(ras_test) <- 'raster'
  write_png(ras_test, "tests/testthat/image/rainbow.png")
  
  
  
  ras_test <- matrix(tolower(grDevices::rainbow(100 * 75)), 75, 100)
  class(ras_test) <- 'raster'
  write_png(ras_test, testthat::test_path("image/rainbow-rgb.png"))
  
  ras_test <- matrix('green', 75, 100)
  class(ras_test) <- 'raster'
  write_png(ras_test, testthat::test_path("image/rainbow-rgb.png"))
}





test_that("Write PNG works", {

  png_file <- testthat::test_path("image/rainbow.png")
  png_data <- readBin(png_file, raw(), n = file.size(png_file))
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Read in as specific formats
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  nara <- read_png(png_data, type = "nativeraster")
  ras  <- read_png(png_data, type = 'raster')
  rgb  <- read_png(png_data, type = 'array', rgba = FALSE)
  rgba <- read_png(png_data, type = 'array')
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Vanilla 'write_png' works
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  png_data_nara <- write_png(nara)
  png_data_ras  <- write_png(ras)
  png_data_rgb  <- write_png(rgb)
  png_data_rgba <- write_png(rgba)
  
  expect_identical(png_data_nara, png_data_ras)
  expect_identical(png_data_nara, png_data_rgba)
  
  
  nara3 <- read_png(png_data_nara, type = "nativeraster")
  ras3  <- read_png(png_data_ras , type = 'raster')
  rgb3  <- read_png(png_data_rgb , type = 'array', rgba = FALSE)
  rgba3 <- read_png(png_data_rgba, type = 'array')
  
  expect_identical(nara, nara3)
  expect_identical(ras, ras3)
  expect_identical(rgb, rgb3)
  expect_identical(rgba, rgba3)
})


test_that("Read PNG works", {
  
  png_file <- testthat::test_path("image/rainbow.png")
  png_data <- readBin(png_file, raw(), n = file.size(png_file))
  
  nara <- read_png(png_data, type = "nativeraster")
  ras  <- read_png(png_data, type = 'raster')
  rgba <- read_png(png_data, type = 'array')
  rgb  <- read_png(png_data, type = 'array', rgba = FALSE)
  
  nara2 <- read_png(png_file, type = "nativeraster")
  ras2  <- read_png(png_file, type = 'raster')
  rgba2 <- read_png(png_file, type = 'array')
  rgb2  <- read_png(png_file, type = 'array', rgba = FALSE)
  
  expect_identical(nara, nara2)
  expect_identical(ras, ras2)
  expect_identical(rgba, rgba2)
  expect_identical(rgb, rgb2)
  
  
})


test_that("write/read round trip is idempotent", {
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Test write/read cycle for nativeraster
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  for (im in test_image$nativeraster) {
    raw_vec <- write_png(im)
    im2 <- read_png(raw_vec, type = "nativeraster")
    
    # equal except for rounding 
    expect_identical(im, im2)    
  }
  
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Test write/read cycle for raster RGBA
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  im <- test_image$raster$rgba
  raw_vec <- write_png(im)
  im2 <- read_png(raw_vec, type = 'raster')
  expect_identical(im, im2)
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Test write/read cycle for raster RGB
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # RGB raster written as RGBA unless 'trns' is set
  im <- im_orig <- test_image$raster$rgb
  raw_vec <- write_png(im)
  im2 <- read_png(raw_vec, type = 'raster')

  # manually add alpha = FF so we can compre input/output
  im <- as.matrix(im)
  im[] <- paste0(im, "FF")
  im <- as.raster(im)

  expect_identical(im, im2)
  
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Test write/read cycle for raster of named colours
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  im <- test_image$raster$named
  raw_vec <- write_png(im)
  im2 <- read_png(raw_vec, type = 'raster')
  
  im_hex <- as.vector(im)
  im_hex <- rgb(t(col2rgb(im_hex)), alpha = 255, maxColorValue = 255)
  
  im2_hex <- as.vector(im2)
  expect_identical(im_hex, im2_hex)

  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Test write/read cycle for array
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  im <- test_image$array$rgba
  
  for (nm in names(test_image$array)) {
    # print(nm)
    # if (nm == 'gray_alpha') next
    im <- test_image$array[[nm]]
    
    raw_vec <- write_png(im)
    im2 <- read_png(raw_vec, type = 'array')
    
    # equal except for rounding 
    expect_equal(im, im2, tolerance = 1/255/2)    
  }
  
  
  im <- test_image$array16$gray
  
  for (nm in names(test_image$array16)) {
    # print(nm)
    # if (nm == 'gray_alpha') next
    im <- test_image$array16[[nm]]
    
    raw_vec <- write_png(im, bits = 16)
    im2 <- read_png(raw_vec, type = 'array')
    
    # equal except for rounding 
    expect_equal(im, im2, tolerance = 1/255/2)    
  }
  
})




if (FALSE) {
  
  im <- test_image$array$gray_alpha
  raw_vec <- write_png(im)
  im2 <- read_png(raw_vec, type = 'array', rgba = FALSE)
  
  im2 <- read_png(raw_vec, type = 'array', rgba = TRUE)
  im2 <- read_png(raw_vec, type = "nativeraster")
  im2 <- read_png(raw_vec, type = 'raster')

  
  ras <- as.raster(matrix(c('red', 'white', 'blue'), 4, 3))
  plot(ras, interpolate = FALSE)
  write_png(ras)  
  
}

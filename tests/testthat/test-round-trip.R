
test_that("write/read round trip is idempotent", {
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Test write/read cycle for nativeraster
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  for (im in test_image$native_raster) {
    raw_vec <- write_png(im)
    im2 <- read_png(raw_vec, type = 'native_raster')
    
    # equal except for rounding 
    expect_identical(im, im2)    
  }
  
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Test write/read cycle for raster
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  im <- test_image$raster$rgba

  for (im in test_image$raster) {
    raw_vec <- write_png(im)
    im2 <- read_png(raw_vec, type = 'raster')

    # equal except for rounding
    expect_identical(im, im2)
  }
  
  
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
  
})



if (FALSE) {
  
  im <- test_image$array$gray_alpha
  raw_vec <- write_png(im)
  im2 <- read_png(raw_vec, type = 'array', rgba = FALSE)
  
  im2 <- read_png(raw_vec, type = 'array', rgba = TRUE)
  im2 <- read_png(raw_vec, type = 'native_raster')
  im2 <- read_png(raw_vec, type = 'raster')

}
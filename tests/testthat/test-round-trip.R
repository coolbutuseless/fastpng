
test_that("write/read round trip is idempotent", {
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Test write/read cycle for nativeraster
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  for (im in rimage$native_raster) {
    raw_vec <- write_png(im)
    im2 <- read_png(raw_vec, type = 'native_raster')
    
    # equal except for rounding 
    expect_identical(im, im2)    
  }
  
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Test write/read cycle for raster
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  im <- rimage$raster$rgba

  for (im in rimage$raster) {
    raw_vec <- write_png(im)
    im2 <- read_png(raw_vec, type = 'raster')

    # equal except for rounding
    expect_identical(im, im2)
  }
  
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Test write/read cycle for array
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  im <- rimage$array$rgba
  
  for (nm in names(rimage$array)) {
    # print(nm)
    # if (nm == 'gray_alpha') next
    im <- rimage$array[[nm]]
    
    raw_vec <- write_png(im)
    im2 <- read_png(raw_vec, type = 'array')
    
    # equal except for rounding 
    expect_equal(im, im2, tolerance = 1/255/2)    
  }
  
  
  im <- rimage$array16$gray
  
  for (nm in names(rimage$array16)) {
    # print(nm)
    # if (nm == 'gray_alpha') next
    im <- rimage$array16[[nm]]
    
    raw_vec <- write_png(im, bits = 16)
    im2 <- read_png(raw_vec, type = 'array')
    
    # equal except for rounding 
    expect_equal(im, im2, tolerance = 1/255/2)    
  }
  
  
  
})



if (FALSE) {
  
  im <- rimage$array$gray_alpha
  raw_vec <- write_png(im)
  im2 <- read_png(raw_vec, type = 'array', rgba = FALSE)
  
  im2 <- read_png(raw_vec, type = 'array', rgba = TRUE)
  im2 <- read_png(raw_vec, type = 'native_raster')
  im2 <- read_png(raw_vec, type = 'raster')

}

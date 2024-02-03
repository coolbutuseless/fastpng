
test_that("write/read round trip is idempotent", {
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Test write/read cycle for nativeraster
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  for (im in test_image$native_raster) {
    raw_vec <- write_png(im)
    im2 <- read_png(raw_vec, type = 'nara')
    
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
    if (nm == 'gray_alpha') {
      # SPNG library does not seem to want to read GA8 images
      # into a 2-plane array.  So instead grey+alpha is always
      # promoted to RGBA.  With R,G+B channels just replicating
      # the gray value.
      # Mike 2024-02-03
      expect_equal(im2[,,1], im [,,1], tolerance = 0.01) # R = grey
      expect_equal(im2[,,4], im [,,2], tolerance = 0.01) # Alpha matches
      expect_equal(im2[,,1], im2[,,2], tolerance = 0.01) # Red = Green
      expect_equal(im2[,,1], im2[,,3], tolerance = 0.01) # Red = Blue
    } else {
      expect_equal(im, im2, tolerance = 0.01)    
    }
  }
  
})



if (FALSE) {
  
  im <- test_image$array$gray_alpha
  raw_vec <- write_png(im)
  im2 <- read_png(raw_vec, type = 'array', rgba = FALSE)
  
  im2 <- read_png(raw_vec, type = 'array', rgba = TRUE)
  im2 <- read_png(raw_vec, type = 'nara')
  im2 <- read_png(raw_vec, type = 'raster')

}
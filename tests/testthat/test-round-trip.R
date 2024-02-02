
test_that("round trip is idempotent works", {
  
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
  im <- test_image$array$gray_alpha
  
  for (nm in names(test_image$array)) {
    # print(nm)
    if (nm == 'gray_alpha') next
    im <- test_image$array[[nm]]
    
    raw_vec <- write_png(im)
    im2 <- read_png(raw_vec, type = 'array')
    
    # equal except for rounding 
    expect_equal(im, im2, ignore_attr = TRUE, tolerance = 0.01)    
  }
  
})

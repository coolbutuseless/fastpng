
test_that("tRNS transparency works", {
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Raster tRNS specification
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  im1 <- test_image$raster$rgb
  c1 <- im1[[1,1]] # first color
    
  im2 <- write_png(im1, trns = c1) |> read_png(type = 'raster')
  
  expect_true(all(endsWith(im2[im1 != c1], 'FF')))
  expect_true(all(endsWith(im2[im1 == c1], '00')))
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Raster tRNS specification
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  im1 <- test_image$raster$rgb
  c1 <- im1[[1,1]] # first color
  int_c1 <- c(0L, 0L, 3L)
  
  im2 <- write_png(im1, trns = int_c1) |> read_png(type = 'raster')
  
  expect_true(all(endsWith(im2[im1 != c1], 'FF')))
  expect_true(all(endsWith(im2[im1 == c1], '00')))
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Raster tRNS specification
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  im1 <- test_image$raster$rgb
  c1 <- im1[[1,1]] # first color
  dbl_c1 <- c(0, 0, 3)
  
  im2 <- write_png(im1, trns = dbl_c1) |> read_png(type = 'raster')
  
  expect_true(all(endsWith(im2[im1 != c1], 'FF')))
  expect_true(all(endsWith(im2[im1 == c1], '00')))
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Grayscale transparency
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  im1 <- test_image$array$gray
  c1 <- im1[[1,1]] 
  
  im2 <- write_png(im1, trns = c1) |> read_png(type = 'raster')
  
  expect_true(all(endsWith(im2[im1 != c1], 'FF')))
  expect_true(all(endsWith(im2[im1 == c1], '00')))
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Grayscale transparency
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  im1 <- test_image$array_int$gray
  c1 <- im1[[1,1]] 
  
  im2 <- write_png(im1, trns = c1) |> read_png(type = 'raster')
  
  expect_true(all(endsWith(im2[im1 != c1], 'FF')))
  expect_true(all(endsWith(im2[im1 == c1], '00')))
  
})

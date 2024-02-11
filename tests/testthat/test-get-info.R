

test_that("get_png_info works", {
  
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # RGBA File
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  tmp <- tempfile()
  im <- rimage$array$rgba
  write_png(im, file = tmp) 
  info <- get_png_info(tmp)
  expect_true(inherits(info, 'list'))
  
  expect_equal(info$bit_depth, 8)
  expect_equal(info$width, ncol(im))
  expect_equal(info$height, nrow(im))
  expect_equal(info$color_desc, 'SPNG_COLOR_TYPE_TRUECOLOR_ALPHA')
  
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # RGBA
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  im <- rimage$array$rgba
  info <- write_png(im) |> get_png_info()
  expect_true(inherits(info, 'list'))
  
  expect_equal(info$bit_depth, 8)
  expect_equal(info$width, ncol(im))
  expect_equal(info$height, nrow(im))
  expect_equal(info$color_desc, 'SPNG_COLOR_TYPE_TRUECOLOR_ALPHA')
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # RGB
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  im <- rimage$array$rgb
  info <- write_png(im) |> get_png_info()
  expect_true(inherits(info, 'list'))
  
  expect_equal(info$bit_depth, 8)
  expect_equal(info$width, ncol(im))
  expect_equal(info$height, nrow(im))
  expect_equal(info$color_desc, 'SPNG_COLOR_TYPE_TRUECOLOR')
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # gray alpha
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  im <- rimage$array$gray_alpha
  info <- write_png(im) |> get_png_info()
  expect_true(inherits(info, 'list'))
  
  expect_equal(info$bit_depth, 8)
  expect_equal(info$width, ncol(im))
  expect_equal(info$height, nrow(im))
  expect_equal(info$color_desc, 'SPNG_COLOR_TYPE_GRAYSCALE_ALPHA')
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # gray 
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  im <- rimage$array$gray
  info <- write_png(im) |> get_png_info()
  expect_true(inherits(info, 'list'))
  
  expect_equal(info$bit_depth, 8)
  expect_equal(info$width, ncol(im))
  expect_equal(info$height, nrow(im))
  expect_equal(info$color_desc, 'SPNG_COLOR_TYPE_GRAYSCALE')
  
  
  
  
  
  
})

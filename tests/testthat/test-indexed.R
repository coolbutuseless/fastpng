
test_that("Indexed images", {
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Integer index
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  im <- write_png(rimage$indexed$integer_index, palette = rimage$indexed$palette) |> 
    read_png(type = 'indexed')
  
  expect_identical(
    im$index,
    rimage$indexed$integer_index
  )

  expect_identical(
    im$palette,
    rimage$indexed$palette
  )  

  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Numeric index
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  im <- write_png(rimage$indexed$numeric_index, palette = rimage$indexed$palette) |> 
    read_png(type = 'indexed')
  
  expect_identical(
    im$index,
    rimage$indexed$integer_index
  )
  
  expect_identical(
    im$palette,
    rimage$indexed$palette
  )  
  
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Read indexed as RGBA array
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  im <- write_png(rimage$indexed$numeric_index, palette = rimage$indexed$palette) |> 
    read_png(type = 'array')
  
  expect_true(inherits(im, 'array'))
  
  expect_identical(
    dim(im),
    c(dim(rimage$indexed$numeric_index), 4L)
  )
  
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Read indexed as RGBA array
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  im <- write_png(rimage$indexed$numeric_index, palette = rimage$indexed$palette) |> 
    read_png(type = 'raster')
  
  expect_true(inherits(im, 'raster'))
  
  expect_true(all(unique(as.vector(im)) %in% rimage$indexed$palette))
  
  
})

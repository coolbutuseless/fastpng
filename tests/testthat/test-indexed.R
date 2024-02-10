
test_that("Indexed images", {
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Integer index
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  im <- write_png(test_image$indexed$integer_index, palette = test_image$indexed$palette) |> 
    read_png(type = 'indexed')
  
  expect_identical(
    im$index,
    test_image$indexed$integer_index
  )

  expect_identical(
    im$palette,
    test_image$indexed$palette
  )  

  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Numeric index
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  im <- write_png(test_image$indexed$numeric_index, palette = test_image$indexed$palette) |> 
    read_png(type = 'indexed')
  
  expect_identical(
    im$index,
    test_image$indexed$integer_index
  )
  
  expect_identical(
    im$palette,
    test_image$indexed$palette
  )  
  
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Read indexed as RGBA array
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  im <- write_png(test_image$indexed$numeric_index, palette = test_image$indexed$palette) |> 
    read_png(type = 'array')
  
  expect_true(inherits(im, 'array'))
  
  expect_identical(
    dim(im),
    c(dim(test_image$indexed$numeric_index), 4L)
  )
  
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Read indexed as RGBA array
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  im <- write_png(test_image$indexed$numeric_index, palette = test_image$indexed$palette) |> 
    read_png(type = 'raster')
  
  expect_true(inherits(im, 'raster'))
  
  expect_true(all(unique(as.vector(im)) %in% test_image$indexed$palette))
  
  
})

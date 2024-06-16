


test_that("raw 8bit read/write other formats", {
  
  expect_identical(
    write_png(test_image$array$gray) |> read_png(type = 'raw'),
    test_image$raw$gray
  )  
  
  expect_identical(
    write_png(test_image$array$gray_alpha) |> read_png(type = 'raw'),
    test_image$raw$gray_alpha
  )  
  
  expect_identical(
    write_png(test_image$array$rgb) |> read_png(type = 'raw'),
    test_image$raw$rgb
  )  
  
  expect_identical(
    write_png(test_image$array$rgba) |> read_png(type = 'raw'),
    test_image$raw$rgba
  )  

  
  im <- test_image$raw$gray
  expect_identical(
    write_png(im, raw_spec = attributes(im)) |> read_png(),
    test_image$array$gray
  )  
  
  im <- test_image$raw$gray_alpha
  expect_identical(
    write_png(im, raw_spec = attributes(im)) |> read_png(),
    test_image$array$gray_alpha
  )  
  
  im <- test_image$raw$rgb
  expect_identical(
    write_png(im, raw_spec = attributes(im)) |> read_png(),
    test_image$array$rgb
  )  
  
  im <- test_image$raw$rgba
  expect_identical(
    write_png(im, raw_spec = attributes(im)) |> read_png(),
    test_image$array$rgba
  )  
  
})



test_that("raw 16bit read/write other formats", {
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Gray
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  expect_identical(
    write_png(test_image$array_16bit$gray, bits = 16) |> read_png(type = 'raw'),
    test_image$raw_16bit$gray
  )  
  
  im <- test_image$raw_16bit$gray
  expect_identical(
    write_png(im, raw_spec = attributes(im)) |> read_png(type = 'raw'),
    test_image$raw_16bit$gray
  )  
  
  im <- test_image$raw_16bit$gray
  expect_equal(
    write_png(im, raw_spec = attributes(im), bits = 16) |> read_png(type = 'array'),
    test_image$array_16bit$gray
  )
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # RGB
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  expect_identical(
    write_png(test_image$array_16bit$rgb, bits = 16) |> read_png(type = 'raw'),
    test_image$raw_16bit$rgb
  )  
  
  im0 <- test_image$array_int_16bit$rgb
  expect_identical(
    im1 <- write_png(test_image$array_int_16bit$rgb, bits = 16) |> read_png(type = 'raw'),
    im2 <- test_image$raw_16bit$rgb
  )  
  
  im <- test_image$raw_16bit$rgb
  expect_identical(
    write_png(im, raw_spec = attributes(im)) |> read_png(type = 'raw'),
    test_image$raw_16bit$rgb
  )  
  
  im <- test_image$raw_16bit$rgb
  expect_equal(
    write_png(im, raw_spec = attributes(im), bits = 16) |> read_png(type = 'array'),
    test_image$array_16bit$rgb
  )
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # RGBA
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  expect_identical(
    write_png(test_image$array_16bit$rgba, bits = 16) |> read_png(type = 'raw'),
    test_image$raw_16bit$rgba
  )  
  
  im <- test_image$raw_16bit$rgba
  expect_identical(
    write_png(im, raw_spec = attributes(im)) |> read_png(type = 'raw'),
    test_image$raw_16bit$rgba
  )  
  
  im <- test_image$raw_16bit$rgba
  expect_equal(
    write_png(im, raw_spec = attributes(im), bits = 16) |> read_png(type = 'array'),
    test_image$array_16bit$rgba
  )
  
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # RGBA
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  expect_identical(
    write_png(test_image$array_16bit$gray, bits = 16) |> read_png(type = 'raw'),
    test_image$raw_16bit$gray
  )  
  
  im <- test_image$raw_16bit$gray
  expect_identical(
    write_png(im, raw_spec = attributes(im)) |> read_png(type = 'raw'),
    test_image$raw_16bit$gray
  )  
  
  im <- test_image$raw_16bit$gray
  expect_equal(
    write_png(im, raw_spec = attributes(im), bits = 16) |> read_png(type = 'array'),
    test_image$array_16bit$gray
  )
  
  
})





test_that("native_raster read/write other formats", {
  
  expect_identical(
    write_png(test_image$native_raster$rgba) |> read_png(type = 'native_raster'),
    test_image$native_raster$rgba
  )  
  
  expect_identical(
    write_png(test_image$native_raster$rgba) |> read_png(type = 'raster'),
    test_image$raster$rgba
  )  
  
  expect_identical(
    write_png(test_image$native_raster$rgba) |> read_png(type = 'array'),
    test_image$array$rgba
  )    
  
})



test_that("raster read/write other formats", {
  
  expect_identical(
    write_png(test_image$raster$rgba) |> read_png(type = 'raster'),
    test_image$raster$rgba
  )  
  
  expect_identical(
    write_png(test_image$raster$rgba) |> read_png(type = 'native_raster'),
    test_image$native_raster$rgba
  )  
  
  expect_identical(
    write_png(test_image$raster$rgba) |> read_png(type = 'array'),
    test_image$array$rgba
  )    
  
})



test_that("8bit array: Exhastive check of r/w sanity between formats", {
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # RGBA 
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  expect_identical(
    write_png(test_image$array$rgba) |> read_png(),
    test_image$array$rgba
  )  
  
  expect_identical(
    write_png(test_image$array$rgba) |> read_png(rgba = FALSE),
    test_image$array$rgba
  )  
  
  expect_identical(
    write_png(test_image$array$rgba) |> read_png(rgba = TRUE),
    test_image$array$rgba
  )  
  
  expect_identical(
    write_png(test_image$array$rgba) |> read_png(type = 'raster'),
    test_image$raster$rgba
  )  
  
  expect_identical(
    write_png(test_image$array$rgba) |> read_png(type = 'native_raster'),
    test_image$native_raster$rgba
  )
  
  expect_identical(
    write_png(test_image$array$rgba) |> png::readPNG(native = TRUE),
    test_image$native_raster$rgba
  )
  
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # RGB 
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  expect_identical(
    write_png(test_image$array$rgb) |> read_png(),
    test_image$array$rgb
  )  
  
  expect_identical(
    write_png(test_image$array$rgb) |> read_png(rgba = FALSE),
    test_image$array$rgb
  )  
  
  expect_identical(
    (write_png(test_image$array$rgb) |> read_png(rgba = TRUE))[,,1:3],
    test_image$array$rgb
  )  
  
  expect_identical(
    write_png(test_image$array$rgb) |> read_png(type = 'raster'),
    test_image$raster$rgb
  )  
  
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Gray
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  expect_identical(
    write_png(test_image$array$gray) |> read_png(),
    test_image$array$gray
  )  
  
  expect_identical(
    write_png(test_image$array$gray) |> read_png(rgba = FALSE),
    test_image$array$gray
  )  
  
  expect_identical(
    (write_png(test_image$array$gray) |> read_png(rgba = TRUE))[,,1],
    test_image$array$gray
  )  
  

  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Gray + Alpha
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  expect_identical(
    write_png(test_image$array$gray_alpha) |> read_png(),
    test_image$array$gray_alpha
  )  
  
  expect_identical(
    write_png(test_image$array$gray_alpha) |> read_png(rgba = FALSE),
    test_image$array$gray_alpha
  )  
  
  im <- write_png(test_image$array$gray_alpha) |> read_png(rgba = TRUE)
  expect_identical(
    im[,,1], # gray channel
    test_image$array$gray
  )  
  
  expect_identical(
    im[,,4], # alpha channel
    test_image$array$gray_alpha[,,2]
  )  
})



test_that("16bit array: Exhastive check of r/w sanity between formats", {
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # RGBA 
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  expect_equal(
    write_png(test_image$array_16bit$rgba, bits = 16) |> read_png(),
    test_image$array_16bit$rgba,
    tolerance = 1/65535
  )  
  
  expect_equal(
    write_png(test_image$array_16bit$rgba, bits = 16) |> read_png(rgba = FALSE),
    test_image$array_16bit$rgba,
    tolerance = 1/65535
  )  
  
  expect_equal(
    write_png(test_image$array_16bit$rgba, bits = 16) |> read_png(rgba = TRUE),
    test_image$array_16bit$rgba,
    tolerance = 1/65535
  )
  
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # RGB 
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  expect_equal(
    write_png(test_image$array_16bit$rgb, bits = 16) |> read_png(),
    test_image$array_16bit$rgb,
    tolerance = 1/65535
  )  
  
  expect_equal(
    write_png(test_image$array_16bit$rgb, bits = 16) |> read_png(rgba = FALSE),
    test_image$array_16bit$rgb,
    tolerance = 1/65535
  )  
  
  expect_equal(
    (write_png(test_image$array_16bit$rgb, bits = 16) |> read_png(rgba = TRUE))[,,1:3],
    test_image$array_16bit$rgb,
    tolerance = 1/65535
  )
  
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Gray
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  expect_equal(
    write_png(test_image$array_16bit$gray, bits = 16) |> read_png(),
    test_image$array_16bit$gray,
    tolerance = 1/65535
  )  
  
  expect_equal(
    write_png(test_image$array_16bit$gray, bits = 16) |> read_png(rgba = FALSE),
    test_image$array_16bit$gray,
    tolerance = 1/65535
  )  
  
  expect_equal(
    (write_png(test_image$array_16bit$gray, bits = 16) |> read_png(rgba = TRUE))[,,1],
    test_image$array_16bit$gray,
    tolerance = 1/65535
  )
  
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Gray + Alpha
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  expect_equal(
    write_png(test_image$array_16bit$gray_alpha, bits = 16) |> read_png(),
    test_image$array_16bit$gray_alpha,
    tolerance = 1/65535
  )  
  
  expect_equal(
    write_png(test_image$array_16bit$gray_alpha, bits = 16) |> read_png(rgba = FALSE),
    test_image$array_16bit$gray_alpha,
    tolerance = 1/65535
  )  
  
  im <- write_png(test_image$array_16bit$gray_alpha, bits = 16) |> read_png(rgba = TRUE)
  expect_equal(
    im[,,1], # gray channel
    test_image$array_16bit$gray,
    tolerance = 1/65535
  )

  expect_equal(
    im[,,4], # alpha channel
    test_image$array_16bit$gray_alpha[,,2],
    tolerance = 1/65535
  )
  
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # RGBA double -> integer round trip
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  expect_equal(
    write_png(test_image$array_16bit$rgba, bits = 16) |> read_png(array_type = 'int'),
    test_image$array_int_16bit$rgba
  )
  
  expect_equal(
    write_png(test_image$array_int_16bit$rgba, bits = 16) |> read_png(array_type = 'dbl'),
    test_image$array_16bit$rgba
  )
  
  
  
})







if (FALSE) {
  im <- write_png(test_image$native_raster$rgba) 
  get_png_info(im)
  read_png(im) |> dim()
}

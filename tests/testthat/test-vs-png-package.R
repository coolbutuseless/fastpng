

test_that("identical to PNG package in basic case", {
  
  expect_identical(
    write_png(test_image$array$rgba),
    png::writePNG(test_image$array$rgba)
  )
  
  expect_identical(
    write_png(test_image$array$rgb),
    png::writePNG(test_image$array$rgb)
  )
  
  expect_identical(
    write_png(test_image$array$gray),
    png::writePNG(test_image$array$gray)
  )  

  
  expect_identical(
    write_png(test_image$array$gray_alpha),
    png::writePNG(test_image$array$gray_alpha)
  )

  
  expect_identical(
    write_png(test_image$native_raster$rgba),
    png::writePNG(test_image$native_raster$rgba)
  )
  
})



test_that("identical writes to PNG package in basic case", {
  
  expect_identical(
    write_png(rimage$array$rgba),
    png::writePNG(rimage$array$rgba)
  )
  
  expect_identical(
    write_png(rimage$array$rgb),
    png::writePNG(rimage$array$rgb)
  )
  
  expect_identical(
    write_png(rimage$array$gray),
    png::writePNG(rimage$array$gray)
  )  

  
  expect_identical(
    write_png(rimage$array$gray_alpha),
    png::writePNG(rimage$array$gray_alpha)
  )

  
  expect_identical(
    write_png(rimage$native_raster$rgba),
    png::writePNG(rimage$native_raster$rgba)
  )
  
})




test_that("identical reads to PNG package in basic case", {
  
  expect_identical(
    write_png    (rimage$array$rgba) |> read_png(),
    png::writePNG(rimage$array$rgba) |> png::readPNG()
  )
  
  expect_identical(
    write_png    (rimage$array$rgb) |> read_png(),
    png::writePNG(rimage$array$rgb) |> png::readPNG()
  )
  
  expect_identical(
    write_png    (rimage$array$gray) |> read_png(),
    png::writePNG(rimage$array$gray) |> png::readPNG()
  )  
  
  
  expect_identical(
    write_png    (rimage$array$gray_alpha) |> read_png(),
    png::writePNG(rimage$array$gray_alpha) |> png::readPNG()
  )
  
  
  expect_identical(
    write_png    (rimage$native_raster$rgba) |> read_png(),
    png::writePNG(rimage$native_raster$rgba) |> png::readPNG()
  )
  
})

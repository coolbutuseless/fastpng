

test_that("identical writes to PNG package in basic case", {
  
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
    write_png(test_image$nativeraster$rgba),
    png::writePNG(test_image$nativeraster$rgba)
  )
  
})




test_that("identical reads to PNG package in basic case", {
  
  expect_identical(
    write_png    (test_image$array$rgba) |> read_png(),
    png::writePNG(test_image$array$rgba) |> png::readPNG()
  )
  
  expect_identical(
    write_png    (test_image$array$rgb) |> read_png(),
    png::writePNG(test_image$array$rgb) |> png::readPNG()
  )
  
  expect_identical(
    write_png    (test_image$array$gray) |> read_png(),
    png::writePNG(test_image$array$gray) |> png::readPNG()
  )  
  
  
  expect_identical(
    write_png    (test_image$array$gray_alpha) |> read_png(),
    png::writePNG(test_image$array$gray_alpha) |> png::readPNG()
  )
  
  
  expect_identical(
    write_png    (test_image$nativeraster$rgba) |> read_png(),
    png::writePNG(test_image$nativeraster$rgba) |> png::readPNG()
  )
  
})

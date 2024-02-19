
test_that("raw_spec detects missing specs", {
  
  im <- as.raw(1:10)
  write_png(im, raw_spec = list(bits = 8, height = 2, width = 5, depth = 1))
  
  expect_error(
    write_png(im, raw_spec = list(bits = 8, height = 2, width = 5, greg = 1)),
    "depth"
  )
  
  
  expect_error(
    write_png(im, raw_spec = list(greg = 8, height = 2, width = 5, depth = 1)),
    "bits"
  )
  
})

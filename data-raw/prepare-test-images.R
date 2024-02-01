


test_image_gray8 <- matrix(
  seq(0, 1, length.out = 200 * 150),
  nrow = 150,
  ncol = 200
)


test_image_raster <- matrix(
  grDevices::rainbow(200 * 150, alpha = 1), 150, 200
)
class(test_image_raster) <- 'raster'


usethis::use_data(test_image_gray8, test_image_raster, overwrite = TRUE)
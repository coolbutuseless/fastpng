
library(grid)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# ARRAY: Gray
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
test_image_gray <- outer(1:150, 1:200) 
test_image_gray <- test_image_gray / max(test_image_gray)
grid.raster(test_image_gray)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# ARRAY: RGB 
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
test_image_array_rgb <- c(
  test_image_gray,
  rev(test_image_gray),
  test_image_gray[nrow(test_image_gray):1, ]
)
dim(test_image_array_rgb) <- c(150, 200, 3)
grid.newpage(); grid.raster(test_image_array_rgb)


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# ARRAY: RGBA
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
test_image_array_rgba <- c(
  test_image_gray,
  rev(test_image_gray),
  test_image_gray[nrow(test_image_gray):1, ],
  (test_image_gray[, ncol(test_image_gray):1]) ^ 0.15
)
dim(test_image_array_rgba) <- c(150, 200, 4)
grid.newpage(); grid.raster(test_image_array_rgba)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# ARRAY: Gray + Alpha
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
test_image_array_ga <- c(
  test_image_gray,
  test_image_gray[nrow(test_image_gray):1, ] ^ 0.15
)
dim(test_image_array_ga) <- c(150, 200, 2)
# grid.newpage(); grid.raster(test_image_array_ga)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# RASTER
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
test_image_raster_rgb  <- as.raster(test_image_array_rgb )
test_image_raster_rgba <- as.raster(test_image_array_rgba)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Native raster
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
test_image_nativeraster <- nara::raster_to_nr(test_image_raster_rgba)


test_image <- list(
  array = list(
    gray       = test_image_gray,
    gray_alpha = test_image_array_ga,
    rgb        = test_image_array_rgb,
    rgba       = test_image_array_rgba
  ),
  raster = list(
    rgb  = test_image_raster_rgb,
    rgba = test_image_raster_rgba
  ),
  native_raster = list(
    rgba = test_image_nativeraster
  )
)



usethis::use_data(test_image, overwrite = TRUE)

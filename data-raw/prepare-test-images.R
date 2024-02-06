
library(grid)

w <- 400
h <- 300


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# ARRAY: Gray
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
test_image_gray <- outer(1:h, 1:w) 
test_image_gray <- test_image_gray / max(test_image_gray)
test_image_gray <- (round(test_image_gray * 1024) %% 256) / 255
grid.raster(test_image_gray)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# ARRAY: RGB 
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
test_image_array_rgb <- c(
  test_image_gray,
  rev(test_image_gray),
  test_image_gray[nrow(test_image_gray):1, ]
)
dim(test_image_array_rgb) <- c(h, w, 3)
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
dim(test_image_array_rgba) <- c(h, w, 4)
grid.newpage(); grid.raster(test_image_array_rgba)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# ARRAY: Gray + Alpha
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
test_image_array_ga <- c(
  test_image_gray,
  test_image_gray[nrow(test_image_gray):1, ] ^ 0.15
)
dim(test_image_array_ga) <- c(h, w, 2)
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


index_dbl <- test_image_gray * 255
index_int <- as.integer(index_dbl)
dim(index_int) <- dim(index_dbl)

palette <- viridisLite::viridis(256)

class(index_dbl[1, 1])
class(index_int[1, 1])

if (FALSE) {
  im <- test_image_palette
  im[] <- palette[test_image_palette + 1]
}




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
  ),
  indexed = list(
    integer_index = index_int,
    numeric_index = index_dbl,
    palette       = palette
  )
)



usethis::use_data(test_image, overwrite = TRUE)

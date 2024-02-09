
library(grid)

w <- 400
h <- 300



#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# ARRAY: Gray
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
test_image_gray16 <- outer(1:h, 1:w) 
test_image_gray16 <- test_image_gray16 / max(test_image_gray16)
test_image_gray16 <- (round(test_image_gray16 * 2^18) %% (2^16)) / 2^16
grid.raster(test_image_gray16)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# ARRAY: RGB 
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
test_image_array_rgb16 <- c(
  test_image_gray16,
  rev(test_image_gray16),
  test_image_gray16[nrow(test_image_gray16):1, ]
)
dim(test_image_array_rgb16) <- c(h, w, 3)
grid.newpage(); grid.raster(test_image_array_rgb16)


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# ARRAY: RGBA
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
test_image_array_rgba16 <- c(
  test_image_gray16,
  rev(test_image_gray16),
  test_image_gray16[nrow(test_image_gray16):1, ],
  (test_image_gray16[, ncol(test_image_gray16):1])
)
dim(test_image_array_rgba16) <- c(h, w, 4)
grid.newpage(); grid.raster(test_image_array_rgba16)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# ARRAY: Gray + Alpha
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
test_image_array_ga16 <- c(
  test_image_gray16,
  test_image_gray16[nrow(test_image_gray16):1, ]
)
dim(test_image_array_ga16) <- c(h, w, 2)






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
  (test_image_gray[, ncol(test_image_gray):1]) 
)
dim(test_image_array_rgba) <- c(h, w, 4)
grid.newpage(); grid.raster(test_image_array_rgba)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# ARRAY: Gray + Alpha
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
test_image_array_ga <- c(
  test_image_gray,
  test_image_gray[nrow(test_image_gray):1, ]
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
library(nara)
# test_image_nativeraster <- nara::raster_to_nr(test_image_raster_rgba)

ras <- test_image_raster_rgba
nr <- matrix(colour_to_integer((ras)), nrow = nrow(ras), ncol = ncol(ras))
class(nr) <- 'nativeRaster'
attr(nr, 'channels') <- 4L
grid.newpage(); grid.raster(nr)
test_image_nativeraster <- nr



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


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Integer arrays
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
igray <- outer(1:h, 1:w) 
igray <- igray / max(igray)
igray <- round(igray * 1024) %% 256
igray[] <- as.integer(igray)
grid.raster(igray/255)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# ARRAY: RGB 
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
irgb <- c(
  igray,
  rev(igray),
  igray[nrow(igray):1, ]
)
dim(irgb) <- c(h, w, 3)
grid.newpage(); grid.raster(irgb/255)


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# ARRAY: RGBA
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
irgba <- c(
  igray,
  rev(igray),
  igray[nrow(igray):1, ],
  (igray[, ncol(igray):1]) 
)
dim(irgba) <- c(h, w, 4)
grid.newpage(); grid.raster(irgba/255)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# ARRAY: Gray + Alpha
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
iga <- c(
  igray,
  igray[nrow(igray):1, ]
)
dim(iga) <- c(h, w, 2)




#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Integer arrays
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
igray16 <- outer(1:h, 1:w) 
igray16 <- igray16 / max(igray16)
igray16 <- (round(igray16 * 2^18) %% (2^16))
igray16[] <- as.integer(igray16)
grid.raster(igray16/65535)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# ARRAY: RGB 
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
irgb16 <- c(
  igray16,
  rev(igray16),
  igray16[nrow(igray16):1, ]
)
dim(irgb16) <- c(h, w, 3)
grid.newpage(); grid.raster(irgb16/65535)


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# ARRAY: RGBA
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
irgba16 <- c(
  igray16,
  rev(igray16),
  igray16[nrow(igray16):1, ],
  (igray16[, ncol(igray16):1]) 
)
dim(irgba16) <- c(h, w, 4)
grid.newpage(); grid.raster(irgba16/65535)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# ARRAY: Gray + Alpha
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
iga16 <- c(
  igray16,
  igray16[nrow(igray16):1, ]
)
dim(iga16) <- c(h, w, 2)









test_image <- list(
  array = list(
    gray       = test_image_gray,
    gray_alpha = test_image_array_ga,
    rgb        = test_image_array_rgb,
    rgba       = test_image_array_rgba
  ),
  array_16bit = list(
    gray       = test_image_gray16,
    gray_alpha = test_image_array_ga16,
    rgb        = test_image_array_rgb16,
    rgba       = test_image_array_rgba16
  ),
  array_int = list(
    gray       = igray,
    gray_alpha = iga,
    rgb        = irgb,
    rgba       = irgba
  ),
  array_int_16bit = list(
    gray       = igray16,
    gray_alpha = iga16,
    rgb        = irgb16,
    rgba       = irgba16
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



usethis::use_data(test_image, overwrite = TRUE, compress = 'xz')

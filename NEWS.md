
# spng 0.1.2.9000  2024-01-26

* Re-visit, Update, Re-factor
* `read_png(type = X)`
    * Native Raster (integer matrix with class 'nativeRaster')
    * Raster (character matrix with class 'raster') with hex colour vaules "#RRGGBBAA"
    * RGBA 3D numeric array values in [0, 1]
    * RGB 3D numeric array values in [0, 1]
* `write_png()` to write:
    * Native Raster (integer matrix with class 'nativeRaster')
    * Raster (character matrix with class 'raster') with hex colour vaules "#RRGGBBAA"
    * RGBA 3D numeric array values in [0, 1]
    * RGB 3D numeric array values in [0, 1]

# spng 0.1.2

* Allow user to choose `fmt` and `flags` for PNG decoding


# spng 0.1.1

* Get meta-info (width, height, etc) from image.


# spng 0.1.0

* Initial release

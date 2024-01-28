
# spng 0.1.2.9000  2024-01-26

* `read_png()` read from files or raw vectors
* `write_png()` write to files or raw vectors
* `get_png_info()` fetch a named list of information about the PNG
* Types supported:
    * Native Raster (integer matrix with class 'nativeRaster')
    * Raster (character matrix with class 'raster') with hex colour vaules "#RRGGBBAA"
    * RGBA 3D numeric array values in [0, 1]
    * RGB 3D numeric array values in [0, 1]
    * Grey 2D numeric matrix values in [0, 1] (write only)

# spng 0.1.2

* Allow user to choose `fmt` and `flags` for PNG decoding


# spng 0.1.1

* Get meta-info (width, height, etc) from image.


# spng 0.1.0

* Initial release


# spng 0.1.2.9000  2024-01-26

* Re-visit, Update, Re-factor
* `read_png_as_X()`
    * Rename `depng()` to `read_png_as_raw()`
    * Add `read_png_as_nara()` to decode to native raster
    * Add `read_png_as_raster()` to decode to image raster (with colours
      represented as hex strings e.g. `#445566FF`)
    * Add `read_png_as_rgba()` to decode as numeric array with 4 planes (RGBA)
    * Add `read_png_as_rgb()` to decode as numeric array with 3 planes (RGB)
* `write_png_from_X()`
    * `write_png_from_raw()`

# spng 0.1.2

* Allow user to choose `fmt` and `flags` for PNG decoding


# spng 0.1.1

* Get meta-info (width, height, etc) from image.


# spng 0.1.0

* Initial release

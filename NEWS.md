
# fastpng 0.1.5.9000 2024-12-24

* Use `{colorfast}` for fast color lookup
* Set NO_REMAP in C

# fastpng 0.1.5  2024-08-31

* Clarify copyright information.  Sections withing spng.c are attributed 
  to various authors.

# fastpng 0.1.4  2024-08-25

* Standardise on spelling of `color`
* Include fast hashed color lookup when writing rasters
* Read/write PNGs from `raw` vectors
* Read/write from integer arrays
* Read/write indexed PNGs
* Support `trns` PNG transparency
* Support for both 8bit and 16bit values when erading/writing with arrays
* Bug fixes
* More test images and tests

# fastpng 0.1.3  2024-02-03

* `read_png()` to read a PNG from a file or a raw vector
* `write_png()` to write data as a PNG file or PNG data in a raw vector
* `get_png_info()` - interrogate a vector of raw values containing a PNG image
  to determine image information i.e. width, height, bit_depth, color_type, 
  compression_method, filter_method, interlace_method.
  
Supported R image types:

* Native Raster (integer matrix with class 'nativeRaster')
* Raster (character matrix with class 'raster') with hex color values of the 
  form `#RRGGBBAA` or `#RRGGBB`.  Note: R color names are not supported here.
* 3D numeric array containing RGBA values in the range [0, 1]
* 3D numeric array containing RGB values in the range [0, 1]
* 2D numeric matrix containing greyscale values in the range [0, 1] 

# spng 0.1.2

* Allow user to choose `fmt` and `flags` for PNG decoding


# spng 0.1.1

* Get meta-info (width, height, etc) from image.


# spng 0.1.0

* Initial release

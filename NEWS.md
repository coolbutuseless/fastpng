
# fastpng 0.1.3.9009 2024-03-19

* normlize pathnames

# fastpng 0.1.3.9008 2024-02-10

* Adding `raw()` test images
* More tests
* some bug fixes for 16 bit read/write

# fastpng 0.1.3.9007 2024-02-10

* Fix a couple of bugs reading 16 bit PNGs
* Fix some bugs when setting `rgba = TRUE`
* More tests 

# fastpng 0.1.3.9006 2024-02-09

* Support 16-bit raw vectors 

# fastpng 0.1.3.9005 2024-02-08

* Refactored array transposition to be more compact
* Support read/write from integer arrays.  For 8bit integer arrays, values should be
  in range [0,255].  For 16bit integer arrays, values should be in 
  range [0,65535]

# fastpng 0.1.3.9004 2024-02-08

* Read/Write PNGs with `raw()` vectors.  Raw vectors contain the pixel data in 
  row-major packed pixel format e.g. `RGBARGBARGBA`.

# fastpng 0.1.3.9003 2024-02-07

* Writing now supports specification of a single colour for transparency 
 in RGB and GrayScale images.  New argument: `trns` 

# fastpng 0.1.3.9002 2024-02-06

* Read/write 16-bit PNGs from/to arrays
* raster and nativeraster will read 16-bit PNGs with 8bit precision.
* PNG does not support indexed images at 16bits.

# fastpng 0.1.3.9001 2024-02-05

* Indexed PNGs now support alpha by including `tRNS` chunk

# fastpng 0.1.3.9000 2024-02-04

* Read/Write indexed PNG
* Fix GA8 reading

# fastpng 0.1.3  2024-02-03

* `read_png()` to read a PNG from a file or a raw vector
* `write_png()` to write data as a PNG file or PNG data in a raw vector
* `get_png_info()` - interrogate a vector of raw values containing a PNG image
  to determine image information i.e. width, height, bit_depth, color_type, 
  compression_method, filter_method, interlace_method.
  
Supported R image types:

* Native Raster (integer matrix with class 'nativeRaster')
* Raster (character matrix with class 'raster') with hex colour values of the 
  form `#RRGGBBAA` or `#RRGGBB`.  Note: R colour names are not supported here.
* 3D numeric array containing RGBA values in the range [0, 1]
* 3D numeric array containing RGB values in the range [0, 1]
* 2D numeric matrix containing greyscale values in the range [0, 1] 

# spng 0.1.2

* Allow user to choose `fmt` and `flags` for PNG decoding


# spng 0.1.1

* Get meta-info (width, height, etc) from image.


# spng 0.1.0

* Initial release

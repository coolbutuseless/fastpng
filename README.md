
<!-- README.md is generated from README.Rmd. Please edit that file -->

# spng

<!-- badges: start -->

![](https://img.shields.io/badge/cool-useless-green.svg)
[![R-CMD-check](https://github.com/coolbutuseless/spng/actions/workflows/R-CMD-check.yaml/badge.svg)](https://github.com/coolbutuseless/spng/actions/workflows/R-CMD-check.yaml)
<!-- badges: end -->

`{spng}` provides functions for read/write of PNG image from files and
raw vectors.

In contrast to the standard [`{png}`]() library, `{spng}`

- Provides explicit support for images as rasters, native rasters,
  numeric RGBA arrays and numeric RGB arrays.
- Flags to configure reading of PNG
  - gamma correction
- Flags to configure writing of PNG
  - Compression level
  - PNG filter settings

`spng` is a R wrapper for
[libspng](https://github.com/randy408/libspng) - current v0.7.4

- [libspng API docs](https://libspng.org/docs/api/)

<img src="man/figures/README-unnamed-chunk-2-1.png" width="100%" />

<img src="man/figures/README-unnamed-chunk-3-1.png" width="100%" />

## ToDo

- 16 bit support
- Palette based images
- Write transparency via the `tRNS` chunk

## Installation

You can install from [GitHub](https://github.com/coolbutuseless/spng)
with:

``` r
# install.package('remotes')
remotes::install_github('coolbutuseless/spng')
```

## What’s in the box

- `read_png(src, type, flags)`
- `write_png(image)`
- `get_png_info(src)` - interrogate a vector of raw values containing a
  PNG image to determine image information i.e. width, height,
  bit_depth, color_type, compression_method, filter_method,
  interlace_method.

Supported R image types:

- Native Raster (integer matrix with class ‘nativeRaster’)
- Raster (character matrix with class ‘raster’) with hex colour vaules
  “\#RRGGBBAA”
- RGBA 3D numeric array values in \[0, 1\]
- RGB 3D numeric array values in \[0, 1\]
- Grey 2D numeric matrix values in \[0, 1\] (write only)

## Example: Decompress a PNG in memory

``` r
library(spng)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# A PNG file everyone should have!
# Read in the raw bytes
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
png_file <- system.file("img", "Rlogo.png", package="png")
png_data <- readBin(png_file, 'raw', n = file.size(png_file))
png_data[1:100]
#>   [1] 89 50 4e 47 0d 0a 1a 0a 00 00 00 0d 49 48 44 52 00 00 00 64 00 00 00 4c 08
#>  [26] 06 00 00 00 9b 1d 12 0f 00 00 00 06 62 4b 47 44 00 ff 00 ff 00 ff a0 bd a7
#>  [51] 93 00 00 00 09 70 48 59 73 00 00 2e 23 00 00 2e 23 01 78 a5 3f 76 00 00 00
#>  [76] 07 74 49 4d 45 07 d5 02 10 10 08 0e 97 b9 27 bc 00 00 20 00 49 44 41 54 78
```

``` r
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Get info about the PNG 
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
(get_png_info <- spng::get_png_info(png_data))
#> $width
#> [1] 100
#> 
#> $height
#> [1] 76
#> 
#> $bit_depth
#> [1] 8
#> 
#> $color_type
#> [1] 6
#> 
#> $compression_method
#> [1] 0
#> 
#> $filter_method
#> [1] 0
#> 
#> $interlace_method
#> [1] 0
#> 
#> $color_desc
#> [1] "SPNG_COLOR_TYPE_TRUECOLOR_ALPHA"
#> 
#> $filter_desc
#> [1] "SPNG_FILTER_NONE"
#> 
#> $interlate_desc
#> [1] "SPNG_INTERLACE_NONE"
```

### Read PNG as native raster

``` r
nara <- read_png(png_data, type = 'nara')
nara[1:10, 1:10]
#>       [,1] [,2]        [,3]        [,4] [,5] [,6]     [,7]     [,8] [,9] [,10]
#>  [1,]    0    0    -9406092           0    0    0 -8091002 -7695987    0     0
#>  [2,]    0    0    -9340300           0    0    0 -8156795 -7630450    0     0
#>  [3,]    0    0    -9340556    29278912    0    0 -8288124 -7630194    0     0
#>  [4,]    0    0    -9406349   747804820    0    0 -8353917 -7564401    0     0
#>  [5,]    0    0    -9472142  1837534601    0    0 -8485503 -7498608    0     0
#>  [6,]    0    0    -9537678 -1367110777    0    0 -8551296 -7432815    0     0
#>  [7,]    0    0    -9537679  -410941308    0    0 -8682881 -7301229    0     0
#>  [8,]    0    0  -126846604    -8551039    0    0 -8814211 -7235436    0     0
#>  [9,]    0    0  -663651979    -8616832    0    0 -9011590 -7169644    0     0
#> [10,]    0    0 -1317963403    -8617088    0    0 -9143177 -7169643    0     0
grid::grid.raster(nara, interpolate = FALSE)
```

<img src="man/figures/README-unnamed-chunk-5-1.png" width="100%" />

### Read PNG as raster

``` r
ras <- read_png(png_data, type = 'raster')
ras[1:10, 1:10]
#>       [,1]        [,2]        [,3]        [,4]        [,5]        [,6]       
#>  [1,] "#00000000" "#00000000" "#00000000" "#00000000" "#00000000" "#00000000"
#>  [2,] "#00000000" "#00000000" "#00000000" "#00000000" "#00000000" "#00000000"
#>  [3,] "#00000000" "#00000000" "#00000000" "#00000000" "#00000000" "#00000000"
#>  [4,] "#00000000" "#00000000" "#00000000" "#00000000" "#00000000" "#00000000"
#>  [5,] "#00000000" "#00000000" "#00000000" "#00000000" "#00000000" "#00000000"
#>  [6,] "#00000000" "#00000000" "#00000000" "#00000000" "#00000000" "#00000000"
#>  [7,] "#00000000" "#00000000" "#00000000" "#00000000" "#00000000" "#00000000"
#>  [8,] "#00000000" "#00000000" "#00000000" "#00000000" "#00000000" "#00000000"
#>  [9,] "#00000000" "#00000000" "#00000000" "#00000000" "#00000000" "#00000000"
#> [10,] "#00000000" "#00000000" "#00000000" "#00000000" "#00000000" "#00000000"
#>       [,7]        [,8]        [,9]        [,10]      
#>  [1,] "#00000000" "#00000000" "#00000000" "#00000000"
#>  [2,] "#00000000" "#00000000" "#00000000" "#00000000"
#>  [3,] "#00000000" "#00000000" "#00000000" "#00000000"
#>  [4,] "#00000000" "#00000000" "#00000000" "#00000000"
#>  [5,] "#00000000" "#00000000" "#00000000" "#00000000"
#>  [6,] "#00000000" "#00000000" "#00000000" "#00000000"
#>  [7,] "#00000000" "#00000000" "#00000000" "#00000000"
#>  [8,] "#00000000" "#00000000" "#00000000" "#00000000"
#>  [9,] "#00000000" "#00000000" "#00000000" "#00000000"
#> [10,] "#00000000" "#00000000" "#00000000" "#00000000"
plot(ras, interpolate = FALSE)
```

<img src="man/figures/README-unnamed-chunk-8-1.png" width="100%" />

### Read PNG as RGBA array

``` r
arr <- read_png(png_data, type = 'array')
arr[1:10, 1:10, 1]
#>       [,1] [,2] [,3] [,4] [,5] [,6] [,7] [,8] [,9] [,10]
#>  [1,]    0    0    0    0    0    0    0    0    0     0
#>  [2,]    0    0    0    0    0    0    0    0    0     0
#>  [3,]    0    0    0    0    0    0    0    0    0     0
#>  [4,]    0    0    0    0    0    0    0    0    0     0
#>  [5,]    0    0    0    0    0    0    0    0    0     0
#>  [6,]    0    0    0    0    0    0    0    0    0     0
#>  [7,]    0    0    0    0    0    0    0    0    0     0
#>  [8,]    0    0    0    0    0    0    0    0    0     0
#>  [9,]    0    0    0    0    0    0    0    0    0     0
#> [10,]    0    0    0    0    0    0    0    0    0     0
plot(as.raster(arr), interpolate = FALSE)
```

<img src="man/figures/README-unnamed-chunk-9-1.png" width="100%" />

## Acknowledgements

- R Core for developing and maintaining the language.
- CRAN maintainers, for patiently shepherding packages onto CRAN and
  maintaining the repository


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

<img src="man/figures/README-unnamed-chunk-2-1.png" width="80%" />

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
grid::grid.raster(nara, interpolate = FALSE)
```

<img src="man/figures/README-unnamed-chunk-4-1.png" width="80%" />

### Read PNG as raster

``` r
ras <- read_png(png_data, type = 'raster')
plot(ras, interpolate = FALSE)
```

<img src="man/figures/README-unnamed-chunk-7-1.png" width="80%" />

### Read PNG as RGBA array

``` r
arr <- read_png(png_data, type = 'rgba')
plot(as.raster(arr), interpolate = FALSE)
```

<img src="man/figures/README-unnamed-chunk-8-1.png" width="80%" />

### Read PNG as RGB array

Ignoring any alpha channel in the image.

``` r
arr <- read_png(png_data, type = 'rgb')
plot(as.raster(arr), interpolate = FALSE)
```

<img src="man/figures/README-unnamed-chunk-9-1.png" width="80%" />

## Acknowledgements

- R Core for developing and maintaining the language.
- CRAN maintainers, for patiently shepherding packages onto CRAN and
  maintaining the repository


<!-- README.md is generated from README.Rmd. Please edit that file -->

# fastpng

<!-- badges: start -->

![](https://img.shields.io/badge/cool-useless-green.svg)
[![R-CMD-check](https://github.com/coolbutuseless/fastpng/actions/workflows/R-CMD-check.yaml/badge.svg)](https://github.com/coolbutuseless/fastpng/actions/workflows/R-CMD-check.yaml)
<!-- badges: end -->

`{fastpng}` reads and writes PNG images.

`{fastpng}` exposes configuration options so that the user can make a
trade-off between speed of writing and PNG size. These options include:

- Compression level
- Filter use
- Image transposition

For example, writing uncompressed PNG images can be 100x faster than
writing with regular compression settings.

`fastpng` is an R wrapper for
[libspng](https://github.com/randy408/libspng) - current v0.7.4

## Features

### Supported image data in R

Supported images each have examples in the `test_image` as part of this
package.

- **Native rasters**
- **Rasters**
  - With hex colour formats: \#RGB, \#RGBA, \#RRGGBB, \#RRGGBBAA
  - Standard R colour names also supported e.g. ‘red’, ‘white’
- **Numeric arrays**
  - Values in range \[0,1\]
  - 1-, 2-, 3- and 4-plane numeric arrays (interpreted as gray,
    gray+alpha, RGB and RGBA images)
- **Integer arrays**
  - Values in range \[0,255\] treated as 8-bit values
  - Values in range \[0,65535\] treated as 16-bit for PNG writing
- **Integer matrix + an indexed palette of colours**
- **Raw vectors** with a specification for data layout

### Supported PNG image types

- 8-bit and 16-bit PNGs
- RGBA, RGB, Gray + Alpha, Gray PNGs
- Indexed colour PNGs
- RGB PNGs with a specified transparency colour (using [tRNS
  chunk](https://www.w3.org/TR/PNG-Chunks))

### Comparison to standard `{png}` library

|                            | `{fastpng}` | `{png}` |
|:---------------------------|-------------|---------|
| Numeric arrays             | Yes         | Yes     |
| Native rasters             | Yes         | Yes     |
| Rasters                    | Yes         |         |
| Integer Arrays             | Yes         |         |
| Indexed PNGs               | Yes         |         |
| `tRNS` transparency        | Yes         |         |
| Configurable compression   | Yes         |         |
| Configurable filtering     | Yes         |         |
| Configurable transposition | Yes         |         |

## Compression Settings: Speed / size tradeoff

<details>
<summary>
Click to reveal benchmark code and results table
</summary>

``` r
library(png)
im <- test_image$array$rgba

res <- bench::mark(
  fastpng::write_png(im, compression_level =  0),
  fastpng::write_png(im, compression_level =  1),
  fastpng::write_png(im, compression_level =  2),
  fastpng::write_png(im, compression_level =  3),
  fastpng::write_png(im, compression_level =  4),
  fastpng::write_png(im, compression_level =  5),
  fastpng::write_png(im, compression_level =  6),
  fastpng::write_png(im, compression_level =  7),
  fastpng::write_png(im, compression_level =  8),
  fastpng::write_png(im, compression_level =  9),
  check = FALSE,
  relative = FALSE
)

res <- res %>% 
  select(writes_per_second = `itr/sec`) %>%
  mutate(
    compression = 0:9,
    package = "fastpng"
  )


sizes <- vapply(0:9, \(x) fastpng::write_png(im, compression_level = x) |> length(), integer(1))
df <- data.frame(compression = 0:9, size = sizes)


plot_df <- left_join(res, df, by = 'compression')


png_speed <- bench::mark(writePNG(im))$`itr/sec`
png_size  <- length(writePNG(im))

plot_df <- plot_df %>% 
  add_row(writes_per_second = png_speed, size = png_size, package = "png") %>%
  mutate(
    compression_ratio = prod(dim(im)) * 8 / size
  )

knitr::kable(plot_df)
```

| writes_per_second | compression | package |   size | compression_ratio |
|------------------:|------------:|:--------|-------:|------------------:|
|        6279.13219 |           0 | fastpng | 240651 |          7.978359 |
|         293.24912 |           1 | fastpng |  62456 |         30.741642 |
|         253.79242 |           2 | fastpng |  58017 |         33.093748 |
|         159.59145 |           3 | fastpng |  54119 |         35.477374 |
|         182.61166 |           4 | fastpng |  46436 |         41.347231 |
|         121.17569 |           5 | fastpng |  43177 |         44.468120 |
|          63.98832 |           6 | fastpng |  41303 |         46.485727 |
|          41.11499 |           7 | fastpng |  40799 |         47.059977 |
|          14.10135 |           8 | fastpng |  40758 |         47.107316 |
|          12.91560 |           9 | fastpng |  40776 |         47.086522 |
|          67.35916 |          NA | png     |  41303 |         46.485727 |

</details>

<img src="man/figures/README-unnamed-chunk-3-1.png" width="100%" />

## Installation

You can install from [GitHub](https://github.com/coolbutuseless/fastpng)
with:

``` r
# install.package('remotes')
remotes::install_github('coolbutuseless/fastpng')
```

## What’s in the box

- `read_png()` to read a PNG from a file or a raw vector
- `write_png()` to write an R image as a PNG file or PNG data in a raw
  vector
- `get_png_info()` - interrogate a vector of raw values containing a PNG
  image to determine image information i.e. width, height, bit_depth,
  color_type, compression_method, filter_method, interlace_method.
- `test_image` is a list of images. These are images contained in
  different datastructures and of differing bitdepth etc: RGBA and RGB
  numeric arrays, raster, native raster.

## Example: Read a PNG into R

``` r
library(fastpng)
png_file <- system.file("img", "Rlogo.png", package="png")
fastpng::get_png_info(png_file)
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

``` r

ras <- read_png(png_file, type = 'raster') 
grid::grid.raster(ras, interpolate = FALSE)
```

<img src="man/figures/README-unnamed-chunk-4-1.png" width="100%" />

#### Read as a raster (of hex colours)

``` r
ras <- read_png(png_file, type = "raster")
ras[7:11, 79:83]
#>      [,1]        [,2]        [,3]        [,4]        [,5]       
#> [1,] "#686D6597" "#7579711F" "#00000000" "#00000000" "#00000000"
#> [2,] "#5F645CFF" "#5D635AF9" "#63696098" "#7B80781C" "#00000000"
#> [3,] "#686D64FF" "#61665DFF" "#5B6158FF" "#5C6158F5" "#656A6280"
#> [4,] "#71766EFF" "#6B6F67FF" "#656A61FF" "#5E635BFF" "#595E55FF"
#> [5,] "#797D75FF" "#747971FF" "#6E736AFF" "#686D64FF" "#60655DFF"
```

#### Read as a numeric array

``` r
ras <- read_png(png_file, type = "array")
ras[7:11, 79:83, 1] # red channel
#>           [,1]      [,2]      [,3]      [,4]      [,5]
#> [1,] 0.4078431 0.4588235 0.0000000 0.0000000 0.0000000
#> [2,] 0.3725490 0.3647059 0.3882353 0.4823529 0.0000000
#> [3,] 0.4078431 0.3803922 0.3568627 0.3607843 0.3960784
#> [4,] 0.4431373 0.4196078 0.3960784 0.3686275 0.3490196
#> [5,] 0.4745098 0.4549020 0.4313725 0.4078431 0.3764706
```

#### Read as an integer array

``` r
ras <- read_png(png_file, type = "array", array_type = 'integer')
ras[7:11, 79:83, 1] # red channel
#>      [,1] [,2] [,3] [,4] [,5]
#> [1,]  104  117    0    0    0
#> [2,]   95   93   99  123    0
#> [3,]  104   97   91   92  101
#> [4,]  113  107  101   94   89
#> [5,]  121  116  110  104   96
```

#### Read as a native raster

``` r
im <- read_png(png_file, type = "nativeraster")
im[7:11, 79:83]
#>          [,1] [,2] [,3]     [,4]      [,5]
#> [1,] -7235693    0    0 -7301485  -7767184
#> [2,] -7367279    0    0 -7367022  -5864589
#> [3,] -7498608    0    0 -7301485  -4219764
#> [4,] -7432815    0    0 -7235692   -995135
#> [5,] -6648959    0    0 -7501694 -10268343
```

## Write an image to PNG with/without compression

``` r
png_file <- tempfile()
write_png(im, png_file)  # standard compression
file.size(png_file)
#> [1] 11938
```

``` r
write_png(im, png_file, compression_level = 0) # no compression, but fast!
file.size(png_file)
#> [1] 30580
```

## Write integer matrix as indexed PNG

``` r
indices <- test_image$indexed$integer_index
palette <- test_image$indexed$palette

dim(indices)
#> [1] 200 300
```

``` r
indices[1:10, 1:10]
#>       [,1] [,2] [,3] [,4] [,5] [,6] [,7] [,8] [,9] [,10]
#>  [1,]    0    0    0    0    0    0    0    0    0     0
#>  [2,]    0    0    0    0    0    0    0    0    0     0
#>  [3,]    0    0    0    0    0    0    0    0    0     1
#>  [4,]    0    0    0    0    0    0    0    1    1     1
#>  [5,]    0    0    0    0    0    1    1    1    1     1
#>  [6,]    0    0    0    0    1    1    1    1    1     1
#>  [7,]    0    0    0    0    1    1    1    1    1     1
#>  [8,]    0    0    0    1    1    1    1    1    1     1
#>  [9,]    0    0    0    1    1    1    1    1    1     2
#> [10,]    0    0    1    1    1    1    1    1    2     2
```

``` r
palette[1:10]
#>  [1] "#440154FF" "#440256FF" "#450457FF" "#450559FF" "#46075AFF" "#46085CFF"
#>  [7] "#460A5DFF" "#460B5EFF" "#470D60FF" "#470E61FF"
```

``` r
tmp <- tempfile()
write_png(image = indices, palette = palette, file = tmp)
```

<img src="man/figures/README-unnamed-chunk-13-1.png" width="100%" />

## Acknowledgements

- R Core for developing and maintaining the language.
- CRAN maintainers, for patiently shepherding packages onto CRAN and
  maintaining the repository

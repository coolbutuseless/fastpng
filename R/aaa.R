#' @useDynLib spng, .registration=TRUE
NULL



#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Decode PNG as the specified format type
#'
#' A good default is \code{SPNG_FMT_RGBA8}
#'
#' \describe{
#'   \item{SPNG_FMT_RGBA8  }{RGBA color.  8-bit color. Convert from any PNG format and bit depth to this format}
#'   \item{SPNG_FMT_RGBA16 }{RGBA color.  8-bit color. Convert from any PNG format and bit depth to this format}
#'   \item{SPNG_FMT_RGB8   }{RGB color . 16-bit color. Convert from any PNG format and bit depth to this format}
#'   \item{SPNG_FMT_GA8    }{Only valid for 1, 2, 4, 8-bit grayscale PNGs}
#'   \item{SPNG_FMT_GA16   }{Only valid for 16-bit grayscale PNGs}
#'   \item{SPNG_FMT_G8     }{Only valid for 1, 2, 4, 8-bit grayscale PNGs}
#'   \item{SPNG_FMT_PNG    }{The PNG’s format in host-endian}
#'   \item{SPNG_FMT_RAW    }{The PNG’s format in big-endian}
#' }
#'
#' @export
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
spng_format <- list(
  SPNG_FMT_RGBA8  =   1L,
  SPNG_FMT_RGBA16 =   2L,
  SPNG_FMT_RGB8   =   4L,
  SPNG_FMT_GA8    =  16L,
  SPNG_FMT_GA16   =  32L,
  SPNG_FMT_G8     =  64L,
  SPNG_FMT_PNG    = 256L, # No conversion or scaling - host-endian
  SPNG_FMT_RAW    = 512L  # No conversion or scaling - big-endian
)



#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Flags for decoding
#'
#' \describe{
#'   \item{SPNG_DECODE_TRNS}{Apply transparency}
#'   \item{SPNG_DECODE_GAMMA}{Apply gamma correction}
#' }
#'
#' @export
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
spng_decode_flags <- list(
    SPNG_DECODE_TRNS        =   1L, # Apply transparency
    SPNG_DECODE_GAMMA       =   2L  # Apply gamma correction
    #SPNG_DECODE_PROGRESSIVE = 256L # Initialize for progressive reads
)

#' @useDynLib spng, .registration=TRUE
NULL


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


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Normalize named R colors to hex colors of the form \code{#RRGGBB}
#' 
#' @param x vector, matrix or raster which includes named colors like 'white'
#' @param alpha logical. Should the alpha value be included
#' 
#' @return new data with named R colors replaced by their hex value
#' @examples
#' normalize_colors(matrix(c('red', 'white', '#444444'), 3, 4))
#' normalize_colors(matrix(c('red', 'white', '#444444'), 3, 4), alpha = TRUE)
#' @importFrom grDevices colors
#' @export
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
normalize_colors <- function(x, alpha = FALSE) {
  att <- attributes(x)
  
  if (requireNamespace('farver', quietly = TRUE)) {
    tmp <- farver::decode_colour(x, alpha = alpha)
    if (alpha) {
      x <- farver::encode_colour(tmp, alpha = TRUE)
    } else {
      x <- farver::encode_colour(tmp)
    }
  } else {
    stop("Please install the 'farver' package")
  }
  
  attributes(x) <- att
  x
}

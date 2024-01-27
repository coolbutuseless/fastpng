
color_types <- list()

color_types[[0 + 1]] <- "Grayscale"
color_types[[2 + 1]] <- "RGB"
color_types[[3 + 1]] <- "Paletted color"
color_types[[4 + 1]] <- "Grayscale + Alpha"
color_types[[6 + 1]] <- "RGB + Alpha"





#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Get information about a PNG file
#'
#' @param src filename or raw vector containing PNG data
#'
#' @export
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
get_png_info <- function(src) {
  res <- .Call('get_png_info_', src)

  res$color_desc <- color_types[[res$color_type + 1L]]

  res
}

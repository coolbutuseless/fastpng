
color_types <- list()

color_types[[0 + 1]] <- "Grayscale"
color_types[[2 + 1]] <- "RGB"
color_types[[3 + 1]] <- "Paletted color"
color_types[[4 + 1]] <- "Grayscale + Alpha"
color_types[[6 + 1]] <- "RGB + Alpha"





#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Get information about a PNG file
#'
#' @inheritParams read_png_as_raw
#'
#' @export
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
extract_png_info <- function(raw_vec) {
  res <- .Call('extract_png_info_', raw_vec)

  res$color_desc <- color_types[[res$color_type + 1L]]

  res
}

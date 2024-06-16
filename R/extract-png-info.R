

spng_color_type <- list()
spng_color_type[[1L + 0]] <- "SPNG_COLOR_TYPE_GRAYSCALE"
spng_color_type[[1L + 2]] <- "SPNG_COLOR_TYPE_TRUECOLOR"
spng_color_type[[1L + 3]] <- "SPNG_COLOR_TYPE_INDEXED"
spng_color_type[[1L + 4]] <- "SPNG_COLOR_TYPE_GRAYSCALE_ALPHA"
spng_color_type[[1L + 6]] <- "SPNG_COLOR_TYPE_TRUECOLOR_ALPHA"


spng_filter <- list()
spng_filter[[1L + 0]] <- "SPNG_FILTER_NONE" 
spng_filter[[1L + 1]] <- "SPNG_FILTER_SUB" 
spng_filter[[1L + 2]] <- "SPNG_FILTER_UP" 
spng_filter[[1L + 3]] <- "SPNG_FILTER_AVERAGE" 
spng_filter[[1L + 4]] <- "SPNG_FILTER_PAETH" 

spng_interlace_method <- list(
  'SPNG_INTERLACE_NONE', 
  'SPNG_INTERLACE_ADAM7'
)


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Get information about a PNG file
#'
#' @param src PNG filename or raw vector containing PNG data
#' 
#' @return Name list of information about the PNG image:
#' \describe{
#'   \item{width,height}{Dimensions of PNG}
#'   \item{bit_depth}{Bit depth. 8 or 16 bits}
#'   \item{color_type,color_desc}{color type and its description}
#'   \item{compression_method}{Compression setting}
#'   \item{filter_method,filter_desc}{Filter method and description}
#'   \item{interlace_method,interlace_desc}{Interlace method and description}
#' }
#' 
#' @examples
#' # Create a small grayscale PNG image and fetch its PNG info
#' mat <- matrix(c(0L, 255L), 3, 4)
#' png_data <- write_png(mat)
#' get_png_info(png_data)
#' 
#' @export
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
get_png_info <- function(src) {
  res <- .Call('get_png_info_', src)

  res$color_desc <- spng_color_type[[res$color_type + 1L]]
  res$filter_desc <- spng_filter[[res$filter_method + 1L]]
  res$interlate_desc <- spng_interlace_method[[res$interlace_method + 1L]]

  res
}

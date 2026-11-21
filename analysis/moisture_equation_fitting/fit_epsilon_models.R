#!/usr/bin/env Rscript

# Required Excel columns:
#   e_prime, e_double_prime, theta
#
# theta is the supplied target. It is NOT calculated from e_prime/e_double_prime.
# ea is calculated from the supplied sticky-note formula.

args <- commandArgs(trailingOnly = TRUE)
input_file <- if (length(args) >= 1) args[1] else "C:/Users/User/Desktop/Projects/thesis/soil-moisture-sensor-thesis/analysis/moisture_equation_fitting/dummy_epsilon_data.xlsx"
output_dir <- if (length(args) >= 2) args[2] else "C:/Users/User/Desktop/Projects/thesis/soil-moisture-sensor-thesis/analysis/moisture_equation_fitting/output"

if (!requireNamespace("readxl", quietly = TRUE)) {
  stop("Install readxl once with: install.packages('readxl')")
}

dir.create(output_dir, recursive = TRUE, showWarnings = FALSE)

raw <- readxl::read_excel(input_file, sheet = 1)

needed <- c("e_prime", "e_double_prime", "theta")
if (!all(needed %in% names(raw))) {
  stop("The first sheet must contain columns: e_prime, e_double_prime, theta")
}

d <- data.frame(
  e_prime = as.numeric(raw$e_prime),
  e_double_prime = as.numeric(raw$e_double_prime),
  theta = as.numeric(raw$theta)
)

d <- d[complete.cases(d), ]

if (nrow(d) < 5 ||
    any(!is.finite(as.matrix(d))) ||
    any(d$e_prime <= 0)) {
  stop("Use at least five complete numeric rows, with e_prime > 0.")
}

ratio <- d$e_double_prime / d$e_prime

# Sticky-note formula:
# ea = {1 + [1 + (e_double_prime/e_prime)^2]^(1/2)} * e_prime/2

d$ea <- (1 + sqrt(1 + (d$e_double_prime / d$e_prime)^2)) * d$e_prime / 2

if (any(!is.finite(d$ea)) || any(d$ea <= 0)) {
  stop("The ea calculation produced invalid values. Check the input ratios.")
}

fits <- list(
  sqrt_e_prime  = lm(theta ~ sqrt(e_prime), data = d),
  sqrt_ea       = lm(theta ~ sqrt(ea), data = d),
  cubic_e_prime = lm(theta ~ I(e_prime^3) + I(e_prime^2) + e_prime, data = d),
  cubic_ea      = lm(theta ~ I(ea^3) + I(ea^2) + ea, data = d)
)

plot_specs <- data.frame(
  model = names(fits),
  x = c("e_prime", "ea", "e_prime", "ea"),
  file = c(
    "fit_1_sqrt_e_prime.png",
    "fit_2_sqrt_ea.png",
    "fit_3_cubic_e_prime.png",
    "fit_4_cubic_ea.png"
  ),
  title = c(
    "Model 1: theta = a*sqrt(ε'r) + b",
    "Model 2: theta = a*sqrt(εa) + b",
    "Model 3: cubic polynomial in ε'r",
    "Model 4: cubic polynomial in εa"
  ),
  stringsAsFactors = FALSE
)

for (i in seq_len(nrow(plot_specs))) {
  fit <- fits[[plot_specs$model[i]]]
  x_name <- plot_specs$x[i]
  x <- d[[x_name]]

  grid_data <- setNames(
    data.frame(seq(min(x), max(x), length.out = 400)),
    x_name
  )

  png(file.path(output_dir, plot_specs$file[i]),
      width = 1400, height = 900, res = 150)

  plot(
    x, d$theta,
    type = "n",
    xlab = x_name,
    ylab = "θ (teros12)",
    main = plot_specs$title[i],
    sub = sprintf(
      "R-squared = %.6f | RMSE = %.6f",
      summary(fit)$r.squared,
      sqrt(mean(residuals(fit)^2))
    )
  )
  grid()
  points(x, d$theta, pch = 19)
  lines(
    grid_data[[x_name]],
    predict(fit, newdata = grid_data),
    lwd = 2
  )
  legend(
    "topleft",
    legend = c("supplied theta", "fitted curve"),
    pch = c(19, NA),
    lty = c(NA, 1),
    bty = "n"
  )
  dev.off()
}

# Save ea, the supplied target, and all fitted values.
for (model_name in names(fits)) {
  d[[paste0("prediction_", model_name)]] <- predict(fits[[model_name]])
}

write.csv(
  d,
  file.path(output_dir, "data_with_ea_and_predictions.csv"),
  row.names = FALSE
)

# Save model-comparison metrics.
metrics <- do.call(rbind, lapply(names(fits), function(model_name) {
  fit <- fits[[model_name]]
  s <- summary(fit)

  data.frame(
    model = model_name,
    formula = paste(deparse(formula(fit)), collapse = ""),
    n = nobs(fit),
    r_squared = s$r.squared,
    adjusted_r_squared = s$adj.r.squared,
    rmse = sqrt(mean(residuals(fit)^2)),
    aic = AIC(fit)
  )
}))

write.csv(
  metrics,
  file.path(output_dir, "fit_metrics.csv"),
  row.names = FALSE
)

# Save coefficients with names matching the handwritten equations.
parameter_names <- list(
  sqrt_e_prime = c(
    "(Intercept)" = "beta1",
    "sqrt(e_prime)" = "alpha1"
  ),
  sqrt_ea = c(
    "(Intercept)" = "beta2",
    "sqrt(ea)" = "alpha2"
  ),
  cubic_e_prime = c(
    "(Intercept)" = "d1",
    "I(e_prime^3)" = "a1",
    "I(e_prime^2)" = "b1",
    "e_prime" = "gamma1"
  ),
  cubic_ea = c(
    "(Intercept)" = "d2",
    "I(ea^3)" = "a2",
    "I(ea^2)" = "b2",
    "ea" = "gamma2"
  )
)

coefficients <- do.call(rbind, lapply(names(fits), function(model_name) {
  b <- coef(fits[[model_name]])

  data.frame(
    model = model_name,
    parameter = unname(parameter_names[[model_name]][names(b)]),
    term = names(b),
    estimate = unname(b)
  )
}))

write.csv(
  coefficients,
  file.path(output_dir, "fit_coefficients.csv"),
  row.names = FALSE
)

# Save the standard lm() reports.
capture.output({
  for (model_name in names(fits)) {
    cat("\n", strrep("=", 60), "\n", model_name, "\n", sep = "")
    print(summary(fits[[model_name]]))
  }
}, file = file.path(output_dir, "model_summaries.txt"))

cat("Done. Results saved in:", normalizePath(output_dir), "\n")

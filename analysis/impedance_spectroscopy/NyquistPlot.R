# List of files
files <- paste0("C:/Users/User/Desktop/variations/Trace0", 1:2, ".csv")

# Initialize result
result <- NULL

# Loop through each file
for (f in files) {
  df <- read.csv(f, skip = 4, header = TRUE, stringsAsFactors = FALSE)

  # Drop NA rows
  df <- na.omit(df)

  # Convert numeric columns
  df[] <- lapply(df, function(x) {
    if (all(grepl("^[-+]?[0-9]*\\.?[0-9]+$", x))) {
      as.numeric(x)
    } else {
      x
    }
  })

  # Append
  if (is.null(result)) {
    result <- df
  } else {
    if (!all(colnames(result) == colnames(df))) {
      stop(paste("Column names of file", f, "do not match."))
    }
    result <- rbind(result, df)
  }
}

# Save combined file
write.csv(result, "C:/Users/User/Desktop/data/result.csv", row.names = FALSE)

# ---- Impedance conversion ----

# Degrees → radians
convert_to_rad <- function(deg) {
  deg * pi / 180
}

theta_rad <- convert_to_rad(result$theta.z.deg..data)
Z <- result$X.Z..Ohm..data

# Define components
df1 <- Z * cos(theta_rad)     # Z'
df2 <- -Z * sin(theta_rad)    # -Z''

# Create dataframe
impedance_df <- data.frame(
  Z_real = df1,      # Z'
  Z_imag = df2       # -Z''
)

# Plot Nyquist plot
plot(
  impedance_df$Z_real,
  impedance_df$Z_imag,
  xlab = "Z' (Ohm)",
  ylab = "-Z'' (Ohm)",


)



p <- ggplot(df, aes(x = x, y = y)) +
  geom_line(color = "blue", size = 1.5) +
  labs(x = "X-axis", y = "Y-axis", title = "Line Plot of Y vs X") +
  theme_minimal()

# Save to PNG
ggsave("C:/Users/User/Desktop/data/result_new_plot_data_analysis_eketa_2025_meh.png", plot = p, width = 8, height = 6, dpi = 300)

# ==========================================
# Mean Absolute Error (MAE)
# Sensor measurements vs. TEROS 12
# ==========================================

# 1. Enter your measurements manually
sensor_measurements <- c(0.055, 0.071, 0.083, 0.11, 0.132, 0.181, 0.234, 0.27, 0.312, 0.318, 0.32)

teros12_measurements <- c(0.064, 0.081, 0.09, 0.1, 0.128, 0.171, 0.21, 0.272, 0.294, 0.314, 0.338)

# 2. Check that both sets have the same number of values
if (length(sensor_measurements) != length(teros12_measurements)) {
  stop("Error: Both sets must contain the same number of measurements.")
}

# 3. Calculate the absolute errors
absolute_errors <- abs(sensor_measurements - teros12_measurements)

# 4. Calculate the Mean Absolute Error
MAE <- mean(absolute_errors)

# 5. Display the results
cat("Sensor measurements:\n")
print(sensor_measurements)

cat("\nTEROS 12 measurements:\n")
print(teros12_measurements)

cat("\nAbsolute errors:\n")
print(absolute_errors)

cat("\nMean Absolute Error (MAE):", round(MAE, 4), "\n")
# Load data
file <- "C:/Users/User/Desktop/new-dataset/trace60.csv"
df <- read.csv(file, skip = 4, header = TRUE, stringsAsFactors = FALSE)

# Select columns
zreal  <- df$X.Z..Ohm..data
zideal <- df$X.Z..Ohm..mem

ph_real  <- df$theta.z.deg..data
ph_ideal <- df$theta.z.deg..mem

# Combine into a data frame and drop NAs
clean_df <- na.omit(data.frame(zreal, zideal, ph_real, ph_ideal))

# MSE function
mse <- function(a, b) mean((a - b)^2)

mse_z  <- mse(clean_df$zreal, clean_df$zideal)
mse_ph <- mse(clean_df$ph_real, clean_df$ph_ideal)

cat("\n========== Error Metrics ==========\n")
cat(sprintf("MSE (Impedance Magnitude): %.6f\n", mse_z))
cat(sprintf("MSE (Phase Angle): %.6f\n", mse_ph))
cat("===================================\n")

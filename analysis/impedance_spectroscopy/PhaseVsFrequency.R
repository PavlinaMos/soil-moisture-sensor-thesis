df_1 <- read.csv(
  "C:/Users/User/Desktop/data/result.csv",
  header = TRUE,
  stringsAsFactors = FALSE
)

# Define variables with meaningful names
Frequency_Hz <- df_1[['Frequency.Hz.']]
Z_Ohm <- df_1[['X.Z..Ohm..data']]
Theta_Z_deg <- df_1[['theta.z.deg..data']]


# Plot 2: Z (Ohm) vs Frequency
plot(
  Frequency_Hz, Z_Ohm,
  xlab = "Frequency (Hz)",
  ylab = "Z (Ohm)",

)

# Plot 1: Theta Z vs Frequency
plot(
  Frequency_Hz, Theta_Z_deg,
  xlab = "Frequency (Hz)",
  ylab = "Phase (degrees)",

)

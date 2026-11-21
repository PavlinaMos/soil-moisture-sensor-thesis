
sensor <- c(0.055, 0.071, 0.083, 0.110, 0.132,
            0.181, 0.234, 0.270, 0.312, 0.318, 0.320)

teros <- c(0.064, 0.081, 0.090, 0.100, 0.128,
           0.171, 0.210, 0.272, 0.294, 0.314, 0.338)

draw_plot <- function() {
  par(mar = c(5, 5.5, 1, 2), xaxs = "i", yaxs = "i")
  
  plot(teros, sensor,
       xlab = expression(theta[TEROS12] ~ (m^3/m^3)),
       ylab = expression(theta[sensor] ~ (m^3/m^3)),
       pch = 16, cex = 0.65,
       xlim = c(0, 0.4), ylim = c(0, 0.4),
       asp = 1, main = "",
       cex.lab = 1.1, cex.axis = 0.9,
       las = 1, bty = "l",
       xaxt = "n", yaxt = "n")
  
  axis(1, at = seq(0, 0.4, 0.1),
       labels = sprintf("%.1f", seq(0, 0.4, 0.1)))
  axis(2, at = seq(0, 0.4, 0.1),
       labels = sprintf("%.1f", seq(0, 0.4, 0.1)),
       las = 1)
  
  abline(0, 1, col = "red", lty = 2, lwd = 1)
}

# Αποθήκευση
png("sensor_vs_teros.png",
    width = 1800, height = 1800, res = 300)
draw_plot()
dev.off()

# Εμφάνιση στο RStudio Plots
if (dev.cur() != 1) dev.off()
dev.new()
draw_plot()


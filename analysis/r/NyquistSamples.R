library(dplyr)
library(ggplot2)

# =========================
# 1. ΠΡΩΤΟ PATH
# =========================
folder1 <- "C:/Users/User/Desktop/variations/"
files1 <- c("Trace31.csv","Trace32.csv","Trace02.csv")
files1 <- paste0(folder1, files1)

# =========================
# 2. ΔΕΥΤΕΡΟ PATH
# =========================
folder2 <- "C:/Users/User/Desktop/data/"
files2 <- c("3.csv","4.csv","5.csv","6.csv")
files2 <- paste0(folder2, files2)

# =========================
# 3. ΣΥΝΑΡΤΗΣΗ ΑΝΑΓΝΩΣΗΣ CSV
# =========================
read_trace <- function(f) {
  df <- read.csv(f, skip = 4, header = TRUE, stringsAsFactors = FALSE)
  df <- na.omit(df)

  df[] <- lapply(df, function(x) {
    if (all(grepl("^[-+]?[0-9]*\\.?[0-9]+$", x))) as.numeric(x) else x
  })

  df$file <- basename(f)
  df
}

# =========================
# 4. ΑΝΑΓΝΩΣΗ ΟΛΩΝ ΤΩΝ ΑΡΧΕΙΩΝ
# =========================
df_list <- c(
  lapply(files1, read_trace),
  lapply(files2, read_trace)
)

result <- bind_rows(df_list)

# =========================
# 5. ΟΜΑΔΟΠΟΙΗΣΗ TRACES
# =========================
result <- result %>%
  mutate(group = case_when(
    file %in% c("Trace01.csv", "Trace02.csv", "Trace03.csv") ~ "gwc 7%",
    file %in% c("1.csv", "2.csv", "3.csv","4.csv", "5.csv","6.csv") ~ "gwc 10%",
    file %in% c("Trace30.csv", "Trace31.csv", "Trace32.csv") ~ "gwc 16%",
    TRUE ~ file
  ))

# =========================
# 6. ΥΠΟΛΟΓΙΣΜΟΣ NYQUIST
# =========================
convert_to_rad <- function(degrees) degrees * pi / 180

result <- result %>%
  mutate(
    rads = convert_to_rad(theta.z.deg..data),
    x = X.Z..Ohm..data * cos(rads),
    y = -X.Z..Ohm..data * sin(rads)
  )

# Αν υπάρχει στήλη συχνότητας
# result <- result %>% arrange(group, freq)

# =========================
# 7. ΔΙΑΓΡΑΜΜΑ NYQUIST
# =========================
result$group <- factor(result$group, levels = c("gwc 7%", "gwc 10%", "gwc 16%"))

my_colors <- c(
  "gwc 7%"  = "#1f78b4",   # blue
  "gwc 10%" = "#33a02c",   # green
  "gwc 16%" = "#e31a1c"    # red
)


ggplot(result, aes(x = x, y = y, color = group, group = group)) +
  geom_line(size = 1.2) +
  scale_color_manual(values = my_colors) +
  labs(
    x = "Z' (Ohm)",
    y = "-Z'' (Ohm)"
  ) +

  theme_minimal() +
  theme(legend.title = element_blank())

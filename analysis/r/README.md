# R analysis scripts

These scripts were used to process impedance measurements and create plots for
the thesis:

- `MeanSquaredError.R`: compares measured and modeled impedance and phase.
- `NyquistPlot.R`: plots Nyquist traces from CSV exports.
- `NyquistSamples.R`: compares selected trace groups using `dplyr` and `ggplot2`.
- `PhaseVsFrequency.R`: plots impedance magnitude and phase against frequency.

## Requirements

Install R. `NyquistSamples.R` additionally requires these packages:

```r
install.packages(c("dplyr", "ggplot2"))
```

## Input data

The measurement CSV datasets were not included in the supplied archive. The
scripts currently contain the original absolute Windows paths, such as
`C:/Users/User/Desktop/data/`. Before running a script, replace those paths with
the location of the corresponding CSV files on your computer.

The scripts are preserved as thesis analysis sources; they are not executed by
the ESP32 firmware build.

# IHU Thesis - updated fast pdfLaTeX project

This is the complete drop-in replacement LaTeX project generated from the
latest `Thesis.docx`. The Word document is the source of truth: its thesis
wording, equations, numerical values, bibliography entries, and newly added
Chapter 4 material have been transferred without paraphrasing.

`main.tex` is located at the project root and the required compiler is
**pdfLaTeX**.

## Build in Overleaf

1. Upload the ZIP as a new project.
2. Select **pdfLaTeX** as the compiler.
3. Select `main.tex` as the main document.
4. Recompile normally. If the project previously used another compiler, choose
   **Recompile from scratch** once.

## Local build

```bash
latexmk -pdf main.tex
```

No BibTeX or Biber pass is required. The bibliography is stored as a static
`thebibliography` list, keeping compilation fast and preserving the current
reference text and links.

## Preserved document decisions

- Chapter 1 contains only its chapter title.
- Greek and English abstract pages are intentionally blank.
- The conclusions and abbreviations sections remain disabled.
- All pages use portrait A4 dimensions.
- Table captions are placed below their tables.
- The paired model-comparison figures remain vertically stacked on portrait
  pages at a consistent page width.
- Chapter 4 includes the latest system-design continuation and Figures 4.7-4.11.
- No editorial comments or migration notes are rendered in the thesis.

## Main files

- `metadata.tex`: institutional and student metadata.
- `chapters/`: editable thesis chapters transferred from the latest Word file.
- `frontmatter/`: title page, declaration, and blank abstracts.
- `figures/`: all figures used by the thesis.
- `bibliography/references-static.tex`: current linked bibliography.
- `ihu-thesis.cls`: formatting and fast-build configuration.

# md2pdf

Render a Markdown file to a printable PDF, one section per page. Made for the talk outline
(`docs/plan/talk-outline.md`), but it works on any Markdown file.

- Each `## ` heading starts a new page (`--break-level` changes the level). The text before the first one is page 1.
- A section that does not fit on one page is set in smaller type, a quarter point at a time, down to `--min-font`.
  The script prints each section it shrank. A section still too long at that size flows onto a second page, and the
  script says so.
- The footer shows the file name (or `--footer` text) and the page number.

## Setup

```bash
pip install markdown playwright
```

```bash
python3 -m playwright install chromium
```

Or skip the second step and point at an installed Chrome with `--browser`, for example
`--browser "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome"`.

## Use

```bash
python3 tools/md2pdf/md2pdf.py docs/plan/talk-outline.md -o talk-outline.pdf --footer "FieldUnit talk outline"
```

Options: `--page Letter|A4` (default Letter), `--margin` in inches (default 0.6), `--font` in pt (default 9.1),
`--min-font` in pt (default 7.5).

`.gitignore` ignores `*.pdf`, so a generated PDF is not committed by accident.

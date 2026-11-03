#!/usr/bin/env python3
"""Render a Markdown file to a printable PDF, one section per page.

Each heading at --break-level (default 2, that is "## ") starts a new page; the text before the first such
heading is page 1. A section that does not fit on one page is set in smaller type, step by step, down to
--min-font; a section still too long at that size flows onto a second page.

Usage:
    python3 tools/md2pdf/md2pdf.py docs/plan/talk-outline.md -o talk-outline.pdf

Requires:
    pip install markdown playwright
    python3 -m playwright install chromium     # or pass --browser /path/to/chrome
"""

import argparse
import html
import re
import sys
from pathlib import Path

try:
    import markdown
except ImportError:
    sys.exit("md2pdf: needs the 'markdown' package: pip install markdown")
try:
    from playwright.sync_api import sync_playwright
except ImportError:
    sys.exit("md2pdf: needs Playwright for Python: pip install playwright && python3 -m playwright install chromium")

PAGE_SIZES_IN = {"Letter": (8.5, 11.0), "A4": (8.27, 11.69)}
CSS_PX_PER_IN = 96.0

CSS = """
body { font-family: Georgia, 'DejaVu Serif', serif; font-size: %(font)spt; line-height: 1.28; color: #111; margin: 0; }
.page { break-after: page; }
.page:last-child { break-after: auto; }
h1 { font-size: 18pt; margin: 0 0 6pt; }
h2 { font-size: 14pt; margin: 0 0 8pt; border-bottom: 1.5px solid #333; padding-bottom: 3pt; }
h3 { font-size: 11pt; margin: 8pt 0 4pt; }
p { margin: 0 0 5pt; }
ul, ol { margin: 0 0 5pt 0; padding-left: 16pt; }
li { margin: 0 0 2pt; }
code { font-family: 'DejaVu Sans Mono', Menlo, monospace; font-size: 0.9em; background: #f1f1f1; padding: 0 2px; }
pre { font-size: 0.85em; background: #f6f6f6; padding: 4pt; white-space: pre-wrap; }
table { border-collapse: collapse; margin: 4pt 0 8pt; font-size: 0.95em; }
th, td { border: 1px solid #999; padding: 2pt 5pt; text-align: left; vertical-align: top; }
hr { display: none; }
"""


def split_sections(text, level):
    """Split Markdown text before every heading of exactly `level` hashes. Empty sections are dropped."""
    marker = "#" * level + " "
    parts = re.split(r"(?m)^(?=%s)" % re.escape(marker), text)
    return [p for p in parts if p.strip()]


def build_html(sections, font_pt, title):
    body = "".join(
        '<section class="page">%s</section>' % markdown.markdown(s, extensions=["tables", "fenced_code"])
        for s in sections
    )
    return '<!doctype html><html><head><meta charset="utf-8"><title>%s</title><style>%s</style></head><body>%s</body></html>' % (
        html.escape(title), CSS % {"font": font_pt}, body)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("input", type=Path, help="Markdown file")
    ap.add_argument("-o", "--output", type=Path, help="PDF file (default: input name with .pdf)")
    ap.add_argument("--break-level", type=int, default=2, help="heading level that starts a new page (default 2)")
    ap.add_argument("--font", type=float, default=9.1, help="body font size in pt (default 9.1)")
    ap.add_argument("--min-font", type=float, default=7.5, help="smallest size used to fit a long section (default 7.5)")
    ap.add_argument("--page", choices=sorted(PAGE_SIZES_IN), default="Letter", help="paper size (default Letter)")
    ap.add_argument("--margin", type=float, default=0.6, help="page margin in inches (default 0.6)")
    ap.add_argument("--footer", default=None, help="footer text before the page number (default: the file name)")
    ap.add_argument("--browser", default=None, help="path to a Chrome or Chromium binary, instead of Playwright's own")
    args = ap.parse_args()

    out = args.output or args.input.with_suffix(".pdf")
    sections = split_sections(args.input.read_text(encoding="utf-8"), args.break_level)
    footer = html.escape(args.footer if args.footer is not None else args.input.name)

    width_in, height_in = PAGE_SIZES_IN[args.page]
    top_in, bottom_in = args.margin, args.margin + 0.05      # a little extra room for the footer
    content_w = (width_in - 2 * args.margin) * CSS_PX_PER_IN
    content_h = (height_in - top_in - bottom_in) * CSS_PX_PER_IN

    with sync_playwright() as pw:
        browser = pw.chromium.launch(executable_path=args.browser) if args.browser else pw.chromium.launch()
        page = browser.new_page(viewport={"width": int(content_w), "height": int(content_h)})
        page.emulate_media(media="print")
        page.set_content(build_html(sections, args.font, args.input.stem))

        # Shrink the type of each section that overflows one page, in steps, down to --min-font.
        sizes = page.evaluate(
            """([limit, start, floor]) => {
                const out = [];
                for (const s of document.querySelectorAll('.page')) {
                    let pt = start;
                    while (s.scrollHeight > limit && pt - 0.25 >= floor) {
                        pt -= 0.25;
                        s.style.fontSize = pt + 'pt';
                    }
                    out.push([s.querySelector('h1,h2,h3,h4')?.textContent ?? '(untitled)', pt, s.scrollHeight > limit]);
                }
                return out;
            }""",
            [content_h, args.font, args.min_font],
        )
        for heading, pt, overflow in sizes:
            if pt != args.font or overflow:
                note = " (still longer than one page)" if overflow else ""
                print("md2pdf: %s set at %.2fpt%s" % (heading.strip(), pt, note), file=sys.stderr)

        page.pdf(
            path=str(out),
            format=args.page,
            print_background=True,
            display_header_footer=True,
            header_template="<span></span>",
            footer_template=(
                '<div style="font-size:7pt;width:100%%;text-align:center;color:#666">'
                '%s · <span class="pageNumber"></span>/<span class="totalPages"></span></div>' % footer
            ),
            margin={"top": "%gin" % top_in, "bottom": "%gin" % bottom_in,
                    "left": "%gin" % args.margin, "right": "%gin" % args.margin},
        )
        browser.close()
    print("md2pdf: wrote %s" % out, file=sys.stderr)


if __name__ == "__main__":
    main()

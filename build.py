#!/usr/bin/env python3
"""
Build script to combine all slide files into index.html
Usage: python build.py
"""

import os
from pathlib import Path

# Get the directory where this script is located
SCRIPT_DIR = Path(__file__).parent
SLIDES_DIR = SCRIPT_DIR / "slides"
CSS_FILE = SCRIPT_DIR / "css" / "style.css"
JS_FILE = SCRIPT_DIR / "js" / "app.js"
OUTPUT_FILE = SCRIPT_DIR / "index.html"


def read_file(filepath):
    """Read and return file contents"""
    with open(filepath, "r", encoding="utf-8") as f:
        return f.read()


def get_all_slides():
    """Get all slide files in order"""
    slide_files = sorted(SLIDES_DIR.glob("*.html"))
    slides_content = []

    for slide_file in slide_files:
        content = read_file(slide_file)
        slides_content.append(content)

    return "\n\n            ".join(slides_content)


def build_html():
    """Build the complete index.html file"""
    css_content = read_file(CSS_FILE)
    js_content = read_file(JS_FILE)
    slides_content = get_all_slides()

    html_template = f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Spooky Scary Security: The IoT Nightmare</title>
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/reveal.js@5.0.4/dist/reset.css">
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/reveal.js@5.0.4/dist/reveal.css">
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/reveal.js@5.0.4/dist/theme/black.css">
    <style>
{css_content}
    </style>
</head>
<body>
    <div class="scale-control">
        <label for="scaleSlider">SCALE</label>
        <input type="range" id="scaleSlider" class="scale-slider" min="50" max="175" value="85" orient="vertical">
        <div class="scale-value" id="scaleValue">85%</div>
    </div>

    <div class="reveal">
        <div class="slides">
            {slides_content}
        </div>
    </div>

    <script src="https://cdn.jsdelivr.net/npm/reveal.js@5.0.4/dist/reveal.js"></script>
    <script>
{js_content}
    </script>
</body>
</html>"""

    with open(OUTPUT_FILE, "w", encoding="utf-8") as f:
        f.write(html_template)

    print(f"✅ Built {OUTPUT_FILE}")
    print(f"   - Included {len(list(SLIDES_DIR.glob('*.html')))} slides")


if __name__ == "__main__":
    build_html()

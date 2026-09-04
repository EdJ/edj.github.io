#!/usr/bin/env python3
"""Every plate, every treatment, one palette per panel.

The house pipeline in ~/git/lhmp-brand/scratchpad/build_dither.py colorises
everything onto cream, because it is describing ink on paper. This page is a
lit screen on a near-black ground, so every treatment is inverted at the root:
the DOTS carry the light, sized by brightness rather than by darkness, and the
'ink' role is the panel's own accent while the 'stock' role is its ground.

Palettes are the ones already on the page — read off the baked plates in 10,
not invented — so switching style cannot restyle the site.
"""
import os
from PIL import Image, ImageOps, ImageDraw, ImageChops, ImageEnhance

# The sources were A1/M1/Q3/B1_raw.jpg in a session scratchpad, which is long
# gone — this script could not be run. They are the four 1800px copies kept in
# plates12/ for concept 12's shader, which are wider than W and so re-bake
# identically. Sourced from Unsplash/Pexels under licences that require no
# attribution, chosen on that basis; there is no credit line to carry.
SH  = 'concepts/assets/plates12'
OUT = 'concepts/assets/styles'
W   = 1400

PLATES = [
    # key      raw                    ground        accent        high
    ('hero',  SH+'/lasers-src.jpg',  (11,12,15), (125,251,253), (237,239,243)),
    ('perf',  SH+'/backs-src.jpg',   (11,12,15), (255,106,0),   (242,239,230)),
    ('mid',   SH+'/hands-src.jpg',   (11,12,15), (125,251,253), (255,45,140)),
    ('close', SH+'/arms-src.jpg',    (21,20,26), (240,164,106), (255,241,226)),
]

def grade(im):
    """One grade for every treatment, so the style is the only variable."""
    return ImageOps.autocontrast(im.convert('L'), cutoff=(1, 1))

def duotone(g, ground, accent, high):
    return ImageOps.colorize(g, black=ground, white=high, mid=accent)

def dotmask(g, cell):
    """One dot per cell, radius from BRIGHTNESS. White inside the dot.
       On a dark ground the light is the ink, so the screen runs the other
       way up from the house version."""
    w, h = g.size
    m = Image.new('L', (w, h), 0)
    d = ImageDraw.Draw(m)
    px = g.load()
    rmax = cell * 0.80
    for y in range(0, h, cell):
        cy = y + cell / 2
        for x in range(0, w, cell):
            b = px[min(x + cell // 2, w - 1), min(y + cell // 2, h - 1)]
            r = (b / 255.0) * rmax
            if r > 0.35:
                cx = x + cell / 2
                d.ellipse([cx - r, cy - r, cx + r, cy + r], fill=255)
    return m

def flat(size, rgb):
    return Image.new('RGB', size, rgb)

# ── the treatments ───────────────────────────────────────────────────────
def s_photo(im, gr, ac, hi):
    """Control: the graded photograph, no treatment at all."""
    return ImageOps.autocontrast(im.convert('RGB'), cutoff=(1, 1))

def s_duo(im, gr, ac, hi):
    """Flat duotone, no screen. The quietest of the six."""
    return duotone(grade(im), gr, ac, hi)

def s_riso(im, gr, ac, hi):
    """Duotone with a dot screen laid over it, the screen at partial strength
       so the picture survives it — the house A-riso, inverted for a screen."""
    g = grade(im)
    duo = duotone(g, gr, ac, hi)
    screened = Image.composite(duo, flat(duo.size, gr), dotmask(g, 6))
    return Image.blend(duo, screened, 0.62)

def s_news(im, gr, ac, hi):
    """Mono dot screen: one ink, the panel's highlight, on its ground."""
    g = grade(im)
    return Image.composite(flat(g.size, hi), flat(g.size, gr), dotmask(g, 5))

def s_print(im, gr, ac, hi):
    """Screenprint: posterised to 8 steps (2-bit collapsed 76% of a frame to
       one value when 10 tried it), plates misregistered, grain over the top."""
    g = ImageOps.autocontrast(im.convert('L'), 2)
    duo = duotone(ImageOps.posterize(g, 3), gr, ac, hi)
    r, gg, b = duo.split()
    duo = Image.merge('RGB', (ImageChops.offset(r, 2, 1), gg,
                              ImageChops.offset(b, -2, -1)))
    noise = Image.effect_noise(duo.size, 26).convert('L')
    grain = ImageOps.colorize(noise, black=(0, 0, 0), white=(255, 255, 255))
    return Image.blend(duo, ImageChops.overlay(duo, grain), 0.12)

STYLES = [('photo', s_photo), ('duo', s_duo), ('riso', s_riso),
          ('news', s_news), ('print', s_print)]

os.makedirs(OUT, exist_ok=True)
for key, raw, gr, ac, hi in PLATES:
    im = Image.open(raw).convert('RGB')
    im = im.resize((W, round(im.size[1] * W / im.size[0])), Image.LANCZOS)
    for name, fn in STYLES:
        p = f'{OUT}/{key}-{name}.jpg'
        fn(im, gr, ac, hi).save(p, quality=84, optimize=True)
        print(f'  {p}  {os.path.getsize(p)//1024}K')

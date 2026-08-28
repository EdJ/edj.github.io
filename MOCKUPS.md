# UI mockups

Static HTML recreations of the Jolt/Inertia app screens, for landing-page use. These are
**not** simulator screenshots — they are hand-built HTML/CSS that reuses the app's real
design tokens, real fonts and real icon assets, with content staged to look good rather
than to reproduce a particular app state.

Built 2026-07-31. Sampler and chord-progression frames added 2026-08-27.

## Why not real screenshots

Screenshots need a simulator or device, and they capture whatever state the app happens to
boot into. For marketing we want an idealised pattern, healthy meter levels and a lit-up
grid, exported at arbitrary resolution. HTML gives all of that and stays editable.

The tradeoff: these will drift from the app as the UI changes. Treat them as *marketing
assets derived from the design system*, not as documentation of current behaviour.

## Files

| File | What it is |
| --- | --- |
| `mockups.html` | **Master source.** Standalone gallery: 5 screens × 2 platforms + 2 iPhone-only, each in a device frame with a caption. Open directly in a browser. |
| `index-mockups.html` | Trial integration — a copy of `index.html` with 5 screens dropped into matching sections. Not live; `index.html` is untouched. |
| `mockup-screens.css` | Screen styles extracted from the master, tokens scoped under `.jolt-mock`. |
| `mockup-screens.js` | Staging data + scaling script for the integrated page. |

`mockup-screens.css` / `.js` are **generated from `mockups.html`**, not maintained by hand.
Edit the master, then re-extract (see below).

## Screens

Twelve frames total — five screens at two size classes, plus two iPhone-only screens
added 2026-08-27:

- **Pattern Editor** — step grid, page bar, instrument palette, live parameter panel
- **Note Editor** — scale-locked piano roll, two lanes
- **Pattern Composer** — loop-group carousel + pattern pool
- **Mixer** — voice strips, FX returns, master
- **Macro FX** — performance macro sliders
- **Sampler** *(iPhone only)* — sliced waveform, slice count, tune and gain
- **Generate · Chord Progression** *(iPhone only)* — degree timeline, chord editor

The two newer screens are static HTML end to end — unlike the step grid and note lanes,
nothing in `mockup-screens.js` builds them, so dropping them into a page needs the CSS
and the markup only.

iPhone frames are **390×844 portrait**; iPad frames are **1194×834 landscape**. This matches
the app: `ContentView` forces landscape on iPad, and the iPhone build is portrait-only.
Each frame is a fixed logical canvas scaled to fit its container by JS (`fitScreens()`), so
frames stay pixel-consistent regardless of viewport.

The two size classes are genuinely different layouts, not the same layout stretched —
matching what `AdaptivePanelView` and the size-class checks actually do:

| | Compact (iPhone) | Regular (iPad) |
| --- | --- | --- |
| Editor panel | sub-tabs (Sound/Filter/F. Env/…) | all clusters merged on one surface |
| Mixer | one panel + `Voices \| FX` tab strip | Voices beside FX, no tabs |
| Composer | pool scrolls horizontally, cut off | all 4 segments visible + Effects rail |
| Note editor | 8 steps per lane | 16 steps per lane |
| Pattern rows | two rows of 8, one page | four pages, one row of 16 each, numbered, rules between — a 50/50 split with the panel |
| Macro FX | fullscreen cover | 420×600 popover |

## Source of truth

Everything visual traces to the app repo at `../Easter`:

| What | Where |
| --- | --- |
| Colour tokens | `Inertia/InertiaApp.swift` → `struct ColorScheme` (LHMP "Clinical Dark") |
| Type ramp | `Inertia/InertiaApp.swift` → `customFont(_:)` |
| Fonts | `Inertia/fonts/` — ChakraPetch Bold/SemiBold, GeistMono Regular/Medium |
| Button chrome | `Inertia/Components/BasicControls/Buttons.swift` → `ButtonChrome`, 6pt diagonal chamfer |
| Chamfer geometry | `Inertia/Shapes/ChamferedRectangle.swift` |
| Spacing | `Inertia/Components/Convenience/Metrics.swift` |
| Instrument colours | `Inertia/Components/InstrumentPalette.swift` → `InstrumentColors.fixed` |
| Pattern colours | `Inertia/TopLevelViews/PatternComposerView.swift` → `PatternColors` |
| Instrument icons | `Inertia/Assets.xcassets/icons/*.imageset/*.pdf` |
| Sampler slicing | `Inertia/Components/SliceView.swift`, `Components/SlicedWaveform.swift` |
| Sampler panel shape | `Inertia/Components/ProjectSamplesView.swift` (`PanelGroup("Sample")`) |
| Tune control | `Inertia/Components/BasicControls/SamplerTuneEditor.swift` |
| Chord progression UI | `docs/chord-progression-gen.md` §E1 — an ASCII layout of the panel as built |
| Chord model | `Inertia/Extensions/PatternGeneration/ChordProgression.swift` |
| iPad pattern row | `Inertia/TopLevelViews/UnifiedPatternView.swift` → `MultipageEntry`, `regular:` branch |
| Step row geometry | `Inertia/Components/CanvasSequencerPage.swift` — `steps: 16`, 2pt separators at 4, 8, 12 |

Screen layouts came from reading:
`UnifiedPatternView.swift` (editor + notes), `PatternComposerView.swift` +
`CanvasPatternGrid.swift` (composer), `Mixer.swift`, `MasterFx/FxMacros.swift`,
`Convenience/AdaptivePanelView.swift`, `PlayMenu.swift`.

**Note:** `Inertia/TopLevelViews/PatternEditView.swift` is dead code — it defines only
`PageMarker` and `StepMicrotimingConfig`, neither referenced anywhere live. The real pattern
editor is `UnifiedPatternView`. Likewise `MultipassView` / `SinglePattern` /
`MultipassPlayhead` in `PatternComposerView.swift` have no call sites.

## How the assets were made

Both are embedded as data-URIs in `mockups.html` / `mockup-screens.css`, so the pages are
fully self-contained (no external requests, works from `file://`).

**Fonts** — subset to the glyphs used, converted to woff2 (~5–15KB each vs ~78–150KB TTF):

```sh
python3 -m venv venv && ./venv/bin/pip install fonttools brotli

for f in ChakraPetch-Bold ChakraPetch-SemiBold GeistMono-Regular GeistMono-Medium; do
  ./venv/bin/pyftsubset ../Easter/Inertia/fonts/$f.ttf \
    --unicodes="U+0020-007E,U+215B-215E,U+00BC-00BE,U+2190-2193,U+25B6,U+25CF,U+2022" \
    --flavor=woff2 --output-file=$f.woff2
  printf "@font-face{font-family:'%s';src:url(data:font/woff2;base64,%s) format('woff2');}\n" \
    "$f" "$(base64 -i $f.woff2)" >> fonts.css
done
```

**Instrument icons** — the asset catalog holds vector PDFs; rasterise and embed as CSS
masks so they can be tinted the way the app tints them (`ColorScheme.background` silhouette
on a coloured cell):

```sh
for i in kick snare hihat chord sine arp waves string bell noise; do
  sips -s format png -Z 64 \
    ../Easter/Inertia/Assets.xcassets/icons/$i.imageset/$i.pdf --out icons/$i.png
done
# then: --ic-<name>: url(data:image/png;base64,...) custom properties, applied via mask
```

Only 10 of the ~28 available icons are embedded — add more from the same catalog if a
screen needs them.

## Previewing and exporting

No build step, no server needed — open the HTML directly.

Headless Chrome works for verification and for exporting raster assets:

```sh
CHROME="/Applications/Google Chrome.app/Contents/MacOS/Google Chrome"

# full page
"$CHROME" --headless --disable-gpu --hide-scrollbars \
  --screenshot=out.png --window-size=1200,12000 "file://$PWD/mockups.html"

# a specific band (sips crops from the BOTTOM, which is confusing — offset is from top
# only for --cropOffset's first arg; verify what you got)
sips out.png -c <height> <width> --cropOffset <y> <x> --out crop.png
```

To shoot ONE section, append an override stylesheet and hide the rest. Append — do not try
to inject into `<head>`: **`mockups.html` has no `<head>`/`</head>` at all**, so a
`sed "s|</head>|…|"` recipe substitutes nothing, exits 0, and you screenshot the wrong
section without being told.

```sh
{ cat mockups.html; cat <<'EOF'
<style>
  section.shot{display:none!important}
  section.shot#sampler{display:block!important}
  header.masthead,footer,.platform-divider,.shot-note{display:none!important}
</style>
EOF
} > tmp.html
"$CHROME" --headless --disable-gpu --hide-scrollbars --force-device-scale-factor=2 \
  --screenshot=out.png --window-size=620,1010 "file://$PWD/tmp.html"
```

The artifact viewer sandboxes pages in a cross-origin iframe that swallows scroll events,
so headless Chrome is the reliable way to inspect the whole page.

## Modifier-class collisions

The screens share one flat namespace, and the app-chrome classes are short. A modifier that
reads naturally in isolation can silently pick up an unrelated rule:

- **`.bar`** is the top/bottom app chrome — `display:flex; padding:8px; background; z-index:2`.
  A `<span class="beat bar">` meant as a 1px bar line renders **17px wide** with a background,
  because it inherits that padding. Both new screens had this bug; the beat and bar ticks are
  now `.barline`.
- **`.pad`** scopes the iPad frames (`.pad .fader`, `.pad .gcard`, …). Safe as a bare modifier
  only by luck — nothing styles `.pad` on its own today.

Before adding a modifier, check it: `grep -oE '^\.[a-z-]+' mockups.html | sort -u`. Prefer a
compound name (`.barline`) over a bare adjective.

### …and with the page you embed into

The same trap crosses documents. `mockup-screens.css` scopes only the *tokens* under
`.jolt-mock`; the class names stay flat, so they collide with whatever the host page calls
things. A host rule at (0,1,0) loses to `.tabcard .foot` (0,2,0) **only on the properties the
mockup rule actually sets** — everything else leaks straight through.

Found in `concepts/07-open.html`, where `.foot` is the page footer
(`padding:clamp(70px,10vh,130px); flex-direction:column`): the editor's panel tab strip
inherited both, stacking `Sound / Filter / F. Env / Noise / Amp` into a column and pushing
four of the five out of the 32px strip. It shipped that way — `concept07.html` still shows
it. `concepts/08-open.html` carries the reset:

```css
.jolt-mock .tabcard .foot{padding:0;flex-direction:row;gap:0}
.jolt-mock .tabcard .panel{padding:8px}
```

When embedding a screen in a new page, diff the computed styles of `.tabcard`, `.panel` and
`.foot` against `mockups.html` before trusting the render.

## Regenerating the extracted CSS/JS

`mockup-screens.css` and `.js` are derived from `mockups.html` by:

1. Pulling the `<style>` block, renaming `:root {` → `.jolt-mock {` so the app tokens don't
   collide with the site's own `--text` / `--accent` / `--mono` (they differ — site text is
   `#d9d9d9`, app text `#EDEFF3`).
2. Dropping the gallery-only rules: `html`/`body` background, `.wrap`, `.masthead` block,
   `footer`, `.platform-divider`.
3. Pulling the `<script>` block verbatim, then guarding each `getElementById(...).innerHTML`
   with a null check (the site page only includes some screens).
4. Appending the `.feature-split` / `.mock-wide` layout helpers.
5. Extracting each `<div class="device">…</div>` by id and adding the `jolt-mock` class.

Device frames inside `.feature-split` are capped at 330px wide by `fitScreens()`.

## Staged vs. faithful

Faithful: colours, fonts, chamfer geometry, control sizes, layout structure, state styling
(selected/playing/multi-page cues), icon silhouettes.

Staged (invented for looks — change freely):

- Song name "Midnight Taxi", the pattern arrangement, which steps are lit
- Meter levels and fader positions
- Dial values and parameter names on the editor panel
- The macro slot assignments (Reverb+Delay, Low Pass, Beat Repeat, Phaser+Gater, High Pass)
- Note content, C minor, the C4–C5 row range
- **The iPad Effects rail contents** — the spec only confirmed the rail exists at 200pt with
  an "Effects" header; the three send dials in it are a guess
- **The iPad editor is a 50/50 split**, pattern left and merged panel right. The pattern column
  holds four pages of 16 and the cells flex to fill, so they land at ~34px and are squared by
  `aspect-ratio` rather than keeping `.steps.big`'s fixed 58. The column is top-aligned: the page
  list grows downward from the page bar.
- **All four page chips read as selected** on the iPad (`.pg.num.on`, bright grey) — every page
  is on screen and editable at once, so there is no single "current" one the way there is on the
  phone, where only page 1 carries `.on`. The cyan `.playing` strip is separate and follows the
  transport on both.
- **A rule above page 1 and below page 4.** The app draws a rule only *between* pages, so the
  outer two are added — they close the block off against the page bar and the instrument strip.
- **Pages 2-4 are staged variations** of page 1, generated by masking the shared `rows` data.
  Page 1 is the same sixteen cells in the same order as the phone, which is what lets one index
  drive the hero playhead across both devices; only page 1 carries a `play`.
- **The sampler's tab row** — `Type / Sample / Grains / LFO / Mix`. `Sample` and `Grains` are
  real `PanelGroup`s; the rest are carried over from the synth editor's row so the frame keeps
  the five-tab rhythm. The panel foot (`Sample / Amp`) *is* exhaustive — those are the only two
  panels the sampler has.
- **`break_02.wav`, its eight slices and the selected slice five.** The waveform is synthesised,
  not a real file; the unsliced 3% head exists so the "gaps are painted out at 0.6 ground" rule
  has something to act on.
- **The Cm–Ab–Bb–Fm progression**, the twelve-step rhythm and the selected chord. The doc says
  the *first* chord is selected on open; this frame shows a chord mid-edit instead, so the
  inversion and octave controls have something to read against.

## Known limitations / next steps

- The integrated page ships ~150KB of inline CSS. The Chakra/Geist fonts are embedded as
  data-URIs *and* the site loads its own copies from `fonts/` and `geist/` — duplicated.
  If page weight matters, export the frames as 2× PNGs and swap in `<img>` tags. That also
  makes art direction (tilt, overlap, parallax, cropping) much easier.
- Frames are static. No animation, no playhead movement.
- `index-mockups.html` is a trial. Promoting it = rename over `index.html`.
- These will drift as the app UI changes. Re-derive from the Swift source rather than
  patching from memory.

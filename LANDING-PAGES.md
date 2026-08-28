# Landing-page landscape — joltgroovebox.com vs the field

Sixteen competitor homepages read for **structure**, not brand identity. Written 2026-08-10 as a companion to
`REDESIGN.md`.

Scope note: `../lhmp-brand/brand/05-landscape.md` already surveys the *brand systems* of this field. This document
deliberately does not repeat that. It asks a narrower question: **what does the first screen do, how is a long feature
list compressed, what proof is offered, and how is the buy action presented?**

Visual board with screenshots: `moodboard.html` (see "Artifacts" at the end).

## Who was read

| Territory | Sites |
| --- | --- |
| Direct — mobile music apps | Koala Sampler, Loopy Pro, Ableton Note, AudioKit Pro |
| Hardware grooveboxes | Elektron, Teenage Engineering, Polyend, Dirtywave (M8) |
| Boutique software | Slate + Ash, Output, Baby Audio |
| DAW / big-tent | Bitwig, Ableton, Arturia |
| Non-music, dark + dense | Linear, Raycast |

Homepages captured via headless Chrome at 1440×1300. Palette and typeface values below were read from served CSS and
webfont filenames where verifiable; everything else is described qualitatively.

## The headline finding

Jolt already owns two assets that would out-perform every page in this survey, and neither is on the live site.

- **`mockups.html`** is a live-DOM interface figure built on the real house tokens. This is precisely the technique that
  makes Linear's page work, and *no music brand in the survey does it* — they all ship flat screenshots or video.
- **`onboarding2.html`** is a working in-browser groovebox running the real DSP. **Zero of sixteen** competitors offer a
  playable web demo. Two offer inline audio at all.

Meanwhile the live page is a pre-brand layout with a stock crowd photograph and **no App Store link anywhere on the
site** — against sixteen of sixteen competitors carrying a buy or download action above the fold.

The problem is not that Jolt lacks assets. It is that the strongest ones are orphaned.

## What the whole field agrees on

1. **A buy action above the fold, always.** 16/16. Jolt: none.
2. **Achromatic or near-achromatic ground; colour comes from the product.** Elektron `#eeeef2`/`#151515`, TE white +
   near-black, Dirtywave `#0d0d0d`. Polyend (forest green + mint) is the sole outlier, and it reads friendly rather than
   serious — useful as a boundary marker.
3. **Neutral grotesques, no personality faces.** Neue Haas Grotesk, Univers Next Pro, Albert Sans, Montserrat, Futura PT,
   Pilat, Inter. Weight and scale do the expressive work.
4. **Hard numbers on the homepage.** Nobody sells "creativity" without also selling track counts, voice counts or prices.
5. **Restrained motion.** No scroll-jacking, no parallax, no autoplay-with-sound anywhere. Carousel or muted loop at most.
6. **Community as a first-class section**, not a footer link — Elektronauts, Polyend Backstage, Dirtywave's Discord in
   the second paragraph.

## What almost nobody does — the open ground

| Move | Field | Jolt's position |
| --- | --- | --- |
| Playable web demo | 0 / 16 | **Built, unlinked** |
| Live-DOM app figures instead of screenshots | 2 / 16 (both non-music) | **Built, unlinked** |
| Monospace type system | 3 / 16 | Already mono-led |
| Any audio on the homepage | 2 / 16 (AudioKit, Arturia) | None |
| Price stated on the homepage | 6 / 16 | None |
| Rating / review proof | 6 / 16 | None |
| Sticky CTA | 3 / 16 | No CTA at all |

The first three rows are the strategic point: Jolt is *already positioned* on all three and is spending none of it.

## The four solutions to "we have 200 features"

Jolt's effect chain is currently eleven equal-weight cards (`index.html:71-116`), which is the one approach nobody in
the survey uses. The proven alternatives:

1. **Quantify, don't enumerate** *(Bitwig)*. One hero paragraph of countable nouns — "140+ instruments and effects",
   "over 20GB (and counting)" — and the page never lists a feature again.
2. **Sell the spec sheet** *(Dirtywave)*. ~80% of the homepage is three bulleted spec columns, and it reads as
   confidence rather than a datasheet dump. This also matches what `02-identity.md` §8 already specifies for Jolt.
3. **Progressive disclosure** *(Ableton Note)*. Tabs plus nine accordions; zero features visible above the fold.
4. **Compress the long tail into one-liners** *(Raycast)*. Twelve verb-first sentences — "It can take notes." / "Run
   scripts." — capped by "And much, much more."

Loopy Pro, the strongest page in the direct-competitor set, enumerates **nothing** and sells outcomes instead.
Koala Sampler, the weakest, dumps 37 raw bullets.

## The two pages that actually solve Jolt's problem

Neither is a music brand. Both are dark, dense, screen-only products that refuse video entirely.

### Linear — visuals as documentation

No `<video>`, no `.mp4`, no `.webm` anywhere. Every product visual is a live DOM recreation built from the shipping
app's own primitives, and each is captioned **`FIG 0.2` / `FIG 0.3` / `FIG 0.4`** in monospace at 40% opacity. That
caption is the whole trick: it reframes marketing imagery as *documentation*, which is exactly the "technical-manual"
register the identity spec asks for — delivered mechanically rather than as styling.

Their dark ground is a five-step token ramp (`#08090a → #0f1011 → #141516 → #191a1b → #1c1c1f`) plus one accent
(`#7170ff`) and a 256px noise tile at `mix-blend-mode: overlay`. That grain rule is the single biggest reason their
near-black reads as engineered rather than as a default dark theme. Jolt currently has no grain or halftone layer
anywhere, which `REDESIGN.md` §5 already flags as a missed differentiator.

### Raycast — the hero metaphor *is* the feature grid

The hero is a DOM keyboard, and four of its keycaps are the feature cards: `Fast.` Think in milliseconds. /
`Ergonomic.` Keyboard first. / `Native.` Pure performance. / `Reliable.` 99.8% crash-free rate.

The formula — **`Adjective.` + at most six words, at least half carrying a hard number** — costs zero extra vertical
space and reinforces the product's central object. The direct analogue for Jolt is four claims on four pads of the pad
grid.

## Near-neighbour risk

**Dirtywave is the collision, and it is palette-only.** The M8 site is already `#0d0d0d` ground, `#212121` surface, and a
single electric cyan `#00e5ff` — and the M8 is *the* reference object for pocketable tracker/sequencer UI. A dark +
cyan music tool will read as M8-adjacent to anyone in this scene.

Two escapes are available and both are already Jolt's stated direction:

- Dirtywave ships **Montserrat with no monospace face at all**. A genuine mono type system is unoccupied in this set.
- Dirtywave has **no radius or grid signature whatsoever** — it is a plain Shopify single column. A rigorous chamfer is
  likewise unclaimed.

So: keep the cyan, win on type and geometry. Do not fight this on colour.

**Elektron is a secondary, inverted risk.** Monochrome greyscale, `border-radius: 0` dominant, and custom webfonts
(`digi one v2`, `Analog One V2.`) that mimic their own hardware LCDs. If Jolt renders device-screen type it is near
their signature move — but their ground is light, which keeps the two apart.

Worth stealing from both regardless: Elektron ships its **hardware's own screen typeface** as a webfont and sets site
labels in it. A software instrument has a stronger claim to that move than they do — the UI font *is* the product.
And Teenage Engineering scales `border-radius` as a **ratio of container width** (`radius = 0.0367 × container`), so
corner softness stays optically constant at every breakpoint. That applies directly to the chamfer, which `REDESIGN.md`
§3 already flags as wrongly fixed at a flat 8px.

## Recommended moves, by leverage

Ordered by impact per unit of work. Provenance in brackets.

1. **Ship an App Store CTA — any CTA.** Not a design question, and it outranks everything below. *[16/16]*
2. **Promote `mockups.html` into the hero, captioned as numbered figures.** The asset exists and is already on-token.
   Adding `FIG 0.2`-style monospace captions costs one span per section. *[Linear]*
3. **Link the playable demo prominently.** A category first, not a parity feature. *[0/16 — open ground]*
4. **Break the eleven-card effect wall** into a monospace spec sheet or twelve one-liners. *[Dirtywave, Raycast]*
5. **Four claims on four pads** — `Adjective.` + a measurable, rendered inside the pad grid. *[Raycast]*
6. **A trust strip below the fold**, at whatever scale is honest. Loopy Pro's is four flat facts on one line and is most
   of why it is the strongest page in the direct set. Small real numbers beat adjectives; nothing beats nothing.
   *[Loopy Pro, Baby Audio]*
7. **State the price.** In a market drifting to subscriptions, a plain one-time price is a differentiator — Loopy Pro
   makes "$29.99 once, no subscription" an explicit fairness argument and puts Pricing in the nav. Omitting it is a
   default, not a strategy. *[Loopy Pro]*
8. **Escape Dirtywave on type and geometry, not colour.** *[Dirtywave, Teenage Engineering]*
9. **Consider "ways in" instead of a feature index.** Ableton Note and Loopy Pro independently landed on the same
   structure — not what the app does, but which kind of maker you are. Two of four direct competitors converging is a
   strong signal. *[Ableton Note, Loopy Pro]*

## Traps

- **Do not copy Koala's minimalism as if it caused the outcome.** Its markup carries an A/B experiment flag
  (`ab-experiment-home_landing_2026_07`) and click-instrumented store redirects — elf audio does not think that page is
  finished either. The app succeeded on App Store search and YouTube beatmaker culture, which Jolt does not have.
- **Do not copy Slate + Ash's zero-proof homepage.** It works on ten years of catalogue and a publishing arm doing the
  persuading off-page. An unknown running that playbook is a mystery box.
- **Do not copy Output's form without its roster.** Nineteen A-list names, bespoke photography, a 16-file custom
  typeface. Copying the shape promises reputation you don't have — and their SPA renders as an empty `<div>` without JS.
- **Do not adopt the hardware compositional grammar wholesale.** All four hardware sites are built around an
  orthographic product still floating on a neutral field. App screenshots substitute badly: they're rectangular, so they
  tile into monotony rather than floating; they're text-bearing, so they compete with headline type; and they don't
  survive being shrunk to a grid cell. Isolated UI fragments treated as objects, or live-DOM figures, work — a
  screenshot grid does not.

## Artifacts

| File | What |
| --- | --- |
| `moodboard.html` | Visual board — all 18 captures, annotated, on house tokens |
| `shots/*.png` | Full-resolution captures, 1440×1300 |
| `web/*.jpg` | Downscaled copies used by the board |

These live in the session scratchpad rather than the repo — third-party homepage captures are reference material with
someone else's trade dress in them, so they should not be committed without a deliberate decision.

# Site redesign — bringing joltgroovebox.com onto the LHMP house style

Audit of the current site against the LHMP brand system, plus a tiered plan for closing the gap.
Written 2026-08-10, before any changes were made.

The live site (`index.html` + `styles.css`) dates from March 2026 and **predates the brand sprint
entirely**. It is not "off-brand" through drift — it was built before there was a brand to be on.
That framing matters for the plan below: this is adoption, not correction.

## Source of truth

The house system lives in a separate repo, `../lhmp-brand`:

| What | Where |
| --- | --- |
| Strategy, voice, naming, brand architecture | `brand/01-foundations.md` |
| **The authoritative visual spec** — colour, geometry, type, registers | `brand/02-identity.md` |
| Deliberately-unresolved decisions | `brand/open-parameters.md` |
| Competitive landscape / differentiation guardrails | `brand/05-landscape.md`, `06-nearby-deep.md` |

Jolt is a **Tier-1 flagship** (`01-foundations.md` §5): it runs on its own identity and own domain,
and carries LHMP only as a discreet endorsement. So the site is *not* obliged to look like the LHMP
hub — but it must read as LHMP-made through the shared DNA: chamfer geometry, reserved cyan,
mono-led type.

## Where we are

Four artifacts in this repo, only one of which is live:

| File | State |
| --- | --- |
| `index.html` + `styles.css` | **Live** at joltgroovebox.com. March 2026, pre-brand. |
| `index-mockups.html` | July trial — app screens dropped into the live layout. Never promoted. |
| `onboarding.html` | Working in-browser WASM sequencer demo. **Not linked from anywhere.** |
| `mockups.html` | Master source for the device frames. Correctly built on real LHMP tokens. |

The two assets that best express the product — real app screens and a playable demo — are both
orphaned, while the oldest asset is the one being served.

## Gap analysis

### 1. Colour — the biggest tell

The site's palette is warm-neutral grey; the house ground is cool blue-black. Visibly different
families, not a near-miss.

| Role | Site (`styles.css:18-30`) | LHMP (`02-identity.md` §1) |
| --- | --- | --- |
| Ground | `#1c1c1c` *warm* | `#0B0C0F` *cool near-black* |
| Surface | `#2a2a2a` | `#14161D` / `#1E222C` |
| Line | `#2a2a2a` *(= surface)* | `#3A3F4B` |
| Text | `#d9d9d9` | `#EDEFF3` |
| Muted | `#9a9a8e` *(khaki)* | `#8B93A6` *(cool)* |
| Accent | `#7DFBFD` ✅ | `#7DFBFD` |

Cyan is the only token that matches.

Specific defects:

- **`--text-dim` is set to the same value as `--text`** (`styles.css:27-28`). The entire
  secondary-text tier silently does not exist. This is a large part of why the page reads flat.
- **`--border` is `1px solid #2a2a2a`** — identical to `--panel`, so borders are invisible against
  surfaces. There is no line token doing any work.
- **`--hot` / `--cold` (`styles.css:24-25`) are defined and never used.** They are also not house
  colours; the system's secondaries are signal orange `#FF6A00` (strictly subordinate) and acid
  `#E8FF00` (rare, high-impact).
- The hero dot-grid uses `rgba(232,255,71,0.07)` (`styles.css:195`) — that *is* acid `#E8FF00`, but
  at 7% opacity it's invisible. The one house colour reserved for eruption is being spent on
  texture nobody can see.

### 2. Type — off-system, and the right fonts are already vendored

House spec (`02-identity.md` §4): **Chakra Petch** display, **Geist Mono** body/data/labels.

The site runs GeistPixel Grid for display and **Share Tech Mono** for body — which is not in the
house system in any role — fetched from Google Fonts at runtime (`styles.css:6`).

Meanwhile, already in the repo and unreferenced by `styles.css`:

- `geist/GeistMono/webfonts/*.woff2` — the **full** Geist Mono family, variable and static
- `fonts/ChakraPetch-Bold.ttf`, `ChakraPetch-SemiBold.ttf` — TTF only, no woff2 yet

So the body-type fix costs nothing but a `@font-face` block and removes a third-party network
request. Chakra Petch needs a woff2 conversion first (`MOCKUPS.md` documents the `pyftsubset`
recipe — but note that subset's unicode range is deliberately narrow and is **not** suitable for
body copy; subset wider or ship the full face).

GeistPixel is not a mistake — `02-identity.md` §4 lists it as a legitimate product-flavour display
face. See *Open decisions* below.

### 3. Geometry — half-right

The chamfer is present on cards (`styles.css:250-255`) and pills (`styles.css:305-310`). Good: the
spec calls it "the single most ownable thing we have."

But:

- It's a **flat 8px on everything**. Spec says scale by ratio (`size × 0.2`) on dynamic elements,
  so a large card and a small pill should not share a cut size.
- Spec says **asymmetry is beneficial** — prefer asymmetric cuts. Everything here is symmetric.
- `clip-path` **cannot take a border**, so chamfered cards render as flat fills with no edge line.
  The engineered-panel quality that justifies the shape is lost. Needs a border-image, a layered
  pseudo-element, or an inset background approach.
- Honeycomb tessellation and the node-matrix motif (`02-identity.md` §2) are absent entirely.

### 4. Space, rhythm and layout

- Every `<section>` uses identical `5rem 2.5rem` padding (`styles.css:209-214`). No section carries
  more weight than any other, so there is no hierarchy — the page reads as one long uniform list.
- The body is essentially **eleven rounds of the same 2-up card grid**. The effect chain alone is 11
  equal-weight cards (`index.html:71-116`), which is a wall, not an index.
- `.two-column` sets a `--panel-edge` background *and* a `2rem` gap (`styles.css:234-242`), creating
  an unintended double-frame around the grid.
- The `index-mockups.html` trial places device frames floated beside text with no shared alignment,
  leaving large dead zones. The frames themselves are good; the composition around them isn't.

The result reads as *unfinished* rather than *restrained*. `02-identity.md` asks for Swiss-grid
rigour, which is about deliberate structure, not uniform stacking.

### 5. The two-register system is missing

`02-identity.md` §6 is explicit — a **clinical** base (flat, gridded, monospace, cyan-led) that
**erupts** into the **rave-flyer** register at hero and marketing moments. Attitude dial sits
balanced.

The site is monotone clinical throughout. The one moment that should erupt — the hero — is a stock
crowd/stage photograph (`images/jolt-hero.jpg`, `styles.css:166-168`), which is neither register and
sits close to the "no neon-fog / game-trailer look" guardrail in §10. There is no halftone, glitch
or print-grain layer anywhere, which §5 identifies as a genuine differentiator in a uniformly
flat-clean field.

## Issues that aren't styling

These are worth separating out — they'd matter even if the visual direction stayed as-is.

1. **There is no App Store link on the site.** Not in `index.html`, not in `privacy.html`, not in
   `styles.css`. The demo's own download CTA points at `/` (`onboarding.html:3025`, `:3334`).
   Highest-value fix in this document regardless of design direction.
2. **The footer credits an individual** — "Copyright © 2026 EdJ / Jolt Groovebox"
   (`index.html:187`). `01-foundations.md` §6.2 decided LHMP is **faceless**: no named founder on
   customer-facing surfaces, personal credits retired. A Tier-1 flagship carries a discreet
   "by LHMP" endorsement instead (§5).
3. **The demo is unlinked.** A playable in-browser groovebox is the strongest proof-of-product
   available and costs one nav item to expose.
4. **No LHMP endorsement mark** anywhere on the site, which is the one thing Tier-1 flagships are
   actually required to carry.

## Dead weight

| Asset | Size | Status |
| --- | --- | --- |
| `images/cropped.jpg` | 1.1 MB | Unreferenced by any HTML/CSS |
| `images/Automate-1/2/3.svg` | ~146 KB | Unreferenced |
| Google Fonts request | — | Removable once Geist Mono is wired locally |

`index-mockups.html` also ships ~150KB of inline CSS with fonts embedded as data-URIs *while* the
site loads its own copies from `fonts/` and `geist/` — see `MOCKUPS.md` "Known limitations."

## Proposed work

Tiered so each stage is shippable on its own and later stages build on earlier ones.

### Tier 1 — Retokenise *(contained, high impact)*

Scope: `styles.css` `:root` and type rules only. No markup changes.

- Rewrite the token block against `02-identity.md` §1 values.
- Restore a genuinely dimmer `--text-dim`; give `--line` a real value distinct from `--panel`.
- Drop the Google Fonts import; wire Geist Mono from `geist/GeistMono/webfonts/`.
- Resolve the display face (see Open decisions) and wire it.
- Remove the unused `--hot` / `--cold`, or repoint them at the house secondaries if a use exists.

This alone moves the site into the family and is the prerequisite for everything below.

### Tier 2 — Space and hierarchy

- Differentiate section weight; stop every band sharing one padding value.
- Rework the 11-item effect chain as a dense technical index — monospace spec table with model/rev
  codes per `02-identity.md` §8 — rather than 11 equal cards.
- Give the chamfer a visible edge, and scale the cut by element size.
- Fix the `.two-column` double-frame.

### Tier 3 — Content and proof

- Replace the stock hero with real app screens from `mockups.html`, art-directed rather than
  centred-and-floated.
- Persistent App Store CTA.
- Link the demo from the nav.
- LHMP endorsement in the footer; retire the personal credit.

### Tier 4 — Register

- Introduce the rave-flyer eruption at the hero: halftone / grain / glitch layer, acid used rarely
  and deliberately rather than at 7% opacity.
- Honeycomb texture and/or node-matrix motif as supporting graphics.

## Open decisions

These need a human call — they are not derivable from the spec.

1. **Display face: Chakra Petch vs GeistPixel.** `02-identity.md` §4 explicitly parks this to be
   "resolved with specimens in situ" — and the site *is* the in-situ. Chakra reads as
   house-deadpan and matches the app's own display face; GeistPixel is Jolt's established
   playful/lo-fi flavour and is what's live today. As a Tier-1 flagship, Jolt is permitted its own
   voice, so this is genuinely open rather than a compliance question.
2. **Evolve `index.html` or rebuild it.** Retokenising gets most of the way cheaply, but the
   underlying single stacked column actively fights the Tier-2 hierarchy goals. A fresh page built
   around the mockup frames may be less work than retrofitting.
3. **Attitude by tier** (`02-identity.md`, still-open item 3) — whether a Tier-1 flagship like Jolt
   runs calmer than the LHMP hub. Directly determines how far Tier 4 should go.
4. **App Store status** — whether there is a live listing to link to yet, or whether the CTA should
   be a TestFlight / mailing-list capture in the interim.

## Notes for whoever picks this up

- The app screens in `mockups.html` are already correct against the house tokens — they're the best
  reference in this repo for what "on-style" looks like in HTML.
- Verify rendering with headless Chrome, not the artifact viewer (`MOCKUPS.md` explains why). Note
  that a tall `--window-size` distorts `svh` units, so the hero needs its `min-height` pinned before
  full-page capture or it will balloon to thousands of pixels.
- Re-read `02-identity.md` §10 (differentiation guardrails) before adding anything expressive — the
  near-neighbour traps are specific and several are easy to fall into accidentally.

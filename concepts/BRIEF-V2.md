# Concept brief — v2

Read this in full before building. It supersedes the v1 briefs. Your per-concept prompt names your
reference and your output file; everything else is here.

---

## 1. What went wrong in v1 — do not repeat it

**a. The pages were far too dense.** Every v1 concept tried to carry the whole feature set. Section 2
below is a hard space budget, and it is the single most important requirement in this document.

**b. Three claims were false.** They came from the v1 brief, not from the agents:

- **There is no all-at-once eight-lane overview.** It was built and then *sunset* — it became
  impossible to follow interactions once song sequencing got complex. Any copy about "the whole
  pattern at once", "all eight at once", "one screen", "no page-flipping" is wrong. Delete on sight.
- **The `jolt-a…e.png` renders were early mockups of that removed view.** They are quarantined in
  `assets/_stale/`. `assets/ui/jolt-*.png` are now placeholder cards. **Do not use either.**
- **The iPad advantage is real but much smaller than v1 claimed.** From `AdaptivePanelView` in the
  app: iPad merges the editor clusters onto one surface where iPhone uses sub-tabs; the note editor
  shows three lanes on iPad against one on iPhone; the composer shows all four segments rather than a
  cut-off scroll. That is *more controls resident, less menu-diving*. Nothing more.

---

## 2. Space — the hard budget

**Target: the whole page is ≤ 5 viewport-heights at 1440×900 (≈4500px).** Verify this. Render at
`--window-size=1440,4500` and the footer must be visible in that shot. If it is not, cut content —
not type size, not padding.

Rules that get you there:

- **Six sections maximum**, excluding nav, final CTA and footer.
- **One idea per section.** A section is: a headline, at most ~40 words of body, and one figure.
- **One list per page, maximum.** The eleven insert effects, four sends, five macros and three
  modulation layers must NOT all be enumerated. Name the counts, or name three examples and stop.
  If your reference genuinely demands a full index (02 only), that index *is* your one list and the
  rest of the page gets correspondingly leaner.
- **Generous vertical rhythm** — section padding of at least `clamp(5rem, 10vw, 9rem)`. Air is the
  deliverable here; resist filling it.
- Body copy sits in a measure of ~60–70 characters. Do not run text the full page width.

If you finish and the page feels sparse, that is the target, not a failure.

---

## 3. The product figures — use the real mockups

`../mockups.html` is the master gallery: 5 screens × 2 platforms, hand-built from the app's real
colour tokens, real fonts and the real icon assets, traced to specific SwiftUI views. `../MOCKUPS.md`
documents it. These are idealised marketing staging, not simulator captures — but they are derived
from the design system and are the best product truth available.

**How to embed them as live DOM:**

1. `<link rel="stylesheet" href="../mockup-screens.css">` — tokens are scoped under `.jolt-mock`
2. Copy the screen markup you need from `../index-mockups.html`. The wrapper pattern is:
   ```html
   <div class="jolt-mock device"><div class="viewport"><div class="screen" id="scr-editor">…</div></div></div>
   ```
   iPad variants use `class="screen pad"` and their own ids (`scr-composer-pad`, etc.)
3. `<script src="../mockup-screens.js"></script>` at the end of body — it populates the mixer strips
   and the pattern grid by id.

Available screens: **Pattern Editor · Note Editor · Pattern Composer · Mixer · Macro FX**, each in an
iPhone (390×844 portrait) and an iPad (1194×834 landscape) frame.

**Pick one or two. Not five.** A single well-placed screen beats a gallery.

---

## 4. Real values — corrected

**Instrument colours** come from `InstrumentColors.fixed`, via `mockup-screens.js`:

```
KICK #7DFBFD · SNARE #CFFB52 · HAT #FF9500 · CHORD #EA3EE7
```

Note HAT is **orange**, not the magenta the v1 brief gave you. Use these, not the v1 spectrum.

**Geometry is settled: the app already uses chamfers.** `ChamferedRectangle(chamferSize: 8)`
throughout, 6pt diagonal chamfer on button chrome. So the chamfer is canonical in the product — no
conflict with the house style. Corners are cut, never rounded.

**The pattern editor shows one voice's lane, 16 steps as 2 rows of 8.** Each step carries its own
instrument icon in its own colour, and steps can show ratchet counts and slice letters. This means
**the per-step-instrument mechanic is directly visible in the real UI** — that screen is your best
possible asset for the central claim.

---

## 5. Product facts — the whole verified list

8 voices, each with its own step sequencer. Any voice plays any instrument on any step; multiple
instruments mix into one pattern. Patterns up to 64 steps — programmed, MIDI input, or generative.
Patterns chain into songs with automation. Performance mode with live parameter control. Light
external MIDI sequencing. iPhone (portrait) and iPad (landscape).

Instruments — **Classic Synth** (wavetable; 4 modes: spread, 2-op FM, paraphony, in-scale chords) ·
**Macro Synth** (16 sub-synths based on a classic Eurorack module) · **Sampler** (pitched or slicing,
old-skool repitching, per-step reversing, mic recording, mp3/wav) · **Drum Synth** (x0x-style).

11 insert effects, any instrument, any number, any order: Overdrive, Saturator, Chorus, Filter
(4-stage ladder or resonant SVF LP/HP/BP with mod envelope), Band/EQ, Erode, Bitcrush, Sample Rate
Reduction, Phaser, Gater (16-step synced), Stereo Pan.
4 sends: Reverb (plate), Delay (synced ping-pong + feedback), Multi-tap (8-tap), Sidechain (kick-keyed).
Up to 5 global macros from: reverb, delay, phaser, beat repeat, resonant LP DJ filter, resonant HP DJ filter.
Modulation, 3 layers: per-instrument LFO (sine/ramp/random, freely assignable), pattern automation
lane, song automation lane (bar-to-bar or real-time fader override).

**Canonical count: 11 insert effects and 4 sends, stated separately.** Do not aggregate to "15".

---

## 6. Positioning

Tagline: **"Groovebox simplicity, Tracker flexibility"**

- The mechanic: **any voice plays a different instrument on any step.** Tracker semantics, groovebox
  interface. It is the most unusual thing in the product.
- The benefit: **a groovebox you don't outgrow.** Fun immediately, deep enough you don't hit a wall.
- Audience: hardware-culture people who own an iPad — Elektron / Teenage Engineering / M8 curious.
  Also people who find trackers interesting but intimidating. *Not* iOS power users who live in AUM.
- Voice: LHMP house essence is **"Precision instruments, delivered deadpan."** Dry, technical-manual,
  understated. Never aspirational — no "unleash your creativity", no exclamation marks.

**A line worth using.** The overview view was removed because it stopped being comprehensible once
songs got complex. Choosing legibility over density, and saying so plainly, is direct evidence for
the *simplicity* half of the tagline and is exactly the register the house voice wants.

---

## 7. Hard constraints

- **Invent NO social proof.** No ratings, review counts, press quotes, artist names, user numbers,
  awards. None exists.
- **Invent no features.** If it is not in §5, it does not go on the page. v1 invented a "hold a step"
  gesture, a global scale lock, automation *recording*, and an "add voice" control. Do not.
- **Describe the browser demo consistently and modestly**: a slice of the engine — the real DSP,
  compiled to WebAssembly, behind a simple sequencer. It is not the app. Do not claim four
  instruments or the full engine.
- **No unverified negative claims.** "No subscription / no watermark / no track limits" are
  *unconfirmed*; "no account / no ads" were invented. Leave all of them out this round.
- **Footer: no personal credit.** `01-foundations.md` §6.2 makes LHMP faceless — no named founder on
  customer-facing surfaces. Use an LHMP endorsement, not "© EdJ".
- App Store CTA → `#appstore`. Demo link → `../onboarding.html`.
- No external network requests. Fonts: `../geist/GeistMono/webfonts/GeistMono-{Regular,Medium,Bold}.woff2`,
  `../fonts/ChakraPetch-{Bold,SemiBold}.ttf`. Chakra Petch display, Geist Mono body/labels.
- Marks available: `assets/marks/jolt-wordmark.svg` (hard-cornered — Tier-1),
  `assets/marks/lhmp-wordmark-chamfer.svg`, `assets/marks/lhmp-dial-jolt.svg`.
- Photographic plates: `assets/plates/*.png` — halftone/riso/newsprint treatments. At most one.

## 8. Tokens

```
--ground:#0B0C0F  --s1:#14161D  --s2:#1E222C  --line:#3A3F4B
--text:#EDEFF3  --muted:#8B93A6
--cyan:#7DFBFD   (reserved — state and accent only)
--orange:#FF6A00 (subordinate)   --acid:#E8FF00 (rare, high impact)
```

`clip-path` cannot take a border — for a visible chamfered edge, layer an outer chamfered element in
`--line` over a 1px-inset inner in the surface colour.

## 9. Verify before you finish

```sh
CHROME="/Applications/Google Chrome.app/Contents/MacOS/Google Chrome"
"$CHROME" --headless=old --disable-gpu --hide-scrollbars --no-first-run \
  --user-data-dir=/tmp/cc-vN --window-size=1440,4500 \
  --screenshot=/tmp/vN.png "file:///Users/edjames/git/joltsite/concepts/<your-file>"
```

Read the PNG. **The footer must be visible.** Then check: does any screen-height of it feel crowded?
If yes, cut. Iterate at least twice.

Gotchas: don't use bare `vh` (headless sets the window to full page height); headless Chrome on macOS
enforces a ~500px minimum width, so narrow renders crop rather than reflow.

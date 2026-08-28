# Concept 06 — the synthesis brief

Written 2026-08-20. Supersedes nothing; it sits alongside `BRIEF-V2.md`, which still governs facts,
claims and the space budget. This document decides **which mechanism comes from which concept**, and
why, so that 06 is a considered assembly rather than a collage.

Direction from Ed, verbatim in intent: the style and open space of **04**, the movement of **05**,
the ways-in thinking of **03**, the specification of **02**, the iconography of **01** — with *more*
open space and *more* visual intensity than any of the five currently has.

---

## 1. What we take, and the mechanism behind it

The point of this section is that each item names a real, transplantable mechanism. "The style of 04"
is not buildable; `min-height:min(100svh,446px)` on a scroll-snapped panel is.

### From 04 — the chassis, and the source of the open space

**This is the base file.** Everything else is grafted onto it.

| Mechanism | Where | Why it is the one |
| --- | --- | --- |
| `.panel` — `min-height:min(100svh,446px)`, `scroll-snap-align:start`, `justify-content:center` | `04:88-101` | The open space is *structural*. Space comes from the panel being taller than its content, not from padding. This is why 04 breathes and the other four do not — and it is why more content does not automatically mean a longer page. |
| Duotone theme pairs — `--bg / --ink / --dim / --hi / --rule` per panel | `04:104-110` | Every panel is a deliberate colour event, and every component inside inherits the right values automatically. Adding a panel costs one class. |
| Tone-on-tone ink | header comment, `04:20-22` | Display type is a tonal step of *its own ground*, never accent-on-dark. Full-strength cyan is spent exactly once on the page. This is the discipline that lets the two eruption panels land. |
| Display scale — `clamp(2.4rem,7.8vw,6.9rem)`, and `clamp(2.75rem,11.2vw,10.5rem)` on the acid panel | `04:117-124` | The type carries the load, so the panel needs almost nothing else in it. Intensity and emptiness from the same move. |
| Fixed grain — `opacity:.15; mix-blend-mode:overlay`, SVG `feTurbulence`, no image file | `04:215-221` | `REDESIGN.md` §5 flags the absence of any grain/halftone layer as a missed differentiator, and `LANDING-PAGES.md` identifies it as the single biggest reason Linear's near-black reads as engineered. 04 is the only concept that has it. |
| The tDR apparatus — exposed 12-column grid, registration targets, hazard chevrons, chamfered stamps, deadpan smallprint | `04:243-297` | Corporate/technical language at rave intensity. All CSS, no images. This *is* the "visual intensity" Ed is asking for, and it is the register `02-identity.md` §6 asks for. |
| Right-edge floorplan index | `04:203-213` | A panel-based page needs a position indicator. Costs one fixed element. |
| The arrow — one SVG symbol, never a filled button | `04:155-174` | Consistent with the deadpan voice. A filled CTA button is the one exception, reserved for the App Store line. |

### From 05 — the movement

**Take the hero mechanic. Leave the density.**

The running playhead is the smaller half of it. The real find is that **every step in the hero lane is
a button that cycles its instrument on click** (`05` script, `paint()` / click handler). The central
claim — *any voice plays any instrument on any step* — is demonstrated by the visitor's own finger
instead of asserted in a headline. Nothing else in the five does this, and per `LANDING-PAGES.md`
nothing in the sixteen-site survey does either.

Specifics worth preserving as built:

- Transport at 125ms — 16ths at 120bpm — gated behind `prefers-reduced-motion: reduce`.
- A live readout, `01 / 16 — KICK`, on hover and on focus.
- `aria-label` per step, rewritten on every repaint. It is keyboard-reachable and screen-reader-correct.
- Blinking transport dot, `steps(2)` — no eased fade. Machine, not breathing.

**Do not take** 05's page length or its browser-demo copy: it claims "four instruments" and "the
shipping synthesis code", which `CLAIMS-AUDIT.md` §3 marks as contradicting 02. 02's framing wins.

### From 03 — the ways in

The tabs are the obvious part and the least interesting. The mechanism worth transplanting is that
**one product screen stays on the page and a labelled focus rectangle moves across it**, tinted to
that door's instrument colour — `data-foc="left,top,width,height"` in percentages plus `data-label`,
driven by `moveFocus()`.

That device *is* the argument: four different doors, visibly one machine. It also solves the problem
that made 02 weak — a device frame with nothing directing the eye leaves a void. Here the rectangle
does the directing.

Proper `role="tablist"` with roving `tabindex` is already implemented; keep it.

**Rewrite every word of the copy.** `CLAIMS-AUDIT.md` §2 lists 03 as the most specific and least
sourced page in the set: it invents a hold-a-step gesture, a global scale lock, automation *recording*
and an "+ Add voice" control. Structure survives, copy is rebuilt from `BRIEF-V2.md` §5.

### From 02 — the specification

| Mechanism | Where | Why |
| --- | --- | --- |
| Asymmetric four-column grid `1fr 1.62fr 1.16fr .96fr` with hairline column rules | `02:156-159` | Reads as a datasheet, not a table. The unequal columns are what stop it looking like a pricing grid. |
| Group heads with cyan index and a right-aligned reference code | `02:161-164` | Technical-manual register delivered mechanically. |
| Key/value rows, value right-aligned in cyan, optional description spanning both columns | `02:170-175` | Dense without being cramped. |
| **`.r.hi`** — a cyan gradient wash on exactly one row | `02:176-177` | The best single idea in 02. The spec sheet *argues*: `PER-STEP INSTRUMENT · Yes` is lit and everything around it is not. A specification that makes a point is far stronger than a specification that lists. |

This section is the page's **one list**, as `BRIEF-V2.md` §2 requires. Nothing else on the page
enumerates.

### From 01 — the iconography

The instrument icons are **the app's own icon assets**, embedded in `mockup-screens.css` as base64
PNGs (`--ic-kick` … `--ic-noise`) and applied through `.ink` as a CSS `mask`, not as an `<img>`
(`mockup-screens.css:241-247`).

Because they are masks they tint to any instrument colour, at any size, on any ground, with one
custom property. That is what makes the coloured step cells work — icon and cell are the same object.
It is also, per `LANDING-PAGES.md`, the Elektron move (ship the product's own screen typeface/assets
as site furniture) which a software instrument has a stronger claim to than a hardware brand does.

Also take:

- Chamfered coloured cells carrying icon + step number + ratchet ticks + slice letters.
- `FIG 0.x` captions in mono at low opacity — the Linear documentation device.
- The five-step elevation ramp `--e0 … --e4` (`01:29-30`) rather than three flat surfaces.

---

## 2. What we deliberately leave

- **01's temperature.** Its own note asks "does it argue hard enough?" and the answer is no. Take the
  figures, not the calm.
- **02's iPad hero.** Its stated flaw is real: the step grid ends up the smallest thing in a large
  frame. Use iPhone frames, or crop to the region that matters.
- **05's page density**, and its demo claims.
- **03's 1×16 schematic.** The app draws 2 rows of 8. 05 and 04 both get this right; 03 does not.
- **04's invented standards marks.** `CLASS II`, `LHMP ◆ STD 04`, `ISSUE 02`, `JLT-8V REV 12`. The
  *form* is exactly right and stays. Every *string* must become true or go — see open question Q2.
- **A second photographic plate.** `BRIEF-V2.md` §7 allows at most one. Both 04 and 05 currently run
  a crowd image; 06 gets one, and the treated riso/dither plate is preferred over the colour-halftone,
  which reads closest to the "no neon-fog" guardrail in `02-identity.md` §10.

---

## 3. Tensions that had to be resolved

**a. Panel-snap versus the spec sheet.** 04's chassis is fixed-height snapped panels. 02's
specification is one tall dense block. They genuinely fight.

*Resolution:* the specification is the one panel that opts out of `--panelh` and `scroll-snap-align`.
The page goes quiet, dense and long exactly once, in the middle. A deliberate break in rhythm reads as
intent; it is also the only honest way to carry a real datasheet.

**b. Length.** This is the hard one. 04 is 8 panels ≈ 3600px. The specification alone is ≈1200px, a
ways-in section ≈900px. Naively assembled, 06 lands near 6000px against a **4500px budget**.

*Resolution:* ways-in **replaces** a panel rather than adding one — it takes over 04's `p05`
"Interface" slot, which already held a device frame and made a weaker version of the same point. The
four instrument-engine cards from 05 fold into the specification's second column instead of getting
their own section. Net: 8 panels, one of them tall.

**c. Display face — this resolves `REDESIGN.md` open decision #1.** 04 already runs both, with a clean
division of labour: **Chakra Petch** for all display type, **GeistPixel** demoted to machine codes,
the floorplan index and stamp furniture. That is a better answer than choosing one, because it gives
GeistPixel a *job* rather than a *role*. Adopt it. (Chakra is currently loaded as TTF; convert to
woff2 — `MOCKUPS.md` has the recipe, but subset wider than the documented range.)

**d. Eruption count.** `BRIEF-V2.md` §8 says acid is "rare, high impact" and orange "subordinate";
04 spends a full-bleed panel on each. Ed's direction — more intensity — settles it: **two eruptions
stand**, and the discipline moves to everywhere else staying clinical. One acid panel, one orange
panel, and full-strength cyan spent once, on the App Store line.

---

## 4. The shape of 06

Nine panels. One list. One plate. Two eruptions. (Eight in the original plan — see §6.)

| # | Panel | Tone | Built from |
| --- | --- | --- | --- |
| 01 | **Cover** — the iPhone frame as the subject, its pattern editor running, every step clickable | ground | 04 chrome + type · **05 hero mechanic**, moved into the real UI · 01 icons |
| 02 | **The mechanic** — "Any voice. Any instrument. Any step." | cyan-quiet | 04 display scale · 01 icon cells as `FIG 01` |
| 03 | **Ways in** — four doors, one iPhone screen, focus rectangle moving per door | s1 | **03 mechanism**, copy rebuilt from `BRIEF-V2.md` §5 |
| 04 | **The point** — "A groovebox you don't outgrow." | **acid** | 04 `p04`, unchanged — the page's loudest moment |
| 05 | **Specification** — four-column datasheet, `PER-STEP INSTRUMENT · Yes` washed cyan. Opts out of snap. | s1 | **02 spec grid** + 05's engine cards folded in |
| 06 | **Performance** — "Program it. Generate it. Play it live." Chevrons, stamps, smallprint. | **orange** | 04 `p06` + tDR apparatus |
| 07 | **Demo** — 02's honest framing: a slice of the engine, real DSP to WebAssembly, not the app | plate | 04 `p07` staging · **02 copy** |
| 08 | **Get it** — wordmark, App Store, LHMP endorsement, no personal credit | ground | 04 `p08` |

Grain layer over all of it. Floorplan index at the right edge throughout.

---

## 5. Open questions

**Q1 — is the ways-in section earning its panel?** It is the one import whose copy has to be written
from scratch rather than lifted, and `LANDING-PAGES.md` rates the convergence of Ableton Note and
Loopy Pro on this structure as a strong signal. But it is also the softest, least technical thing on
an otherwise hard page. Build it and judge it in situ.

**Q2 — the standards marks.** *Settled — `wsp` decision `jolt-site d2`.* Two marks survive and both
are true: `JLT-8V` as the model code, and `LHMP ◆ IOS 01` — the first iOS instrument in the LHMP line.
`CLASS II`, `ISSUE 02` and `REV 12` are gone; all three asserted conformance or a revision history
that does not exist. The apparatus that carried them stays. It was never the stamps that made that
register work, it was the exposed grid, the registration targets and the chevrons.

**Q3 — App Store CTA.** Settled: `wsp` decision `jolt-site d1`. The `#appstore` placeholder stands,
the real URL drops in at launch, and copy does not hedge about availability.

---

## 6. What the build settled

**The height budget is suspended for this iteration** (`wsp` decision `jolt-site d3`) — see it whole,
then cut. Suspended, not repealed. The one-list rule is untouched: the specification is the only
enumeration on the page.

That changed one thing in §4's plan. Ways in no longer has to *replace* the Interface panel, so both
run, and 06 is nine panels rather than eight. The Interface panel lost its device frame in the
process — Ways in already has one, and two copies of the same screen on one page is repetition rather
than evidence. It now carries the legibility-over-density argument on type and stamps alone.

**A gotcha worth writing down.** `mockup-screens.css` scopes the instrument icon tokens
(`--ic-kick` … `--ic-noise`) under **`.jolt-mock`**, not `:root` — 04 only ever used them inside the
embedded editor, so it never noticed. Anything drawing those icons *outside* a `.jolt-mock` wrapper —
the hero lane, for one — gets no mask and renders a solid filled square instead of an icon. 05 worked
around it by re-declaring the tokens in its own `:root`. 06's build script extracts the four it needs
from the stylesheet at build time, so the two copies cannot drift.

Build script: it is not checked in — 06 is a normal file now and should be hand-edited from here.

## 7. The cover, second pass

Rebuilt on direction: more space, more visual intensity, the device as the subject.

- **The mockup is the hero object, and the sequencer inside it moves.** Rather than a stylised lane
  beside a screenshot, the page script takes over the cells `mockup-screens.js` builds and runs the
  playhead and the click-to-reassign gesture **in the product UI itself**. Every step is a real
  button — `role="button"`, keyboard-reachable, `aria-label` rewritten on each change.
- **The phone is deliberately subordinate.** `fitScreens()` sizes the canvas to `parentWidth - 30`
  and caps it at the screen's own 390px, so the parent width *is* the device size; it is set to 280px,
  well under the cap, and the headline runs at `clamp(2.5rem,7.2vw,6.8rem)` — hotter than the page's
  own xl step. The headline leads, the device corroborates.
- **No eyebrow, no figure caption on this panel.** The cover is the one screen that carries no
  technical apparatus; the sub-text is hand-broken to two lines rather than left to wrap.
- **The four claim pads moved off the cover** onto the mechanic panel. They were the densest thing on
  the first screen and the cover is the one place that had to breathe.
- **Two CTAs of equal weight.** The App Store badge, and the demo as a chamfered cyan button with a
  live dot rather than a text link. Per `LANDING-PAGES.md` the demo is the thing 0 of 16 competitors
  have, so it is not a footnote to the buy action.
- **The floorplan index stands down over the cover** and arrives with panel 02. It sits exactly where
  the device does, and the cover has nothing to index.

**The App Store badge is Apple's own artwork**, from the official pack now in the repo at
`Download-on-the-App-Store/`. The US/UK **black lockup** is the one in use — its `#a6a6a6` border is
what keeps it legible on a near-black ground, and it stays tonally quiet beside the cyan demo button.
The white lockup sits next to it in `assets/marks/` if the buy action ever needs to shout.

**Open — the click affordance is now unsignposted.** The cover's figure caption said "click any step
to change the instrument that plays it", and it was cut with the rest of the technical text. The
gesture still works and is still keyboard-reachable, but nothing on the page tells anyone it is
there. Either it earns one short line back somewhere quieter, or the hover state has to do the whole
job of advertising itself.

**Open — there are now two iPhones on the page**, the cover and Ways in, both showing the pattern
editor. They are doing different jobs at different scales and are two panels apart, but it is
repetition. The fix, if it bothers: move Ways in to the **iPad**, which would also give its focus
rectangle more room and put real weight behind the platform copy. That means re-deriving the four
`data-foc` rectangles against the iPad geometry, which has to be measured in a browser rather than
guessed.


---

## 8. Concept 07 — Open

A cut-down copy of 06, built on direction toward the openness of **dirtywave.com**: less chrome,
more air. `07-open.html`. 06 is untouched and both are in the browser.

**Running order.** Hero · The mechanic · *spacer* · Four doors, one machine · Specification ·
*spacer* · Demo · footer strip. Seven sections against 06's nine.

**Cut.** The Interface panel and the Get-it panel — the latter reduced to a footer strip carrying the
wordmark, both CTAs and the LHMP endorsement. The two eruption panels are repurposed as the spacers:
orange after the mechanic, acid before the demo.

**Apparatus removed**, which is most of what "less chrome" meant: registration targets, the exposed
column grid, hazard chevrons, stamps, deadpan small print, the floorplan index, the section numbers
and rules inside the eyebrows, the in-page "next section" arrows, and the vertical rules between the
specification's columns. The claim pads lost their boxes and became four numbers behind a hairline.
The grain stays at roughly half strength — it is the one texture doing structural rather than
decorative work.

**Space is padding now, not a fixed panel height.** 04's chassis sized every panel to
`min(100svh,446px)` and centred its content. Here panels size to their content and carry
`clamp(96px,15vh,200px)` top and bottom, so a gap between two sections is the sum of two paddings —
about 400px at desktop. Scroll snapping is off. The hero keeps its viewport height.

**Placeholder imagery.** The demo plate is marked as placeholder in the markup — a treated plate
standing in until there is real photography, with the scrim and layout built to survive the swap. The
spacers are colour fields; `.panel.spacer` will take a plate the same way the demo panel does if a
picture should carry one of them instead.

**Note it did not get shorter.** ~6,680px against 06's ~6,440. Fewer sections, but the gaps are the
point and they cost what the deleted panels saved. If length is itself the target rather than
openness, the next cut is content — the specification is 60% of the remaining page.


### 07, second pass

- **The acid spacer is gone** and the demo panel took the room — `min(92svh,760px)` of plate against
  the 520px it had. Six sections now: hero · mechanic · orange spacer · four doors · specification ·
  demo, then the footer.
- **The cover is the wordmark.** `assets/marks/jolt-wordmark.svg` at `min(52vw,600px)`, the same mark
  the footer sets, with the sub-text carrying the description underneath.
- **The footer pairs the logo with the wordmark**, set to a common *height* rather than a common
  width — `images/logo.svg` is 2:1 and the wordmark 3.3:1, so matching widths would have made the
  logo tower over it.
- **The mechanic figure is the app's own step grid.** It was a schematic in lettered cells (`K`/`S`/
  `H`/`C`); it is now cloned from the same built markup the cover uses, so figure and product cannot
  drift. Not interactive — the cover already offers the gesture, this one is making a point. The
  eyebrow, the colour legend and the `FIG. 02` caption went with it.

**Open — the tagline is now nowhere on the page.** "Groovebox simplicity, tracker flexibility" lived
in the cover headline, which is the wordmark now, and in the old Get-it panel's footer line, which
the footer strip replaced. It survives only in the `<title>`. `BRIEF-V2.md` §6 has it as the
positioning line, so it probably wants a home — the obvious one is a line under the wordmark in the
footer, or above the sub-text on the cover.


### 07, third pass — captions out, halftone back, plates in

**Every `FIG` caption is gone.** They read as lab notes rather than as the product's voice, and with
the figures now drawn from the app's own markup they were labelling something self-evident anyway.

**Where the halftone went, and the rule behind it.** The dot screen (`.tex-halftone`) came off the
deleted acid panel. It is now on:

| Panel | Strength | Why |
| --- | --- | --- |
| Orange spacer | full (`.28`) | A flat colour field is exactly where it lived before. This is the direct rehousing. |
| The mechanic | soft (`.10`) | Replaces the scanline texture. Print grain reads as engineered; scanlines read as sci-fi, which is closer to the guardrails in `02-identity.md` §10. |
| Footer | soft (`.09`), rising from the bottom edge | Closes the page on the same texture it opened with, without competing with anything. |

**The rule, so the next panel does not have to be argued from scratch: dots go on flat fields only.**
Never over photography — the plates carry their own screen and the two moiré — and never behind the
specification, where texture costs legibility and buys nothing. That leaves the hero out too: it
already has the node matrix, and both at once is noise.

**Two placeholder plates**, copied from `../lhmp-brand/brand/references/dither-tests` into
`concepts/assets/placeholder/`:

- **`ice__A-riso.png`** between the four doors and the specification. Cold, wide, bright — the one
  light band on a near-black page, and a hard tonal break before the densest section. Cropped at
  `object-position:50% 72%` so the ridge sits in frame rather than the sky.
- **`gold__A-riso.png`** between the specification and the demo. Dark and cyan; a run-up into the
  demo rather than a competitor for it.

Both are full-bleed bands of a fixed height with the picture set to `cover`, so swapping in a
different crop cannot change the page's rhythm. Both carry a `Placeholder imagery` mark at the
bottom right — **delete that span when the real photography lands**.

**Two bugs worth recording.**

1. The first pass masked the plates *inside out*. `mask-image` reads **alpha**: opaque keeps,
   transparent hides. The gradient ran `rgba(…,0)` through the middle, which masked the picture out
   and left only the faded edges — both bands rendered as pale smears. The fade now lives in CSS as
   `transparent → #000 → transparent` and both plates render.
2. The 07 build set each panel's `data-label` with a regex across the whole section block, which also
   overwrote the four `data-label` attributes on the ways-in panes. Those name the focus rectangle,
   so all four doors labelled it "Ways in" instead of "Steps · two rows of eight" and the rest.
   Restored.

---

## 9. 07, fourth pass — one screen per section, and a public preview

**Every section is now a full viewport.** `min-height:100svh` on `main > .panel`, on `.panel.hero`,
on `.panel.spacer`, on the plate bands and on the demo's `.plate-in`. `svh` rather than `vh`
deliberately: `vh` is the mobile-toolbar trap, and headless Chrome resolves bare `vh` against the
whole page.

It is `min-height`, never `height`. The specification is allowed to run past a screen rather than be
clipped by one — a datasheet that has to fit a short laptop viewport stops being a datasheet.

Padding came **down** in the same pass, which sounds backwards and is not: the panel now holds a
screen of height by itself, so padding is no longer making the space — it is only keeping content off
the edges. Panels went from `clamp(96px,15vh,200px)` to `clamp(52px,7.5vh,118px)`, the specification
to `clamp(40px,5.5vh,88px)`, the demo plate to `clamp(48px,6.5vh,110px)`.

Measured at 1440 wide (panel height / viewport):

| Viewport | p01 | p02 | p03 | p04 doors | p05 | p06 spec | p07 | p08 demo |
|---|---|---|---|---|---|---|---|---|
| 1920×1080 (vh 993) | 993 | 993 | 993 | **1004** | 993 | 993 | 993 | 993 |
| 1440×900 (vh 813) | 813 | 813 | 813 | **961** | 813 | **889** | 813 | 813 |
| 1280×800 (vh 713) | 713 | 713 | 713 | **936** | 713 | **877** | 713 | 713 |

Six of eight are exactly one screen at every size. The four-doors panel and the specification run
over on short viewports because their content is genuinely taller than 713px — that is content, not
layout, and the fix if it matters is to cut content.

### The `.panel` collision — worth knowing about

`mockup-screens.css` has its own `.tabcard .panel`: the **instrument panel inside the phone**. The
page's unscoped `.panel` rule was reaching into the mockup and giving that little panel the page's
min-height — 446px inside a 390px-wide phone before this pass, and a whole viewport after it. Every
page-level rule keyed on bare `.panel` is now scoped to `main > .panel`.

Two consequences to remember:

- **`main >` is load-bearing.** Do not "tidy" it away.
- Raising the page rule to a descendant selector (specificity 0,1,1) means single-class rules that
  used to override it now lose. `.photoplate{padding:0}` was one — the demo panel silently gained a
  second screen of padding until it was rescoped to `main > .panel.photoplate`. Check specificity
  when adding panel modifiers.

**06 still has the collision**, left alone deliberately so the two concepts stay comparable. Its
instrument panels are stretched to 446px inside the mockups.

### Rendering 07 in headless Chrome

Two traps, both cost time:

1. **The scroll reveal.** Elements below the first viewport sit at `opacity:0` until an
   IntersectionObserver fires, and nothing scrolls in headless — so a screenshot is a page of blanks.
   Stand it down before capturing: remove `reveal-on` from `<html>` and clear the inline opacity.
2. **Tall windows do not paint.** `--window-size=1440,7101` returns a correct-height PNG with
   everything below roughly 4000px missing, whatever you do about the reveal. It is a compositing
   failure, not a layout bug — the DOM measures correctly at that size.

What works: capture nine viewport-sized slices with `body{margin-top:-Npx}` stepping by the viewport
height, then stitch them. `concepts/render-07.sh` does this. The stitched image repeats
the fixed header at every slice boundary — an artifact of the method, not the page.

### The public preview

`concept07.html` at the site root, generated by `concepts/publish-07.sh`. **The source is
`concepts/07-open.html`; the root copy is generated and must not be hand-edited.**

The root copy exists so the page can be shown to people outside the project *without* publishing
`concepts/` — the briefs, the claims audit and the other six concepts stay unpublished. The script
copies out only the six assets 07 actually references, into `concept-assets/`, rewrites `../foo` to
`foo` and `assets/` to `concept-assets/`, and adds `<meta name="robots" content="noindex,nofollow">`
so an unlisted preview stays unlisted.

---

## 10. 07, fifth pass — the cover regression, the close, the tape

**The cover was broken, and it was the scoping pass that broke it.** `main > .panel > *`
(specificity 0,1,1) outranks `.herostage`, `.tex` and `.plateimg` (all 0,1,0), so every
absolutely-positioned layer in the page was dragged back into normal flow. The hero stage collapsed
to `height:0`, which meant `.herodev`'s `top:50%` resolved against nothing and the phone hung off the
top edge, and the node matrix rendered as a 150px band across the middle of the panel instead of
filling it.

**That one rule has to stay at `.panel > *`.** It is the exception to §9's scoping, and the comment
in the file says so. It reaches into `.tabcard .panel` as a side effect, which costs nothing:
`position:relative` and `z-index:2` on siblings sharing a stacking context change nothing.

The hero's vertical padding is symmetric now. The top pad clears the fixed header; matching it at the
bottom is what puts the copy on the same centreline as the phone. With the old short bottom pad the
copy block sat 25px low against a device that centres on the panel — measured, both are now at 377
of a 726px viewport.

### The close — demo and footer on one screen

They were two consecutive screens repeating the same two CTAs. Now one section: the photograph holds
the top, a `.blend` layer fades the lower 70% to `#0B0C0F` (solid by 60% of the blend, which lands
just above the wordmark), and the footer sits on that. **The delineation is tonal rather than a
second panel costing another whole screen.**

The footer keeps all its own classes and only loses the ground it used to paint for itself. Two
notes: it is a nested `<footer>` now, so it carries `role="contentinfo"` to keep the landmark; and
its duplicate "Play the browser demo" button is gone, because its twin is three hundred pixels up the
same screen. The App Store badge stays — it is the page's last conversion point.

`.foot .tex-halftone` is deleted with it. The footer no longer has a flat field of its own, and dots
over the photograph would moiré — see the rule in §8.

### Cut and restored

- **The gold plate is gone.** Ice stays; it is the one light band on the page and the break before
  the specification.
- **The hazard chevrons are back**, from 06's performance panel, above and below the orange headline.
  The CSS was already in 07 and unused — only the two `<div class="chev">` were missing.
- **p02's sub-line** was "Tracker semantics, groovebox interface. Multiple instruments mix into one
  pattern." It named two other product categories before making its own claim, and the first half
  was a near-relative of the parked tagline. Now: "One pattern holds all eight voices. No track is
  locked to a single sound." — the mechanic, and the thing a classic groovebox cannot do.

Seven surfaces, ~5,450px at a 726px viewport, down from nine and ~7,100.

---

## 11. Editing the copy

The layout is settled enough that the words are the thing being iterated on, so
there is an in-place editor rather than a round trip through this file.

```
./concepts/edit-07.sh                       # generates concepts/07-edit.html
open concepts/07-edit.html                  # file:// is fine — fonts load
```

Every string on the page is editable where it sits. A bar at the bottom counts what
changed and offers **Copy patch** / **Download** / **Reset**; changed strings outline in
orange. Then:

```
python3 concepts/apply-copy.py ~/Downloads/copy-patch.json [--dry-run]
./concepts/publish-07.sh
```

**07-edit.html is generated and local. It is never published** — `publish-07.sh` only
ever reads `07-open.html`.

### What the editor deliberately does

- **The app mockups are not editable.** Their labels belong to the product, not the page;
  editing them here would put marketing copy inside a figure that is supposed to be the
  real UI.
- **An element is edited whole** — a sentence keeps its `<span class="n">64</span>` inline —
  **unless it also holds something drawn**, a live dot or an arrow glyph. Those cannot
  survive a select-all-and-retype, so there the bare text is wrapped and only the words
  become editable.
- **`contenteditable="true"`, not `plaintext-only`.** Chrome renders a plaintext-only
  editable as `pre-wrap`, and no author `white-space` rule overrides it, so the source's
  own indentation after a `<br>` shows up as a real indent and the preview stops matching
  the page. Enter (→ `<br>`) and paste (→ plain text) are intercepted instead.

### What the patch is

A list of `{before, after, nth}`. A string is identified by its **original html** plus which
occurrence of it this is, in document order — readable in a diff, and it survives the file
being regenerated, which an auto-generated selector would not.

`apply-copy.py` matches entity-aware, because the editor reads `innerHTML` where the browser
has already decoded everything: `&mdash;` in the file matches the em dash the editor was
handed, and characters go back in the source's own entity style. **Nothing is written unless
every entry matches** — a partial apply would leave the page in a state neither of us chose.
The previous version is kept at `07-open.html.bak`.

One wrinkle worth knowing: replacing a string that spans several source lines collapses it
onto one. Harmless, but it shows up in the diff.

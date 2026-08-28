# Claims audit — the five concepts

Every factual assertion made across `01`–`05` that is **not** traceable to `index.html` (the only
written spec of the app in this repo). Strike or correct, then the next build inherits a clean list.

Legend — **[?]** unverifiable from this repo · **[X]** believed wrong · **[!]** internally contradictory

---

## 1. The overview claim — [X], and it is in all five

**Status: a sunset feature, not an invention.** An all-at-once view did exist and was removed —
it became impossible to follow the interactions once complex song sequencing was involved. So the
`jolt-*.png` mockups depict a real past state of the app, not a fiction. They are *stale*, not fake.

Two consequences:

1. Every "this is the shipping build" caption is false as written, and must go.
2. **The iPad argument I pushed has to be rebuilt.** I told all five agents the platform advantage
   was "the overview hardware can't give you." The opposite is true: Jolt *deliberately declined*
   that view because it stopped being comprehensible. The remaining, honest iPad advantage is
   smaller but real — larger targets, more parameters resident per panel, less menu-diving than a
   2.8" hardware screen. Argue that, not whole-pattern overview.

There is also a positioning asset buried in the removal. Choosing legibility over density, and
saying so plainly, is direct evidence for the *"groovebox simplicity"* half of the tagline — and it
is exactly the deadpan engineering-honesty register the house voice wants. Worth considering as
copy rather than hiding.

| Concept | Text | Fix |
| --- | --- | --- |
| 01 | Whole section §02 *"The overview hardware can't give you"* · "Grooveboxes and the M8 show you one lane at a time. An iPad shows all eight at once — every voice, every step, the whole pattern, and the parameters beside it. No paging to remember what you wrote." | Delete section + `FIG 0.2` |
| 02 | "designed for iPad, because the overview — the whole pattern, its instruments and the edit panel on one surface — is the thing hardware grooveboxes and pocket trackers cannot give you" · spec row `iPad layout / Overview` | Delete both |
| 03 | "The iPad layout uses the extra room rather than stretching the phone one" **[?]** | Verify or cut |
| 04 | Panel: **"Eight voices, one screen."** · `Fig. 03 — pattern editor` | Rewrite panel |
| 05 | "room for the whole pattern" · "Every lane, every step, the automation, at the same time. No page-flipping to find out what a voice is doing. **You already own the screen.**" · `FIG 0.4` "seven lanes dimmed but still legible" | Delete block + FIG |

**Knock-on:** the `jolt-*.png` renders themselves show eight lanes simultaneously. If that view does
not exist, those files are aspirational mockups, not product truth — and every figure drawn from
them inherits the fiction. This needs resolving before any rebuild.

## 2. Interaction claims invented by concept 03 — [?]

Concept 03's tab copy is the most specific in the set and the least sourced.

- "Hold a step and it stops having to be a drum." — invents a gesture
- "Choose a scale and the wrong notes are simply not there." — implies a global scale lock.
  `index.html` only says the Classic Synth has an *in-scale chords* mode
- "The automation lane records what you moved while it ran." — `index.html` says the song lane takes
  "real-time fader overrides". Recording is a stronger claim
- "no separate generated mode to escape from" — generative behaviour detail
- "**+ Add voice**" in the drum-lanes figure — the app has eight fixed voices
- MIDI figure routes individual voices out on numbered channels (`Rack · MIDI out · ch 4`), which may
  overstate "light external sequencing"

## 3. The browser demo — [!] the concepts contradict each other

| Concept | Claim |
| --- | --- |
| 02 | "A single drum voice of the engine" · `Engine: 1 drum voice · Instruments: None · Insert FX: None` · "It is **not the app**" |
| 05 | `FIG 0.7` "Demo pad layout — **four instruments**, sixteen steps" · "The **shipping** synthesis code" |
| 03 | "runs the app's own sound engine, cut down to a single screen" |
| 04 | "Part of the Jolt engine" |

One of these is right. 02's framing is the most honest and the most defensible.

Also 05: *"As far as we know, no other groovebox lets you do that."* — true against the sixteen sites
surveyed, but it is a competitive claim and reads as a boast. Suggest cutting the hedge and the claim.

## 4. Negative spec list — [?] concept 05 only

Three were sanctioned; two were added. **All five need confirming.**

- No subscription · No watermark · No track limits — *sanctioned, still unverified*
- **No account** · **No ads** — added by the agent

## 5. Effect count — [!]

- 01, 02, 03, 05 → **11 insert effects** (+ 4 sends listed separately)
- 04 → **"Four instruments, fifteen effects, five macros"** (11 + 4 aggregated)

Both are arithmetically fine. Pick one and make it canonical.

## 6. Terminology — [X]

**05: "Polyphonic. 8 voices. 8 step sequencers."** Eight voices each running an independent
instrument is **multitimbral**. Polyphony is notes per voice — which is what the Classic Synth's
paraphony mode does. `Multitimbral.` is correct *and* more impressive to a hardware-literate reader.

## 7. Platform / project claims — [?]

- 01 spec: "Platform · iPad and iPhone — **same project either way**" (project portability)
- 01 `FIG 0.2`: "iPhone runs the same editor, **one panel at a time**"

## 8. Default lane names — [?]

`Kick · Snare · Hat · Bass · Lead · Pad · Arp · FX` appear as defaults in 01, 02, 04, 05. These come
from the brand-repo mockups, not from any written spec. 02 hedges correctly — "named here, but the
name is a label" — the others assert them.

## 9. Footer credit — [X] against brand policy

01, 02, 03, 05 all carry **"© 2026 EdJ / Jolt Groovebox"**, inherited from the live site.
`01-foundations.md` §6.2 decided LHMP is faceless: no named founder on customer-facing surfaces.
04 uses a bare "2026" and is the only one compliant.

---

## Clean by construction

Worth recording, because it was the constraint most at risk: **no concept invented social proof.**
No ratings, review counts, press quotes, artist names, user numbers or awards appear anywhere. Every
concept carries an App Store CTA (`#appstore` placeholder) and a demo link.

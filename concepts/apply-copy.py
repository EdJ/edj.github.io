#!/usr/bin/env python3
"""Write a copy-editor patch into a concept page, cutting a new version.

    python3 concepts/apply-copy.py copy-patch.json --from 07 --to 08
    python3 concepts/apply-copy.py copy-patch.json --from 07 --to 08 --dry-run
    python3 concepts/apply-copy.py copy-patch.json --in-place 07

The patch is a list of {"before", "after", "nth"} produced by the bar at the
bottom of NN-edit.html. Each entry names a string by its ORIGINAL html plus
which occurrence of that html it is, counting in document order.

MATCHING. `before` is always the complete innerHTML of one element, so a match
must be bracketed by that element's own tags — an opening `>` before it, its
closing `</` after — and must fall outside every comment, <script>, <style>,
and every subtree copy-editor.js skips. Drop any one of those and the patch
still applies cleanly, to entirely the wrong place: "M" lands in a @font-face
rule, "Performance" in a data-label attribute, "Sampler" in an app mockup, and
"Let it generate" on the tab button rather than the heading. `nth` then counts
only among real matches, which is the order the editor walked the DOM.

Entities are handled both ways: `&mdash;` in the file matches the em dash the
browser handed the editor, and characters go back in the source's own style.

Nothing is written unless every entry resolves. A partial apply would leave the
page in a state neither of us chose.
"""
import html.entities
import json
import re
import shutil
import sys
from html.parser import HTMLParser
from pathlib import Path

HERE = Path(__file__).resolve().parent

# The editor reads innerHTML, where the browser has already decoded every
# entity: the source's `&mdash;` arrives as an em dash, `&nbsp;` as U+00A0.
# So a string is matched as a pattern in which any character the source could
# have written as an entity matches either form.
ESCAPED = {'&': ['&amp;', '&#38;'], '<': ['&lt;'], '>': ['&gt;']}


def alternatives(ch):
    """Every way the source could spell this one character."""
    forms = [re.escape(ch)]
    cp = ord(ch)
    if ch in ESCAPED:
        forms += [re.escape(f) for f in ESCAPED[ch]]
    elif cp > 127:
        name = html.entities.codepoint2name.get(cp)
        if name:
            forms.append('&%s;' % name)
        forms += ['&#%d;' % cp, '&#x%X;' % cp, '&#x%x;' % cp]
    return forms[0] if len(forms) == 1 else '(?:%s)' % '|'.join(forms)


# Kept in step with SKIP in copy-editor.js. The editor never marks anything in
# these subtrees, so it never assigns them an occurrence index — and if the
# matcher counts them, every `nth` after the first one is off by however many
# it saw. The app mockups are the dangerous case: they are full of short strings
# ("8", "Sampler", "Sound", "Amp") that are also real copy elsewhere on the page.
SKIP_TAGS = {'svg'}
SKIP_CLASSES = {'jolt-mock', 'tex', 'tex-node', 'tex-grid', 'tex-scan',
                'tex-halftone', 'regs', 'chev', 'grain', 'floorplan',
                'asbadge', 'placemark'}
SKIP_IDS = {'ce-bar'}


class _SkipFinder(HTMLParser):
    """Byte spans of every subtree the copy editor refuses to mark."""

    def __init__(self, src):
        super().__init__(convert_charrefs=False)
        self.src = src
        starts = [0]
        for line in src.splitlines(keepends=True):
            starts.append(starts[-1] + len(line))
        self._starts = starts
        self.spans = []
        self._depth = 0
        self._skip_at = None      # depth at which the current skip began
        self._skip_start = None

    def _abs(self):
        line, col = self.getpos()
        return self._starts[line - 1] + col

    def _matches(self, tag, attrs):
        if tag in SKIP_TAGS:
            return True
        a = dict((k, v or '') for k, v in attrs)
        if a.get('id') in SKIP_IDS:
            return True
        return bool(set(a.get('class', '').split()) & SKIP_CLASSES)

    def handle_starttag(self, tag, attrs):
        if self._skip_start is None and self._matches(tag, attrs):
            self._skip_at = self._depth
            self._skip_start = self._abs()
        self._depth += 1

    def handle_startendtag(self, tag, attrs):
        if self._skip_start is None and self._matches(tag, attrs):
            start = self._abs()
            self.spans.append((start, self.src.index('>', start) + 1))

    def handle_endtag(self, tag):
        self._depth -= 1
        if self._skip_start is not None and self._depth == self._skip_at:
            start = self._abs()
            self.spans.append((self._skip_start, self.src.index('>', start) + 1))
            self._skip_start = self._skip_at = None


def dead_regions(src):
    """Spans holding no editable copy.

    Comments, script and style bodies — the page carries a long authoring
    comment in the head and a stylesheet full of English words, and a
    one-character `before` will happily match in there — plus every subtree
    copy-editor.js skips.
    """
    spans = []
    for m in re.finditer(r'<!--.*?-->', src, re.S):
        spans.append(m.span())
    for m in re.finditer(r'<(script|style)\b[^>]*>.*?</\1\s*>', src, re.S | re.I):
        spans.append(m.span())
    f = _SkipFinder(src)
    f.feed(src)
    spans.extend(f.spans)
    return spans


def occurrences(src, needle, dead):
    """Where this innerHTML really appears, in document order.

    Bounded by `>` and `<` because the editor only ever hands back a whole
    element's contents, so anything matching mid-tag or mid-word is not the
    string the person edited.
    """
    pattern = ''.join(alternatives(c) for c in needle)
    out = []
    for m in re.finditer(pattern, src):
        start, end = m.span()
        # `<` after it must open that element's CLOSING tag …
        if not src.startswith('</', end):
            continue
        # … and the `>` before it must end an OPENING one. Without this second
        # half, a trailing text run matches too: in
        # `<span><em>04</em>Let it generate</span>` the words sit between a `>`
        # and a `<`, so a bare "Let it generate" would resolve to the tab
        # button rather than to the <h3> the editor actually meant.
        lt = src.rfind('<', 0, start)
        if lt < 0 or src.startswith('</', lt):
            continue
        if any(a <= start < b for a, b in dead):
            continue
        out.append((start, end))
    return out


# Written back in the source's own style rather than as raw glyphs, so the
# file stays consistent with the hand-authored markup around it.
PRETTY = {'\u00a0': '&nbsp;', '\u2014': '&mdash;', '\u2013': '&ndash;',
          '\u2122': '&trade;', '\u00a9': '&copy;', '\u2022': '&#8226;',
          '\u2264': '&le;', '\u2265': '&ge;', '\u2197': '&#8599;'}


def restyle(text):
    for ch, ent in PRETTY.items():
        text = text.replace(ch, ent)
    return text


def page(version):
    p = HERE / ('%s-open.html' % version)
    if not p.exists():
        sys.exit('no such page: %s' % p)
    return p


def main(argv):
    args = [a for a in argv[1:] if not a.startswith('-')]
    flags = [a for a in argv[1:] if a.startswith('-')]
    dry = '--dry-run' in flags or '-n' in flags

    def opt(name):
        if name in argv:
            i = argv.index(name)
            if i + 1 < len(argv):
                return argv[i + 1]
        return None

    src_v, out_v, inplace = opt('--from'), opt('--to'), opt('--in-place')
    if inplace:
        src_v = out_v = inplace
    patches = [a for a in args if a not in (src_v, out_v)]
    if len(patches) != 1 or not src_v or not out_v:
        sys.exit(__doc__)

    SRC, OUT = page(src_v), HERE / ('%s-open.html' % out_v)

    patch = json.loads(Path(patches[0]).read_text())
    if not isinstance(patch, list):
        sys.exit('patch should be a list of {before, after, nth}')

    src = SRC.read_text()
    dead = dead_regions(src)
    edits, problems, skipped = [], [], []
    claimed = {}

    for k, item in enumerate(patch):
        before, after, nth = item['before'], item['after'], item.get('nth', 0)
        if before == after:
            continue
        hits = occurrences(src, before, dead)
        if not hits:
            problems.append('%d: not found as element text — %r' % (k, before[:70]))
        elif nth >= len(hits):
            # The editor walks main, then footer and header, which sit INSIDE
            # main — so a footer string is marked twice and the second copy
            # arrives with an nth the page cannot have. Same element, same
            # edit: take the first and say so.
            if len(hits) == 1 and before in claimed:
                skipped.append('%d: duplicate of entry %d — %r'
                               % (k, claimed[before], before[:70]))
            else:
                problems.append('%d: wanted occurrence %d, found %d — %r'
                                % (k, nth, len(hits), before[:70]))
        else:
            start, end = hits[nth]
            claimed.setdefault(before, k)
            edits.append((start, end, src[start:end], restyle(after)))

    if problems:
        print('Nothing written. The source has moved on since this patch was made:\n')
        print('\n'.join('  ' + p for p in problems))
        return 1

    if not edits:
        print('No changes in the patch.')
        return 0

    # right to left, so earlier offsets stay valid
    for start, end, before, after in sorted(edits, key=lambda e: -e[0]):
        src = src[:start] + after + src[end:]

    for _, _, before, after in edits:
        print('  - %s\n  + %s\n' % (before, after))
    for s in skipped:
        print('  ~ ' + s)

    if dry:
        print('%d change(s) — dry run, nothing written.' % len(edits))
        return 0

    if OUT == SRC:
        shutil.copy2(SRC, SRC.with_suffix('.html.bak'))
        note = ' (previous version at %s.bak)' % SRC.name
    else:
        if OUT.exists():
            sys.exit('%s already exists — refusing to overwrite a cut' % OUT.name)
        note = ' (from %s, which is untouched)' % SRC.name
    OUT.write_text(src)
    print('%d change(s) written to %s%s' % (len(edits), OUT.name, note))
    print('Now: ./concepts/edit.sh %s      to keep editing' % out_v)
    print('     ./concepts/publish.sh %s   to update the preview' % out_v)
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))

/* ── COPY EDITOR ─────────────────────────────────────────────────────
   Makes every piece of copy on the page editable in place, and exports
   what changed as a patch that apply-copy.py writes back into
   07-open.html. Loaded only by the generated 07-edit.html.

   The patch identifies a string by its ORIGINAL html plus which
   occurrence of that html it is, counting in document order. That
   survives re-generation and is readable in a diff, which an
   auto-generated selector would not be. */
(function () {
  'use strict';

  // Subtrees that are not copy: the app mockups (their labels belong to the
  // product, not the page), decoration, and anything drawn rather than set.
  var SKIP = '.jolt-mock, svg, .tex, .tex-node, .tex-grid, .tex-scan, .tex-halftone, .regs, .chev, .grain, .floorplan, .asbadge, .placemark, #ce-bar';

  var editables = [];

  function hasOwnText(el) {
    for (var n = el.firstChild; n; n = n.nextSibling)
      if (n.nodeType === 3 && n.nodeValue.trim()) return true;
    return false;
  }

  // An element is edited whole — so a sentence with a <span class="n">64</span>
  // in it stays one field — UNLESS it also carries something drawn: a live
  // dot, an arrow glyph. Those cannot survive a select-all-and-retype, so
  // there the bare text is wrapped and only the words become editable.
  function hasDrawnChild(el) {
    for (var i = 0; i < el.children.length; i++) {
      var c = el.children[i];
      if (c.tagName === 'SVG' || c.getAttribute('aria-hidden') === 'true'
          || c.querySelector('svg')) return true;
    }
    return false;
  }

  // Wrap each of the element's own text nodes so the drawn children sit
  // outside anything editable. The wrapper holds the text EXACTLY as the
  // source has it, whitespace included, so the patch still matches the file.
  function wrapText(el) {
    var nodes = [];
    for (var n = el.firstChild; n; n = n.nextSibling)
      if (n.nodeType === 3 && n.nodeValue.trim()) nodes.push(n);
    nodes.forEach(function (n) {
      var w = document.createElement('span');
      w.setAttribute('data-ce-w', '');
      n.parentNode.insertBefore(w, n);
      w.appendChild(n);
      mark(w);
    });
  }

  // Top-down: the outermost element holding its own text wins, so nothing is
  // ever nested inside another editable.
  function walk(el) {
    if (el.nodeType !== 1) return;
    if (el.matches && el.matches(SKIP)) return;
    if (el.closest && el.closest(SKIP)) return;
    if (hasOwnText(el)) {
      if (hasDrawnChild(el)) wrapText(el); else mark(el);
      return;
    }
    for (var i = 0; i < el.children.length; i++) walk(el.children[i]);
  }

  var seen = Object.create(null);   // original html -> how many so far

  function mark(el) {
    var html = el.innerHTML;
    var n = seen[html] === undefined ? 0 : seen[html] + 1;
    seen[html] = n;

    el.setAttribute('data-ce', String(editables.length));
    // 'true', not 'plaintext-only': Chrome renders plaintext-only editables as
    // pre-wrap and no author white-space rule overrides it, so the source's
    // indentation after a <br> shows as a real indent and the preview stops
    // matching the page. Enter and paste are intercepted below instead.
    el.setAttribute('contenteditable', 'true');
    el.setAttribute('spellcheck', 'true');
    editables.push({ el: el, before: html, nth: n });

    el.addEventListener('input', refresh);
    el.addEventListener('blur', refresh);
    // Enter inserts the design's own line break rather than a new block
    el.addEventListener('keydown', function (e) {
      if (e.key === 'Enter') {
        e.preventDefault();
        document.execCommand('insertLineBreak');
      } else if (e.key === 'Escape') {
        el.blur();
      }
    });
  }

  function changed() {
    return editables.filter(function (r) { return r.el.innerHTML !== r.before; });
  }

  function patch() {
    return changed().map(function (r) {
      return { before: r.before, after: r.el.innerHTML, nth: r.nth };
    });
  }

  var count, btnCopy, btnSave, btnReset, msg, msgTimer;

  function refresh() {
    var c = changed();
    count.textContent = c.length;
    btnCopy.disabled = btnSave.disabled = btnReset.disabled = !c.length;
    editables.forEach(function (r) {
      if (r.el.innerHTML !== r.before) r.el.setAttribute('data-ce-dirty', '');
      else r.el.removeAttribute('data-ce-dirty');
    });
  }

  function say(text) {
    msg.textContent = text;
    clearTimeout(msgTimer);
    msgTimer = setTimeout(function () { msg.textContent = ''; }, 2400);
  }

  function buildBar() {
    var bar = document.createElement('div');
    bar.id = 'ce-bar';
    bar.setAttribute('contenteditable', 'false');
    bar.innerHTML =
      '<span><b>0</b> changed</span>' +
      '<span class="ce-sp"></span>' +
      '<button type="button" data-a="copy" class="ce-primary">Copy patch</button>' +
      '<button type="button" data-a="save">Download</button>' +
      '<button type="button" data-a="reset">Reset</button>' +
      '<span class="ce-msg"></span>';
    document.body.appendChild(bar);

    count = bar.querySelector('b');
    msg = bar.querySelector('.ce-msg');
    btnCopy = bar.querySelector('[data-a="copy"]');
    btnSave = bar.querySelector('[data-a="save"]');
    btnReset = bar.querySelector('[data-a="reset"]');

    btnCopy.addEventListener('click', function () {
      var text = JSON.stringify(patch(), null, 2);
      navigator.clipboard.writeText(text).then(
        function () { say('Copied'); },
        function () { window.prompt('Copy this:', text); }
      );
    });

    btnSave.addEventListener('click', function () {
      var blob = new Blob([JSON.stringify(patch(), null, 2)], { type: 'application/json' });
      var a = document.createElement('a');
      a.href = URL.createObjectURL(blob);
      a.download = 'copy-patch.json';
      a.click();
      setTimeout(function () { URL.revokeObjectURL(a.href); }, 4000);
      say('Saved');
    });

    btnReset.addEventListener('click', function () {
      if (!window.confirm('Discard every change on the page?')) return;
      editables.forEach(function (r) { r.el.innerHTML = r.before; });
      refresh();
      say('Reset');
    });
  }

  function start() {
    document.body.classList.add('ce-on');

    // The footer sits INSIDE main on this page, so walking the list naively
    // marks every footer string twice — once from main, once on its own — and
    // the second copy leaves with an nth the source cannot have. Keep only the
    // roots that no other root already contains.
    var roots = ['main', 'footer', 'header']
      .map(function (sel) { return document.querySelector(sel); })
      .filter(Boolean);
    roots.filter(function (r) {
      return !roots.some(function (o) { return o !== r && o.contains(r); });
    }).forEach(walk);

    // Links would navigate away mid-edit, and the hero's step cells would
    // cycle instruments under the cursor.
    document.addEventListener('click', function (e) {
      var hit = e.target.closest('a, .stepwrap, [role="button"]');
      if (hit && !hit.closest('#ce-bar')) { e.preventDefault(); e.stopPropagation(); }
    }, true);

    // Paste arrives as plain text — no pasted fonts, colours or spans
    document.addEventListener('paste', function (e) {
      if (!e.target.closest || !e.target.closest('[data-ce]')) return;
      e.preventDefault();
      var t = (e.clipboardData || window.clipboardData).getData('text/plain');
      document.execCommand('insertText', false, t.replace(/\s*\n\s*/g, ' '));
    });

    window.addEventListener('beforeunload', function (e) {
      if (changed().length) { e.preventDefault(); e.returnValue = ''; }
    });

    buildBar();
    refresh();
    // test hook: lets a headless run drive the round trip without a clipboard
    window.__cePatch = patch;
    console.log('[copy-editor] ' + editables.length + ' editable strings');
  }

  // after mockup-screens.js has built its screens, so nothing is marked twice
  if (document.readyState === 'complete') setTimeout(start, 300);
  else window.addEventListener('load', function () { setTimeout(start, 300); });
})();

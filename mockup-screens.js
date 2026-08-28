
// Populate mixer strips.
(function () {
  const voiceVals = [0.78, 0.62, 0.71, 0.5, 0.66, 0.58, 0.44, 0.7];
  const voiceMeters = [[0.62,0.55],[0.4,0.44],[0.5,0.52],[0.3,0.27],[0.44,0.47],[0.35,0.3],[0.2,0.24],[0.5,0.46]];

  function strip(v, m, label, small, soloed, muted, noSM) {
    return `<div class="col">
      <div class="fader">
        <div class="meters"><b style="--m:${m[0]}"></b><b style="--m:${m[1]}"></b></div>
        <span class="rule" style="top:25%"></span><span class="rule" style="top:50%"></span><span class="rule" style="top:75%"></span>
        <div class="handle" style="--v:${v}"></div>
      </div>
      ${noSM ? '<div style="height:28px"></div><div style="height:28px"></div>'
             : `<span class="chip${soloed ? ' sel' : ''}"><i>S</i></span><span class="chip${muted ? ' sel' : ''}"><i>M</i></span>`}
      <div class="strip-label${small ? ' small' : ''}">${label}</div>
    </div>`;
  }

  const vEl = document.getElementById('voices');
  if (vEl) vEl.innerHTML =
    voiceVals.map((v, i) => strip(v, voiceMeters[i], i + 1, false, false, i === 6)).join('');

  // iPad: Voices and FX panels side by side
  const fxVals = [0.55, 0.6, 0.35, 0.82];
  const fxMeters = [[0.3,0.34],[0.38,0.35],[0.18,0.2],[0.66,0.6]];
  const fxNames = ['Delay', 'Reverb', 'Multitap', 'Master'];
  const vp = document.getElementById('voices-pad'), fp = document.getElementById('fxbus-pad');
  if (vp) vp.innerHTML = voiceVals.map((v, i) => strip(v, voiceMeters[i], i + 1, false, false, i === 6)).join('');
  if (fp) fp.innerHTML = fxVals.map((v, i) => strip(v, fxMeters[i], fxNames[i], true, false, false, i === 3)).join('');
})();

// Pattern editor: 2×8 step grid. Colors = InstrumentColors.fixed.
(function () {
  const KICK = '#7DFBFD', SNARE = '#CFFB52', HAT = '#FF9500', CHORD = '#EA3EE7';
  // one entry per step: {c: color, ic: icon} | null; extras: ratchet, slice, play
  const rows = [
    [
      {c: KICK, ic: 'kick'}, null, {c: HAT, ic: 'hihat', ratchet: 3}, null,
      {c: SNARE, ic: 'snare', play: true}, null, {c: HAT, ic: 'hihat'}, {c: KICK, ic: 'kick', slice: 'A'},
    ],
    [
      {c: KICK, ic: 'kick'}, null, {c: HAT, ic: 'hihat'}, {c: CHORD, ic: 'chord'},
      {c: SNARE, ic: 'snare'}, {c: HAT, ic: 'hihat', ratchet: 2}, null, {c: KICK, ic: 'kick'},
    ],
  ];
  const stepsHtml = rows.map(row =>
    `<div class="steprow">` + row.map((s, i) => {
      const gut = i === 4 ? '<span class="gut"></span>' : '';
      if (!s) return gut + `<span class="stepwrap"><span class="step"></span></span>`;
      const cls = ['step', 'on', s.slice ? 'slice' : ''].join(' ');
      const inner =
        `<span class="ink" style="--ic:var(--ic-${s.ic})"></span>` +
        (s.ratchet ? `<span class="ratchets">${'<b></b>'.repeat(s.ratchet)}</span>` : '') +
        (s.slice ? `<span class="slice-label">${s.slice}</span>` : '');
      return gut + `<span class="stepwrap${s.play ? ' play' : ''}"><span class="${cls}" style="--c:${s.c}">${inner}</span></span>`;
    }).join('') + `</div>`
  ).join('');
  // Regular size class draws each page as ONE row of 16 with its number beside
  // it, in a vertical scroll, with a rule between pages — UnifiedPatternView's
  // `MultipageEntry`, regular branch:
  //   ForEach(pages) { HStack { Text("\(page.id + 1)").frame(width: 20)
  //                             CanvasSequencerPage() } }
  // CanvasSequencerPage defaults to steps:16, offset:0, with a 2pt separator at
  // 4, 8 and 12. Page 1 is the same sixteen cells in the same order as the
  // phone, so one index drives the playhead on both; 2-4 are staged variations
  // and carry no `play`.
  const flat = rows.flat();
  const padPages = [
    flat,
    flat.map((s, i) => ((i % 8 === 3 || i % 8 === 5) ? null : s)),
    flat.map((s, i) => (i === 11 ? {c: CHORD, ic: 'chord'} : (i === 6 ? null : s))),
    flat.map((s, i) => (i % 4 === 0 ? s : (i === 14 ? {c: HAT, ic: 'hihat'} : null))),
  ];
  const padRow = (page, n) =>
    `<div class="padpage"><span class="pgnum">${n + 1}</span><div class="steprow">` +
    page.map((s, i) => {
      const gut = (i % 4 === 0 && i !== 0) ? '<span class="gut"></span>' : '';
      if (!s) return gut + `<span class="stepwrap"><span class="step"></span></span>`;
      const cls = ['step', 'on', s.slice ? 'slice' : ''].join(' ');
      const inner =
        `<span class="ink" style="--ic:var(--ic-${s.ic})"></span>` +
        (s.ratchet ? `<span class="ratchets">${'<b></b>'.repeat(s.ratchet)}</span>` : '') +
        (s.slice ? `<span class="slice-label">${s.slice}</span>` : '');
      const play = (n === 0 && s.play) ? ' play' : '';
      return gut + `<span class="stepwrap${play}"><span class="${cls}" style="--c:${s.c}">${inner}</span></span>`;
    }).join('') +
    `</div></div>`;
  const padHtml = `<div class="padrule"></div>` +
    padPages.map((p, n) => padRow(p, n) + `<div class="padrule"></div>`).join('');
  const sEl = document.getElementById('stepgrid');
  if (sEl) sEl.innerHTML = stepsHtml;
  const padGrid = document.getElementById('stepgrid-pad');
  if (padGrid) padGrid.innerHTML = padHtml;
})();

// Notes view: two piano-roll lanes for the chord instrument (C minor).
(function () {
  const C = '#EA3EE7';
  // 8 rows, C minor, identical in both lanes (top → bottom)
  const ROWS_N = [
    {n: 'C5', root: true}, {n: 'Bb4'}, {n: 'Ab4'}, {n: 'G4'},
    {n: 'F4'}, {n: 'Eb4'}, {n: 'D4'}, {n: 'C4', root: true},
  ];
  // per lane: notes as "row,col" -> kind (on | ghost | sel), active step cols
  const lanes = [
    {notes: {
      '7,0': 'on', '5,0': 'ghost', '3,0': 'ghost',
      '5,2': 'on', '3,2': 'ghost', '1,2': 'ghost',
      '3,4': 'sel', '1,4': 'ghost',
      '4,6': 'on', '2,6': 'ghost', '0,6': 'ghost',
    }, steps: [0, 2, 4, 6], playCol: 4},
    {notes: {
      '4,1': 'on', '6,3': 'on',
      '7,7': 'on', '5,7': 'ghost', '3,7': 'ghost',
    }, steps: [1, 3, 7], playCol: 4},
  ];

  function lane(l, cols = 8) {
    let cells = '';
    for (let r = 0; r < ROWS_N.length; r++) {
      for (let col = 0; col < cols; col++) {
        const kind = l.notes[`${r},${col}`];
        const cls = ['ncell',
          col === 0 ? 'c0' : '', (col > 0 && col % 4 === 0) ? 'beat' : '', col % 2 ? 'wash' : '',
          ROWS_N[r].root ? 'root' : '',
          kind === 'on' ? 'on' : '', kind === 'ghost' ? 'ghost' : '', kind === 'sel' ? 'sel' : '',
        ].join(' ');
        const loz = kind === 'sel' ? '<i class="loz"></i>' : '';
        const label = (col === 0 || kind) ? `<span>${ROWS_N[r].n}</span>` : '';
        cells += `<div class="${cls}" style="--c:${C}">${loz}${label}</div>`;
      }
    }
    const steps = Array.from({length: cols}, (_, i) =>
      l.steps.includes(i)
        ? `<span class="stepwrap" style="flex:1"><span class="step on" style="--c:${C}"><span class="ink" style="--ic:var(--ic-chord)"></span></span></span>`
        : `<span class="stepwrap" style="flex:1"><span class="step"></span></span>`
    ).join('');
    return `<div class="lane">
      <div class="notegrid${cols === 16 ? ' w16' : ''}"><div class="phead" style="left:${l.playCol * (100 / cols)}%;width:${100 / cols}%"></div>${cells}</div>
      <div class="steps"><div class="steprow">${steps}</div></div>
    </div>`;
  }

  const el = document.getElementById('notelanes');
  if (el) el.innerHTML = lane(lanes[0]) + '<div class="lane-sep"></div>' + lane(lanes[1]);

  // iPad: sixteen steps across, same rows — second bar varies the phrase
  const padLanes = [
    {notes: {
      ...lanes[0].notes,
      '7,8': 'on', '5,8': 'ghost', '3,8': 'ghost',
      '6,10': 'on', '4,10': 'ghost',
      '3,12': 'on', '1,12': 'ghost',
      '5,14': 'on', '2,14': 'ghost', '0,14': 'ghost',
    }, steps: [0, 2, 4, 6, 8, 10, 12, 14], playCol: 9},
    {notes: {
      ...lanes[1].notes,
      '4,9': 'on', '6,11': 'on',
      '7,15': 'on', '5,15': 'ghost',
    }, steps: [1, 3, 7, 9, 11, 15], playCol: 9},
  ];
  const elPad = document.getElementById('notelanes-pad');
  if (elPad) elPad.innerHTML = lane(padLanes[0], 16) + '<div class="lane-sep"></div>' + lane(padLanes[1], 16);
})();

// Pattern composer: loop-group cards + 4×4×4 pattern pool.
(function () {
  const ROWS = [
    ['#7DFBFD', '#64D2FF', '#A3DFE5', '#32E5C8'],
    ['#CFFB52', '#99E673', '#34C759', '#00FF41'],
    ['#FFBA36', '#FF9500', '#FF5500', '#FF3B30'],
    ['#EA3EE7', '#BF5AF2', '#0080FF', '#5E5CE6'],
  ];

  // Loop-group cards: 8 voices × 4 sections, values are row colors or null.
  function card(cells, time, opts = {}) {
    const grid = cells.map(c => `<b${c ? ` style="background:${c}"` : ''}></b>`).join('');
    const prog = opts.prog
      ? `<div class="gprog"><b style="left:${opts.prog[0]}%;width:${opts.prog[1]}%"></b></div>`
      : '<div class="gprog"></div>';
    return `<div class="gcol"><div class="gcard${opts.hot ? ' hot' : ''}">
      <div class="gcell-grid">${grid}</div>
      <div class="gtime">${time}</div>${prog}
    </div>${opts.hot ? '<div class="gplay"><svg class="si" viewBox="0 0 16 16"><path class="f" d="M5.5 3.5v9l7.5-4.5z"/></svg></div>' : ''}</div>`;
  }

  // stage: intro sparse / main dense (playing) / breakdown
  const C = ROWS;
  const introCells = [
    C[0][0], null, C[0][0], null,  null, null, null, null,
    null, null, C[1][0], null,     null, null, null, C[2][1],
    null, null, null, null,        C[3][2], null, C[3][2], null,
    null, null, null, null,        null, null, null, null,
  ];
  const mainCells = [
    C[0][0], C[0][0], C[0][0], C[0][0],  C[0][1], null, C[0][1], C[0][1],
    C[1][0], C[1][0], null, C[1][0],     null, C[1][2], C[1][2], C[1][2],
    C[2][1], null, C[2][1], C[2][1],     C[3][0], C[3][0], null, C[3][0],
    C[3][2], C[3][2], C[3][2], null,     null, C[2][0], null, C[2][0],
  ];
  const outroCells = [
    C[0][0], null, null, C[0][0],  null, null, C[0][1], null,
    null, C[1][2], null, null,     C[3][0], null, null, null,
    C[3][2], null, C[3][2], null,  null, C[2][0], null, null,
    null, null, null, null,        null, null, null, null,
  ];
  // cells arrays above are row-major 8 rows × 4 cols
  const groupsHtml =
    card(introCells, '0:00') +
    card(mainCells, '0:15', {hot: true, prog: [25, 16]}) +
    card(outroCells, '0:45') +
    `<div class="gafter">
       <span class="sq"><svg class="si" viewBox="0 0 16 16"><path d="M8 3.5v9M3.5 8h9"/></svg></span>
       <span class="sq"><svg class="si" viewBox="0 0 16 16"><rect x="5.5" y="2.5" width="8" height="9.5" rx="1"/><path d="M3 5.5v8h7"/></svg></span>
     </div>`;
  const grEl = document.getElementById('groups');
  if (grEl) grEl.innerHTML = groupsHtml;
  const gp = document.getElementById('groups-pad');
  if (gp) gp.innerHTML = groupsHtml;

  // Pattern pool: 4 segments × 4 rows × 4 cols.
  const fills = {
    0: {'0,0': {ic: 'kick', sel: true}, '1,0': {ic: 'snare', multi: 2}, '2,1': {ic: 'hihat'}, '0,2': {ic: 'kick'}},
    1: {'0,0': {ic: 'chord'}, '2,1': {ic: 'arp'}, '3,2': {ic: 'waves'}},
    2: {'1,1': {ic: 'string'}, '2,3': {ic: 'noise'}},
    3: {'0,1': {ic: 'bell'}, '2,0': {ic: 'sine'}},
  };
  const GROUPS = ['#7DFBFD', '#CFFB52', '#FF5500', '#EA3EE7'];
  const poolHtml = GROUPS.map((gc, seg) => {
    let cells = '';
    for (let r = 0; r < 4; r++) for (let col = 0; col < 4; col++) {
      const f = fills[seg][`${r},${col}`];
      if (!f) { cells += '<span class="pcell"><b></b></span>'; continue; }
      const cls = ['pcell', 'on', f.sel ? 'selct' : '', f.multi ? 'multi' : ''].join(' ');
      cells += `<span class="${cls}" style="--c:${ROWS[seg][r]}"><b><span class="ink" style="--ic:var(--ic-${f.ic})"></span></b>${f.multi ? `<span class="badge">${f.multi}</span>` : ''}</span>`;
    }
    return `<div class="pseg"><div class="cells">${cells}</div><div class="gfoot" style="--gc:${gc}"></div></div>`;
  }).join('');
  const pgEl = document.getElementById('pgrid');
  if (pgEl) pgEl.innerHTML = poolHtml;
  const pp = document.getElementById('pgrid-pad');
  if (pp) pp.innerHTML = poolHtml;
})();

// Scale each fixed-canvas screen to its container width.
function fitScreens() {
  document.querySelectorAll('.viewport').forEach(vp => {
    const screen = vp.querySelector('.screen');
    const device = vp.closest('.device');
    const w = screen.offsetWidth, h = screen.offsetHeight;
    const cap = device.closest('.feature-split') ? 330 : Infinity;
    const avail = Math.min(device.parentElement.clientWidth - 30, w, cap);
    const s = avail / w;
    screen.style.transform = `scale(${s})`;
    vp.style.width = w * s + 'px';
    vp.style.height = h * s + 'px';
  });
}
addEventListener('resize', fitScreens);
fitScreens();

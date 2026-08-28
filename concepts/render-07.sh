#!/usr/bin/env bash
# Full-page render of the concept 07 preview, for review.
#
# Headless Chrome cannot screenshot this page in one shot — see SYNTHESIS.md §9.
# So: capture viewport-sized slices with the body pushed up by one viewport each
# time, then stitch. The stitched image repeats the fixed header at every slice
# boundary; that is the method, not the page.
#
# Needs a local server on $PORT serving the repo root, and Pillow.
#   python3 -m http.server 8123 &   ./concepts/render-07.sh out.png
set -euo pipefail
cd "$(dirname "$0")/.."

OUT="${1:-07-full.png}"
PORT="${PORT:-8123}"
VH="${VH:-813}"          # viewport height to simulate
W="${W:-1440}"
SLICES="${SLICES:-9}"
CH="${CHROME:-/Applications/Google Chrome.app/Contents/MacOS/Google Chrome}"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP" _slice.html' EXIT

for ((i=0; i<SLICES; i++)); do
  python3 - "$((i * VH))" <<'PY'
import sys
off = sys.argv[1]
s = open('concept07.html').read()
s = s.replace('</head>', '<style>body{margin-top:-%spx}</style>\n</head>' % off, 1)
# stand the scroll reveal down — nothing scrolls in headless, so everything
# below the first viewport would capture at opacity:0
s = s.replace('</body>',
  "<script>setTimeout(function(){document.documentElement.classList.remove('reveal-on');"
  "document.querySelectorAll('.rv').forEach(function(e){e.style.opacity='1';e.style.transform='none';});},700);"
  "</script>\n</body>", 1)
open('_slice.html', 'w').write(s)
PY
  ( "$CH" --headless=old --disable-gpu --hide-scrollbars --window-size="$W,$VH" \
      --virtual-time-budget=9000 --screenshot="$TMP/s$i.png" \
      "http://localhost:$PORT/_slice.html" >/dev/null 2>&1 & )
  # Chrome writes the PNG and then lingers, so poll for the file and kill it
  n=0; until [ -s "$TMP/s$i.png" ] || [ $n -ge 20 ]; do sleep 2; n=$((n+1)); done
  pkill -f "headless=old" || true; sleep 1
  [ -s "$TMP/s$i.png" ] || { echo "slice $i failed" >&2; exit 1; }
done

python3 - "$TMP" "$OUT" "$SLICES" "$VH" <<'PY'
import sys
from PIL import Image
tmp, out, n, vh = sys.argv[1], sys.argv[2], int(sys.argv[3]), int(sys.argv[4])
ims = [Image.open(f"{tmp}/s{i}.png").convert("RGB") for i in range(n)]
canvas = Image.new("RGB", (ims[0].width, n * vh))
for i, im in enumerate(ims):
    canvas.paste(im, (0, i * vh))
canvas.save(out)
print("wrote", out, canvas.size)
PY

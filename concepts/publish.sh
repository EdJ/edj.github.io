#!/usr/bin/env bash
# Regenerate the public preview of a concept at the site root.
#
#   ./concepts/publish.sh 08     →   concept08.html + concept-assets/
#
#   concepts/08-open.html   is the source. Edit that.
#   concept08.html          is generated. Never hand-edit it.
#
# The root copy exists so joltgroovebox.com/concept08.html can be shown to
# people outside the project WITHOUT publishing concepts/ — the briefs, the
# audit and the earlier cuts stay unpublished. Only the assets the page
# actually references are copied out, into concept-assets/.
set -euo pipefail
cd "$(dirname "$0")/.."

V="${1:?usage: publish.sh <version>   e.g. publish.sh 08}"
SRC="concepts/$V-open.html"
OUT="concept$V.html"
[ -f "$SRC" ] || { echo "no such page: $SRC" >&2; exit 1; }

# whatever the page references under concepts/assets/, flattened one level down
# (a read loop, not mapfile: macOS ships bash 3.2)
ASSETS=()
while IFS= read -r a; do ASSETS+=("$a"); done < <(
  grep -oE '"assets/[^"]+"' "$SRC" | tr -d '"' | sed 's|^assets/||' | sort -u)
for a in "${ASSETS[@]}"; do
  [ -f "concepts/assets/$a" ] || { echo "missing asset: concepts/assets/$a" >&2; exit 1; }
  mkdir -p "concept-assets/$(dirname "$a")"
  cp "concepts/assets/$a" "concept-assets/$a"
done

# rewrite paths, add the banner, keep the preview out of search results
python3 - "$SRC" "$OUT" <<'PY'
import sys, re
src, p = sys.argv[1], sys.argv[2]
s = open(src).read()
# the page moves up one level, so ../foo -> foo …
s = s.replace('"../', '"').replace("('../", "('")
# … and its own assets move sideways into a folder that is safe to publish
s = s.replace('"assets/', '"concept-assets/')
banner = ('<!-- GENERATED from %s by concepts/publish.sh.\n'
          '     Do not hand-edit: your changes will be overwritten. -->\n' % src)
s = re.sub(r'^\s*<!DOCTYPE html>', banner + '<!DOCTYPE html>', s, count=1, flags=re.I)
if 'name="robots"' not in s:
    s = s.replace('<meta charset', '<meta name="robots" content="noindex,nofollow">\n<meta charset', 1)
open(p, 'w').write(s)
PY

echo "wrote $OUT + concept-assets/ (${#ASSETS[@]} files)"
grep -oE '(src|href)="[^"]+"' "$OUT" | grep -vE '="#|data:|^.*="https?:' | sort -u

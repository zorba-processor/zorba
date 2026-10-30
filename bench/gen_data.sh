#!/bin/sh
# Generate the benchmark datasets into DIR (default: bench-data). Existing files are kept.
#   xmark-{0.01,0.1,1}.xml   XMark auction documents (xmlgen, ~1/10/100 MB)
#   orders-{10mb,100mb}.json  JSON customers/orders (gen_json.py)
# xmlgen is the XMark generator by CWI (public domain), single C file, pinned by checksum.
set -eu
HERE=$(cd "$(dirname "$0")" && pwd)
DIR=${1:-bench-data}
XMLGEN_URL=https://projects.cwi.nl/xmark/Assets/unix.c
mkdir -p "$DIR"
DIR=$(cd "$DIR" && pwd)

if [ ! -x "$DIR/xmlgen" ]; then
  work=$(mktemp -d)
  for try in 1 2 3; do
    curl -fsSL --retry 3 -o "$work/unix.c" "$XMLGEN_URL" && break
    sleep 10
  done
  (cd "$work" && sha256sum -c "$HERE/xmlgen.sha256")
  # 1990s K&R-style C: build as gnu89 and silence its warnings
  gcc -std=gnu89 -O2 -w -o "$DIR/xmlgen" "$work/unix.c" -lm
  rm -rf "$work"
fi

for f in 0.01 0.1 1; do
  [ -s "$DIR/xmark-$f.xml" ] || "$DIR/xmlgen" -f "$f" -o "$DIR/xmark-$f.xml"
done
[ -s "$DIR/orders-10mb.json" ] || python3 "$HERE/gen_json.py" "$DIR/orders-10mb.json" 10
[ -s "$DIR/orders-100mb.json" ] || python3 "$HERE/gen_json.py" "$DIR/orders-100mb.json" 100
ls -l "$DIR"

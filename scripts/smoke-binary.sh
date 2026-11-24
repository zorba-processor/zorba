#!/bin/sh
# Smoke test of an unpacked release binary tarball, run where none of its
# libraries are installed and away from the prefix it was built for: the
# zorba CLI (XQuery, JSONiq, and a module with its external functions, found
# relative to lib/), zorba-mcp, and a C++ program compiled against include/
# and linked against lib/.
# Needs a C++ compiler as $CXX (default c++).
# Usage: smoke-binary.sh <unpacked-dir>
set -eu
dir=$(cd "${1:?usage: smoke-binary.sh <unpacked-dir>}" && pwd)
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

expect() { # description, expected output, command...
  what=$1 want=$2
  shift 2
  got=$("$@")
  if [ "$got" = "$want" ]; then
    echo "ok    $what: $got"
  else
    echo "FAIL  $what: expected '$want', got '$got'"
    exit 1
  fi
}

expect "XQuery" 2 "$dir/bin/zorba" -r -q '1+1'
expect "JSONiq" '{ "sum" : 6 }' "$dir/bin/zorba" -j -r -q '{ "sum" : sum(parse-json("[1, 2, 3]")[]) }'
expect "module file" true "$dir/bin/zorba" -r -q \
  'import module namespace file = "http://expath.org/ns/file"; file:is-directory("/")'

mkdir "$work/data"
echo '{"jsonrpc":"2.0","id":1,"method":"initialize","params":{"protocolVersion":"2025-11-25"}}' \
  | "$dir/bin/zorba-mcp" --root "$work/data" > "$work/mcp.out"
grep -q '"serverInfo"' "$work/mcp.out" || { echo "FAIL  zorba-mcp: $(cat "$work/mcp.out")"; exit 1; }
echo "ok    zorba-mcp initialize"

cat > "$work/main.cpp" <<'EOF'
#include <iostream>
#include <zorba/store_manager.h>
#include <zorba/zorba.h>

int main() {
  void* store = zorba::StoreManager::getStore();
  zorba::Zorba* engine = zorba::Zorba::getInstance(store);
  {
    zorba::XQuery_t query = engine->compileQuery("string-join(for $i in 1 to 3 return string($i * $i), ',')");
    zorba::Iterator_t it = query->iterator();
    zorba::Item item;
    it->open();
    while (it->next(item))
      std::cout << item.getStringValue() << '\n';
    it->close();
  }
  engine->shutdown();
  zorba::StoreManager::shutdownStore(store);
}
EOF
"${CXX:-c++}" -std=c++23 -I"$dir/include" "$work/main.cpp" \
  -L"$dir/lib" -lzorba_simplestore -Wl,-rpath,"$dir/lib" -o "$work/embed"
expect "C++ API" 1,4,9 "$work/embed"

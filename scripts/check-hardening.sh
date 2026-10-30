#!/bin/sh
# Verify that the Linux build carries the hardening features enabled by
# ZORBA_HARDENING: PIE executable, full RELRO + BIND_NOW, stack protector,
# fortified libc calls. Usage: check-hardening.sh <build-dir>
set -u
dir=${1:?usage: check-hardening.sh <build-dir>}
fail=0
check() { # description, command-status
  if [ "$2" -eq 0 ]; then echo "ok    $1"; else echo "FAIL  $1"; fail=1; fi
}

exe=$(find "$dir" -type f -name zorba -perm -u+x | head -n 1)
lib=$(find "$dir" -type f -name 'libzorba_simplestore.so*' | head -n 1)
[ -n "$exe" ] && [ -n "$lib" ] || { echo "binaries not found under $dir"; exit 1; }
echo "exe: $exe"
echo "lib: $lib"

readelf -h "$exe" | grep -q 'Type:.*DYN'
check "executable is PIE" $?

for f in "$exe" "$lib"; do
  readelf -lW "$f" | grep -q 'GNU_RELRO'
  check "$(basename "$f"): RELRO segment" $?
  readelf -dW "$f" | grep -Eq 'BIND_NOW|FLAGS.*NOW'
  check "$(basename "$f"): BIND_NOW" $?
done

nm -D --undefined-only "$lib" | grep -q '__stack_chk_fail'
check "library references __stack_chk_fail" $?
nm -D --undefined-only "$lib" | grep -Eq '__[a-z_]+_chk'
check "library references fortified (_chk) libc functions" $?

exit $fail

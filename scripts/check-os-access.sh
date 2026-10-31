#!/bin/sh
# Audit of the code that reaches the operating system on a query's behalf.
#
# Zorba enforces its SecurityPolicy (include/zorba/security.h) at a few choke
# points: URI resolution, the XML parser configuration, the module import
# gate and the per-call checks of the file, http-client and ftp-client
# modules. Code that opens files, loads libraries, reads the environment,
# connects to the network or parses XML outside those choke points bypasses
# the policy.
#
# This script greps src/ and modules/ for such calls and compares the number
# of lines per file that match each pattern (#include lines aside) with scripts/check-os-access.allow. It fails
# if a file has a hit that the allow-list does not cover, if a count differs
# from the allow-list (new calls in an allowed file need a review too, and
# removed calls must be removed from the list so that the slack cannot be
# reused), or if an entry has no reason.
#
# Usage: scripts/check-os-access.sh          (from any directory)
#        scripts/check-os-access.sh --list   print the current hits in the
#                                            allow-list format (count only)
set -eu

root=$(cd "$(dirname "$0")/.." && pwd)
cd "$root"
allow=scripts/check-os-access.allow

# name|extended regular expression. Names are what the allow-list refers to.
patterns='ifstream|\b(std::)?(basic_)?ifstream\b
ofstream|\b(std::)?(basic_)?ofstream\b
fstream|\b(std::)?(basic_)?fstream\b
fopen|\b(fopen|freopen|_wfopen|_fdopen|fdopen|popen|_popen)[[:space:]]*\(
open|(^|[^A-Za-z0-9_.>:])(::)?open(at)?[[:space:]]*\([^;]*O_(RDONLY|WRONLY|RDWR|CREAT)
dlopen|\bdlopen[[:space:]]*\(
LoadLibrary|\bLoadLibrary(Ex)?[AW]?[[:space:]]*\(
getenv|\b(secure_getenv|getenv|_wgetenv|_dupenv_s)[[:space:]]*\(
environ|\b(environ|_environ)\b
CURLOPT_URL|\bCURLOPT_URL\b
socket|\bsocket[[:space:]]*\(
xmlCtxtUseOptions|\bxmlCtxtUseOptions\b
xmlRead|\bxml(Ctxt)?Read(Memory|File|Doc|Fd|IO)\b
xmlParse|\bxmlParse(Doc|File|Memory)\b
xmlCreateParserCtxt|\bxmlCreate(Push|Memory|File|URL|IO)ParserCtxt\b
HttpStream|\bHttpStream\b
uri::fetch|\buri::fetch\b'

hits=$(mktemp)
trap 'rm -f "$hits"' EXIT

printf '%s\n' "$patterns" | while IFS='|' read -r name regex; do
  grep -rEn \
       --include='*.c' --include='*.cc' --include='*.cpp' --include='*.cxx' \
       --include='*.h' --include='*.hh' --include='*.hpp' --include='*.ipp' \
       --include='*.tcc' --include='*.y' --include='*.l' --include='*.h.in' \
       --include='*.cpp.in' \
       -e "$regex" src modules 2>/dev/null |
    grep -Ev '^[^:]+:[0-9]+:[[:space:]]*#[[:space:]]*include' |
    cut -d: -f1 | sort | uniq -c |
    awk -v n="$name" '{ print $2 " " n " " $1 }'
done | sort > "$hits"

if [ "${1:-}" = "--list" ]; then
  cat "$hits"
  exit 0
fi

awk '
  FNR == NR {
    # allow-list: <file> <pattern> <count> <reason...>; "#" starts a comment.
    line = $0
    sub(/^[ \t]+/, "", line)
    if (line == "" || substr(line, 1, 1) == "#") next
    n = split(line, f, /[ \t]+/)
    key = f[1] " " f[2]
    if (n < 4) {
      printf "FAIL  %s:%d: entry for %s has no reason\n", FILENAME, FNR, key
      bad = 1
      next
    }
    if (f[3] !~ /^[0-9]+$/) {
      printf "FAIL  %s:%d: count \"%s\" is not a number\n", FILENAME, FNR, f[3]
      bad = 1
      next
    }
    if (key in allowed) {
      printf "FAIL  %s:%d: duplicate entry for %s\n", FILENAME, FNR, key
      bad = 1
    }
    allowed[key] = f[3] + 0
    next
  }
  {
    key = $1 " " $2
    found[key] = $3 + 0
    if (!(key in allowed)) {
      printf "FAIL  %s: %d unreviewed %s call(s)\n", $1, $3, $2
      bad = 1
    } else if (allowed[key] != $3) {
      printf "FAIL  %s: %d %s call(s), the allow-list says %d\n", \
             $1, $3, $2, allowed[key]
      bad = 1
    }
  }
  END {
    for (key in allowed)
      if (!(key in found)) {
        split(key, k, " ")
        printf "FAIL  %s: no %s call left; remove the allow-list entry\n", \
               k[1], k[2]
        bad = 1
      }
    if (bad) {
      print ""
      print "Code that reaches the file system, the network, the environment,"
      print "native libraries or the XML parser must go through the security"
      print "policy (include/zorba/security.h, src/util/security_check.h,"
      print "src/util/xml_parser_security.h). After reviewing a new or removed"
      print "call, update scripts/check-os-access.allow with the count and the"
      print "reason it is safe."
      exit 1
    }
    print "ok    every OS access in src/ and modules/ is in the allow-list"
  }
' "$allow" "$hits"

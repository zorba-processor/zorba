#!/bin/sh
# Which shape of query fires the join rule without the guard. $1 = zorba
Z=$1; mkdir -p /tmp/j
P='declare namespace an = "http://zorba.io/annotations"; declare %an:assignable variable $g := 0; declare %an:sequential function local:bump() { $g := $g + 1; $g }; declare %an:nondeterministic function local:g() { $g }; declare variable $f := local:bump#0;'
w() { printf '%s\n%s\n' "$P" "$2" > /tmp/j/$1.xq; }
w q1 '(for $a in (1, 2, 3) for $b in (1, 2, 3) where ($a + xs:integer($f())) eq $b return ($a, $b), "end")'
w q2 '(for $a in 1 to 3 for $b in 1 to 3 where ($a + xs:integer($f())) eq $b return ($a, $b), "end")'
w q3 '(for $a in 1 to 3 for $b in 1 to 3 where ($a + xs:integer($f())) = $b return ($a, $b), "end")'
w q4 '(for $a in 1 to 3 for $b in (1 to 3)[. gt 0] where $a eq $b return ($f(), $a, $b), "end")'
w q5 '(for $a in 1 to 3 for $b in (for $j in 1 to 3 return $j + xs:integer(local:g())) where $a eq $b return ($f(), $a, $b), "end")'
w q6 '(for $a in 1 to 3 for $b in (for $j in 1 to 3 return $j + xs:integer(local:g())) where $a = $b return ($f(), $a, $b), "end")'
for q in q1 q2 q3 q4 q5 q6; do
  echo "JOIN $q probes=$($Z -f -q /tmp/j/$q.xq --iterator-tree xml --no-serializer 2>&1 | grep -c 'Probe')"
  for o in 0 1 2; do echo "JOIN $q O$o $($Z -f -q /tmp/j/$q.xq -O $o 2>&1 | tail -1 | cut -c1-70)"; done
done

declare namespace an = "http://zorba.io/annotations";

declare function local:h($x) { $x + 1 };
declare %an:nondeterministic function local:g($x) { local:h($x) };

for $j in 1 to 2
for $i in 1 to 3
return local:g($j) + $i

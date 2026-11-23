declare namespace an = "http://zorba.io/annotations";

declare %an:nondeterministic function local:g($x) { $x + 1 };

for $j in 1 to 2
for $i in 1 to 3
return local:g($j) + $i

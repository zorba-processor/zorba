(: A small ray tracer in the shape of QT3 app-Demos/raytracer: a recursion
   (trace -> shade -> trace) inside a FLWOR, with calls of non-inlined
   functions nested in the arguments. Every evaluation of a call resets it;
   resetting a call used to reset each argument subplan both directly and
   through the parameters of the callee, at every level reached so far, so
   the reset work grew by a factor of about 4 per level (depth 8: 18 s,
   depth 10: minutes). Now it is linear in the work done (milliseconds). :)
declare function local:leaf($x as xs:double) as xs:double { $x };

(: Not a leaf function, so not inlined. :)
declare function local:nl($x as xs:double) as xs:double { local:leaf($x) };

declare function local:add($a as xs:double*, $b as xs:double*) as xs:double*
{ for $x at $p in $a return local:nl($x + $b[$p]) };

declare function local:scale($a as xs:double*, $s as xs:double) as xs:double*
{ for $x in $a return local:nl($x * $s) };

declare function local:dot($a as xs:double*, $b as xs:double*) as xs:double
{ local:nl(sum(for $x at $p in $a return $x * $b[$p])) };

declare function local:trace($e as element(), $src as xs:double*, $dir as xs:double*,
                             $depth as xs:integer) as xs:double*
{
  subsequence((
    for $c in $e/*
    let $d := local:dot($src, $dir) + xs:double($c/@w)
    where $d gt 0
    order by $d
    return local:shade($e, local:add($src, local:scale($dir, $d)), $dir, $depth),
    (0, 0, 0)), 1, 3)
};

declare function local:shade($e as element(), $pos as xs:double*, $dir as xs:double*,
                             $depth as xs:integer) as xs:double*
{
  local:add(local:scale($pos, 0.5),
            if ($depth gt 0)
            then local:scale(local:trace($e, $pos, local:scale($dir, -0.9), $depth - 1), 0.5)
            else (0, 0, 0))
};

let $e := <s><c w="1"/><c w="2"/></s>
return round-half-to-even(
  sum(for $i in 1 to 50 return local:trace($e, ($i, 1, 2), (0.1, 0.2, 0.3), 10)),
  6)

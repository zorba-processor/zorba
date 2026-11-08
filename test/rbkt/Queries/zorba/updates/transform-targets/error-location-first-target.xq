(: Twelve targets outside the copy, each renamed on its own line. The error is
   reported at the first of them in the query text, whatever the order in which
   the store holds the targets. :)
declare namespace err = "http://www.w3.org/2005/xqt-errors";
let $o1 := <o/>, $o2 := <o/>, $o3 := <o/>, $o4 := <o/>, $o5 := <o/>, $o6 := <o/>,
    $o7 := <o/>, $o8 := <o/>, $o9 := <o/>, $o10 := <o/>, $o11 := <o/>, $o12 := <o/>
return
  try {
    copy $c := <x/>
    modify (
      rename node $o1 as "a",
      rename node $o2 as "a",
      rename node $o3 as "a",
      rename node $o4 as "a",
      rename node $o5 as "a",
      rename node $o6 as "a",
      rename node $o7 as "a",
      rename node $o8 as "a",
      rename node $o9 as "a",
      rename node $o10 as "a",
      rename node $o11 as "a",
      rename node $o12 as "a")
    return $c
  } catch * { $err:code, $err:line-number }

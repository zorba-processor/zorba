(: Node construction: 600 000 elements and attributes, each labelled as the
   next child of its parent. :)
count(
  <r>{
    for $i in 1 to 100000
    return <a i="{$i}"><b>{$i}</b><c/><d j="{$i}"/></a>
  }</r>//*)

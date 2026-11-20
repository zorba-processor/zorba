(: Updates: labels made between existing siblings (before, after, between). :)
copy $d := <r>{ for $i in 1 to 20000 return <a><b/><c/></a> }</r>
modify (
  for $a in $d/a
  return (insert node <x/> before $a/c,
          insert node <y/> as first into $a,
          insert node <z/> after $a/b)
)
return count($d//*)

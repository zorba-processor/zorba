(: In JSONiq "." is the lookup operator, so the context item is $$. :)
[
  let $people := ({ "name": "ana", "age": 31 }, { "name": "luis", "age": 17 })
  return (
    $people[$$.age ge 18].name,
    $people ! upper-case($$.name)
  )
]

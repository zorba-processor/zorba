(: A duplicate key is an error also when its first occurrence matched the keys of the
   value before. :)
count(jn:parse-json('{"a":1,"b":2}
{"a":1,"a":2}', { "jsoniq-multiple-top-level-items" : true() }))

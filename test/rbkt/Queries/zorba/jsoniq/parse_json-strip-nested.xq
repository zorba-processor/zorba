(: jsoniq-strip-top-level-array strips a top-level array: an array nested in a
 : top-level object stays. :)
let $o := jn:parse-json('{"a":[1,2]}', { "jsoniq-strip-top-level-array" : true() })
return jn:size($o("a"))

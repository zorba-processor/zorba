(: Sequence normalization (Serialization 3.0 2): only adjacent atomic
   values are separated by a space; text nodes are copied as they are. :)
let $t := text { "t" }
return ($t, "a", "b", $t, $t, 1, <e/>, 2)

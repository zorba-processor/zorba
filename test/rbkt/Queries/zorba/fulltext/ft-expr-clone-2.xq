(: This tests that full-text expressions with match options and positional
   filters are cloned properly: the function is inlined at both call sites,
   so each copy must own its options and filters. :)

declare function local:f($w as xs:string) {
  let $x := <msg>Hello big wide world</msg>
  return $x contains text $w ftand "WORLD"
         using case insensitive
         using diacritics insensitive
         ordered window 5 words
};

local:f("big"), local:f("planet")

(: vim:set et sw=2 ts=2: :)

declare ft-option using language "en";

let $x := <p>It was the best of times, it was the worst of times.</p>
return $x contains text "best any times"
using stop words ( "any" )

(: vim:set syntax=xquery et sw=2 ts=2: :)

declare ft-option using language "en";

let $x := <p>It was the best of times, it was the worst of times.</p>
return $x contains text "best of lunch"
using stop words default union ( "lunch" )

(: vim:set syntax=xquery et sw=2 ts=2: :)

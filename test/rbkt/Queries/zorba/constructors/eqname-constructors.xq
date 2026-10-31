(: URIQualifiedNames in computed element and attribute constructors, both
   as literal names and as computed name strings. :)
let $e := element Q{http://example.com/x}x { attribute Q{}a { 1 } }
let $f := element { " Q{http://example.com/y}y " } {
            attribute { "Q{}b" } { 2 } }
return (namespace-uri($e), local-name($e), string($e/@a),
        namespace-uri($f), local-name($f), string($f/@b),
        namespace-uri(element Q{z&#x20;z}x {}))

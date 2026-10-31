declare namespace err = "http://www.w3.org/2005/xqt-errors";
(: A numeric cast to a type derived from xs:nonNegativeInteger first truncates
   to xs:integer; a value outside the target type is FORG0001
   (F&O 3.0 19.1.2.4, 19.2). :)
(for $f in (function() { (-2.5) cast as xs:unsignedLong },
            function() { (-2.5e0) cast as xs:unsignedInt },
            function() { xs:float(-2.5) cast as xs:nonNegativeInteger },
            function() { (-1.0) cast as xs:positiveInteger })
 return try { string($f()) } catch * { local-name-from-QName($err:code) },
 string((-0.5) cast as xs:unsignedLong),
 string((-0.5e0) cast as xs:unsignedShort),
 string((2.7) cast as xs:unsignedByte))

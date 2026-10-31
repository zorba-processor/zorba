declare namespace err = "http://www.w3.org/2005/xqt-errors";
(: Only the XSD lexical forms are accepted for xs:double and xs:float
   (XSD 1.1 Part 2, 3.3.4-3.3.5; F&O 3.0 19.1.1.1). :)
(for $s in ("0x1A", "1e", "e5", "1e+", "1.5x", "0x1p3")
 return try { string(xs:double($s)) } catch * { local-name-from-QName($err:code) },
 try { string(xs:float("0x1A")) } catch * { local-name-from-QName($err:code) },
 xs:double(" 1.5E2 ") eq 150e0,
 xs:double("1.") eq 1e0,
 xs:double(".5e-1") eq 0.05e0,
 xs:float("-3E+0") eq xs:float(-3))

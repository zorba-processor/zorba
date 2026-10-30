(: Annotations on inline function expressions and function tests
   (XQuery 3.0 3.1.7, 2.5.4.3). Zorba accepts and ignores them. :)
declare namespace eg = "http://example.com";

(%eg:sequential("abc", 3) %eg:memo-function function () { "bar" } (),
 %Q{http://example.com}x function ($a) { $a + 1 } (1),
 () instance of %eg:x function(*),
 %eg:y function ($a as xs:integer) as xs:integer { $a }
   instance of %eg:x(1) function(xs:integer) as xs:integer)

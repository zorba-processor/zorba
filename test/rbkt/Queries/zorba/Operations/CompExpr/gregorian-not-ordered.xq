declare namespace err = "http://www.w3.org/2005/xqt-errors";
(: xs:gYear, xs:gYearMonth, xs:gMonth, xs:gMonthDay and xs:gDay have equality
   but no ordering: lt/gt and general < raise XPTY0004 (XQuery 3.0 3.7.1,
   B.2; F&O 3.0 10.4.13-10.4.17). :)
(for $f in (function() { xs:gYear("-0001") lt xs:gYear("2000Z") },
            function() { xs:gYearMonth("2000-01") gt xs:gYearMonth("2000-02") },
            function() { xs:gMonth("--01") le xs:gMonth("--02") },
            function() { xs:gMonthDay("--01-01") ge xs:gMonthDay("--01-02") },
            function() { xs:gDay("---01") < xs:gDay("---02") })
 return try { string($f()) } catch * { local-name-from-QName($err:code) },
 xs:gYear("2000") eq xs:gYear("2000"),
 xs:gDay("---01") = xs:gDay("---02"))

declare namespace err = "http://www.w3.org/2005/xqt-errors";
(: A duration * or div result that cannot be represented is FODT0002
   (F&O 3.0 10.1.1, 10.6.3-10.6.6). :)
for $f in (function() { xs:yearMonthDuration("P1Y") * 1e300 },
           function() { xs:yearMonthDuration("P1Y") div 1e-300 },
           function() { xs:dayTimeDuration("P1D") * 1e300 },
           function() { xs:dayTimeDuration("PT1S") div 1e-300 },
           function() { xs:dayTimeDuration("-P1D") * 1e300 })
return try { string($f()) } catch * { local-name-from-QName($err:code) }

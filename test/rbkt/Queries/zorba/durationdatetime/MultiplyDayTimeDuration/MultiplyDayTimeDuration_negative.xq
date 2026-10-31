(: The sign of a negative duration is kept by * and div (F&O 3.0 10.6.3-10.6.6). :)
(string(xs:dayTimeDuration("-PT1S") * 1.5),
 string(xs:dayTimeDuration("-PT1S") * -1),
 string(xs:dayTimeDuration("-PT1.5S") div 2),
 string(xs:dayTimeDuration("-P1DT1S") * 2),
 string(xs:dayTimeDuration("-PT1S") div xs:dayTimeDuration("PT1S")),
 string(xs:yearMonthDuration("-P1Y") * 1.5))

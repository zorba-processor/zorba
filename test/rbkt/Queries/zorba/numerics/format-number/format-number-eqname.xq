(: A decimal format can be named with a URIQualifiedName, both in its
   declaration and in the third argument of fn:format-number (QT3
   numberformat87). :)
declare decimal-format Q{http://a.ns/}test decimal-separator="," grouping-separator=".";
format-number(12.34, '0.000,00', ' Q{http://a.ns/}test ')

(: fn:upper-case uses the full Unicode case mappings, which may map one
   character to several (F&O 3.0 5.4.7). :)
string-to-codepoints(upper-case("&#xDF;&#x1C5;&#xFB03;&#x149;aBc&#x3C2;"))

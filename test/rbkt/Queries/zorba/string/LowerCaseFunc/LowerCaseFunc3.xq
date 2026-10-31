(: fn:lower-case uses the full Unicode case mappings, which may map one
   character to several (F&O 3.0 5.4.8). :)
string-to-codepoints(lower-case("&#x1C5;&#x130;AbC&#x1E9E;&#x345;"))

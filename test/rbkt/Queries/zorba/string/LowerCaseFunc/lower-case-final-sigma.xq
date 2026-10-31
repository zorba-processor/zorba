(: F&O 3.0 5.4.8: lower-case uses the Unicode case mappings without the
   language-sensitive ones, so the Final_Sigma rule of SpecialCasing.txt
   applies: a capital sigma (U+03A3) after a cased letter that does not
   precede one becomes a final sigma (U+03C2), otherwise U+03C3. :)
string-join(
  for $s in ("&#x39F;&#x394;&#x39F;&#x3A3;", "&#x3A3;", "A&#x3A3;B", "A&#x3A3;'",
             "&#x3A3;&#x3A3;", "&#x3A3;.&#x3A3;", "A&#x3A3; B")
  return string-join(string-to-codepoints(lower-case($s)) ! string(.), " "),
  "|")

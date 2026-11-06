(: U+FEFF is neither XML white space nor dropped from a CDATA section. :)
string-to-codepoints(normalize-space("&#xFEFF;a"))[1],
string-to-codepoints(parse-xml("<a><![CDATA[&#xFEFF;x]]></a>")/a)[1]

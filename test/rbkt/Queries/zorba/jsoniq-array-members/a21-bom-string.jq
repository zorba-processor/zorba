jsoniq version "1.0";
(: A string that starts with a byte order mark: parse-json ignores the mark (F&O 3.1). :)
[ parse-json(codepoints-to-string(65279) || "[1,2]")[] ]
